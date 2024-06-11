/*
  Basic Arduino example for K-Series sensor
  Created by Jason Berger
  Co2meter.com  
*/

#include "kSeries.h"


kSeries kSensor(0,0);


void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);         //Opens the main serial port to communicate with the computer
}

void loop() 
{
  double valCO2 = kSensor.getCO2('P');
  Serial.print("Co2 ppm = ");
  Serial.println(valCO2);
  
  double valTemp = kSensor.getTemp('C');
  Serial.print("Temp C = ");
  Serial.println(valTemp);

  double valRH = kSensor.getRH();
  Serial.print("RH % = ");
  Serial.println(valRH);  
  delay(2000);
  
}
