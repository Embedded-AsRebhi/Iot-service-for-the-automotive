
#define TINY_GSM_MODEM_SIM800  // Modem is SIM800L
#define SerialAT Serial1
#define Serialaff Serial      
#define TINY_GSM_DEBUG Serialaff 

unsigned long previousMillis = 0;     // variable to store the last time the loop was executed
const long interval = 5 * 60 * 1000; // interval between loops in milliseconds (5 minutes)

#define TINY_GSM_USE_GPRS true
//set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[] = "internet.ooredoo.tn";  
const char gprsUser[] = "";
const char gprsPass[] = "";



//const char* topicInit      = "GsmClientTest/init";
//const char* topicLedStatus = "GsmClientTest/ledStatus";

#include <TinyGsmClient.h>
#include <PubSubClient.h> 
#include <ArduinoJson.h>
//#include <Ethernet.h>
//#include <SoftwareSerial.h>
// MQTT broker  details 
const char* broker ="mqtt-dashboard.com";


//const char* broker = "localhost";
const char* mqttUsername = "";  // MQTT username
const char* mqttPassword = "";
const int mqtt_port = 1883;
//const char* mqtt_topic = "GsmClientTest";
const char* mqtt_topic = "GsmClientTest";


TinyGsm modem(SerialAT);


TinyGsmClient gprs_client (modem);


PubSubClient mqtt_client(broker, mqtt_port, gprs_client);
uint32_t lastReconnectAttempt = 0;

.

void setup() {
  delay(7000); //delay for 7 seconds to make sure the modules get the signal
  Serialaff.begin(9600);
  //_buffer.reserve(50);
  Serialaff.println("System Started...");
  SerialAT.begin(9600);
  
  Serialaff.println("the value of BER and RSSi are : ");
  Serialaff.println(SerialAT.println("AT+CSQ")); 
  delay(3000);
  
  TinyGsmAutoBaud(SerialAT);
  delay(6000);
  //Sets the GSM Module in Text Mod
  Serialaff.println("Initializing modem ...");
  modem.restart();
  delay(600);

  String modemInfo = modem.getModemInfo();
  Serialaff.println("Modem Info: ");
  Serialaff.println(modemInfo);

//get the date 
 
  
  // GPRS connection parameters are usually set after network registration
 #if TINY_GSM_USE_GPRS
   Serialaff.print(F("intializing to GPRS protocol: "));
   Serialaff.print(apn);
   if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
   Serialaff.println(" fail");
   modem.restart();
   return;   }
   Serialaff.print(" ok ...");
   if (modem.isGprsConnected())
   { Serialaff.println("GPRS connected"); }
 #endif
 // MQTT Broker setup
 //The setServer() function is a method of the MQTT client class that is used to set the address and port number of the MQTT broker
 mqtt_client.setServer(broker, mqtt_port );
 Serialaff.println("I finished setserver:");
 // mqtt_client.setCallback(mqttCallback);
 //Serialaff.println("I finished setcallback:");
}
boolean mqttConnect() 
{
  Serialaff.print("Connecting to : ");
  Serialaff.print(broker);
  boolean status = mqtt_client.connect("GsmClientTest", mqttUsername, mqttPassword);

  // Or, if you want to authenticate MQTT:
  // boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");
  if (status == false) 
  {
    Serialaff.println(" ,the connexion fail");        
    //mqtt_client.restart();
    return false;
  }
  Serialaff.println(" ,success");
  //mqtt_client.subscribe(mqtt_topic);
 // Serialaff.println(" ,I subscribed the topic");
  return mqtt_client.connected();
}
void loop() {
 unsigned long currentMillis = millis();
 
 if (!mqtt_client.connected()) {
    // Reconnect every 10 seconds
    Serialaff.println("try to connect with MQTTconnect : ");
    uint32_t t = millis();
    //If the difference between the current time (t) and the time of the last reconnection attempt
    //(lastReconnectAttempt) is greater than 10,000 milliseconds (or 10 seconds),it may be time to attempt a reconnection again.
    while (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (mqttConnect() == true ) { 
        lastReconnectAttempt = 0; 
        break; 
      }    
    }
    delay(100);
    return;
  }
 
    
 Serialaff.println ("i will send data as Json form  ");
  // Create a dynamic JSON document with a buffer size of 4096 bytes
  DynamicJsonDocument jsonBuffer(4096);
  // Create a nested JSON object within the document
 //The create NestedObject() function is part of the ArduinoJSON library and allows you to create a new JSON object within an existing JSON object
 JsonObject nested = jsonBuffer.createNestedObject("Acc Data");                                                                       
  //jsonDoc["time"] = now.toString("HH:mm:ss");
  //JsonObject& root = jBuffer.Createobject();
  nested["ID"] = 1195;
  nested["acceleration_x"] = 0.5;
  
  String jsonString;
  serializeJson(jsonBuffer, jsonString);
  //jsonString += "\n"; // add newline character after each payload

 // Serial.println(jsonString);
  
  mqtt_client.publish("GsmClientTest", jsonString.c_str());

  mqtt_client.loop();
  delay(9000);
 }
