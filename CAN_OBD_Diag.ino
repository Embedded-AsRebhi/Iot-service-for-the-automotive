#include <OBD2.h>
#include <DueTimer.h>
/********************************************************************
This example is built upon the CANAcquisition class and the OBDParmameter class using 11bit (non-extended) OBD2 ID's

This example shows how to set up periodic data acquisition of OBD2 paramters based upon
standard PID's. If you'd like to add another paramter,simply copy one of the definitions and modify it accordingly. 
You may also need to add a new PID to the "OBD_PID" enum in the header file. 
/********************************************************************/
//create the CANport acquisition schedulers
cAcquireCAN CANport0(CAN_PORT_0);

/***** DEFINITIONS FOR OBD MESSAGES ON CAN PORT 0, see https://en.wikipedia.org/wiki/OBD-II_PIDs to add your own ***************/
//char _name[10], char _units[10], OBD_PID pid,  uint8_t OBD_PID_SIZE size, bool _signed, OBD_MODE_REQ mode, float32 slope,
// float32 offset, cAcquireCAN *, extended ID;

  cOBDParameter OBD_Speed(      "Speed "        , " KPH"		,  SPEED       , _8BITS,   false,   CURRENT,  1,      0,  &CANport0, false);
  cOBDParameter OBD_EngineSpeed("Engine Speed " , " RPM"		,  ENGINE_RPM  , _16BITS,  false,   CURRENT,  0.25,   0,  &CANport0, false);
  cOBDParameter OBD_Throttle(   "Throttle "     , " %"  		,  THROTTLE_POS, _8BITS,   false,   CURRENT,  0.3922, 0,  &CANport0, false);
  cOBDParameter OBD_Coolant(    "Coolant "      , " C"  		,  COOLANT_TEMP, _8BITS,   false ,  CURRENT,  1,    -40,  &CANport0, false);
  cOBDParameter OBD_EngineLoad( "Load "         , " %"  		,  ENGINE_LOAD , _8BITS,   false,   CURRENT,  0.3922, 0,  &CANport0, false);
  cOBDParameter OBD_DTCError(   "DTC_Error "   , " /"  		,  FREEZE_DTC  , _16BITS,   false ,  CURRENT,  1,    -40,  &CANport0, false);

    
  cCANFrame  RAW_CAN_Frame1;
  cCANFrame  RAW_CAN_Frame2;
void setup()
{
  delay(2000); //allow USB time to settle
	//output pin that can be used for debugging purposes

	//start serial port 
	Serial.begin(115200);

	//debugging message for monitor to indicate CPU resets are occuring
	Serial.println("System Reset");
       

  RAW_CAN_Frame1.ID = 0x100;
  RAW_CAN_Frame1.rate  = _5Hz_Rate;
  
  RAW_CAN_Frame2.ID = 0x200;
  RAW_CAN_Frame2.rate  = _10Hz_Rate;

  CANport0.addMessage(&RAW_CAN_Frame1, TRANSMIT);
  CANport0.addMessage(&RAW_CAN_Frame2, RECEIVE);
  
  //set up the transmission/reception of messages to occur at 500Hz (2mS) timer interrupt
  Timer3.attachInterrupt(CAN_RxTx).setFrequency(500).start();
}


UINT8 i;
UINT32 maxTime;


void loop()
{
  // this single byte should continously change in our raw CAN transmissions
  RAW_CAN_Frame1.U.b[0] = i;
  RAW_CAN_Frame2.U.b[7] = i;
  i = i++;

  //print out our latest OBDII data
   Serial.print(OBD_Speed.getName()); 
  Serial.print(OBD_Speed.getData());
  Serial.println(OBD_Speed.getUnits()); 
  
  Serial.print(OBD_EngineSpeed.getName()); 
  Serial.print(OBD_EngineSpeed.getData());
  Serial.println(OBD_EngineSpeed.getUnits()); 
    
  Serial.print(OBD_Throttle.getName()); 
  Serial.print(OBD_Throttle.getData());
  Serial.println(OBD_Throttle.getUnits()); 
    
  Serial.print(OBD_Coolant.getName()); 
  Serial.print(OBD_Coolant.getData());
  Serial.println(OBD_Coolant.getUnits()); 
  

  Serial.print(OBD_EngineLoad.getName()); 
  Serial.print(OBD_EngineLoad.getData());
  Serial.println(OBD_EngineLoad.getUnits()); 

  Serial.print(OBD_DTCError.getName()); 
  Serial.print(OBD_DTCError.getData());
  Serial.println(OBD_DTCError.getUnits()); 



  //pass control to other task
  delay(1000);
}

//this is our timer interrupt handler, called at XmS interval
void CAN_RxTx()
{
  //run CAN acquisition schedulers on both ports including OBD and RAW CAN mesages (RX/TX)     
  CANport0.run(TIMER_2mS);
}