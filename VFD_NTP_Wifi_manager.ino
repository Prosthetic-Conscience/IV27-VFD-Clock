#include <ArduinoOTA.h>
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <time.h>               // time() ctime()
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Timezone.h> 

//I/O config
const int clk = D1;
const int load = D2;
const int din = D3;
const int blank = D4;

//Button config
const int tdSwitch = D7;
boolean dateTimeState = LOW; //LOW is time, HIGH is date

//holds previous milli() value for once a second update
unsigned long previousMillis = 0;
//NTP client config
const long interval = 1000; 
const long utcOffsetInSeconds = -21600;

// Define NTP Client to get time
WiFiUDP ntpUDP;

//TODO add utc as constants, list on wifi manger page, save in EEPROM
//NTPClient timeClient(UDP object, NTP source, utcOffsetInSeconds,update interval); //TODO, look at this offset
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds,60000);

// US Central Time Zone (Chicago)
TimeChangeRule CDT = {"CDT", Second, Sun, Mar, 2, 60};    // Daylight time = UTC - 5 hours /-300
TimeChangeRule CST = {"CST", First, Sun, Nov, 2,  0};     // Standard time = UTC - 6 hours/-360
Timezone Eastern(CDT, CST);

// If TimeChangeRules are already stored in EEPROM, comment out the three
// lines above and uncomment the line below.
//Timezone myTZ(100);       // assumes rules stored at EEPROM address 100
TimeChangeRule *tcr;        // pointer to the time change rule, use to get TZ abbrev

time_t t_Utc;
time_t t_Local;

void shiftOutFour(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, byte val)
{

uint8_t i;

for (i = 0; i < 4; i++)  {
     if (bitOrder == LSBFIRST)
           digitalWrite(dataPin, !!(val & (1 << i)));
     else      
           digitalWrite(dataPin, !!(val & (1 << (7 - i))));
           
     digitalWrite(clockPin, HIGH);
     digitalWrite(clockPin, LOW);            
  }
}
const byte numMatrix[10] = {
  //segments
  //g,f,e,d,c,b,a,dp
  0xFC, //ZERO
  0x60, //ONE //00001100  
  0xDA, //TWO 0xB6,
  0xF2, //THREE
  0x66, //FOUR
  0xB6, //FIVE
  0xBE, //SIX
  0xE0, //SEVEN
  0xFE, //EIGHT
  0xE6 //NINE
};

const byte numMatrixDec[10] = {
  //segments
  //g,f,e,d,c,b,a,dp
  0xFD, //ZERO
  0x61, //ONE //00001100  
  0xDB, //TWO 0xB6,
  0xF3, //THREE
  0x67, //FOUR
  0xB7, //FIVE
  0xBF, //SIX
  0xE1, //SEVEN
  0xFF, //EIGHT
  0xE7 //NINE
};
//first address bank
const byte gridArray[15] = {
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank display 
0x01, //{0,0,0,0,0,0,0,0,0,0,0,1}; // Grid 1
0x02, //{0,0,0,0,0,0,0,0,0,0,1,0}; // Grid 2
0x04,//{0,0,0,0,0,0,0,0,0,1,0,0}; // Grid 3 
0x08,//{0,0,0,0,0,0,0,0,1,0,0,0}; // Grid 4
0x10,//{0,0,0,0,0,0,0,1,0,0,0,0}; // Grid 5
0x20,//{0,0,0,0,0,0,1,0,0,0,0,0}; // Grid 6
0x40,//{0,0,0,0,0,1,0,0,0,0,0,0}; // Grid 7
0x80,//{0,0,0,0,1,0,0,0,0,0,0,0}; // Grid 8
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank Grid 9
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank Grid 10
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank Grid 11
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank Grid 12
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank Grid 13
0x00//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank Grid 14
};
//Last address bank
const byte gridArrayHigh[15] = {
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank display   
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank  Grid 1
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank Grid 2
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank Grid 3 
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank Grid 4
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank Grid 5
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank Grid 6
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank Grid 7
0x00,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Blank Grid 8
0x01,//{0,0,0,1,0,0,0,0,0,0,0,1}; // Grid 9
0x02,//{0,0,1,0,0,0,0,0,0,0,1,0}; // Grid 10
0x04,//{0,1,0,0,0,0,0,0,0,1,0,0}; // Grid 11
0x08, //{1,0,0,0,0,0,0,0,1,0,0,0}; // Grid 12
0x10,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Grid 13
0x20,//{0,0,0,0,0,0,0,0,0,0,0,0}; // Grid 14
};

void setup() {
Serial.begin(115200);
pinMode(din, OUTPUT);
pinMode(load, OUTPUT);
pinMode(clk, OUTPUT);
pinMode(blank, OUTPUT);
pinMode(tdSwitch, INPUT_PULLUP);
digitalWrite(clk, LOW);
digitalWrite(load, LOW);
digitalWrite(din, LOW); 
digitalWrite(blank, LOW);

//WiFiManager
//Local intialization. Once its business is done, there is no need to keep it around
WiFiManager wifiManager;

//exit after config instead of connecting
wifiManager.setBreakAfterConfig(true);

//reset settings - for testing
//wifiManager.resetSettings();


//tries to connect to last known settings
//if it does not connect it starts an access point with the specified name
//here  "AutoConnectAP" with password "password"
//and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "")) {
    Serial.println("failed to connect, we should reset and see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  
//if you get here you have connected to the WiFi
Serial.println("");
Serial.println("WiFi connected");
Serial.println("IP address: ");
Serial.println(WiFi.localIP());
timeClient.begin();
delay(1000);
timeClient.update();
delay(1000);

t_Utc = timeClient.getEpochTime();
Serial.print("UTC time from NTP set to : ");
Serial.println(t_Utc);

//setTime(01, 55, 00, 11, 3, 2012);        //another way to set the time (hr,min,sec,day,mnth,yr)
//setTime(timeClient.getHours(),timeClient.getMinutes(),timeClient.getSeconds(),timeClient.getDay()
Serial.print("UTC local time set to : ");
t_Local = Eastern.toLocal(t_Utc);
Serial.println(t_Local);

Serial.print("Current time set to ");
Serial.print(hour(t_Local));
Serial.print(":");
Serial.print(minute(t_Local));
Serial.print(":");
Serial.println(second(t_Local)); 
timeClient.update(); 
}

void loop() {
unsigned long currentMillis = millis();
//Update time object once a second
if (currentMillis - previousMillis >= interval) 
    {
    // save the last time was called
    previousMillis = currentMillis;     
    //timeClient.update(); //should add a second TODO: research library
    t_Local = Eastern.toLocal(timeClient.getEpochTime());  
    if(digitalRead(tdSwitch) == LOW)
      {
      if(dateTimeState == true) 
        {
        dateTimeState = false;      
        }
      else if(dateTimeState == false)
        {
        dateTimeState = true;      
        }  
      } 
  timeClient.update();      
  }     
        
//Time code block    
if(dateTimeState == false) 
  {
  int hour_first = (hour(t_Local)/10);
  int hour_second = (hour(t_Local) % 10);
  
  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[hour_first]);
  shiftOut(din, clk, LSBFIRST, 0x04); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x00);
  digitalWrite(load, HIGH);
  delay(3);

  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[hour_second]);
  shiftOut(din, clk, LSBFIRST, 0x08); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x00);
  digitalWrite(load, HIGH);  
  delay(3);

  int min_first = (minute(t_Local)/10);
  int min_second = (minute(t_Local) % 10);
  
  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[min_first]);
  shiftOut(din, clk, LSBFIRST, 0x20); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x00);
  digitalWrite(load, HIGH);  
  delay(3);

  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[min_second]);
  shiftOut(din, clk, LSBFIRST, 0x40); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x00);
  digitalWrite(load, HIGH);  
  delay(3);

  int sec_first = (second(t_Local)/10);
  int sec_second = (second(t_Local) % 10);
  
  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[sec_first]);
  shiftOut(din, clk, LSBFIRST, 0x00); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x01);
  digitalWrite(load, HIGH);  
  delay(3);

  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[sec_second]);
  shiftOut(din, clk, LSBFIRST, 0x00); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x02);
  digitalWrite(load, HIGH);  
  delay(3); 
  }
//Year Code block
if(dateTimeState == true) 
  {
  //Month section
  int month_first = (month(t_Local)/10);
  int month_second = (month(t_Local) % 10);
  
  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[month_first]);
  shiftOut(din, clk, LSBFIRST, 0x02); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x00);
  digitalWrite(load, HIGH);
  delay(2);

  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[month_second]);
  shiftOut(din, clk, LSBFIRST, 0x04); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x00);
  digitalWrite(load, HIGH);  
  delay(2);

  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, 0x02); // write "-" character
  shiftOut(din, clk, LSBFIRST, 0x08); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x00);
  digitalWrite(load, HIGH);  
  delay(2);
  
  //Day section
  int day_first = (day(t_Local)/10);
  int day_second = (day(t_Local) % 10);
  
  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[day_first]);
  shiftOut(din, clk, LSBFIRST, 0x10); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x00);
  digitalWrite(load, HIGH);  
  delay(2);

  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[day_second]);
  shiftOut(din, clk, LSBFIRST, 0x20); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x00);
  digitalWrite(load, HIGH);  
  delay(2);

  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, 0x02); // write "-" character
  shiftOut(din, clk, LSBFIRST, 0x40); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x00);
  digitalWrite(load, HIGH);  
  delay(2);

  //Year section
  int year_first = (year(t_Local)/1000 % 10);
  int year_second = (year(t_Local) /100 % 10);
  int year_third = (year(t_Local)/10 % 10);
  int year_fourth = (year(t_Local) % 10);
  
  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[year_first]);
  shiftOut(din, clk, LSBFIRST, 0x80); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x00);
  digitalWrite(load, HIGH);  
  delay(2);

  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[year_second]);
  shiftOut(din, clk, LSBFIRST, 0x00); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x01);
  digitalWrite(load, HIGH);  
  delay(2);

  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[year_third]);
  shiftOut(din, clk, LSBFIRST, 0x00); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x02);
  digitalWrite(load, HIGH);  
  delay(2);

  digitalWrite(load, LOW);  
  shiftOut(din, clk, LSBFIRST, numMatrix[year_fourth]);
  shiftOut(din, clk, LSBFIRST, 0x00); //55 01010101
  shiftOutFour(din, clk, LSBFIRST, 0x04);
  digitalWrite(load, HIGH);  
  delay(2);  
  }  
}
