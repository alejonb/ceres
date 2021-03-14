// #include "gnomoX.h"
// #include <RH_NRF24.h>

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

// struct payload_t {
//     unsigned long ms;
//     unsigned long counter;
// };

void setup() {
    Serial.begin(115200);
    initRadio();
    Serial.println("All good!");
}

void loop() {
    String msg;
    network.update();

    while (network.available()) {
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
        int hum = getHumidity();

        itoa(hum, buff, 10);
        // Serial.print("Hum: ");
        // Serial.println(hum);
        // Serial.print("Buff: ");
        // Serial.println(buff);
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

int getHumidity() {
    return 8963;
    // return map(analogRead(ANALOG_PIN), ANALOG_LOW, ANALOG_HIGH, 100, 0);
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

// char MessageReceived() {
//     while (network.available()) {
//         RF24NetworkHeader header;
//         payload_t payload;
//         network.read(header, &payload, sizeof(payload));
//         Serial.print("Received packet #");
//         Serial.print(payload.counter);
//         Serial.print(" at ");
//         Serial.println(payload.ms);

//         return payload.ms;
//     }

//     return emptyMessage;
// }
