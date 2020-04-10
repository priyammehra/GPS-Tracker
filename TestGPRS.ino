#include<SoftwareSerial.h>
#include<LowPower.h>

SoftwareSerial gprs(8, 9); //(rx,tx pin)
SoftwareSerial  gps(6, 7);


const String THINGSAPIKEY = "EFSN34Q4FWKXZUZ9";
const int PIEZO = 2;
const unsigned long TIMEPERIOD = 60000; //60 min or 1 hour
const unsigned long UPLOADTIME = 20000; //20 min
#define PHONE "+918555547340"

float longi = 28.6106;
float lati = 77.3542;



void setup() {
  Serial.begin(9600);
  gprs.begin(9600);
  gps.begin(9600);
  
  pinMode(PIEZO, INPUT); // so that are pin always remains high and not catch any rogue 0 and 1s, it may happen
  attachInterrupt(digitalPinToInterrupt(PIEZO), sleepModeOFF, HIGH );
  
  gprs.println("ATE1");//echo turned on
  readGPRS(1000);
  Serial.println("***********************************");

  Serial.println("Going Under Sleep Mode.");
  sleepModeON();

}



void loop(){

  timelyUpdates();
    
  }


void uploadData() {

  gprs.println("AT+CIPSHUT");//explicitly turn off any previously open TCP connections
  readGPRS(1000);
  Serial.println("***********************************");

  gprs.println("AT+CSTT=\"airtelgprs.com\"");//feeding access point name
  readGPRS(1000);
  Serial.println("***********************************");

  gprs.println("AT+CIICR");//brings up the wireless connectivity
  readGPRS(1000);
  Serial.println("***********************************");

  gprs.println("AT+CIFSR");//gets a local ip assigned
  readGPRS(1000);
  Serial.println("***********************************");

  gprs.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//Establish TCP conn. with server adds at specified port (TCP, server, port)
  readGPRS(1000);
  Serial.println("***********************************");

  gprs.println("AT+CIPSEND");//start sending data
  delay(1000);
  
  readGPRS(1000);
  String requestToCloud = "GET https://api.thingspeak.com/update?api_key="+String(THINGSAPIKEY)+"&field2=" +String(longi)+"&field1=" +String(lati);
   
  gprs.println(requestToCloud);
  delay(1000);
  gprs.println((char)26);
  delay(1000);
  readGPRS(1000);
  Serial.println("***********************************");


  gprs.println("AT+CIPSHUT");
  readGPRS(1000);
  Serial.println("***********************************");

  
}


void readGPRS(int x){
  delay(x);
  while(gprs.available()!=0){
     Serial.write(gprs.read());
     delay(100); 
  }
 }


void checkGPRS(){
  gprs.println("AT");//module responding or not
  readGPRS(1000);
  Serial.println("***********************************");
 
  gprs.println("AT+CPIN?");//if sim is detected or not
  readGPRS(1000);
  Serial.println("***********************************");
  
  gprs.println("AT+CSPN?");//service provider name
  readGPRS(1000);
  Serial.println("***********************************");
}

void sleepModeON(){
  gprs.println("AT+CSCLK=");//enable GPRS sleep mode, SleepMode 2 is being used
  readGPRS(1000);

  /*  put arduino to sleep, NOTHING EXCEPT watchdog timers, ext.Interrupts and 2wire serial interface will work
      will wake up at any external interrupt so we will use external interrupt pin 2  */
  LowPower.powerDown(SLEEP_FOREVER ,ADC_OFF, BOD_OFF);
  }

void sleepModeOFF(){
  /*  As soon as any external interrupt is triggered, Arduino will come out of sleep
      hence no need to add anything for Arduino to wake up*/  
  detachInterrupt(digitalPinToInterrupt(PIEZO)); //to avoid any unncessary calls   
  gprs.println("AT+CSCLK=0"); //Disables GPRS sleep mode
  readGPRS(1000);
  
  }

void timelyUpdates(){

unsigned long startTime1, startTime2, 
                        endTime1, endTime2;
    
startTime1 = millis();
startTime2 = startTime1;
endTime1 = startTime1;
endTime2 = startTime1; 
 
  while((endTime1-startTime1) <= TIMEPERIOD){
    
    endTime1=millis();
    endTime2=millis();
    
    if((endTime2-startTime2) >= UPLOADTIME)
      {
        startTime2=millis();
        uploadData();
        sendSMS();
       }
    
    }

  attachInterrupt(digitalPinToInterrupt(PIEZO), sleepModeOFF, HIGH );
  sleepModeON();
  
  
}



void sendSMS(){
  
  gprs.println("AT+CMGF=1");   //text mode enabled
  readGPRS(1000);

  gprs.println("AT+CMGS=\"PHONE\"\r");  //receivers mobile no.
  readGPRS(1000);

  gprs.println("Longitude: "+String(longi)+"Latitude: "+String(lati)); //data to send
  delay(100);
  gprs.println((char)26);
  readGPRS(1000);

  gprs.println("AT+CMGF=0");   //text mode diabled
  readGPRS(1000);

  
  }
