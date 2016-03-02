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
  delayMicroseconds(1);
  digitalWrite(parallel_prs, LOW);
  timer = micros();
  while(micros() - timer < 5) 
  {
    parallel_buff = 0x00;
    for(i = 0; i < 8; i++)
    {
      parallel_buff |= digitalRead(parallel_pins[i]) << i;
    }
  }
  
 // if(parallel_buff != 0x00) { // wait for data
 //   start = 1;
 // }
  
  //if (start) {
    //Serial.print("Byte data: ");
    //Serial.write(parallel_buff);
    //Serial.println("");
  
    //int bytes_written = data_log.write(parallel_buff);
    data_log.write(parallel_buff);
    //Serial.print("Bytes written to file: ");
    //Serial.println(bytes_written);
  //}
  //delayMicroseconds(100);
  
  if(timer > 30000000) // 0.5 minutes
  {
    Serial.println("Closing file...");
    data_log.flush();
    data_log.close();
    while(1) ; // HOLD
  }
  
  while((micros() - timer) < 105)
  {
    //Serial.println("PRS Delay");
  }
}
