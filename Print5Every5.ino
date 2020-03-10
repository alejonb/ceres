const int analogInPin = A3;
int sensorValue = 0;
int outputValue = 0;
String message = "humidity,sensor=analog-shield value=";

void setup() {
  Serial.begin(115200);
}

void loop() {
  
  sensorValue = analogRead(analogInPin);
  outputValue = map(sensorValue, 0, 1023, 0, 100);    

  String messageToSend = message; 
  messageToSend.concat(outputValue);
   
  Serial.println(messageToSend);
  delay(5000);
}
