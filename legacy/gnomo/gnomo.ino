#include <RH_NRF24.h>

RH_NRF24 nrf24;

const int humidityInPin = A0;
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
const String dataRequested = "dataCollect-gnomo1";
const String emptyMessage = "";
 
void setup() 
{
  Serial.begin(9600);
  InitializeRadio(); 
}
 
void loop()
{  
  if( MessageReceived() == dataRequested )
  {
    SentMessage( GetHumidity() );
  }
}

void InitializeRadio()
{
  if (!nrf24.init())
    Serial.println("initialization failed");
  if (!nrf24.setChannel(1))
    Serial.println("Channel set failed");
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
    Serial.println("RF set failed"); 
}

int GetHumidity()
{
  int sensorValue = analogRead(humidityInPin);
  int outputValue = map(sensorValue, 500, 1023, 100, 0);

  Serial.println("Sensor Value: ");
  Serial.print(sensorValue);

  return outputValue;
}

void SentMessage(int humidity)
{
  String timestring = String(humidity);
  
  uint8_t dataArray[timestring.length()+1];
  timestring.getBytes(dataArray,timestring.length()+1);
  
  nrf24.send(dataArray,sizeof(dataArray));
  nrf24.waitPacketSent();

  Serial.print("Sent: ");
  Serial.println(timestring);
}

String MessageReceived()
{
  if (nrf24.available())
  {
    uint8_t len = sizeof(buf);
    if(nrf24.recv(buf, &len))
    {
      String message = String( (char*)buf );
      Serial.print("Received: ");
      Serial.println((char*)buf);

      return message;
    }
    else
    {
      Serial.println("recv failed");
    }
  }

  return emptyMessage;
}
