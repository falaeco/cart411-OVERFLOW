const float FLOW_RATE = 13.7f;

int digitalPin = 13;

void setup() {
  Serial.begin(115200);
  pinMode(digitalPin, OUTPUT);
}

int getTimeForVolume(int volume) {
  return int((float(volume) / FLOW_RATE) * 1000);
}

void loop() {
  if(Serial.available()) {
    int mils = Serial.read();
    Serial.println(mils);

    int time = getTimeForVolume(mils);
    digitalWrite(digitalPin, HIGH);
    delay(time);
    digitalWrite(digitalPin, LOW);
  }
  
}
