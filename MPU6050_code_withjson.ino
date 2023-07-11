#define TINY_GSM_MODEM_SIM800  // Modem is SIM800L
//#define gpsSerial Serial1
#define SerialSim Serial1
#define Serialaff Serial      
#define TINY_GSM_DEBUG Serialaff 

unsigned long previousMillis = 0;     
const long interval = 5 * 60 * 1000; 


#define TINY_GSM_USE_GPRS true
//set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[] = "internet.ooredoo.tn";  
const char gprsUser[] = "";
const char gprsPass[] = "";
//String GPSData = "";




// MQTT broker  details 
//const char* broker = "broker.mqttdashboard.com";
const char* broker = "mqtt-dashboard.com";
const char* mqttUsername = "";  // MQTT username
const char* mqttPassword = "";
const int mqtt_port = 1883;
const char* mqtt_topic = "AccgyTest";
//const char* mqtt_topic2 = "GyroTest";
//char* GPSData = ""; 


#include <WireData.h>  //we use this library to set the I2c protocol as communication protocol 
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
 //#include <MPU6050.h> 

 //MPU6050 MPU;
 //const int MPU = 0x68; // // MPU6050 I2C address


TinyGsm modem(SerialSim);

// Create GPRS client instance for SIM800L module
TinyGsmClient gprs_client (modem);

PubSubClient mqtt_client(broker, mqtt_port, gprs_client);
uint32_t lastReconnectAttempt = 0;
 float AccX, AccY, AccZ;
 float GyroX, GyroY, GyroZ;
 
 float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
 float roll, pitch, yaw;
 //float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
 
 float elapsedTime, currentTime, previousTime;
 int c = 0;
 const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

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

 void setup() {
  delay(7000);
  Serialaff.begin(115200); // initialize serial communication at 9600 baud
  Serialaff.println("System Started.............................");

  //gpsSerial.begin(9600); // initialize serial communication at 9600 baud
  SerialSim.begin(115200);

 
  
  TinyGsmAutoBaud(SerialSim);
  delay(6000);

  Serialaff.println("............................Initializing modem ....................");
  modem.restart();
  delay(600);

   
  String modemInfo = modem.getModemInfo();
  Serialaff.println(".......Modem Info.....: ");
  Serialaff.println(modemInfo);


  #if TINY_GSM_USE_GPRS
   Serialaff.println(F("........................intializing to GPRS protocol......................: "));
   Serialaff.print(apn);
   if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
   Serialaff.println(" fail");
   modem.restart();
   return;   }
   Serialaff.print(" ok ...");
   if (modem.isGprsConnected())
   { Serialaff.println("GPRS connected"); }
 #endif

 // //The setServer() function is a method of the MQTT client class that is used to set the address and port number of the MQTT broker
 mqtt_client.setServer(broker, mqtt_port );
 Serialaff.println("I finished setserver");

 Serialaff.println ("........................initializing MPU6050 with I2C protocol .....................");

  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);       // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B);                      // PWR_MGMT_1 register
  Wire.write(0);                        // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}

void loop() {


  unsigned long currentMillis = millis();
 if (!mqtt_client.connected()) {
    // Reconnect every 10 seconds
    Serialaff.println("....................try to connect with MQTTconnect .............................: ");
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
 
 //creates a dynamic JSON document with an initial capacity of 4096 bytes. 
 DynamicJsonDocument jsonBuffer(16384);
 //DynamicJsonDocument jsonBufferg(8192);
 //The variable "nested" is assigned the new JSON object, which can be used to add new data values or nested objects within "data value".
 JsonObject nested = jsonBuffer.createNestedObject("Accel and gyr Data"); 
 //JsonObject nestedg = jsonBufferg.createNestedObject("gyroscope Data"); 

 
  Wire.beginTransmission(0x68);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H) , we start by the high 
  Wire.endTransmission(false);



 Serialaff.println ("...................data is ready to read .................");
 Wire.requestFrom(0x68, 6, true); 
 AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
 AccY = (Wire.read() << 8 | Wire.read ()) / 16384.0; 
 AccZ = (Wire.read() << 8 | Wire.read ()) / 16384.0;
  
 
 Serialaff.println (".......................i will send as Json the date ........................ ");
  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - 0.58; //roll 
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) + 1.58; //pitch
  
  nested["AccX"]= AccX;
  nested["AccY"]= AccY;
  nested["AccZ"]= AccZ;
  
 /* nested["AccRoll"]=  accAngleX;
  nested["Accpitch"]= accAngleY; */

     

  Serialaff.println ("................the Accelerometer parameters are ..................... ");
  Serialaff.print(" AccX: ");
  Serialaff.println(AccX);  
  Serialaff.print("  AccY: ");
  Serialaff.println( AccY)  ;
  Serialaff.print("  AccZ: ");
  Serialaff.println( AccZ)  ; 

  Serialaff.print(" AccRoll: ");
  Serialaff.println(accAngleX);
  Serialaff.print(" AccPitch: ");
  Serialaff.println(accAngleY);
  //delay(2000);
  
  // === Read gyroscope data === //
  previousTime = currentTime;        // Previous time is stored before the actual time read
  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000; // Divide by 1000 to get seconds
  Wire.beginTransmission(0x68);
  Wire.write(0x43); // Gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 6, true); // Read 4 registers total, each axis value is stored in 2 registers
  
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0; // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
  // Correct the outputs with the calculated error values

  
  GyroX = GyroX + 0.56; // GyroErrorX ~(-0.56)
  GyroY = GyroY - 2; // GyroErrorY ~(2)
  GyroZ = GyroZ + 0.79; // GyroErrorZ ~ (-0.8)


  
   // Currently the raw values are in degrees per seconds, deg/s, so we need to multiply by sendonds (s) to get the angle in degrees
  gyroAngleX = gyroAngleX + GyroX * elapsedTime; // deg/s * s = deg
  gyroAngleY = gyroAngleY + GyroY * elapsedTime;
  yaw =  yaw + GyroZ * elapsedTime;
  // Complementary filter - combine acceleromter and gyro angle values
  
  roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;

 nested[" GyroX:"]= GyroX;
 nested[" GyroY:"]= GyroY;
 nested[" GyroZ:"]= GyroZ;

 /*nested[" Gyroll:"]= roll;
 nested[" Gypitch:"]= pitch;
 nested[" Gyyaw:"]= yaw;*/
  
 Serialaff.println(" ...............the gyroscope parameter are..................");

  // Print the values on the serial monitor
  Serialaff.print(" GyroX ");
  Serialaff.println(GyroX);  
  Serialaff.print("  GyroY: ");
  Serialaff.println( GyroY)  ;
  Serialaff.print("  GyroZ: ");
  Serialaff.println(GyroZ)  ;

  
  Serialaff.print("roll:");
  Serialaff.println(roll);
  Serialaff.print("pitch:");
  Serialaff.println(pitch);
  Serialaff.print("yaw:");
  Serialaff.println(yaw);
 
 
  String jsonString;
  String jsonStringg;
  serializeJson(jsonBuffer, jsonString);
 // serializeJson(jsonBufferg, jsonStringg);
  
  jsonString += "\n"; // add newline character after each payload
  //jsonStringg  += "\n";

  Serialaff.println("..................I will publish the data now in AccTest.................");
  mqtt_client.publish("AccgyTest", jsonString.c_str());
  delay(5000);
  
 
  mqtt_client.loop();
  delay(9000);  

    
  
}

  

  



