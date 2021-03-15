
// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
// #include <WiFiClient.h>
#include <RF24.h>
#include <RF24Network.h>
#include <SPI.h>

#define CE_PIN 2
#define CSN_PIN 4

#define NETWORK_CHANNEL 90
#define BASE_ID 00

#define PERIOD 1000

// const char* ssid = "GATOS";
// const char* password = "venceromorir";

unsigned long last_millis = millis();
const uint16_t gnomos[2] = {01, 02};

RF24 radio(CE_PIN, CSN_PIN);
RF24Network network(radio);

void setup() {
    Serial.begin(115200);
    initRadio();
    // initWiFi();
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

        String msg = String(payload).substring(0, payloadSize);
        Serial.print("Got: ");
        Serial.println(msg);

        // sendToCloud( msg );
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

// void initWiFi()
// {
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(ssid, password);
//   Serial.println("\nConnecting to WiFi");
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     Serial.print(".");
//     delay(1000);
//   }
// }

// void sendToCloud(String humidity)
// {
//     String message = "gnomo_0,cultivo=maiz humidity=";
//     message.concat(humidity);

//     Serial.print("Influx query: "); Serial.println(message);
//       if (WiFi.status() == WL_CONNECTED)
//       {
//         WiFiClient client;
//         HTTPClient http;

//         Serial.print("[HTTP] begin...\n");
//         if (http.begin(client, "http://tucanoar.com:8086/write?db=tucanodb"))
//         {  // HTTP

//           Serial.print("[HTTP] POST...\n");
//           // start connection and send HTTP header

//           Serial.println(message);
//           int httpCode = http.POST( message );

//           // httpCode will be negative on error
//           if (httpCode > 0) {
//             // HTTP header has been send and Server response header has been
//             handled Serial.printf("[HTTP] POST... code: %d\n", httpCode);

//             // file found at server
//             if (httpCode == HTTP_CODE_OK || httpCode ==
//             HTTP_CODE_MOVED_PERMANENTLY) {
//               String payload = http.getString();
//               Serial.println(payload);
//             }
//           } else {
//             Serial.printf("[HTTP] POST... failed, error: %s\n",
//             http.errorToString(httpCode).c_str());
//           }

//           http.end();
//         } else {
//           Serial.printf("[HTTP} Unable to connect\n");
//         }
//       }
// }
