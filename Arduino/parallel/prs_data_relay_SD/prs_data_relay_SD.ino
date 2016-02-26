#include <Bridge.h>
#include <FileIO.h>

#if defined(__AVR_YUN__) // doesn't work
#define YUN 1
#else
#define YUN 0
#endif

//if(YUN){
  int parallel_pins[8] = {2,3,4,5,6,7,8,9};
  int parallel_prs = 13;

//else { // Mega
//  int parallel_pins[8] = {35,37,39,41,43,45,47,49};
//  int parallel_prs = 51;



unsigned char parallel_buff;
int i;
unsigned long timer;

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
    //if(YUN){
    Serial.println("Compiling for Yun.");
    Bridge.begin();
    FileSystem.begin();
  //}
  Serial.println("Setup Complete");
  delay(1000);
}

void loop() {
  /*
   * PRS loop delay of 100.33 microseconds
   */
  Serial.println("Strobing...");
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
  

  //Serial.print("Byte data: ");
  
  Serial.write(parallel_buff);

  //if(YUN){
    File data_log = FileSystem.open("/mnt/sda1/data_log.raw");
    if(data_log == true){
      Serial.print("Number of bytes written: ");
      Serial.println(data_log.write(parallel_buff));
      data_log.flush();
      data_log.close();
    }
  //}
  
  delayMicroseconds(100);
  
  /*
  while(micros() - timer < 105)
  {
    //Serial.println("PRS Delay");
  }
  */

}
