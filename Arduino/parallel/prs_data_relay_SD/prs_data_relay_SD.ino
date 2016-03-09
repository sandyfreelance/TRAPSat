#include <SPI.h>
#include <SD.h>

// Mega pins
int parallel_pins[8] = {35,37,39,41,43,45,47,49}; 
int parallel_prs = 51;

// File IO
File data_log;


//unsigned char parallel_buff;
byte parallel_buff;
int i;
unsigned long timer;

boolean start = 0;

/*
 * Buffer for Debugging Start/Stop Flag 
 */
unsigned char flag_buff[4];



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
  Serial.begin(115200);
  while(!Serial);
  if(!SD.begin()) {
    Serial.println("Setup Failed.");
    while(1) ;
  }
  
  if(SD.exists("data_log.raw")) {
    SD.remove("data_log.raw");
    Serial.println("removed old data_log file.");
  }
  data_log=SD.open("data_log.raw", FILE_WRITE);
  data_log.close();

  if(!SD.exists("data_log.raw")) {
    Serial.println("Error: data_log does not exist.");
    while(1) ;
  }

  data_log=SD.open("data_log.raw", FILE_WRITE);
  Serial.println("Setup Complete");
  delay(1000);
}

void loop() {
  /*
   * PRS loop delay of 100.33 microseconds
   */
  //Serial.println("Strobing...");
  digitalWrite(parallel_prs, HIGH);
  delayMicroseconds(5);
  digitalWrite(parallel_prs, LOW);
  timer = micros();
  
  parallel_buff = 0x00;
  for(i = 0; i < 8; i++)
  {
    parallel_buff |= digitalRead(parallel_pins[i]) << i;
  }
  
  while(micros() - timer < 5){delayMicroseconds(1);} // wait
  
  data_log.write(parallel_buff); // write to file on SD
  
  //int t0 = micros();
  //data_log.flush();
  //int tf = micros();
  
  //Serial.print("flush took ");
  //Serial.print(tf-t0);
  //Serial.println(" micros");
  
  if(micros() > 5000000) // 5 secs
  {
    //Serial.println("Closing file...");
    data_log.flush();
    data_log.close();
    Serial.println("File closed.");
    while(1) ; // HOLD 
  }
  
  while((micros() - timer) < 105) // wait
  {
    delayMicroseconds(1); 
  }
}
