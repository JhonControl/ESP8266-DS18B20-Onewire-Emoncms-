/*
 *  This sketch sends data via HTTP GET requests to emoncms.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 * Test  Send temperature to Emoncms Openenergymonitor
 *
 *
 */

#include <ESP8266WiFi.h>
#include <OneWire.h>
OneWire  ds(2);  // on pin 2 (a 4.7K resistor is necessary)
const char* ssid     = "Nitro X";
const char* password = "25364016";

const char* host = "192.168.1.54";
const char* streamId   = "....................";
const char* apikey = "6a36f95a048ff8ba64f5c0eb18204381";

void setup() {
  Serial.begin(115200);

  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

int value = 0;

void loop() {
  delay(5000);

     
  ++value;

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

 float celsius, fahrenheit;
    
    
///Connection One Wire

  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];

  
  if ( !ds.search(addr)) {
  //  Serial.println("No more addresses.");
   /// Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  

  for( i = 0; i < 8; i++) {  
    addr[i];

  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
//  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:

      type_s = 1;
      break;
    case 0x28:

      type_s = 0;
      break;
    case 0x22:
    //  Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
    //  Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);      
  
  delay(1000);    

//  delay(1000);    
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         

  for ( i = 0; i < 9; i++) {       
    data[i] = ds.read();
  }
  OneWire::crc8(data, 8); 
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; 
    if (data[7] == 0x10) {      
      raw = (raw & 0xFFF0) + 12 - data[6];    }
  } else {
    byte cfg = (data[4] & 0x60);

    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    
  }

    celsius = (float)raw / 16.0;

 
   Serial.print("temperature float:");
   Serial.println(celsius);
   
    char outstr[15];
    dtostrf(celsius,4, 2, outstr);   //float to char  4 numero de caracteres  3 cifras sin espacio
    String valor= outstr;   // char to string
    
    Serial.print("temperature String:");
    Serial.println(valor);

    String url = "/emoncms/input/post.json?json={Temperature_ESP8266:"+ valor +"}&apikey="+apikey;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(2000);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}

