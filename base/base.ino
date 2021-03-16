
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <RF24.h>
#include <RF24Network.h>
#include <SPI.h>

#define CE_PIN 2
#define CSN_PIN 4

#define NETWORK_CHANNEL 90
#define BASE_ID 00

#define PERIOD 1000

const char* ssid = "GATOS";
const char* password = "venceromorir";

unsigned long last_millis = millis();
const uint16_t gnomos[2] = {01, 02};

RF24 radio(CE_PIN, CSN_PIN);
RF24Network network(radio);

void setup() {
    Serial.begin(115200);
    initRadio();
    initWiFi();
}

void loop() {
    network.update();

    if (millis() - last_millis > PERIOD) {
        requestData();
        last_millis = millis();
    }

    if (network.available()) {
        RF24NetworkHeader header;
        uint16_t payloadSize = network.peek(header);
        char payload[payloadSize];

        network.read(header, &payload, payloadSize);

        String fields = String(payload).substring(0, payloadSize);
        Serial.print("Got: ");
        Serial.println(fields);

        sendToCloud(fields);
    }
}

void requestData() {
    Serial.print("---------- ");
    Serial.println(millis());
    for (int gnomo = 0; gnomo < 2; gnomo++) {
        Serial.print("Reaching gnomo ");
        Serial.print(gnomos[gnomo]);

        RF24NetworkHeader header(gnomos[gnomo]);
        const char* payload = "DATA_COLLECT";
        bool ok = network.write(header, payload, strlen(payload));

        if (ok)
            Serial.println("... OK");
        else
            Serial.println("... failed");
    }
}

void initRadio() {
    SPI.begin();
    if (!radio.begin()) {
        Serial.println("Radio hardware not responding!");
        while (true) {
            // do nothing
        }
    }
    network.begin(NETWORK_CHANNEL, BASE_ID);
}

void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("Connected!");
}

void sendToCloud(String fields) {
    String line = String("gnomo,base_id=123,gnomo_id=321 ") + fields;

    Serial.print("Influx line: ");
    Serial.println(line);

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("ERROR: WL_CONNECTED");
        return;
    }

    WiFiClient client;
    HTTPClient http;

    if (!http.begin(client, "http://tucanoar.com:8086/write?db=tucanodb")) {
        Serial.println("ERROR: No http.begin");
        return;
    }

    int httpCode = http.POST(line);

    if (httpCode <= 0) {
        Serial.printf("ERROR: POST failed: %s\n", http.errorToString(httpCode).c_str());
        return;
    }

    Serial.printf("[HTTP] POST code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        Serial.print("[HTTP] Response ");
        Serial.print(httpCode);
        Serial.print(": ");
        Serial.println(payload);
    }

    http.end();
}
