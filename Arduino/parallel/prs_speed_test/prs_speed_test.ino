int parallel_pins[8] = {35,37,39,41,43,45,47,49};
int parallel_prs = 51;
char parallel_buff;
int i;
unsigned long timer;

void setup() {  

  pinMode(parallel_prs, OUTPUT);
  
  /*
   * Not sure if this is necessary, may by default be low.. 
   */
  digitalWrite(parallel_prs, LOW); 
  
  for(i = 0; i < 8; i++)
  {
    pinMode(parallel_pins[i], INPUT);
  }

  /*
   * Ready: start/wait for serial monitor
   */
  Serial.begin(230400);
  while(!Serial);
  Serial.println("Setup Complete");
  Serial.println("Starting Blink Loop");
  delay(1000);
}

void loop() {
  /*
   * PRS loop delay of 100.33 microseconds
   */
  //Serial.println("Strobing...");
  digitalWrite(parallel_prs, HIGH);
  delayMicroseconds(1);
  digitalWrite(parallel_prs, LOW);
  timer = micros();
  while(micros() - timer < 5) 
  {
    for(i = 0; i < 8; i++)
    { 
      parallel_buff |= digitalRead(parallel_pins[i]) << i;
    }
  }
  Serial.print("Byte data: ");
  Serial.println(parallel_buff);
  parallel_buff = 0;
  while(micros() - timer < 105)
  {
    Serial.println("PRS Delay");
  }  

}
