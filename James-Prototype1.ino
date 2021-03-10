#include "bsec.h"               // Library for BME680 Sensor
#include <SPI.h>                // comunicating with LCD
#include <Wire.h>               // comunicating with LCD
#include <Adafruit_GFX.h>       // LCD Library
#include <Adafruit_PCD8544.h>   // LCD Library

#define LED1 27                 // Defining pin 27 as LED1      
#define LED2 14                 // Defining pin 14 as LED2
#define LED3 12                 // Defining pin 12 as LED3
const int Buzzer = 13;          // defining pin 13 as Buzzer
const int LCD_LED =18;          // defining pin 18 as the LED on the LCD


// Helper functions declarations
void checkIaqSensorStatus(void);

boolean backlight = true;       // setting the backlight boolean as true

int contrast=65;                // setting inital value of contrast
int MaxTemp=30;                 // setting inital value of Max Temp
int MaxHum=50;                  // setting inital value of Max Hum
int MaxIAQ=100;                 // setting inital value of Max IAQ
float MaxVOX=1.00;              // setting inital value of Max VOX            
int MaxC02=1000;                // setting inital value of Max C02                

int menuitem = 1;               // setting a menuitem as 1
int page = 1;                   // inital page = 1

volatile boolean up = false;     // setting the up button as a boolean and inital false
volatile boolean down = false;   // setting the down button as a boolean and inital false
volatile boolean middle = false; // setting the middle button as a boolean and inital false

int downButtonState = 0;          // setting the state of the down button as 0
int upButtonState = 0;            // setting the state of the up button as 0
int selectButtonState = 0;        // setting the state of the select button as 0       
int lastDownButtonState = 0;      // setting the state of the last down button as 0
int lastSelectButtonState = 0;    // setting the state of the last select button as 0
int lastUpButtonState = 0;        // setting the state of the last up button as 0

Adafruit_PCD8544 display = Adafruit_PCD8544(0, 15, 17, 4, 16);  // This is what the LCD is connected to and is named display

Bsec iaqSensor;

String output;  

void setup(void) 
{
  
  //Serial.begin(115200);
  while(!Serial) 
  delay(10);
  Wire.begin();

  // stuff for IAQ sensor

  iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
  output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus();


    // Sensor values
  bsec_virtual_sensor_t sensorList[10] = {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };

  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();

  // Print the header
  output = "Timestamp [ms], raw temperature [°C], pressure [hPa], raw relative humidity [%], gas [Ohm], IAQ, IAQ accuracy, temperature [°C], relative humidity [%], Static IAQ, CO2 equivalent, breath VOC equivalent";
  Serial.println(output);
  

  pinMode(25, INPUT_PULLUP);    // sets the switches as inputs with pull up resistors
  pinMode(33, INPUT_PULLUP);
  pinMode(32, INPUT_PULLUP);
  
  pinMode(Buzzer,OUTPUT);       // sets the Buzzers and LEds as outpus
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  
  pinMode(LCD_LED,OUTPUT);           // sets pin LED in LCD as output

  digitalWrite(LCD_LED,HIGH);         //Turn Backlight ON
  
  
  display.begin();                  // begins LCD
  display.setContrast(contrast);    // Set contrast to preset contrast
  display.setCursor(0,0);           // sets LCD position to 0,0
  display.clearDisplay();           // clears display
  display.display();                // displays onto LCD
  delay(1000);                      // wait 1 second
  display.println("");
  display.println("     Air");
  display.println("  Polluition");      // Displays title page
  display.println("  Monitoring");
  display.println("    System");
  
  display.display();                    // displays onto LCD
  delay(1000);                          // wait 1 second
  
  display.clearDisplay();               // clears display
  display.display();                    // displays onto LCD
}

void loop(void) 
{

    // displays all the available measurements for use later

  if (iaqSensor.run()) { // If new data is available
    //output = String(time_trigger);
    output += ", " + String(iaqSensor.rawTemperature);
    output += ", " + String(iaqSensor.pressure);
    output += ", " + String(iaqSensor.rawHumidity);
    output += ", " + String(iaqSensor.gasResistance);
    output += ", " + String(iaqSensor.iaq);
    output += ", " + String(iaqSensor.iaqAccuracy);
    output += ", " + String(iaqSensor.temperature);
    output += ", " + String(iaqSensor.humidity);
    output += ", " + String(iaqSensor.staticIaq);
    output += ", " + String(iaqSensor.co2Equivalent);
    output += ", " + String(iaqSensor.breathVocEquivalent);
    Serial.println(output);


  Serial.println();
  } else {
    checkIaqSensorStatus();
  }
  
  drawMenu();           // draws the menu, see later

  downButtonState = digitalRead(25);    // sets the down button as 25
  selectButtonState = digitalRead(33);  // sets the select button as 33
  upButtonState =   digitalRead(32);    // sets the up button as 32
 
  checkIfDownButtonIsPressed();         // starts void for checking if down button is pressed
  checkIfUpButtonIsPressed();           // starts void for checking if up button is pressed
  checkIfSelectButtonIsPressed();       // starts void for checking if select button is pressed


  if (up && page == 1 )        // if up button is pressed and page is 1
  {    
      up = false;
      menuitem--;               // Take 1 from the menuitem
    
      if (menuitem==0)          // if the menuitem is zero, make it one!!
      {
          menuitem=1;
      }      
  }
  
  if (up && page == 7 )        // if up button is pressed and page is 7
  {       
      up = false;
      menuitem--;                 // Take 1 from the menuitem
    
      if (menuitem<=5)          // if the menuitem is less than 5
      {
          display.clearDisplay();  // clear the display 
          page=1;                  // go to page 1
          menuitem=5;              // make mean term 5
      }   
  }
  
  if (up && page == 2 )           // if up button is pressed and page is 2
  {
      up = false;
      MaxTemp--;                  // decrease the max temp value by 1
  }
  if (up && page == 3 )           // if up button is pressed and page is 3
  {         
    up = false;
    MaxHum--;                     // decrease the max Hum value by 1
  }
  if (up && page == 4 )           // if up button is pressed and page is 4
  {         
    up = false;
    MaxIAQ--;                     // decrease the max IAQ value by 1
  }
  if (up && page == 5 )           // if up button is pressed and page is 5
  {
    up = false;
    MaxVOX=MaxVOX-0.1;            // decrease the max VOX value by 0.1
  }
  if (up && page == 6 )           // if up button is pressed and page is 6
  {
    up = false;
    MaxC02--;                     // decrease the max C02 value by 0.1
  }
  if (up && page == 8 )           // if up button is pressed and page is 8
  {
    up = false;
    contrast--;                    // decrease the contrast value by 1
    setContrast();                  // set the contrast  (see later)
  }


  if (down && page == 1)            // if down button is pressed and page is 1
  {           
    down = false;
    menuitem++;                     // add 1 from the menuitem  
  }
  if (down && page == 7)            // if down button is pressed and page is 7  
  {          
      down = false;
      menuitem++;                   // add 1 from the menuitem 
     
      if (menuitem==8)              // if menuitem is 8
      {
        menuitem=7;                 // make menuitem 7
      }  
  }
    
    if (down && page == 2 )         // if down button is pressed and page is 2
    {
        down = false;
        MaxTemp++;                  // increase the max temp value by 1
    }
  if (down && page == 3 )           // if down button is pressed and page is 3
  {
      down = false;
      MaxHum++;                     // increase the max Hum value by 1
  }
  if (down && page == 4 )           // if down button is pressed and page is 4
  {
      down = false;
      MaxIAQ++;                     // increase the max IAQ value by 1
  }
  if (down && page == 5 )            // if down button is pressed and page is 5
  {
      down = false;
      MaxVOX=MaxVOX+0.1;             // increase the max VOX value by 0.1
  }
  if (down && page == 6 )            // if down button is pressed and page is 6
  {
      down = false;
      MaxC02++;                      // increase the max C02 value by 1
  }
  if (down && page == 8 )            // if down button is pressed and page is 8
  {
      down = false;
      contrast++;                   // increase the contrast value by 1
      setContrast();                // set contrast (see later)
  }



  if (middle)                         // if midddle button is pressed
  {
        middle = false;
        display.clearDisplay();       // clear LCD
        
      if (page == 1 && menuitem==2)   // if page is 1 and menuitem is 2
      {
          page=3;                     // go to page 3
      }
      else if (page == 3)             // else if page is 3
      {
          page=1;                     // go to page 1
      }

      if(page == 1 && menuitem ==3)   // if page is 1 and menuitem is 3
      {
          page=4;                     // go to page 4
      }
      else if (page == 4)             // else if page is 4
      {
          page=1;                     // go to page 1
      }


      if (page == 1 && menuitem==1)    // if page is 1 and menuitem is 1
      {
          page=2;                       // go to page 2
      }
      else if (page == 2)               // else if page is 2
      {
          page=1;                       // go to page 1
      }
      
      if (page == 1 && menuitem==4)     // if page is 1 and menuitem is 4
      {
          page=5;                       // go to page 5
      }
      else if (page == 5)               // if page is 5
      {
          page=1;                       // got to page 1
      }
      
      if (page == 1 && menuitem==5)     // if page is 1 and menuitem is 5
      {
          page=6;                       // go to page 6
      }
      else if (page == 6)               // if page is 6
      {
          page=1;                       // go to page 1
      }
      
      if (page == 1 && menuitem==7)     // if page is 1 and menuitem is 7
      {
          page=7;                       // page is 7
      }
      
      if (page == 7 && menuitem==6)     // if page is 7 and menuitem is 6
      {
          page=8;                       // got to page 8
      }
      else if(page==8)                  // if page is 8
      {
          page=7;                       // go to page 7               
      }
      
      if (page == 7 && menuitem==7)     // if page is 7 and menuitem is 7
      {
          if (backlight)                // if backlight button is on
          {
            backlight = false;
            turnBacklightOff();         // turn LCD led off
          }
          else                          // else 
          {
              backlight = true; 
              turnBacklightOn();        // turn LCD led on
           }
      }
      
      if(page==9 || page==10 || page==11 || page==12 || page==13)   // if page is 9 or 10 or 11 or 12 or 13
      {
          page=1;                                 // go to page 1
          if(MaxTemp<iaqSensor.rawTemperature)    // if max temp is less than the actual temp
          {
              MaxTemp++;                          // add 1 to the max temp until it is above actual temp 
          }
              
          if(MaxHum<iaqSensor.humidity)           // if max hum is less than the actual hum
          {
              MaxHum++;                           // add 1 to the max hum until it is above actual hum 
          }
              
          if(MaxIAQ<iaqSensor.iaq)                // if max IAQ is less than the actual IAQ
          {
              MaxIAQ++;                           // add 1 to the max IAQ until it is above actual IAQ
          }
              
          if(MaxVOX<iaqSensor.breathVocEquivalent)  // if max VOX is less than the actual VOX
          {
              MaxVOX=MaxVOX+0.1;                    // add 0.1 to the max VOX until it is above actual VOX
          }
              
          if(MaxC02<iaqSensor.co2Equivalent)        // if max C02 is less than the actual C02
          {
              MaxC02++;                             // add 1 to the max C02 until it is above actual C02
          }
    
       }
  }
       
    if(iaqSensor.rawTemperature>MaxTemp)          // if the actual temp is larger than the max temp
    {
        page=9;                                   // go to page 9
    }
    else
    {
        digitalWrite(LED1, HIGH);                 // otherwise turn green led on
    }
    
    if(iaqSensor.humidity>MaxHum)                 // if the actual humidity is larger than the max humidity
    {
        page=10;                                  // go to page 10
    }
    else
    {
        digitalWrite(LED1, HIGH);                 // otherwise turn green led on
    }
    
    if(iaqSensor.iaq>MaxIAQ)                      // if the actual IAQ is larger than the max IAQ
    {
        page=11;                                  // go to page 11
    }
    else
    {
        digitalWrite(LED1, HIGH);                 // otherwise turn green led on
    }
    
    if(iaqSensor.breathVocEquivalent>MaxVOX+0.1)    // if the actual VOX is larger than the max VOX + 0.1
    {
        page=12;                                    // go to page 12
    }
    else
    {
        digitalWrite(LED1, HIGH);                   // otherwise turn green led on
    }
      
    if(iaqSensor.co2Equivalent>MaxC02)               // if the actual C02 is larger than the max C02
    {
        page=13;                                     // go to page 13
    }
    else
    {
        digitalWrite(LED1, HIGH);                    // otherwise turn green led on
    }
    
}
  
void checkIfDownButtonIsPressed()                     // checking if the down button is pressed
  {
    if (downButtonState != lastDownButtonState)       //if the down button is not equal to the last down button state
    {
      if (downButtonState == 0)                       //then if the down button state is 0 
      {
        down=true;                                    //make the button down true
      }
        //delay(50);
    } 
    lastDownButtonState = downButtonState;            //now make the last down button state the down button state
  }

void checkIfUpButtonIsPressed()                       // checking if the up button is pressed
{
  if (upButtonState != lastUpButtonState)             //if the up button is not equal to the last up button state
  {
    if (upButtonState == 0)                           // then if the up button state is 0 
    {
      up=true;                                        // make the button up true
    }
    //delay(50);
  }
   lastUpButtonState = upButtonState;                  //now make the last up button state the up button state
}


void checkIfSelectButtonIsPressed()                   // checking if the select button is pressed
{
   if (selectButtonState != lastSelectButtonState)     //if the select button is not equal to the last select button state
  {
    if (selectButtonState == 0)                        // then if the select button state is 0 
    {                     
      middle=true;                                     // make the button middle true
    } 
    //delay(50);
  }
   lastSelectButtonState = selectButtonState;           //now make the last select button state the select button state
}
  
  void drawMenu()                     // void for drawing the menu 
  {
    
  if (page==1)                        // when page is 1
  {    
    display.setTextSize(1);           // set size to 1
    display.setCursor(0, 0);          // set cursor to 1
   
    if (menuitem==1)                  // if menu item is 1
    { 
      display.setTextColor(WHITE, BLACK);       // change the colours from white to black, and black to white
    } 
    else 
    {
      display.setTextColor(BLACK, WHITE);       // otherwise leave them as they were
    }
    display.print(">Temp: ");                   // print temp
    display.print(String(iaqSensor.rawTemperature));  // print actual value of temp
    display.print("*C");                        // units, degrees
    display.display();                          // put on display
    display.setCursor(0, 10);                   // set cursor to 0,10
   
    if (menuitem==2)                          // if menu item is 2
    {
      display.setTextColor(WHITE, BLACK);     // change the colours from white to black, and black to white
    }
    else 
    {
      display.setTextColor(BLACK, WHITE);     // otherwise leave them as they were
    }    
    display.print(">Hum: ");                      // print Humidity
    display.print(String(iaqSensor.humidity));     // print actual value of hum
    display.print("%");                           // units, %
    display.print("  ");
    display.display();                            // put on display  
    display.setCursor(0, 20);                     // set cursor to 0,20
    
    if (menuitem==3)                              // if menu item is 3
    {   
      display.setTextColor(WHITE, BLACK);         // change the colours from white to black, and black to white
    }
    else 
    {
      display.setTextColor(BLACK, WHITE);         // otherwise leave them as they were
    }  
    display.print(">IAQ: ");                      // print IAQ
    display.print(String(iaqSensor.iaq));         // print actual value of  IAQ
    display.print("ppm");                         // units, ppm
    display.display();                            // put on display 
    display.setCursor(0, 30);                     // set cursor to 0,30
    
    if (menuitem==4)                               // if menu item is 4
    { 
      display.setTextColor(WHITE, BLACK);           // change the colours from white to black, and black to white
    }
    else 
    {
      display.setTextColor(BLACK, WHITE);           // otherwise leave them as they were
    }
    
    display.print(">VOX: ");                              // print VOX
    display.print(String(iaqSensor.breathVocEquivalent)); // print actual value of VOX
    display.print("ppb");                                  // units ppb
    display.print(" ");
    display.display();                                     // put on display 
    display.setCursor(0, 40);                              // set cursor to 0,40
    
    if (menuitem==5)                                // if menu item is 5
    { 
      display.setTextColor(WHITE, BLACK);           // change the colours from white to black, and black to white
    }
    else 
    {
      display.setTextColor(BLACK, WHITE);           // otherwise leave them as they were
    }
      
    display.print(">CO2: ");                        // print C02
    display.print(int(iaqSensor.co2Equivalent));    // print actual value of C02
    display.print("ppm ");                          // units ppm
    display.write(25);                              // print down arrow
    display.display();                              // put on display

    if (menuitem==6)                      // if menu item is 6
    { 
      page=7;                             // set page to 7
      display.clearDisplay();             // clear display
    }
    else 
    {
      page=1;                             /// otherwise page is 1
    }
  }
    
 
  if (page==2)                // page to show more detail about temperatue and to set the max value
  {
    
    display.setTextSize(1);                             // set size to 1
    display.clearDisplay();                             // clear LCD
    display.setTextColor(BLACK, WHITE);                 // set colour to normal 
    display.setCursor(6 , 0);                           // set cursor to 6,0
    display.print(" Temperature");                      // display Temperature
    display.drawFastHLine(0,10,83,BLACK);               // draw a line under Temperature
    display.setCursor(5, 15);                           // set cursor to 5,15
    display.print(String(iaqSensor.rawTemperature));    // display actual Temperature
    display.print("*C");                                // units degrees
    display.setTextSize(1.5);                           // set size to 1.5
    display.setCursor(5, 25);                           // set cursor to 5,25
    display.print("Max Value: ");                       // print max value
    display.setTextSize(1.8);                           // set size to 1.8
    display.setCursor(5, 35);                           // set cursor to 5,35
    display.print(MaxTemp);                             // display max temp int
    display.print(" *C");                               // units degress
 
    display.setTextSize(2);                             // set text size to 2
    display.display();                                  // put on display
  }

  if (page==3)                // page to show more detail about Humidity and to set the max value
  {
    
    display.setTextSize(1);                      // set size to 1
    display.clearDisplay();                      // clear LCD
    display.setTextColor(BLACK, WHITE);          // set colour to normal 
    display.setCursor(15, 0);                    // set cursor to 15,0
    display.print(" Humidity");                  // display Humidity
    display.drawFastHLine(0,10,83,BLACK);        // draw a line under Humidity
    display.setCursor(5, 15);                    // set cursor to 5,15
    display.print(String(iaqSensor.humidity));   // print actual humidity
    display.print(" %");                          // units %
    display.setTextSize(1.5);                     // set size to 1.5
    display.setCursor(5, 25);                     // set cursor to 5,25
    display.print("Max Value: ");                 // print max value
    display.setTextSize(1.8);                     // set size to 1.8
    display.setCursor(5, 35);                     // set cursor to 5,35
    display.print(MaxHum);                        // display max hum int
    display.print(" %");                          // units %
 
    display.setTextSize(2);                       // set text size to 2
    display.display();                            // put on display
  }
  
  if (page==4)                // page to show more detail about IAQ and to set the max value
  {
      
    display.setTextSize(1);                     // set size to 1
    display.clearDisplay();                     // clear LCD
    display.setTextColor(BLACK, WHITE);         // set colour to normal 
    display.setCursor(15, 0);                   // set cursor to 15,0
    display.print("   IAQ");                    // display IAQ
    display.drawFastHLine(0,10,83,BLACK);        // draw a line under IAQ
    display.setCursor(5, 15);                   // set cursor to 5,15
    display.print(String(iaqSensor.iaq));       // print actual IAQ
    display.print(" ppm");                      // units ppm
    display.setTextSize(1.5);                   // set size to 1.5
    display.setCursor(5, 25);                   // set cursor to 5,25
    display.print("Max Value: ");               // print max value
    display.setTextSize(1.8);                   // set size to 1.8
    display.setCursor(5, 35);                   // set cursor to 5,35
    display.print(MaxIAQ);                      // display max IAQ int
    display.print(" ppm");                      // units ppm
 
    display.setTextSize(2);                     // set text size to 2
    display.display();                          // put on display
  }
  
  if (page==5)              // page to show more detail about VOX and to set the max value
  {
    
    display.setTextSize(1);                                 // set size to 1
    display.clearDisplay();                                 // clear LCD
    display.setTextColor(BLACK, WHITE);                     // set colour to normal 
    display.setCursor(15, 0);                               // set cursor to 15,0
    display.print("   VOX");                                // display VOX
    display.drawFastHLine(0,10,83,BLACK);                   // draw a line under VOX
    display.setCursor(5, 15);                               // set cursor to 5,15
    display.print(String(iaqSensor.breathVocEquivalent));   // print actual VOX
    display.print(" ppb");                                  // units ppb
    display.setTextSize(1.5);                               // set size to 1.5
    display.setCursor(5, 25);                               // set cursor to 5,25
    display.print("Max Value: ");                           // print max value
    display.setTextSize(1.8);                               // set size to 1.8
    display.setCursor(5, 35);                               // set cursor to 5,35
    display.print(MaxVOX);                                  // display max VOX int
    display.print(" ppb");                                  // units ppb
 
    display.setTextSize(2);                                 // set text size to 2
    display.display();                                      // put on display
  }
  if (page==6)              // page to show more detail about C02 and to set the max value
  {
    
    display.setTextSize(1);                             // set size to 1
    display.clearDisplay();                             // clear LCD
    display.setTextColor(BLACK, WHITE);                 // set colour to normal 
    display.setCursor(15, 0);                           // set cursor to 15,0
    display.print("   C02");                            // display C02
    display.drawFastHLine(0,10,83,BLACK);               // draw a line under C02
    display.setCursor(5, 15);                           // set cursor to 5,15
    display.print(String(iaqSensor.co2Equivalent));     // print actual C02
    display.print(" ppm");                              // units ppm
    display.setTextSize(1.5);                           // set size to 1.5
    display.setCursor(5, 25);                           // set cursor to 5,25
    display.print("Max Value: ");                       // print max value
    display.setTextSize(1.8);                           // set size to 1.8
    display.setCursor(5, 35);                           // set cursor to 5,35
    display.print(MaxC02);                              // display max C02 int
    display.print(" ppm");                              // units ppm
 
    display.setTextSize(2);                             // set text size to 2
    display.display();                                  // put on display
  }
  if(page==7)                                   // Page 7 is the secind main menu display
  {
    display.setTextSize(1);                     // set size to 1
    display.setCursor(0, 0);                    // set cursor to 0,0
    
    if (menuitem==6)                            // if menu item is 6
    { 
      display.setTextColor(WHITE, BLACK);       // change the colours from white to black, and black to white
    }
    else 
    {
      display.setTextColor(BLACK, WHITE);       // otherwise leave them as they were
    }
    display.print(">Contrast    ");             // display contrast
    display.write(24);                          // display up arrow
    display.display();                          // put on display
    display.setCursor(0, 10);                   // set cursor to 0,10
   
    if (menuitem==7)                            // if menu item is 7
    {
      display.setTextColor(WHITE, BLACK);       // change the colours from white to black, and black to white
    }
    else 
    { 
      display.setTextColor(BLACK, WHITE);       // otherwise leave them as they were
    }    
    display.print(">light: ");                  // display light
    
     if (backlight)                             // backlight is on
    {
      
      display.print("ON");                      // display on on lcd
      turnBacklightOn();                        // go to turn on LCD LED
    }
    else 
    {
      display.print("OFF");                     // otherwise display off on LCD
      turnBacklightOff();                       // go to turn off LCD LED
    }
    display.display();                          // put on display
  }

  if (page==8)                      // page to show more detail about contrast and to change the contrast
  {
    
    display.setTextSize(1);                   // set size to 1
    display.clearDisplay();                   // clear display
    display.setTextColor(BLACK, WHITE);       // set colour to normal 
    display.setCursor(15, 0);                 // set cursor to 15,0
    display.print("CONTRAST");                // display CONTRAST
    display.drawFastHLine(0,10,83,BLACK);     // draw line under contrast
    display.setCursor(5, 15);                 // set cursor to 5,15
    display.print("Value");                   // display value
    display.setTextSize(2);                   // set size to 2
    display.setCursor(5, 25);                 // set cursor to 5,25
    display.print(contrast);                  // print the vlaue of variable contrast
 
    display.setTextSize(2);                   // set size to 2
    display.display();                        // put on the LCD
    
  }
  if (page==9)        // screen when Temperature is greater than set value
  {
    BuzzerAndLEDs();    // turn on buzzers and LEDS
    
    display.setTextSize(1);                           // set size to 1
    display.clearDisplay();                           // clear display
    display.setTextColor(BLACK, WHITE);               // set colour to normal
    display.setCursor(0, 0);                          // set cursor to 0,0
    display.print("  Temperature is ");               // display Temperature is
    display.print(String(iaqSensor.rawTemperature));  // display actual temp
    display.print("*C");                              // units degrees
    display.print(" andis above ");                   // display and is above
    display.print(MaxTemp);                           // display Max temp value
    display.print("*C");                              // units degrees
  
    display.setCursor(0, 30);                         // set cursor to 0,30
    display.println(" Press Select");                 // display press select button
    display.println("    Button");
    display.display();                                // put on the LCD
    
  }
  if (page==10)       // screen when Humidity is greater than set value
  {
    BuzzerAndLEDs();            // turn on buzzers and LEDS
    
     display.setTextSize(1);                      // set size to 1
    display.clearDisplay();                       // clear display
    display.setTextColor(BLACK, WHITE);           // set colour to normal
    display.setCursor(0, 0);                      // set cursor to 0,0
    display.print(" Humidity is  ");              // display Humidity is
    display.print(String(iaqSensor.humidity));    // display actual hum
    display.print("%");                           // units %
    display.print(" and is   above ");            // display and is above
    display.print(MaxHum);                        // display Max hum value
    display.print("%");                           // units %
  
    display.setCursor(0, 30);                     // set cursor to 0,30
    display.println(" Press Select");             // display press select button
    display.println("    Button");
    display.display();                            // put on the LCD
    
  }
  if (page==11)       // screen when IAQ is greater than set value
  {
    
    BuzzerAndLEDs();          // turn on buzzers and LEDS
    
     display.setTextSize(1);                 // set size to 1
    display.clearDisplay();                  // clear display
    display.setTextColor(BLACK, WHITE);      // set colour to normal
    display.setCursor(25, 0);                // set cursor to 25,0
    display.print("IAQ is ");                // display IAQ is
    display.setCursor(7, 10);                // set cursor to 7,10
    display.print(String(iaqSensor.iaq));    // display actual IAQ
    display.print("ppm");                    // units ppm
    display.print(" andis above ");          // display and is above
    display.print(MaxIAQ);                   // display Max IAQ value
    display.print("ppm");                    // units ppm
  
    display.setCursor(0, 30);                // set cursor to 0,30
    display.println(" Press Select");        // display press select button
    display.println("    Button");
    display.display();                       // put on the LCD
    
  }
  if (page==12)    // screen when VOX is greater than set value
  {
    BuzzerAndLEDs();           // turn on buzzers and LEDS
    
     display.setTextSize(1);                                  // set size to 1
    display.clearDisplay();                                   // clear display
    display.setTextColor(BLACK, WHITE);                       // set colour to normal
    display.setCursor(25, 0);                                 // set cursor to 25,0
    display.print("VOX is");                                  // display VOX is
    display.setCursor(0, 10);                                 // set cursor to 0,10
    display.print(String(iaqSensor.breathVocEquivalent));     // display actual VOX
    display.print("ppb");                                     // units ppb
    display.print(" and is");                                 // display and is 
    display.setCursor(0, 20);                                 // set cursor to 0,20
    display.print("above  ");                                 // display above
    display.print(MaxVOX);                                    // display Max VOX value
    display.print("ppb");                                     // units ppb
  
    display.setCursor(0, 30);                                 // set cursor to 0,30
    display.println(" Press Select");                         // display press select button
    display.println("    Button");
    display.display();                                        // put on the LCD
    
  }
  if (page==13)    // screen when C02 is greater than set value
  {
    BuzzerAndLEDs();         // turn on buzzers and LEDS
    
     display.setTextSize(1);                          // set size to 1
    display.clearDisplay();                           // clear display
    display.setTextColor(BLACK, WHITE);               // set colour to normal
    display.setCursor(25, 0);                         // set cursor to 25,0
    display.print("C02 is");                          // display C02 is
    display.setCursor(0, 10);                         // set cursor to 0,10
    display.print(String(iaqSensor.co2Equivalent));   // display actual C02
    display.print("ppm");                             // units ppm
    display.print(" and is");                         // display and is 
    display.setCursor(0, 20);                         // set cursor to 0,20
    display.print("above  ");                         // display above
    display.print(MaxC02);                            // display Max C02 value
    display.print("ppm");                             // units ppm
  
    display.setCursor(0, 30);                         // set cursor to 0,30
    display.println(" Press Select");                 // display press select button
    display.println("    Button");
    display.display();                                // put on the LCD
    
  }
}

  void setContrast()                      // setting contrast
  {
    display.setContrast(contrast);        // set contrast
    display.display();                    // update display
  }

  void turnBacklightOn()                  // turning LCD LED on
  {
    digitalWrite(LCD_LED,HIGH);           // LCD_LED is high
  }

    void turnBacklightOff()                // turning LCD LED off
  {
    digitalWrite(LCD_LED,LOW);             // LCD_LED is low
  }

// This is code that is required in order for the BME680 sensor to work and comes with the library
void checkIaqSensorStatus(void)             // checking the sensor values
{
  if (iaqSensor.status != BSEC_OK)          // if The sensor status is not BSEC_OK
  {        
    if (iaqSensor.status < BSEC_OK)         // if The sensor status is < BSEC_OK
    {       
      output = "BSEC error code : " + String(iaqSensor.status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } 
    else 
    {
      output = "BSEC warning code : " + String(iaqSensor.status);
      Serial.println(output);
    }
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      output = "BME680 error code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BME680 warning code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
    }
  }
  
}
// For the BME680 SENSOR
void errLeds(void)
{
  
}

void BuzzerAndLEDs()          // turing buzzer and REd led on to flash
{
    digitalWrite(LED1, LOW);    // Green led low
    digitalWrite(Buzzer, HIGH); // buzzer is on
    digitalWrite(LED3, HIGH);   // red led is on
    delay(100);                 // wait 100ms
    digitalWrite(Buzzer, LOW);  // buzzer is off
    digitalWrite(LED3, LOW);    // led3 is off
    delay(100);                 // wait 100ms
}
