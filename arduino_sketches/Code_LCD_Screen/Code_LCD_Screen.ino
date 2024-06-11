

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
Some of the code for the LCD screen was taken from code on Arduino.cc
  https://learn.adafruit.com/usb-plus-serial-backpack 
Some of the code for the K30 was created using code available in the library examples.

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

LCD with Serial Backpack:   5V - SCA 1 (on PCB)
                            GND - SCA 2 (on PCB)
                            Tx - SDA 4 (on PCB)
                            See supplementary material for detail.      

Pushbutton: Most push buttons are reversible in direction. Refer to https://www.arduino.cc/en/Tutorial/StateChangeDetection or paper supplementary for further instructions.               
                           
*/

#include "Arduino.h" //library
#include <SPI.h> //SD card library
#include <SD.h> //SD card library
#include <Wire.h> //library for connecting microprocessors together
#include <Adafruit_MPL3115A2.h> //MPL3115A2 library
#include <KSeries.h> //K30 sensor library
#include "RTClib.h" //DS3231 RTC library. You must use the example sketch DS3231 to set the tme before deployment.


Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2(); //define this board in the code as 'baro'.
int pinCS = 10; // Pin 10 on Arduino Uno is set as the ChipSelect pin.
//kSeries K_30(6,7); // Create K30 instance on pin 6 & 7.
File myFile; //Define the file to write to. 
RTC_DS3231 rtc; //define the clock board as 'rtc'.
const int buttonPin = 8; //the number of the pushbutton pin

byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Command packet to read Co2.
byte response[] = {0,0,0,0,0,0,0};  //create an array to store the response

//multiplier for value. default is 1. set to 3 for K-30 3% and 10 for K-33 ICB
int valMultiplier = 1;

int buttonPushCounter = 0;   //counter for the number of button presses
int buttonState = 0;         //variable for the current state of the pushbutton. This will count up with each button push.
int lastButtonState = 0;     //previous state of the button

SoftwareSerial K30 = SoftwareSerial(6,7);
SoftwareSerial lcd = SoftwareSerial(0,2);

void setup() { //Begin setup section of code.
 
 // Open serial communications and wait for port to open:
 Serial.begin(9600);  // baud rate 9600.

  pinMode(pinCS, OUTPUT);
  
  //Board Initialisations
  SD.begin();
  baro.begin();
  lcd.begin(9600);
  K30.begin(9600);

  myFile = SD.open("datalog.txt", FILE_WRITE); // Open up the file we're going to log to.

  // set the contrast, 200 is a good place to start, adjust as desired
  lcd.write(0xFE);
  lcd.write(0x50);
  lcd.write(200);
  delay(10);       
  
  // set the brightness - we'll max it (255 is max brightness)
  lcd.write(0xFE);
  lcd.write(0x99);
  lcd.write(255);
  delay(10);       
  
  // turn off cursors
  lcd.write(0xFE);
  lcd.write(0x4B);
  lcd.write(0xFE);
  lcd.write(0x54);

  // clear screen
  lcd.write(0xFE);
  lcd.write(0x58);
  delay(10);

  // go 'home'
  lcd.write(0xFE);
  lcd.write(0x48);
  lcd.write(0xFE);
  lcd.write(0x52);
  delay(10);
  lcd.print("Hello World");
  lcd.end();
  delay(100);

  // clear screen
  lcd.write(0xFE);
  lcd.write(0x58);
  delay(10);

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

  K30.begin(9600);
  sendRequest(readCO2);
  unsigned long valCO2 = getValue(response);
  Serial.print("Co2 ppm = ");
  Serial.println(valCO2);
  delay(2000);

  // clear screen
  lcd.write(0xFE);
  lcd.write(0x58);
  delay(10);

  lcd.begin(9600);
  delay(100);
  lcd.listen();
  delay(100);
  lcd.print(valCO2);
  lcd.print(" ppm");
  lcd.print(" ");
  lcd.print(now.hour(),DEC);
  lcd.print(":");
  lcd.print(now.minute(),DEC);
  lcd.print(":");
  lcd.print(now.second(),DEC);
  lcd.print(" "); 
  lcd.print(baro.getTemperature()); 
  lcd.print(" *C");
  lcd.end();
  delay(10000);

//Show on screen the logged data. Useful to check components are functioning correctly. Remove /* and */ on lines 157 and 182 to execute.
/* 
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
  Serial.println(valCO2);

// Show on screen the MPL3115A2 data. Useful to track what is happening with the barometer.
  Serial.print(baro.getPressure()); Serial.println(" Pascals"); //print on screen the pressure in kilo Pascals.
  Serial.print(baro.getAltitude()); Serial.println(" meters"); //print on screen the altitude in metres.
  Serial.print(baro.getTemperature()); Serial.println("*C"); //print on screen the temperature in degrees Celsius.
*/

// Use an IF control structure to define when measurements are taken.
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
      myFile.print(valCO2); //now logging CO2 in ppm from K30.
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
  } //end of IF control structure.
  
}

void sendRequest(byte packet[])
{
  while(!K30.available())  //keep sending request until we start to get a response
  {
    K30.write(readCO2,7);
    delay(50);
  }
  
  int timeout=0;  //set a timeoute counter
  while(K30.available() < 7 ) //Wait to get a 7 byte response
  {
    timeout++;  
    if(timeout > 10)    //if it takes to long there was probably an error
      {
        while(K30.available())  //flush whatever we have
          K30.read();
          
          break;                        //exit and try again
      }
      delay(50);
  }
  
  for (int i=0; i < 7; i++)
  {
    response[i] = K30.read();
  }  
}

unsigned long getValue(byte packet[])
{
    int high = packet[3];                        //high byte for value is 4th byte in packet in the packet
    int low = packet[4];                         //low byte for value is 5th byte in the packet

  
    unsigned long val = high*256 + low;                //Combine high byte and low byte with this formula to get value
    return val* valMultiplier;

  delay(1000);
  
 
} //End repeating section of code. This will repeat until power is lost.
