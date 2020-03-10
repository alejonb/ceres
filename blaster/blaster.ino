/*
  Tucano AR
  This code is intended to provide a web client for data uploading to
  a influx database server.
  Payload arrives via Serial (from arduino or any other device).
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>


#define SERIAL_BAUDRATE 115200
#define TUCANO_INFLUX_DB "http://tucanoar.com:8086/write?db=tucanodb"
#define WIFI_SSID "soniafashion"
#define WIFI_PASS "soyfeliz"


ESP8266WiFiMulti WiFiMulti;
WiFiClient client;
HTTPClient http;

char cmd[1024];
unsigned int i = 0;


void setup()
{
  setupSerial();
  setupWiFi();
}

void loop()
{
  if( !Serial.available() ) return;

  char byteRead = Serial.read();

  if(byteRead == '\r'){
    // Emtpty because we just want to skip this character
  } else if (byteRead == '\n'){
    cmd[i] = '\0';
    i = 0;
    processPayload( String(cmd) );
  } else {
    cmd[i] = byteRead;
    i++;
  }
}

void setupSerial()
{
  Serial.begin(SERIAL_BAUDRATE);
  printMessage("Serial Connection" , "OK");
  Serial.flush(); // Clean the toilet :)
}

void setupWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);

  printMessage( "connecting to WiFi ... " );
  while ( WiFiMulti.run() != WL_CONNECTED ) printMessage( "." );
  printMessage( "[HTTP]", "WiFi Connected..." );
}

void processPayload( String message )
{
  if( !isMessageValid( message ) ) return;

  if( !http.begin( client, TUCANO_INFLUX_DB ) )
  {
    printMessage( "[HTTP]", "Unable to connect" );
    return;
  }

  printMessage( "[HTTP]", "Server Connected..." );

  proccessResponse( getServerResponse( message ) );
  closeServerConnection();
}

void printMessage( String initialMessage, String finalMessage )
{
  initialMessage.concat(" : ");
  initialMessage.concat( finalMessage );
  Serial.println( initialMessage );
  delay( 1000 );
}

void printMessage( String message )
{
  Serial.println( message );
  delay( 1000 );
}

bool isMessageValid( String message )
{
  // TODO: find good way to validate message
  return true;
}

int getServerResponse( String message )
{
  int httpCode = http.POST(message);
  printMessage( "[HTTP]", "POST request sent...");
  printMessage( "[HTTP] response code", String( httpCode ) );

  return httpCode;
}

int proccessResponse( int httpCode )
{
  if (httpCode == HTTP_CODE_NO_CONTENT)
    printMessage( "[HTTP]", "Successful response" );
  else
    printMessage( "[HTTP] failed code", http.errorToString(httpCode).c_str() );
}

void closeServerConnection()
{
  http.end();
  printMessage( "[HTTP]", "Connection closed" );
}
