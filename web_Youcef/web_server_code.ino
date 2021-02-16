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
float temperature[12], humidity[12], pressure[12];

// Variables to keep track of data every 2 hours
int index = 0;
int nextTime = 0; 
bool first = true;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  // Function to call when just connected
  server.on("/", handle_OnConnect);
  // Function to call when an unrecognised url is type in
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();

  if (millis() > nextTime)
  {
    // Every 2 hours
    nextTime += 7200000;

    // Set values from sensor (replace 0 with functions to get current values) ****************************************************************************  IMPORTANT
    temperature[index] = 0;
    humidity[index] = 0;
    pressure[index] = 0;

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

  // Check every 5 seconds as we don't need fast loop
  delay(5000);
  
}

// Display page when client connects
void handle_OnConnect() {
  Serial.println("Client connected, sending info.");
  // Pass data arrays to html
  server.send(200, "text/html", SendHTML(index, first, temperature, humidity, pressure)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

// HTML for webpage
String SendHTML(int ind, bool fr, float t[12],float h[12], float p[12]){
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
      }
      else
      {
        newT[i] = t[11 - i];
        newH[i] = h[11 - i];
        newP[i] = p[11 - 1];
        break;
      }
    }
  }
  

  float newT[12], newH[12], newP[12];

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
  
  ptr +="</table>\n";
  

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
