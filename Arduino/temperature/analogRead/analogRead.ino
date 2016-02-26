#include <math.h>


int sensor1 = A0;
int sensor2 = A2;
int sensor3 = A4;
long int biasResistance = 300000;
long int temp1 = 0;
long int temp2 = 0;
long int temp3 = 0;
long int volt1, volt2, volt3;
int THERM_NOM_BCOEF = 3892;    // K
int THERM_NOM_TEMP = 25; //        degrees C



void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);
   while( !Serial ) ;
   Serial.println("Cryo Test Started");
}

void loop() {

  volt1 = analogRead(sensor1); /*maybe divide by 1000? ads1x15_ex_singleended.py */
  volt2 = analogRead(sensor2);
  volt3 = analogRead(sensor3);
  
  /*
  Serial.print("Temp 1 Voltage (External) :");
  Serial.println(temp1);
  Serial.print("Temp 2 Voltage (Internal) :");
  Serial.println(temp2);
  */
  
  long int therm_res_1 = (biasResistance) / ((1023/volt1) - 1);
  long int therm_res_2 = (biasResistance) / ((1023/volt2) - 1);
  long int therm_res_3 = (biasResistance) / ((1023/volt3) - 1); /* maybe use 4096 for this? should this be the gain (5v) */
  
  /*
  Serial.print("External Resistance:");
  Serial.println(temp1);
  Serial.print("Internal Resistance:");
  Serial.println(temp2);
  */
   
  temp1 = therm_res_1 / 10000;
  temp1 = log(temp1);
  temp1 = temp1 / THERM_NOM_BCOEF;
  temp1 = temp1 + (1.0 / (THERM_NOM_TEMP + 273.15));
  temp1 = 1.0 / temp1;
  temp1 = temp1 - 273.15; //   # convert to C
  
  temp2 = therm_res_2 / 10000;
  temp2 = log(temp2);
  temp2 = temp2 / THERM_NOM_BCOEF;
  temp2 = temp2 + (1.0 / (THERM_NOM_TEMP + 273.15));
  temp2 = 1.0 / temp2;
  temp2 = temp2 - 273.15 ; //   # convert to C

  temp3 = therm_res_3 / 10000;
  temp3 = log(temp3);
  temp3 = temp3 / THERM_NOM_BCOEF;
  temp3 = temp3 + (1.0 / (THERM_NOM_TEMP + 273.15));
  temp3 = 1.0 / temp3;
  temp3 = temp3 - 273.15 ; //   # convert to C
  
  Serial.print("External: (Voltage ");
  Serial.print(volt1);
  Serial.print(")\t (Resistance ");
  Serial.print(therm_res_1);
  Serial.print(")\t (Temperature ");
  Serial.print(temp1);
  Serial.print("C)\n");
  
  Serial.print("Internal: (Voltage ");
  Serial.print(volt2);
  Serial.print(")\t (Resistance ");
  Serial.print(therm_res_2);
  Serial.print(")\t (Temperature ");
  Serial.print(temp2);
  Serial.print("C)\n");

  Serial.print("Other: (Voltage ");
  Serial.print(volt3);
  Serial.print(")\t (Resistance ");
  Serial.print(therm_res_3);
  Serial.print(")\t (Temperature ");
  Serial.print(temp3);
  Serial.print("C)\n");
  
  //Serial.println();
  delay(3000);
}
