/*
Created by Sarah Louise Brown @ University of Manchester Geography Department as part of a submission for a doctoral degree.
Modified by Thomas Bishop @ University of Manchester SEED Laboratories. 
Debugged by Theo Heath @ University of Manchester Faculty of Science and Engineering

First created on 11/08/2017
Last modified on 31/07/2024

Published as part of: Brown, S. L., Goulsbra, C. S., Evans, M. G., Heath, T., & Shuttleworth, E. (2020). Low cost CO2 sensing: A simple microcontroller approach with calibration and field use. Hardware X. https://doi.org/10.1016/j.ohx.2020.e00136
If you use or expand on this design or code please use the reference above.
Active development is documented at https://github.com/tombishop1/CO2Sensor

Some of the code for the MPL3115A2 was taken from code produced by K.Townsend (Adafruit Industries): https://www.adafruit.com/products/1893
Some of the code for the microSD card module was taken from code prodcued by Dejan Nedelkovski: http://howtomechatronics.com/tutorials/arduino/arduino-sd-card-data-logging-excel-tutorial/
Some of the code for the microSD card module was taken from code produced by Tom Igoe.
Some of the code for the DS3231 RTC breakout board was taken from code produced by Tony DiCola (Adafruit Industries). https://learn.adafruit.com/adafruit-ds3231-precision-rtc-breakout
Some of the code for the pushbutton was taken from code on Arduino.cc https://www.arduino.cc/en/Tutorial/StateChangeDetection

CIRCUITRY - Using the Arduino Uno.
MPL3115A2:  Vin - 5V
            GND - GND
            SCL - Analog 5
            SDA - Analog 4
            
MicroSD Card Module:  5V - 5V
                      GND - GND
                      CLK - 13
                      DO - 12
                      DI - 11
                      CS - 10
                      SD card must be in FAT/FAT32 format

DS3231 RTC: Vin - 5V
            GND - GND
            SCL - Analog 5
            SDA - Analog 4
            Can use CR1220 coin cell battery for clock backup power, which is highly recommended.
            You must use the example sketch DS3231 to set the time before integration into final device.
            
SenseAir CO2 Engine K30:  TxD - 7
                          RxD - 6
                          G+ - 5V
                          G0 - GND
                          See Figure 2 in the Datasheet and Manual for board pinouts.

Two wire component:   GND wire - GND (SCA 2nd hole)
                      Power - Digital 5 (SDA 1st hole) or other free digital pin
  
Pushbutton: Most push buttons are reversible in direction. Refer to https://www.arduino.cc/en/Tutorial/StateChangeDetection or paper supplementary for further instructions.                          
*/

#include <SPI.h> //SD card library
#include <SD.h> //SD card library
#include <Wire.h> //library for connecting microprocessors together
#include <Adafruit_MPL3115A2.h> //MPL3115A2 library
#include <KSeries.h> //K30 sensor library
#include <RTClib.h> //DS3231 RTC library

Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2(); //define this board in the code as 'baro'
int pinCS = 10; // Pin 10 on Arduino Uno is set as the ChipSelect pin
kSeries K_30(6,7); // Create K30 instance on pin 6 & 7
File myFile; //Define the file to write to
RTC_DS3231 rtc; //define the clock board as 'rtc'
int twowire = 5; // the pin the LED is connected to
const int buttonPin = 8; //the number of the pushbutton pin
int buttonPushCounter = 0;   //counter for the number of button presses
int buttonState = 0;         //variable for the current state of the pushbutton. This will count up with each button push.
int lastButtonState = 0;     //previous state of the button

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600); // baud rate 9600.
  pinMode(pinCS, OUTPUT);

  //Board Initialisations
  SD.begin();
  baro.begin();

  //Set RTC (check date and time at deployment)  
  rtc.begin();
    DateTime newDT = DateTime(2024, 7, 31, 17, 0, 0);
  rtc.adjust(newDT);

  // Open up the file we're going to log to.
  myFile = SD.open("datalog.txt", FILE_WRITE); 

  // Declare the two wire component as an output
  pinMode(twowire, OUTPUT); 

  // Print success message to debug
  Serial.println("setup done");
}

void loop() {
  //Define the time now
  DateTime dt = rtc.now(); 

  // Push button counter
  // Read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin); 
  // Compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) { 
      buttonPushCounter++; //add 1 (++) to the current push counter
     }    
     lastButtonState = buttonState; // save the current state as the last state, for next time through the loop
     }
  // Delay to avoid bouncing   
  delay(100); 
 
//Show on console the logged data 
  Serial.print("Counter: ");
  Serial.println(buttonPushCounter);   
  Serial.print(dt.year(), DEC); //now showing data from DS3231 RTC. 'DEC' indicates to show the data in decimal format (as opposed to e.g. binary).
  Serial.print('/'); //used to show date in format DD/MM/YYYY.
  Serial.print(dt.month(), DEC);
  Serial.print('/');
  Serial.print(dt.day(), DEC);
  Serial.print("  "); //Create on screen gap between date and time.
  Serial.print(dt.hour(), DEC);
  Serial.print(':'); //used to show time in format HH:MM:SS.
  Serial.print(dt.minute(), DEC);
  Serial.print(':');
  Serial.print(dt.second(), DEC);
  Serial.println();  //start new line in serial monitor.
  Serial.print("Co2 ppm = ");
  Serial.println(K_30.getCO2('p'));
  Serial.print(baro.getPressure()); Serial.println(" Pascals"); //print on screen the pressure in kilo Pascals.
  Serial.print(baro.getAltitude()); Serial.println(" meters"); //print on screen the altitude in metres.
  Serial.print(baro.getTemperature()); Serial.println("*C"); //print on screen the temperature in degrees Celsius.

//Use an IF control structure to switch two wire component on or off
  if (K_30.getCO2('p')>=700){
    digitalWrite(twowire, HIGH);} // Turn the two wire component on
    else{
    digitalWrite(twowire, LOW); // Turn the two wire component off
  }
    
// Use an IF control structure to define when measurements are taken (in this case, every 10 seconds)
  if (dt.second()==0 || dt.second()==10 || dt.second()==20 || dt.second()==30 || dt.second()==40 || dt.second()==50) {
    myFile = SD.open("datalog.csv", FILE_WRITE); // file name will be datalog
      myFile.write("\n");
      myFile.print(baro.getPressure()); // pressure (mbar) from MPL3115A2
      myFile.print(","); 
      myFile.print(buttonPushCounter, DEC); // button state (n)
      myFile.print(",");
      myFile.print(baro.getAltitude()); // altitude (m) from MPL3115A2
      myFile.print(",");
      myFile.print(baro.getTemperature()); // temperature (degC) from MPL3115A2
      myFile.print(",");
      myFile.print(K_30.getCO2('p')); // CO2 (ppm) from K30.
      myFile.print(",");
      myFile.print(dt.year(), DEC);
      myFile.print(",");      
      myFile.print(dt.month(), DEC); // time from DS3231 RTC. 'DEC' indicates to log the data in decimal format.
      myFile.print(",");
      myFile.print(dt.day(), DEC);
      myFile.print(",");
      myFile.print(dt.hour(), DEC);
      myFile.print(',');
      myFile.print(dt.minute(), DEC);
      myFile.print(','); 
      myFile.print(dt.second(), DEC);
      myFile.print(',');
      myFile.close(); // close file on microSD card
      delay(9000); // delay to avoid repeat measurement within same second. X000 milliseconds == X seconds.
  } 
} 
