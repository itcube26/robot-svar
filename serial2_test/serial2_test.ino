void setup() {
  Serial.begin(9600);
  Serial2.begin(115200);
}

void loop() {
  if(Serial2.available()){
    Serial.write(Serial2.read());
  }   

}
