#include <RH_NRF24.h>

RH_NRF24 nrf24;

const int analogInPin = A0;
int sensorValue = 0;
int outputValue = 0;

void setup() 
{
  Serial.begin(9600);
  while (!Serial);      
  if (!nrf24.init())
    Serial.println("initialization failed");
  if (!nrf24.setChannel(1))
    Serial.println("Channel set failed");
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
    Serial.println("RF set failed");         
}
 
void loop()
{
  sensorValue = analogRead(analogInPin);
  outputValue = map(sensorValue, 675, 1023, 100, 0);
  
  Serial.println("Sending data to receiver");
  String timestring = String(outputValue);
  Serial.println(timestring);
  Serial.println(timestring.length());
  uint8_t dataArray[timestring.length()+1];
  timestring.getBytes(dataArray,timestring.length()+1);
  nrf24.send(dataArray,sizeof(dataArray));
  nrf24.waitPacketSent();
  delay(1000);
}
