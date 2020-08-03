#include <RH_NRF24.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "mis raices";
const char* password = "soyfeliz";
const int timeToCollect = 10000;
const int period = 10000;
unsigned long timeNow = 0;

uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

RH_NRF24 nrf24(2,4);
 
void setup() 
{
  Serial.begin(9600);
  InitializeRadio();
  InitializeWiFi();
}
 
void loop()
{
  CollectData();
  SentDataToServer();
}

void InitializeRadio()
{
  if (!nrf24.init())
    Serial.println("initialization failed");                           
  if (!nrf24.setChannel(1))
    Serial.println("Channel Set failed");
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
    Serial.println("RF set failed"); 
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
}

void SentDataToServer()
{
  if( MessageWasReceived() )
  {
      if (WiFi.status() == WL_CONNECTED) 
      {
        WiFiClient client;
        HTTPClient http;
    
        Serial.print("[HTTP] begin...\n");
        if (http.begin(client, "http://tucanoar.com:8086/write?db=tucanodb")) {  // HTTP
    
          Serial.print("[HTTP] POST...\n");
          // start connection and send HTTP header
          String message = "node_genesis,cultivo=matera_tapitas humidity=";
          String humidity = String( (char*)buf );
          message.concat( humidity );
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
}

bool MessageWasReceived()
{
  if (nrf24.available())
  {
    uint8_t len = sizeof(buf);
    if(nrf24.recv(buf, &len))
    {
      Serial.print("Received: ");
      Serial.println((char*)buf);

      return true;
    }
    else
    {
      Serial.println("recv failed");
    }
  }

  return false;
}

void RequestData()
{
  String timestring = String(3059);
  
  uint8_t dataArray[timestring.length()+1];
  timestring.getBytes(dataArray,timestring.length()+1);
  
  nrf24.send(dataArray,sizeof(dataArray));
  nrf24.waitPacketSent();
  
  Serial.print("Sent: ");
  Serial.println(timestring);
}
