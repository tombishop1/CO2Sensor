  /*
Created by Sarah Louise Brown @ University of Manchester Geography Department as part of a submission for a doctoral degree.
First created on 11/08/2017.
Published as part of: Brown, S.L.; Goulsbra, C.G.; Evans, M.G.; Heath, T. (Forthcoming). Low Cost CO2 Sensing: A Simple Microcontroller Approach in the Context of Peatland Fluvial Carbon. 
If you use or expand on this design or code please use the reference above or check for a published reference.

Some of the code for the MPL3115A2 was taken from code produced by K.Townsend (Adafruit Industries).
  https://www.adafruit.com/products/1893
Some of the code for the microSD card module was taken from code prodcued by Dejan Nedelkovski.
  http://howtomechatronics.com/tutorials/arduino/arduino-sd-card-data-logging-excel-tutorial/
Some of the code for the microSD card module was taken from code produced by Tom Igoe.
Some of the code for the DS3231 RTC breakout board was taken from code produced by Tony DiCola (Adafruit Industries).
  https://learn.adafruit.com/adafruit-ds3231-precision-rtc-breakout
Some of the code for the pushbutton was taken from code on Arduino.cc
  https://www.arduino.cc/en/Tutorial/StateChangeDetection

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
#include "RTClib.h" //DS3231 RTC library. You must use the example sketch DS3231 to set the tme before deployment.

Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2(); //define this board in the code as 'baro'.
int pinCS = 10; // Pin 10 on Arduino Uno is set as the ChipSelect pin.
kSeries K_30(6,7); // Create K30 instance on pin 6 & 7.
File myFile; //Define the file to write to. 
RTC_DS3231 rtc; //define the clock board as 'rtc'.
int twowire = 5; // the pin the LED is connected to

const int buttonPin = 8; //the number of the pushbutton pin

int buttonPushCounter = 0;   //counter for the number of button presses
int buttonState = 0;         //variable for the current state of the pushbutton. This will count up with each button push.
int lastButtonState = 0;     //previous state of the button

void setup() { //Begin setup section of code.
 // Open serial communications and wait for port to open:
  Serial.begin(9600); // baud rate 9600.
  pinMode(pinCS, OUTPUT);

  //Board Initialisations
  SD.begin();
  baro.begin();
  
  myFile = SD.open("datalog.txt", FILE_WRITE); // Open up the file we're going to log to.

  pinMode(twowire, OUTPUT); // Declare the two wire component as an output

} //End setup part of code.

void loop() { //Begin repeating section of code.
 DateTime now = rtc.now(); //Define the time now

//Push button counter
 buttonState = digitalRead(buttonPin); //read the state of the pushbutton value:
  // compare the buttonState to its previous state
 if (buttonState != lastButtonState) { // if the current state is NOT equal (!=) to the last button state    
   if (buttonState == HIGH) { // if the current state is HIGH then the button went from off to on:
     buttonPushCounter++; //add 1 (++) to the current push counter
    }    
    lastButtonState = buttonState; // save the current state as the last state, for next time through the loop
    }
   delay(100); // Delay to avoid bouncing
 
//Show on screen the logged data. Useful to check components are functioning correctly. Remove /* and */ on lines 93 and 118 to execute.
 
  Serial.print("Counter: ");
  Serial.println(buttonPushCounter);  
 
  Serial.print(now.year(), DEC); //now showing data from DS3231 RTC. 'DEC' indicates to show the data in decimal format (as opposed to e.g. binary).
  Serial.print('/'); //used to show date in format DD/MM/YYYY.
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print("  "); //Create on screen gap between date and time.
  Serial.print(now.hour(), DEC);
  Serial.print(':'); //used to show time in format HH:MM:SS.
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();  //start new line in serial monitor.
 
// Show on screen the K30 data. Useful to track what is happening to the CO2 sensor.
  Serial.print("Co2 ppm = ");
  Serial.println(K_30.getCO2('p'));

// Show on screen the MPL3115A2 data. Useful to track what is happening with the barometer.
  Serial.print(baro.getPressure()); Serial.println(" Pascals"); //print on screen the pressure in kilo Pascals.
  Serial.print(baro.getAltitude()); Serial.println(" meters"); //print on screen the altitude in metres.
  Serial.print(baro.getTemperature()); Serial.println("*C"); //print on screen the temperature in degrees Celsius.

//Use an IF control structure to switch two wire component on or off
  if (K_30.getCO2('p')>=700){
    digitalWrite(twowire, HIGH);} // Turn the two wire component on
    else{
    digitalWrite(twowire, LOW); // Turn the two wire component off
  }
  
    
//Use an IF control structure to define when measurements are taken.
  if (now.second()==0 || now.second()==10 || now.second()==20 || now.second()==30 || now.second()==40 || now.second()==50) {
  /* log data when the time is at these values. "==" means take records at exactly this time. "||" is equivalent to "or" and 
  measurements will be recorded when the second equals any of these values. If measurements needed to be taken at e.g. 0 and 
  30 seconds, this would change to (now.second()==0 || now.second()==30).
  All data will be called datalog.txt, a comma delimited file. 
  Once data collection has ended,copy this file to permanent storage. */
    myFile = SD.open("datalog.txt", FILE_WRITE); //file name will be datalog
      myFile.write("\n");//start new row each measurement set. Each row in Microsoft Excel will be a new data record.
      myFile.print(baro.getPressure()); //now logging pressure from MPL3115A2.
      myFile.print(","); //comma is used to create a 'comma delimited' document with each column in Microsoft Excel a recorded variable.
      myFile.print(buttonPushCounter, DEC); //now log button state
      myFile.print(",");
      myFile.print(baro.getAltitude()); //now logging altitude from MPL3115A2.
      myFile.print(",");
      myFile.print(baro.getTemperature()); //now logging temperature from MPL3115A2.
      myFile.print(",");
      myFile.print(K_30.getCO2('p')); //now logging CO2 in ppm from K30.
      myFile.print(",");
      myFile.print(now.month(), DEC);//now logging from DS3231 RTC.  'DEC' indicates to log the data in decimal format.
      myFile.print(",");
      myFile.print(now.day(), DEC);
      myFile.print(",");
      myFile.print(now.hour(), DEC);
      myFile.print(',');
      myFile.print(now.minute(), DEC);
      myFile.print(','); 
      myFile.print(now.second(), DEC);
      myFile.print(',');
      myFile.close(); //close file on MicroSD card
      delay(9000); //delay to avoid repeat measurement within same second. X000 milliseconds == X seconds.
  } //end of IF control structure. */
} //End repeating section of code. This will repeat until power is lost.
