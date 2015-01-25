// garagedoor
// January, 2015
// Interface YUN to SHTX11 temperature/humidity sensor
// Report reading to mosquitto mqtt broker server
//
#include <SPI.h>
#include <SHT1x.h>
#include <Bridge.h>
#include <YunClient.h>
#include <PubSubClient.h>

// Variables

// shtx11 to Yun connection -----------
// YUN  > SHTX11
// 3.3v > VCC 8
// GND  > GND 4
// 10   > DATA 1
// 11   > CLK 3jk
#define dataPin  10
#define clockPin 11
// YUN to Relay ---------
// 8 >  In1
// GND > GND
// 5v > VCC
#define relay1 8 // Banggood dual relay module
#define led 13 // Onboard LED

// YUN to Switch ---------------
// 5v > Switch NO
// 2 > Switch Comm
// 2 > GND via pulldown resistor
const int switch1 = 2;

// MQTT Server
#define MQTT_SERVER "192.168.15.22"

unsigned long time;
char message_buff[100];
float temp;
float humidity;
int switch1State = 0;


// Systems Initalization
YunClient yun;
PubSubClient mqtt(MQTT_SERVER, 1883, callback, yun);
SHT1x sht1x(dataPin, clockPin);


void setup() {
  pinMode(led,OUTPUT);
  pinMode(switch1, INPUT);
  pinMode(relay1,OUTPUT);
  digitalWrite(led, LOW);
  Serial.begin(9600);
  Bridge.begin();
  digitalWrite(relay1,LOW);
  digitalWrite(led, HIGH);
}

void loop() {

  if (!mqtt.connected())  {
    // Connect (or reconnect) to mqtt broker on the openhab server
    mqtt.connect("yun1");
    mqtt.publish("openhab/office/temperature", "I'm alive!");
    mqtt.publish("openhab/office/switch1", "I'm alive!");
    mqtt.subscribe("openhab/office/relay1");
    }
       
  // publish Temperature reading every 10 seconds
  temp = sht1x.readTemperatureF();
  humidity = sht1x.readHumidity();
  if (millis() > (time + 10000)) {
    time = millis();
    // publish Temperature
    String pubString = String(temp);
    pubString.toCharArray(message_buff, pubString.length()+1);
    Serial.print(pubString + " F " );
    mqtt.publish("openhab/office/temperature", message_buff);
    
    //publish switch1 status
    switch1State = digitalRead(switch1);
    Serial.println(switch1State);
    pubString = String(switch1State);
    pubString.toCharArray(message_buff, pubString.length()+1);
    mqtt.publish("openhab/office/switch1", message_buff); 
    // Light LED 
    if (switch1State == 1){
      digitalWrite(led,HIGH);
    } else {
      digitalWrite(led,LOW);
    }
    }
  mqtt.loop();
}


void callback(char* topic, byte* payload, unsigned int length) {
  // MQTT inbound Messaging 
  int i = 0;
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  String msgString = String(message_buff);
  Serial.println("Inbound: " + String(topic) +":"+ msgString);
  
  //Bounce relay
  if ( msgString == "GO" ) {
     digitalWrite(relay1,HIGH);
     delay(2000);
     digitalWrite(relay1,LOW);
  }

  
  }
