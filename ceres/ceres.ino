#define SERIAL_BAUDRATE 115200
#define HUMIDITY_SENSOR_PIN A3
#define BASE_INFLUX_QUERY "humidity,sensor=analog-shield value="


void setup()
{
  Serial.begin(SERIAL_BAUDRATE);
}

void loop()
{
  int sensorValue = analogRead(analogInPin);
  int outputValue = map(sensorValue, 0, 1023, 0, 100);

  String messageToSend = BASE_INFLUX_QUERY;
  messageToSend.concat(outputValue);

  Serial.println(messageToSend);
  delay(5000);
}
