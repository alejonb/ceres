#include <RH_NRF24.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "GATOS";
const char* password = "venceromorir";
const int timeToCollect = 10000;
const int period = 10000;
unsigned long timeNow = 0;
String gnomos[2] = { "dataCollect-gnomo1", "dataCollect-gnomo2" };
String humidities[] = {};
String crops[2] = {"albahaca", "besitos"};

uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

RH_NRF24 nrf24(2,4);
 
void setup() 
{
  Serial.begin(9600);
  Serial.println("(:)");
  InitializeRadio();
  InitializeWiFi();
}
 
void loop()
{
  CollectData();
  
  //SentDataToServer();
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

    Serial.println("Collecting data");
      timeNow += period;

      RequestData();
  }
}

void SentDataToServer( String message )
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
  String gnomosIds[2] = { "gnomo1", "gnomo2" };
  
  if ( nrf24.waitAvailableTimeout(1000) )
  {
    uint8_t len = sizeof(buf);
    if(nrf24.recv(buf, &len))
    {
      Serial.print("Received: ");
      Serial.println((char*)buf);

      String message = (char*)buf; //gnomo147
      String gnomoNumber = message.substring(0,6);
      String humidity = message.substring(6);

      Serial.print("gnomoNumber: ");
      Serial.println(gnomoNumber);
      Serial.print("humidity: ");
      Serial.println(humidity);
      
      if( gnomoNumber == gnomosIds[gnomoId] )
      {
        return true;    
      }
      
      return false;
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
  for(int gnomo = 0; gnomo < 2; gnomo++ )
  {    
    uint8_t dataArray[gnomos[gnomo].length() + 1];
    gnomos[gnomo].getBytes(dataArray, gnomos[gnomo].length()+1);
    
    nrf24.send(dataArray,sizeof(dataArray));
    nrf24.waitPacketSent();
    
    Serial.print("Sent: ");
    Serial.println(gnomos[gnomo]);

    if( MessageWasReceived(gnomo) )
    {
        Serial.print("#gnomo > ");
        Serial.println(gnomo+1);
        String gnomoMsg = String( (char*)buf );
        String humidity = gnomoMsg.substring(6);
        String message = "gnomo_";
        message.concat(gnomo+1);
        message.concat(",cultivo=");
        message.concat(crops[gnomo]);
        message.concat(" humidity=");
        message.concat(humidity);

        Serial.println(message);
        SentDataToServer(message);
    }    
  }
}
