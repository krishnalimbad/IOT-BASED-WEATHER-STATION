/*********************************************************
   AUTHOR: KRISHNA LIMBAD
   TOPIC:IOT BASED WEATHER STATION
   DATE CREATED:10-1-2019
   DATE MODIFIED:-31-1-2019

 *********************************************************/

#include <ESP8266WiFi.h>;
#include <PubSubClient.h>;
//*********************************************************
#include <Wire.h>
#include <SFE_BMP180.h> //Including BMP180 library
//*********************************************************

#include <dht11.h>
#define wifi_ssid "Isha"
#define wifi_password "123456789"
#define mqtt_server "test.mosquitto.org" //like "test.mosquitto.org"
//#define mqtt_user "your_username"
//#define mqtt_password "your_password"

//********************TOPIC DECLEARTION***********************************
#define topic1 "temperature/WS"
#define topic2 "humidity/WS"
#define topic3 "BMPtemp/WS"
#define topic4 "BMPpressure/WS"
#define topic5 "BMPsealevel/WS"
#define topic6 "Rain/WS"
//*******************************************************
dht11 DHT11;
#define DHT11PIN D5
#define rainPin A0
const int sensorMin = 0; 
const int sensorMax = 1024;
//*********************************************************
#define ALTITUDE 35.6
SFE_BMP180 pressure; //Creating an object
//*********************************************************
WiFiClient espClient;
PubSubClient client(espClient);
void setup()
{
  pinMode(rainPin,INPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  //*********************************************************
  if (pressure.begin()) //If initialization was successful, continue
    Serial.println("BMP180 init success");
  else //Else, stop code forever
  {
    Serial.println("BMP180 init fail");
    while (1);
  }
  //*******************************************************
}
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("mosquitto")) {
      //* See //NOTE below
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//NOTE: if a user/password is used for MQTT connection use:
//if(client.connect("TestMQTT", mqtt_user, mqtt_password)) {
void pubMQTT(String topic, String topic_val)
{
  Serial.print("Newest topic " + topic + " value:");
  Serial.println(String(topic_val).c_str());
  client.publish(topic.c_str(), String(topic_val).c_str(), true);
}
//Variables used in loop()
long lastMsg = 0;
float t1;
float t2;
//**********RAIN SENSOR********************************************

int value = 0;
String rd = "Raining";
String rnd = "Not Raining";

//****************************************************************************
char status;
double T, P, p0; //Creating variables for temp, pressure and relative pressure
//*************************BMP180*********************************************

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  //2 seconds minimum between Read Sensors and Publish
  long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;

    //Read Sensors (simulate by increasing the values, range:0-90)

    //Put your main sensor code here:----

    status = pressure.startTemperature();
    if (status != 0)
    {
      status = pressure.getTemperature(T);
      Serial.print(T, 1);
      String Bt = String(T) + "deg C";
      status = pressure.startPressure(3);
      status = pressure.getPressure(P, T);
      Serial.print(P);
      String Bp = String(P) + "hPa";
      p0 = pressure.sealevel(P, ALTITUDE);
      Serial.print(p0);
      String Bs = String(p0) + "hPa";
      pubMQTT(topic3, Bt);
      pubMQTT(topic4, Bp);
      pubMQTT(topic5, Bs);
    }
    //****************DHT11 SENSOR*******************************************
    int chk = DHT11.read(DHT11PIN);
    t2 = (float)DHT11.humidity;
    t1 = (float)DHT11.temperature;
    //t1 = t1>90 ? 0 : ++t1;
    //t2 = t2>90 ? 0 : ++t2;
    String temp2 = String(t2) + " %";
    String temp1 = String(t1) + " C";
    //Publish Values to MQTT broker
    pubMQTT(topic1, temp1);
    pubMQTT(topic2, temp2);
    //*****************RAIN SENSOR*****************************
   int value = analogRead(rainPin);
    int range = map(value, sensorMin, sensorMax, 0, 2);
    Serial.println(value);
   
    if (range == 0)
    {
    
      String temp3 = rd;
      pubMQTT(topic6, temp3);
    }
    else
    {
      String temp4 = rnd;
      pubMQTT(topic6, temp4);
    }
  }
}
