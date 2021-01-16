
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

#define CE_PIN 2
#define CSN_PIN 15

const char* ssid = "GATOS";
const char* password = "venceromorir";
const int timeToCollect = 10000;
const int period = 10000;
unsigned long timeNow = 0;
const uint16_t gnomos[2] = { 01, 02 };
String humidities[] = {};
String crops[2] = {"albahaca", "besitos"};

RF24 radio(CE_PIN, CSN_PIN);

RF24Network network(radio);

const uint16_t this_node = 00;
const uint16_t other_node = 01;

unsigned long last_sent;
unsigned long packets_sent;

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

String payload = "";

void setup() 
{
  Serial.begin(115200);
  InitializeRadio();
  InitializeWiFi();
}
 
void loop()
{
  network.update();
  
  CollectData();
}

void InitializeRadio()
{
  SPI.begin();
  radio.begin();
  network.begin(90, this_node);
}

void InitializeWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(1000);
  }
}

void CollectData()
{
  if( millis() >= timeNow + period )
  {
      timeNow += period;

      RequestData();
  }

  network.update();
  
  if( network.available() )
  {  
    RF24NetworkHeader header;
    uint16_t payloadSize = network.peek(header);
    char payload[payloadSize];
    
    network.read(header,&payload,payloadSize);
        
    String msg = String(payload).substring(0,payloadSize);
    Serial.print("msg recibido de base>");
    Serial.println(msg);

    SendMessage( msg );
  }
}

void SendMessage(String humidity)
{
    String message = "gnomo_0,cultivo=maiz humidity=";
    message.concat(humidity);

    Serial.print("Influx query: "); Serial.println(message);
    SendDataToServer(message);  
}

void SendDataToServer( String message )
{
      if (WiFi.status() == WL_CONNECTED) 
      {
        WiFiClient client;
        HTTPClient http;
    
        Serial.print("[HTTP] begin...\n");
        if (http.begin(client, "http://tucanoar.com:8086/write?db=tucanodb")) {  // HTTP
    
          Serial.print("[HTTP] POST...\n");
          // start connection and send HTTP header
          
          Serial.println(message);
          int httpCode = http.POST( message );
    
          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    
            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
              String payload = http.getString();
              Serial.println(payload);
            }
          } else {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
          }
    
          http.end();
        } else {
          Serial.printf("[HTTP} Unable to connect\n");
        }
      }
}

bool MessageWasReceived(int gnomoId)
{
  return false;
}

void RequestData()
{  
  for( int gnomo = 0; gnomo < 2; gnomo++ )
  {    
    Serial.print("Sending...");
    Serial.print("gnomo");
    Serial.print(gnomos[gnomo]);
    //payload_t payload = { millis(), packets_sent++ };
    char *payload = "DATA_COLLECT";
    RF24NetworkHeader header(gnomos[gnomo]);
    Serial.println(strlen(payload));
    bool ok = network.write(header,payload,strlen(payload));
    
    if (ok)
      Serial.println(" ok.");
    else
      Serial.println(" failed."); 
    
//    Serial.print("Sent: ");
//    Serial.println(gnomos[gnomo]);  
  }
}
