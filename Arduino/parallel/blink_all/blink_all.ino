int parallel_pins[8] = {35,37,39,41,43,45,47,49};
int parallel_prs = 51;
char parallel_buff;
int i;

void setup() {  

  pinMode(parallel_prs, OUTPUT);
  
  /*
   * Not sure if this is necessary, may by default be low.. 
   */
  digitalWrite(parallel_prs, LOW); 
  
  for(i = 0; i < 8; i++)
  {
    pinMode(parallel_pins[i], OUTPUT);
    digitalWrite(parallel_pins[i], LOW); 
  }

  /*
   * Ready: start/wait for serial monitor
   */
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Setup Complete");
  Serial.println("Starting blink Loop");
  delay(1000);
}

void loop() {
  for(i = 0; i < 8; i++)
  {
    digitalWrite(parallel_pins[i], LOW); 
  }
  digitalWrite(parallel_prs, HIGH); 
 delay(6000);
   for(i = 0; i < 8; i++)
  {
    digitalWrite(parallel_pins[i], HIGH); 
  }
  digitalWrite(parallel_prs, LOW); 
 delay(6000);
}
