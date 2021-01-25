#define PIR_1 2
#define BEAM 7 // Using PIR_2 plug






void setup() {
  pinMode(PIR_1, INPUT);
  pinMode(BEAM, INPUT_PULLUP);
  // put your setup code here, to run once:
  Serial.begin(57600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println();
  Serial.print("PIR_1: ");
  Serial.println(digitalRead(PIR_1));
  Serial.print("BEAM: ");
  Serial.println(digitalRead(BEAM));


  delay(500);
}
