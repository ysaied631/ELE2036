#include "bsec.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <stdlib.h>

// Youcef 
#include <WiFi.h>
#include <WebServer.h>


// Following config will create a wifi connection for the user, once connected they visit webpage at 192.168.0.1 and table will show

// WIFI connection details
const char* ssid = "ELE2036 Weather Station";  // Enter SSID here
const char* password = "ELE2036Team3";  //Enter Password here

// IP for webpage
IPAddress local_ip(192,168,0,1);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);

// Webserver Port
WebServer server(80);

// Arrays to hold data from sensors
float temperature[12], humidity[12], pressure[12], IAQ[12], CO2[12], VOC[12], SIAQ[12];

// Variables to keep track of data every 2 hours
unsigned long index = 0;
unsigned long nextTime = 0; 
bool first = true;

// End

#define LED_BUILTIN 2
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 
 

// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);

// Create an object of the class Bsec
Bsec iaqSensor;

String output;

// Entry point for the example
void setup(void)
{
  u8g2.begin();
  Serial.begin(115200);
  while(!Serial)
  delay (10);
  Wire.begin();

  iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
  output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus();

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

  // Youcef
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  // Function to call when just connected
  server.on("/", handle_OnConnect);
  // Function to call when an unrecognised url is type in
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
  // End
}

// Function that is looped forever
void loop(void)
{
  unsigned long time_trigger = millis();
  if (iaqSensor.run()) { // If new data is available
    output = String(time_trigger);
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
  } else {
    checkIaqSensorStatus();
  }

   u8g2.clearBuffer();          // clear the internal memory
   u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
   u8g2.drawStr(8,29,"IAQ:");  // write something to the internal memory
   u8g2.sendBuffer();         // transfer internal memory to the display
   delay(500);
   
   u8g2.clearBuffer(); 
   char buffer[4];
   sprintf( buffer, "%02.02f",iaqSensor.iaq) ;
                                      
   u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
   u8g2.drawStr(8,29,buffer);  // write something to the internal memory
   u8g2.sendBuffer();         // transfer internal memory to the display
   delay(3000);
   
   u8g2.clearBuffer();          // clear the internal memory
   u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
   u8g2.drawStr(8,29,"CO2:");  // write something to the internal memory
   u8g2.sendBuffer();         // transfer internal memory to the display
   delay(500);

   u8g2.clearBuffer(); 
   
   sprintf( buffer, "%02.02f", iaqSensor.co2Equivalent) ;
                                      
   u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
   u8g2.drawStr(8,29,buffer);  // write something to the internal memory
   u8g2.sendBuffer();         // transfer internal memory to the display
   delay(3000);

   u8g2.clearBuffer();          // clear the internal memory
   u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
   u8g2.drawStr(8,29,"Voc:");  // write something to the internal memory
   u8g2.sendBuffer();         // transfer internal memory to the display
   delay(500);
   
   u8g2.clearBuffer(); 
   
   sprintf( buffer, "%02.02f",iaqSensor.breathVocEquivalent) ;
                                      
   u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
   u8g2.drawStr(8,29,buffer);  // write something to the internal memory
   u8g2.sendBuffer();         // transfer internal memory to the display
   delay(3000);

   
   u8g2.clearBuffer();          // clear the internal memory
   u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
   u8g2.drawStr(8,29,"SIAQ:");  // write something to the internal memory
   u8g2.sendBuffer();         // transfer internal memory to the display
   delay(500);
   
   
   u8g2.clearBuffer(); 
   
   sprintf( buffer, "%02.02f",iaqSensor.staticIaq) ;
                                      
   u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
   u8g2.drawStr(8,29,buffer);  // write something to the internal memory
   u8g2.sendBuffer();         // transfer internal memory to the display
   delay(3000);

   // Youcef
   server.handleClient();

  if (millis() > nextTime)
  {
    // Every 2 hours
    nextTime += 7200000;

    // Set values from sensor (replace 0 with functions to get current values) ****************************************************************************  IMPORTANT
    temperature[index] = iaqSensor.temperature;
    humidity[index] = iaqSensor.humidity;
    pressure[index] = iaqSensor.pressure;
    IAQ[index] = iaqSensor.iaq;
    CO2[index] = iaqSensor.co2Equivalent;
    VOC[index] = iaqSensor.breathVocEquivalent;
    SIAQ[index] = iaqSensor.staticIaq;

    // Makes sure we re-use indexs of array as only holds 12 readings, prevents out of bounds error
    if (index == 11)
    {
      index = 0;
      first = false;
    }
    else 
    {
      index++;
    }
  }
  // End

  
}


// Helper function definitions
void checkIaqSensorStatus(void)
{
  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      output = "BSEC error code : " + String(iaqSensor.status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
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

void errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

// Youcef
// Display page when client connects
void handle_OnConnect() {
  Serial.println("Client connected, sending info.");
  // Pass data arrays to html
  server.send(200, "text/html", SendHTML(index, first, temperature, humidity, pressure, IAQ, CO2, VOC, SIAQ)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

// HTML for webpage
String SendHTML(int ind, bool fr, float t[12],float h[12], float p[12], float i[12], float c[12], float v[12], float si[12]){
  float newT[12], newH[12], newP[12], newI[12], newC[12], newV[12], newSI[12];
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ELE2036 Weather Station</title>\n";

  // Stylesheet
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  
  ptr +="</head>\n";
  ptr +="<body>\n";

  // Title
  ptr +="<h1>ELE2036 Weather Station</h1>\n";

  // populate table newest at most right
  if (fr)
  {
    for ( int i = 0; i < 12; i++)
    {
      newT[i] = t[ind - i];
      newH[i] = h[ind - i];
      newP[i] = p[ind - 1];
      newI[i] = i[ind - 1];
      newC[i] = c[ind - 1];
      newV[i] = v[ind - 1];
      newSI[i] = si[ind - 1];
    }
    
  }
  else
  {
    for ( int i = 0; i < 12; i++)
    {
      if (i <= ind)
      {
        newT[i] = t[ind - i];
        newH[i] = h[ind - i];
        newP[i] = p[ind - 1];
        newI[i] = i[ind - 1];
        newC[i] = c[ind - 1];
        newV[i] = v[ind - 1];
        newSI[i] = si[ind - 1];
      }
      else
      {
        newT[i] = t[11 - i];
        newH[i] = h[11 - i];
        newP[i] = p[11 - 1];
        newI[i] = i[11 - 1];
        newC[i] = c[11 - 1];
        newV[i] = v[11 - 1];
        newSI[i] = si[11 - 1];
        break;
      }
    }
  }
  

  // Table
  ptr +="<table style="width:100%">\n";
  ptr +="<tr>\n";
  ptr +="<th></th>\n";
  ptr +="<th>24:00</th>\n";
  ptr +="<th>22:00</th>\n";
  ptr +="<th>20:00</th>\n";
  ptr +="</tr>\n";
  ptr +="<tr>\n";
  
  ptr +="<th>Temperature</th>\n";
  ptr +="<td>" + newT[0] + "</td>\n";
  ptr +="<td>" + newT[1] + "</td>\n";
  ptr +="<td>" + newT[2] + "</td>\n";
  ptr +="<td>" + newT[3] + "</td>\n";
  ptr +="<td>" + newT[4] + "</td>\n";
  ptr +="<td>" + newT[5] + "</td>\n";
  ptr +="<td>" + newT[6] + "</td>\n";
  ptr +="<td>" + newT[7] + "</td>\n";
  ptr +="<td>" + newT[8] + "</td>\n";
  ptr +="<td>" + newT[9] + "</td>\n";
  ptr +="<td>" + newT[10] + "</td>\n";
  ptr +="<td>" + newT[11] + "</td>\n";
  ptr +="<td>" + newT[12] + "</td>\n";
  ptr +="</tr>\n";

  ptr +="<tr>\n";
  ptr +="<th>Humidity</th>\n";
  ptr +="<td>" + newH[0] + "</td>\n";
  ptr +="<td>" + newH[1] + "</td>\n";
  ptr +="<td>" + newH[2] + "</td>\n";
  ptr +="<td>" + newH[3] + "</td>\n";
  ptr +="<td>" + newH[4] + "</td>\n";
  ptr +="<td>" + newH[5] + "</td>\n";
  ptr +="<td>" + newH[6] + "</td>\n";
  ptr +="<td>" + newH[7] + "</td>\n";
  ptr +="<td>" + newH[8] + "</td>\n";
  ptr +="<td>" + newH[9] + "</td>\n";
  ptr +="<td>" + newH[10] + "</td>\n";
  ptr +="<td>" + newH[11] + "</td>\n";
  ptr +="<td>" + newH[12] + "</td>\n";
  ptr +="</tr>\n";

  
  ptr +="<tr>\n";
  ptr +="<th>Pressure</th>\n";
  ptr +="<td>" + newP[0] + "</td>\n";
  ptr +="<td>" + newP[1] + "</td>\n";
  ptr +="<td>" + newP[2] + "</td>\n";
  ptr +="<td>" + newP[3] + "</td>\n";
  ptr +="<td>" + newP[4] + "</td>\n";
  ptr +="<td>" + newP[5] + "</td>\n";
  ptr +="<td>" + newP[6] + "</td>\n";
  ptr +="<td>" + newP[7] + "</td>\n";
  ptr +="<td>" + newP[8] + "</td>\n";
  ptr +="<td>" + newP[9] + "</td>\n";
  ptr +="<td>" + newP[10] + "</td>\n";
  ptr +="<td>" + newP[11] + "</td>\n";
  ptr +="<td>" + newP[12] + "</td>\n";
  ptr +="</tr>\n";


  ptr +="<th>IAQ</th>\n";
  ptr +="<td>" + newI[0] + "</td>\n";
  ptr +="<td>" + newI[1] + "</td>\n";
  ptr +="<td>" + newI[2] + "</td>\n";
  ptr +="<td>" + newI[3] + "</td>\n";
  ptr +="<td>" + newI[4] + "</td>\n";
  ptr +="<td>" + newI[5] + "</td>\n";
  ptr +="<td>" + newI[6] + "</td>\n";
  ptr +="<td>" + newI[7] + "</td>\n";
  ptr +="<td>" + newI[8] + "</td>\n";
  ptr +="<td>" + newI[9] + "</td>\n";
  ptr +="<td>" + newI[10] + "</td>\n";
  ptr +="<td>" + newI[11] + "</td>\n";
  ptr +="<td>" + newI[12] + "</td>\n";
  ptr +="</tr>\n";


  ptr +="<th>CO2</th>\n";
  ptr +="<td>" + newC[0] + "</td>\n";
  ptr +="<td>" + newC[1] + "</td>\n";
  ptr +="<td>" + newC[2] + "</td>\n";
  ptr +="<td>" + newC[3] + "</td>\n";
  ptr +="<td>" + newC[4] + "</td>\n";
  ptr +="<td>" + newC[5] + "</td>\n";
  ptr +="<td>" + newC[6] + "</td>\n";
  ptr +="<td>" + newC[7] + "</td>\n";
  ptr +="<td>" + newC[8] + "</td>\n";
  ptr +="<td>" + newC[9] + "</td>\n";
  ptr +="<td>" + newC[10] + "</td>\n";
  ptr +="<td>" + newC[11] + "</td>\n";
  ptr +="<td>" + newC[12] + "</td>\n";
  ptr +="</tr>\n";


  ptr +="<th>VOX</th>\n";
  ptr +="<td>" + newV[0] + "</td>\n";
  ptr +="<td>" + newV[1] + "</td>\n";
  ptr +="<td>" + newV[2] + "</td>\n";
  ptr +="<td>" + newV[3] + "</td>\n";
  ptr +="<td>" + newV[4] + "</td>\n";
  ptr +="<td>" + newV[5] + "</td>\n";
  ptr +="<td>" + newV[6] + "</td>\n";
  ptr +="<td>" + newV[7] + "</td>\n";
  ptr +="<td>" + newV[8] + "</td>\n";
  ptr +="<td>" + newV[9] + "</td>\n";
  ptr +="<td>" + newV[10] + "</td>\n";
  ptr +="<td>" + newV[11] + "</td>\n";
  ptr +="<td>" + newV[12] + "</td>\n";
  ptr +="</tr>\n";


  ptr +="<th>Static IAQ</th>\n";
  ptr +="<td>" + newSI[0] + "</td>\n";
  ptr +="<td>" + newSI[1] + "</td>\n";
  ptr +="<td>" + newSI[2] + "</td>\n";
  ptr +="<td>" + newSI[3] + "</td>\n";
  ptr +="<td>" + newSI[4] + "</td>\n";
  ptr +="<td>" + newSI[5] + "</td>\n";
  ptr +="<td>" + newSI[6] + "</td>\n";
  ptr +="<td>" + newSI[7] + "</td>\n";
  ptr +="<td>" + newSI[8] + "</td>\n";
  ptr +="<td>" + newSI[9] + "</td>\n";
  ptr +="<td>" + newSI[10] + "</td>\n";
  ptr +="<td>" + newSI[11] + "</td>\n";
  ptr +="<td>" + newSI[12] + "</td>\n";
  ptr +="</tr>\n";
  
  ptr +="</table>\n";
  

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

//End
