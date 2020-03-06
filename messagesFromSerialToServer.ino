#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;
WiFiClient client;
HTTPClient http;

int sensorValue = 0;
int outputValue = 0;
char byteRead;
char cmd[1024];
unsigned int i = 0;
int delayTime = 1000;
String messageFromSerial = "Message from Serial";
String loading = "....";
String serverUrl = "http://tucanoar.com:8086/write?db=tucanodb";

void setup() 
{
  setupSerial();
  setupWiFi();
}

void loop() 
{
  if( !isSerialAvailable()  )
  {
    return; 
  } 

  byteRead = Serial.read();
  
  if( byteRead == '\r' )
  {      
  }else if(byteRead == '\n'){
    cmd[i] = '\0';
    i=0;
    SendMessage( String(cmd) );
  }else{
    cmd[i] = byteRead;
    i++;
  }   
}

void setupSerial()
{
  Serial.begin(115200);
  printMessage( loading, loading );
  printMessage( "Serial Connection" , "OK");
  Serial.flush();
}

void setupWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("soniafashion", "soyfeliz");
}

bool isSerialAvailable()
{
  return Serial.available() > 0;
}

void SendMessage( String message )
{  
  if( !isMessageValid( message ) )
  {
    return;
  }

  connectToWiFi(); 
  
  if ( !canOpenServerConnection() ) 
  {     
    printMessage( "[HTTP]", "Unable to connect" );
    return;
  }  
  
  printMessage( "[HTTP]", "Server Connected..." );

  int httpCode = getServerResponse( message );  
  proccessResponse( httpCode );   
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

bool canOpenServerConnection()
{
  return http.begin( client, serverUrl );  
}

bool isMessageValid( String message )
{
  printMessage( message );
 
  return String(message).startsWith("humidity"); 
}

void connectToWiFi()
{
  printMessage( "connecting to WiFi ... " );
  
  while ( WiFiMulti.run() != WL_CONNECTED ) 
  { 
    printMessage( "." );
  }

  printMessage( "[HTTP]", "WiFi Connected..." );
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
  {  
    printMessage( "[HTTP]", "Successful response" );          
  }
  else 
  {
    printMessage( "[HTTP] failed code", http.errorToString(httpCode).c_str() );
  } 
}

void closeServerConnection()
{
  http.end(); 
  printMessage( "[HTTP]", "Connection closed" );  
}
