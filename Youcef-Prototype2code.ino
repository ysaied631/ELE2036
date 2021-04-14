#include "bsec.h"
#include <U8g2lib.h>
#include <WiFi.h>
#include <WebServer.h>

#define LED_BUILTIN 2
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 

// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);

// Create an object of the class Bsec
Bsec iaqSensor;

String output;

// SSID & Password
const char* ssid = "ELE2036 Weather Station";  // Enter SSID here
const char* password = "ELE2036Team3";  //Enter Password here

// Webserver Port
WebServer server(80);

// Arrays to hold data from sensors
float temp[12], hum[12], pres[12], IAQ[12], CO2[12], VOC[12], SIAQ[12];

// Arrays to hold website data
float newT[12], newH[12], newP[12], newI[12], newC[12], newV[12], newSI[12];

// Variables to keep track of data every 2 hours
unsigned long ind = 0;
unsigned long nextTime = 0; 

// Entry point for the example
void setup(void)
{
  Serial.begin(115200);
  u8g2.begin();
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

  // Create SoftAP
  WiFi.softAP(ssid, password);

  server.on("/", handle_root);

  server.begin();
  Serial.println("HTTP server started");
  delay(100);

  // Print the header
  output = "Timestamp [ms], raw temperature [°C], pressure [hPa], raw relative humidity [%], gas [Ohm], IAQ, IAQ accuracy, temperature [°C], relative humidity [%], Static IAQ, CO2 equivalent, breath VOC equivalent";
  Serial.println(output);


  for (int i = 0; i < 12; i++)
  {
    newT[i] = 0;
    newH[i] = 0;
    newP[i] = 0;
    newI[i] = 0;
    newC[i] = 0;
    newV[i] = 0;
    newSI[i] = 0;
  }
  
}

// Function that is looped forever
void loop(void)
{
  server.handleClient();
  
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


  //web
  if (time_trigger > nextTime)
  {
    // Every 2 hours 7200000
    nextTime += 7200000;
  
    // Set values from sensor (replace 0 with functions to get current values) ****************************************************************************  IMPORTANT
    temp[ind] = iaqSensor.temperature;
    hum[ind] = iaqSensor.humidity;
    pres[ind] = iaqSensor.pressure;
    IAQ[ind] = iaqSensor.iaq;
    CO2[ind] = iaqSensor.co2Equivalent;
    VOC[ind] = iaqSensor.breathVocEquivalent;
    SIAQ[ind] = iaqSensor.staticIaq;

    output = String(time_trigger);
    output += ", " + String(temp[ind]);
    output += ", " + String(hum[ind]);
    output += ", " + String(pres[ind]);
    output += ", " + String(IAQ[ind]);
    output += ", " + String(CO2[ind]);
    output += ", " + String(VOC[ind]);
    output += ", " + String(SIAQ[ind]);
    Serial.println(output);

      // populate table newest at most right
    for (int j = 0; j < 11; j++)
    {
      newT[j] = newT[j + 1];
      newH[j] = newH[j + 1];
      newP[j] = newP[j + 1];
      newI[j] = newI[j + 1];
      newC[j] = newC[j + 1];
      newV[j] = newV[j + 1];
      newSI[j] = newSI[j + 1];
    }

    newT[11] = temp[ind];
    newH[11] = hum[ind];
    newP[11] = pres[ind];
    newI[11] = IAQ[ind];
    newC[11] = CO2[ind];
    newV[11] = VOC[ind];
    newSI[11] = SIAQ[ind];
  
    // Makes sure we re-use indexs of array as only holds 12 readings, prevents out of bounds error
    if (ind == 11)
    {
      ind = 0;
    }
    else 
    {
      ind++;
    }
  }
   
}

// Helper function definitions
void checkIaqSensorStatus(void)
{
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

// HTML for webpage
String SendHTML(){
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

  // Table
  ptr +="<table style='width:100%'>\n";
  ptr +="<tr>\n";
  ptr +="<th></th>\n";
  ptr +="<th>24:00</th>\n";
  ptr +="<th>22:00</th>\n";
  ptr +="<th>18:00</th>\n";
  ptr +="<th>16:00</th>\n";
  ptr +="<th>14:00</th>\n";
  ptr +="<th>12:00</th>\n";
  ptr +="<th>10:00</th>\n";
  ptr +="<th>08:00</th>\n";
  ptr +="<th>06:00</th>\n";
  ptr +="<th>04:00</th>\n";
  ptr +="<th>02:00</th>\n";
  ptr +="<th>Current</th>\n";
  ptr +="</tr>\n";
  ptr +="<tr>\n";
  
  ptr +="<th>Temperature</th>\n";
  ptr +="<td>" + String(newT[0]) + "</td>\n";
  ptr +="<td>" + String(newT[1]) + "</td>\n";
  ptr +="<td>" + String(newT[2]) + "</td>\n";
  ptr +="<td>" + String(newT[3]) + "</td>\n";
  ptr +="<td>" + String(newT[4]) + "</td>\n";
  ptr +="<td>" + String(newT[5]) + "</td>\n";
  ptr +="<td>" + String(newT[6]) + "</td>\n";
  ptr +="<td>" + String(newT[7]) + "</td>\n";
  ptr +="<td>" + String(newT[8]) + "</td>\n";
  ptr +="<td>" + String(newT[9]) + "</td>\n";
  ptr +="<td>" + String(newT[10]) + "</td>\n";
  ptr +="<td>" + String(newT[11]) + "</td>\n";
  ptr +="</tr>\n";

  ptr +="<tr>\n";
  ptr +="<th>Humidity</th>\n";
  ptr +="<td>" + String(newH[0]) + "</td>\n";
  ptr +="<td>" + String(newH[1]) + "</td>\n";
  ptr +="<td>" + String(newH[2]) + "</td>\n";
  ptr +="<td>" + String(newH[3]) + "</td>\n";
  ptr +="<td>" + String(newH[4]) + "</td>\n";
  ptr +="<td>" + String(newH[5]) + "</td>\n";
  ptr +="<td>" + String(newH[6]) + "</td>\n";
  ptr +="<td>" + String(newH[7]) + "</td>\n";
  ptr +="<td>" + String(newH[8]) + "</td>\n";
  ptr +="<td>" + String(newH[9]) + "</td>\n";
  ptr +="<td>" + String(newH[10]) + "</td>\n";
  ptr +="<td>" + String(newH[11]) + "</td>\n";
  ptr +="</tr>\n";

  
  ptr +="<tr>\n";
  ptr +="<th>Pressure</th>\n";
  ptr +="<td>" + String(newP[0]) + "</td>\n";
  ptr +="<td>" + String(newP[1]) + "</td>\n";
  ptr +="<td>" + String(newP[2]) + "</td>\n";
  ptr +="<td>" + String(newP[3]) + "</td>\n";
  ptr +="<td>" + String(newP[4]) + "</td>\n";
  ptr +="<td>" + String(newP[5]) + "</td>\n";
  ptr +="<td>" + String(newP[6]) + "</td>\n";
  ptr +="<td>" + String(newP[7]) + "</td>\n";
  ptr +="<td>" + String(newP[8]) + "</td>\n";
  ptr +="<td>" + String(newP[9]) + "</td>\n";
  ptr +="<td>" + String(newP[10]) + "</td>\n";
  ptr +="<td>" + String(newP[11]) + "</td>\n";
  ptr +="</tr>\n";


  ptr +="<th>IAQ</th>\n";
  ptr +="<td>" + String(newI[0]) + "</td>\n";
  ptr +="<td>" + String(newI[1]) + "</td>\n";
  ptr +="<td>" + String(newI[2]) + "</td>\n";
  ptr +="<td>" + String(newI[3]) + "</td>\n";
  ptr +="<td>" + String(newI[4]) + "</td>\n";
  ptr +="<td>" + String(newI[5]) + "</td>\n";
  ptr +="<td>" + String(newI[6]) + "</td>\n";
  ptr +="<td>" + String(newI[7]) + "</td>\n";
  ptr +="<td>" + String(newI[8]) + "</td>\n";
  ptr +="<td>" + String(newI[9]) + "</td>\n";
  ptr +="<td>" + String(newI[10]) + "</td>\n";
  ptr +="<td>" + String(newI[11]) + "</td>\n";
  ptr +="</tr>\n";


  ptr +="<th>CO2</th>\n";
  ptr +="<td>" + String(newC[0]) + "</td>\n";
  ptr +="<td>" + String(newC[1]) + "</td>\n";
  ptr +="<td>" + String(newC[2]) + "</td>\n";
  ptr +="<td>" + String(newC[3]) + "</td>\n";
  ptr +="<td>" + String(newC[4]) + "</td>\n";
  ptr +="<td>" + String(newC[5]) + "</td>\n";
  ptr +="<td>" + String(newC[6]) + "</td>\n";
  ptr +="<td>" + String(newC[7]) + "</td>\n";
  ptr +="<td>" + String(newC[8]) + "</td>\n";
  ptr +="<td>" + String(newC[9]) + "</td>\n";
  ptr +="<td>" + String(newC[10]) + "</td>\n";
  ptr +="<td>" + String(newC[11]) + "</td>\n";
  ptr +="</tr>\n";


  ptr +="<th>VOX</th>\n";
  ptr +="<td>" + String(newV[0]) + "</td>\n";
  ptr +="<td>" + String(newV[1]) + "</td>\n";
  ptr +="<td>" + String(newV[2]) + "</td>\n";
  ptr +="<td>" + String(newV[3]) + "</td>\n";
  ptr +="<td>" + String(newV[4]) + "</td>\n";
  ptr +="<td>" + String(newV[5]) + "</td>\n";
  ptr +="<td>" + String(newV[6]) + "</td>\n";
  ptr +="<td>" + String(newV[7]) + "</td>\n";
  ptr +="<td>" + String(newV[8]) + "</td>\n";
  ptr +="<td>" + String(newV[9]) + "</td>\n";
  ptr +="<td>" + String(newV[10]) + "</td>\n";
  ptr +="<td>" + String(newV[11]) + "</td>\n";
  ptr +="</tr>\n";


  ptr +="<th>Static IAQ</th>\n";
  ptr +="<td>" + String(newSI[0]) + "</td>\n";
  ptr +="<td>" + String(newSI[1]) + "</td>\n";
  ptr +="<td>" + String(newSI[2]) + "</td>\n";
  ptr +="<td>" + String(newSI[3]) + "</td>\n";
  ptr +="<td>" + String(newSI[4]) + "</td>\n";
  ptr +="<td>" + String(newSI[5]) + "</td>\n";
  ptr +="<td>" + String(newSI[6]) + "</td>\n";
  ptr +="<td>" + String(newSI[7]) + "</td>\n";
  ptr +="<td>" + String(newSI[8]) + "</td>\n";
  ptr +="<td>" + String(newSI[9]) + "</td>\n";
  ptr +="<td>" + String(newSI[10]) + "</td>\n";
  ptr +="<td>" + String(newSI[11]) + "</td>\n";
  ptr +="</tr>\n";
  
  ptr +="</table>\n";
  

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

// Handle root url (/)
void handle_root() {
  server.send(200, "text/html", SendHTML());
}
