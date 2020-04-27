//NodeMCU RGB-Controller for Homebridge & HomeKit (Siri)

#include <ESP8266WiFi.h>
//#include <stdio.h>
#include "RGBdriver.h"
#define CLK 4//gpio4 
#define DIO 5//gpio5
RGBdriver Driver(CLK,DIO);

//#define max(a,b) ((a)>(b)?(a):(b))  //added to make max() work with different data types (int | float)

WiFiServer server(80); //Set server port

String readString;           //String to hold incoming request
String hexString = "080100"; //Define inititial color here (hex value), 080100 would be a calm warmtone i.e.
String brightness = "100";   //define brightness value

int state;

int r, g, b; //original rgb value

float R, G, B, V; //calculated RGB after brightnes value

///// WiFi SETTINGS - Replace with your values /////////////////
const char* ssid = "jl2";
const char* password = "H0n3yw3!!123";
IPAddress ip(10, 200, 10, 201);   // set a fixed IP for the NodeMCU
IPAddress gateway(10, 200, 10, 1); // Your router IP
IPAddress subnet(255, 255, 255, 0); // Subnet mask
////////////////////////////////////////////////////////////////////

void WiFiStart() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet); //Set a fixed IP. You can comment this out and set it in your router instead.
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print("_");
  }
  Serial.println();
  Serial.println("Done");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  server.begin();
}

void allOff() {
  state = 0;
  Driver.begin();
  Driver.SetColor (0, 0, 0);
  Driver.end();
}

//Write requested hex-color
void setHex() {
  state = 1;
  long number = (long) strtol( &hexString[0], NULL, 16);
  //Serial.println("sethex number: ");
  //Serial.print(hexString[0]);
  //Serial.println("");
  r = number >> 16;
  g = number >> 8 & 0xFF;
  b = number & 0xFF;
  showValues();
  R = (r * V /100);
  G = (g * V /100);
  B = (b * V /100);
  Driver.begin();
  Driver.SetColor(R, G, B);
  Driver.end();
}


//For serial debugging only
void showValues() {
  Serial.print("Status on/off: ");
  Serial.println(state);
  Serial.print("RGB color: ");
  Serial.print(r);
  Serial.print("-");
  Serial.print(g);
  Serial.print("-");
  Serial.println(b);
  Serial.print("Hex color: ");
  Serial.println(hexString);
  Serial.print("Brightness: ");
  Serial.println(brightness);
  Serial.println("");
  Serial.println("show readstring: ");
  Serial.print(readString);
  Serial.println();
  Serial.println("V: ");
  Serial.print(V);
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  delay(1);
  setHex(); //Set initial color after booting. Value defined above
  WiFi.mode(WIFI_STA);
  WiFiStart();
 // showValues(); //Uncomment for serial output
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while (client.connected() && !client.available()) {
    delay(1);
  }
  //Respond on certain Homebridge HTTP requests
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (readString.length() < 100) {
          readString += c;
        }
        if (c == '\n') {
          //Serial.print("Request: "); //Uncomment for serial output
          //Serial.println(readString); //Uncomment for serial output
          //Send reponse:
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          //On:
          if (readString.indexOf("on") > 0) {
            setHex();
            //showValues();
          }
          //Off:
          if (readString.indexOf("off") > 0) {
            allOff();
            //showValues();
          }
          //Set color:
          if (readString.indexOf("set") > 0) {
            
            hexString = "";
            hexString = (readString.substring(9, 15));
            setHex();
           
            //showValues();
          }
          if (readString.indexOf("brt") > 0) {
            
            brightness = "";
            brightness = (readString.substring(9, 12));
            V = brightness.toFloat();
            //setHex();           
            //showValues();
          }
          
          //Status on/off:
          if (readString.indexOf("status") > 0) {
            client.println(state);
          }
          //Status color (hex):
          if (readString.indexOf("color") > 0) {
            client.println(hexString);
          }
          //Status brightness (%):
          if (readString.indexOf("bright") > 0) {
            client.println(brightness);
          }
          delay(1);
          while (client.read() >= 0);  //added: clear remaining buffer to prevent ECONNRESET
          client.stop();
          readString.remove(0);
        }
      }
    }
  }
}
