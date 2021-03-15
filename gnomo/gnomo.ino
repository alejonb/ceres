#include <RF24.h>
#include <RF24Network.h>
#include <SPI.h>

#define CE_PIN 2
#define CSN_PIN 4

#define NETWORK_CHANNEL 90
#define BASE_ID 00
#define GNOMO_ID 01

#define ANALOG_PIN A0
#define ANALOG_HIGH 1023
#define ANALOG_LOW 500

RF24 radio(CE_PIN, CSN_PIN);
RF24Network network(radio);

void setup() {
    Serial.begin(115200);
    initRadio();
    Serial.println("All good!");
}

void loop() {
    String msg;
    network.update();

    if (network.available()) {
        RF24NetworkHeader header;
        const uint16_t payloadSize = network.peek(header);
        char payload[payloadSize];

        network.read(header, &payload, payloadSize);

        msg = String(payload).substring(0, payloadSize);
        Serial.print("Gnomo got: ");
        Serial.println(msg);
    }

    delay(10);
    if (msg == "DATA_COLLECT") {
        char buff[16];
        int hum = map(analogRead(ANALOG_PIN), ANALOG_LOW, ANALOG_HIGH, 100, 0);
        itoa(hum, buff, 10);
        sendMessage(buff);
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
    network.begin(NETWORK_CHANNEL, GNOMO_ID);
}

void sendMessage(const char* payload) {
    Serial.print("--------- ");
    Serial.println(millis());
    Serial.print("Reaching base ");
    Serial.print(BASE_ID);
    Serial.print(": ");
    Serial.print(payload);

    RF24NetworkHeader header(BASE_ID);
    bool ok = network.write(header, payload, strlen(payload));

    if (ok)
        Serial.println("... OK");
    else
        Serial.println("... failed");
}
