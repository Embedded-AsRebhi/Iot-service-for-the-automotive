#define TINY_GSM_MODEM_SIM800  // Modem is SIM800L
#define gpsSerial Serial1
#define SerialSim Serial2
#define Serialaff Serial      //SerialMon.println("Hello World!");
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
//String GPSData = "";




// MQTT broker  details 
const char* broker = "broker.mqttdashboard.com";
const char* mqttUsername = "";  // MQTT username
const char* mqttPassword = "";
const int mqtt_port = 1883;
const char* mqtt_topic = "GsmClientTest";
//char* GPSData = ""; 


#include <TinyGPS++.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

TinyGPSPlus gps; // the TinyGPS++ object

TinyGsm modem(SerialSim);

// Create GPRS client instance for SIM800L module
TinyGsmClient gprs_client (modem);

PubSubClient mqtt_client(broker, mqtt_port, gprs_client);
uint32_t lastReconnectAttempt = 0;


boolean mqttConnect()
{
  Serialaff.print("Connecting to : ");
  Serialaff.print(broker);
  boolean status = mqtt_client.connect("GsmClientTest", mqttUsername, mqttPassword);

 
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

void setup() {
  delay(7000);
  Serialaff.begin(9600); // initialize serial communication at 9600 baud
  Serialaff.println("...................................System Started.............................");

  gpsSerial.begin(9600); // initialize serial communication at 9600 baud
  SerialSim.begin(9600);


  TinyGsmAutoBaud(SerialSim);
  delay(6000);

  Serialaff.println(".....Initializing modem ...");
  modem.restart();
  delay(600);

   
  String modemInfo = modem.getModemInfo();
  Serialaff.println(".......Modem Info.....: ");
  Serialaff.println(modemInfo);


  #if TINY_GSM_USE_GPRS
   Serialaff.print(F("............................intializing to GPRS protocol......................: "));
   Serialaff.print(apn);
   if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
   Serialaff.println(" fail");
   modem.restart();
   return;   }
   Serialaff.print(" ok ...");
   if (modem.isGprsConnected())
   { Serialaff.println("GPRS connected"); }
 #endif

 
 mqtt_client.setServer(broker, mqtt_port );
 Serialaff.println("I finished setserver");
}

void loop() {

 
  
 unsigned long currentMillis = millis();
 if (!mqtt_client.connected()) {
    // Reconnect every 10 seconds
    Serialaff.println("...........................try to connect with MQTTconnect ...............: ");
    uint32_t t = millis(); 

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


 Serialaff.println (".....................i will send as Json the date ................... ");
 //creates a dynamic JSON document with an initial capacity of 4096 bytes. 
 DynamicJsonDocument jsonBuffer(8192);
 JsonObject nested = jsonBuffer.createNestedObject("GPS Data"); 
                                                                      
 
 Serialaff.println ("...............i will check GPS..................");
  if (gpsSerial.available() > 0) {
    //Serialaff.println ("............... GPS available..................");
    if (gps.encode(gpsSerial.read())) {
      //Serial.println("the gps serial read : done ");
      if (gps.location.isValid()) {
        nested["latitude:"]= gps.location.lat();
        nested["longitude"]= gps.location.lng();
        //Serial.print(F("- altitude: "));
       if (gps.altitude.isValid()){
          nested["altitude"] = gps.altitude.meters();}
        else{
          nested["altitude"] = "INVALID";}
      } 
             
      else {
        nested["the location"] = "- location: INVALID";
      }

      //Serial.print(F("- speed: "));
      if (gps.speed.isValid()) {
       nested["speed:"] = gps.speed.kmph();
        //Serial.println(F(" km/h"));
      } else {
        nested["speed :"] = "INVALID";
      }
      //Serial.println();
    
  
     String jsonString;
     serializeJson(jsonBuffer, jsonString);
  
     jsonString += "\n"; // add newline character after each payload

     Serialaff.println("..................I will publish the data now.................");
     mqtt_client.publish("GsmClientTest", jsonString.c_str());
  
     mqtt_client.loop();
     delay(9000);  
  

}}}

