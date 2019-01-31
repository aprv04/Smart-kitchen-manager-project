/*
  WiFi Web Server

 A simple web server that a repeated counter

 Change the macro WIFI_AP, WIFI_PASSWORD and WIFI_AUTH accordingly.

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 modified 20 Aug 2014
 by MediaTek Inc.
 */
#include <HttpClient.h>
#include <LWiFi.h>
#include <LWiFiServer.h>
#include <LWiFiClient.h>
#include "LGSM.h"
#include "HX711.h"

#define DOUT 12
#define CLK A0
#define SITE_URL "api.mediatek.com"
#define WIFI_AP "Honor 6X_CE93"
#define WIFI_PASSWORD "Sam@9612"
#define WIFI_AUTH  LWIFI_WPA // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP according to your WiFi AP configuration

LWiFiClient content;
HttpClient http(content);
LWiFiServer server(80);

int LED = 13; // Use the onboard Uno LED

HX711 scale(DOUT, CLK);
float calibration_factor = -420;
int obstaclePin = 7;  // This is our input pin
int hasObstacle = HIGH;  // HIGH MEANS NO OBSTACLE
char phonenum[20]="8116527322";
void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(obstaclePin, INPUT);
  LWiFi.begin();
  Serial.begin(9600);

  // keep retrying until connected to AP
  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    delay(1000);
    Serial.println(".");
  }
  Serial.println("AP is connected");
  while(!LSMS.ready())
  {
    delay(1000);
    Serial.println("Sim is initialised");

  }
  Serial.println("SIM ready for work!");
 Serial.println("HX711 Calibration");
Serial.println("Remove all weight from scale");
Serial.println("After readings begin, place known weight on scale");
Serial.println("Press a,s,d,f to increase calibration factor by 10,100,1000,10000 respectively");
Serial.println("Press z,x,c,v to decrease calibration factor by 10,100,1000,10000 respectively");
Serial.println("Press t for tare");
Serial.println("setting scale............");
scale.set_scale();
Serial.println("scale set............");
scale.tare(); //Reset the scale to 0
Serial.println("Scale tared to zero");
long zero_factor = scale.read_average(); //Get a baseline reading
Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
Serial.println(zero_factor);
}

int wait=0;
int count=0;

void loop()
{
  // put your main code here, to run repeatedly:
  ir_cheek();
  load_cheek();
 
}

void ir_cheek()
{
  int count;
  hasObstacle = digitalRead(obstaclePin); //Reads the output of the obstacle sensor from the 7th PIN of the Digital section of the arduino
  if (hasObstacle == LOW) //LOW means something is ahead, so illuminates the 13th Port connected LED
  {
    Serial.println("Lid is closed");
    digitalWrite(LED, HIGH);//Illuminates the 13th Port LED
    wait=0;
  }
  else
  {
    Serial.println("Lid is opened!");
    digitalWrite(LED, LOW);
    wait++;
  }
upload_to_mcs1(count);
if(wait==100)
{
  LSMS.beginSMS(phonenum);
  LSMS.print("your lid is open");
  if(LSMS.endSMS())
  {
    Serial.println("SMS is sent");
  }
  else
  {
    Serial.println("SMS is not sent");
  }
}
  
  delay(200);
}
void load_cheek()
{
  int data;
  data=(int)scale.get_units();
  if(data>=0)
  {
  scale.set_scale(calibration_factor); //Adjust to this calibration factor


   Serial.print("Reading: ");
    Serial.print(data);
    Serial.print(" gram"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
    Serial.print(" calibration_factor: ");
    Serial.print(calibration_factor);
    Serial.println();
    count++;
    
    if(Serial.available() > 0)
      {
            char temp = Serial.read();
            if(temp == '+' || temp == 'a')
            calibration_factor += 10;
            else if(temp == '-' || temp == 'z')
            calibration_factor -= 10;
            else if(temp == 's')
            calibration_factor += 100;
            else if(temp == 'x')
            calibration_factor -= 100;
            else if(temp == 'd')
            calibration_factor += 1000;
            else if(temp == 'c')
            calibration_factor -= 1000;
            else if(temp == 'f')
            calibration_factor += 10000;
            else if(temp == 'v')
            calibration_factor -= 10000;
            else if(temp == 't')
            scale.tare();//Reset the scale to zero
            
      }
      if(count==100)
      {
        count=0;
      if(data == 0)
      {
             LSMS.beginSMS(phonenum);
            LSMS.print("your container is empty");
            if(LSMS.endSMS())
            {
              Serial.println("SMS is sent");
            }
            else
            {
              Serial.println("SMS is not sent");
            }
      }
      else if(data<=100)
      {
                     LSMS.beginSMS(phonenum);
              LSMS.print("your container going to empty");
              if(LSMS.endSMS())
              {
                Serial.println("SMS is sent");
              }
              else
              {
                Serial.println("SMS is not sent");
              }
      }
    }
      upload_to_mcs2(data);
      delay(1000);
  }
}

void upload_to_mcs1(int value){
  while(!content.connect(SITE_URL,80))
  {
    Serial.print("data uploaded to server");
    delay(500);
  }
  content.println("POST /mcs/v2/devices/D3e1iICx/datapoints.csv HTTP/1.1");
//#hint! Please do not add excessive spaces

String data = "lid,,"+String(value);

//#hint! Please do not add excessive spaces
//#timestamp is in unix-time format the milliseconds and is optional. If not provided,

//#system will generate timestamp at the time of receiving this API
int dataLength = data.length();

content.println("Host: api.mediatek.com");
content.println("deviceKey: 4SqYMDQ96cFRn9H3");
content.print("Content-Length: ");
content.println(dataLength);
content.println("Content-Type: text/csv");
content.println("Connection: close");
content.println();
content.println(data);

}
void upload_to_mcs2(int value)
{
   while(!content.connect(SITE_URL,80))
  {
    Serial.print("data uploaded to server");
    delay(500);
  }
  content.println("POST /mcs/v2/devices/D3e1iICx/datapoints.csv HTTP/1.1");

String data = "con_1,,"+String(value);

int dataLength = data.length();

content.println("Host: api.mediatek.com");
content.println("deviceKey: 4SqYMDQ96cFRn9H3");
content.print("Content-Length: ");
content.println(dataLength);
content.println("Content-Type: text/csv");
content.println("Connection: close");
content.println();
content.println(data);

}
