
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

#define GNOMO_ID "gnomo1"
#define HIGH_LIMIT 1023
#define LOW_LIMIT 500

const int humidityInPin = A0;
const String dataRequested = "DATA_COLLECT";
const char emptyMessage = '0';

RF24 radio(8,10);

RF24Network network(radio);
const uint16_t this_node = 02;
const uint16_t baseId = 00;
const uint16_t gnomos[2] = { 00, 01 };

struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

void setup() 
{
  Serial.begin(115200);
  InitializeRadio(); 
}
 
void loop()
{  
  network.update();
  
  if( network.available() )
  {  
    RF24NetworkHeader header;
    uint16_t payloadSize = network.peek(header);
    char payload[payloadSize];
    
    network.read(header,&payload,payloadSize);
        
    String msg = String(payload).substring(0,payloadSize);
    Serial.print("substring>");
    Serial.println(msg);
    
    if( msg == dataRequested )
    {
      char buff[16];
      int hum = GetHumidity();
      
      itoa(hum, buff, 10);
      Serial.print("Hum: "); Serial.println(hum);
      Serial.print("Buff: "); Serial.println(buff);
      SendMessage(buff);
    }
  }
}

void InitializeRadio()
{
  SPI.begin();
  radio.begin();
  network.begin(90, this_node);
}

int GetHumidity()
{
  int sensorValue = analogRead(humidityInPin);
  int outputValue = map(sensorValue, LOW_LIMIT, HIGH_LIMIT, 100, 0);

  // Serial.print("Sensor Value: "); Serial.println(sensorValue);

  return outputValue;
}

void SendMessage(char* payload )
{
//  String message = GNOMO_ID;
//  message.concat( String(humidity) );
//  
//  uint8_t dataArray[message.length()+1];
//  message.getBytes(dataArray,message.length()+1);
//  
//  Serial.print("Sent: ");
//  Serial.println(message);

    RF24NetworkHeader header(baseId);
    bool ok = network.write(header,payload,strlen(payload));
    
    if (ok)
      Serial.println("message sent");
    else
      Serial.println("message dalivery failed");
}

char MessageReceived()
{
  while ( network.available() ) 
  {  
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header,&payload,sizeof(payload));
    Serial.print("Received packet #");
    Serial.print(payload.counter);
    Serial.print(" at ");
    Serial.println(payload.ms);

    return payload.ms;
  }
  
  return emptyMessage;
}
