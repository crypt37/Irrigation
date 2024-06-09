#include <SoftwareSerial.h>
SoftwareSerial LoRa(33, 32);
#include <string.h>
#include <Bonezegei_DHT22.h>
#include <time.h>
#include <Base64.h>


#include "RS-FEC.h"
char message[] = "";
const int msglen = 30;  
const uint8_t ECC_LENGTH = 10;  //Max message lenght, and "gurdian bytes", Max corrected bytes ECC_LENGTH/2
char message_frame[msglen]; // The message size would be different, so need a container
char repaired[msglen];    char encoded[msglen + ECC_LENGTH];

RS::ReedSolomon<msglen, ECC_LENGTH> rs;


#include <Base64.h>
const int trigPin = 27;
const int echoPin = 26;
const int relayPin = 32;
bool motor_state=false;
bool flag=false;
Bonezegei_DHT22 dht(13);
String data_received;
#define SOUND_SPEED 0.034
#define ADDRESS 3
hw_timer_t *My_timer = NULL;
bool sendDataFlag = false;
bool motor_state_2 =false;
void IRAM_ATTR onTimer() {
int payload_length;
String *payload;
sendDataFlag=true;

}
int INTERVAL=20;
long duration;
float distanceCm;
float tempDeg, hum;
  

 

void setup() {

   
  // put your setup code here, to run once:
  Serial.begin(9600);
  LoRa.begin(115200);
  My_timer = timerBegin(0, 80, true);
timerAttachInterrupt(My_timer, &onTimer, true);
timerAlarmWrite(My_timer, 1000000 *INTERVAL, true);
timerAlarmEnable(My_timer);

  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relayPin, OUTPUT);
   digitalWrite(relayPin, HIGH);

    LoRa.println("AT");
  delay(1000);
  LoRa.println("AT+FACTORY");
  delay(1000);
  LoRa.println("AT+ADDRESS=3");
  //  LoRa.println("AT+CPIN=12345678912345678912345678912345");
  delay(2000);
LoRa.println("AT+ADDRESS=3");
 delay(2000);
  
//  LoRa.println("AT+PARAMETER=10,7,4,7");
  delay(2000);
  dht.begin();


}

unsigned long previousMillis = 0;
void loop() {




  String incomingString;
  if (LoRa.available()) 
  {
    incomingString = LoRa.readString();
    Serial.println(incomingString);
    delay(500);
    
      if ( incomingString.indexOf("+OK") == -1) 
      {
    if ( (incomingString.indexOf("ONON")) != -1 && (incomingString.indexOf("+RCV=4") != -1  )) 
    {
      Serial.println("data on ");
      Serial.println(incomingString);
      motor_state=true;
      digitalWrite(relayPin, LOW);
      delay(500);

    }
       if ( incomingString.indexOf("OFFOFF") != -1 && (incomingString.indexOf("+RCV=4") != -1 ) ) {
      Serial.println("data off ");
      Serial.println(incomingString);
      motor_state=false;
  
      digitalWrite(relayPin, HIGH);
      delay(500);
    }
    else if (incomingString.length() > 25 && (incomingString.indexOf("+RCV=2") != -1 ) )
    {
        data_received=incomingString;   
        int startIdx = data_received.indexOf("M:") + 2;
        int endIdx = data_received.indexOf(",", startIdx);
        if (startIdx != -1 && endIdx != -1) 
        {
           String  motor_state = data_received.substring(startIdx, endIdx);
            motor_state_2 = (motor_state =="ON")? true :false;
        } 
        send_received_data(); 
        exit; 
    }
    else if ( (incomingString.length() > 25 ) && (incomingString.indexOf("+RCV=1") != -1 ) )
    {
        data_received=incomingString.substring(incomingString.indexOf("+RCV=1"));;    
        send_received_data(); 
        exit; 
    }

    
    else {
      exit; 
    }
      }
      else {
        Serial.print(incomingString);
        exit;
      }


  }

  
  else {
    
    if (sendDataFlag){
  send_data();
  sendDataFlag=false;
  }
    
    }


delay(10);

}

int calculate_distance() {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED / 2;

  //  Serial.print("Distance (cm): ");
  //  Serial.println(int(floor(distanceCm)));

  return int(distanceCm);

}



void send_data() {

   
  int distance = calculate_distance();
  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();


   
     String pay = "D:" + String(distance) +
                     ",t:" + String(int(temperature)) +
                     ",H:" + String(int(humidity)) +
                     ",M:" + String(motor_state ? "ON" : "OFF"); 
      
    // Calculate payload length
 

   memset(message_frame, 0,pay.length()); 
  rs.Encode(pay.c_str(), encoded);
  pay="";
  for (int i=0;i<sizeof(encoded);i++)
{
  pay+=((encoded[i]));

  }
 
  String cmd = "AT+SEND=4," +String( 2*pay.length()) + "," + pay + ",,,,,\n\n" ;

 
    LoRa.println(cmd);
  Serial.println(cmd);
    delay(random(2000)+1000);
    
}
void send_received_data() {




//  for(uint i = 0; i <= msglen; i++) {    message_frame[i] = messagi[i];     } // Fill with the message
// 
//
    int firstCommaIndex = data_received.indexOf("+RCV=2,"); // Find the index of the first comma
     
    int startOfMessage = firstCommaIndex + 7; // Start of the message part
    int secondComma = data_received.indexOf(",",startOfMessage); 
    int thirdComma = data_received.indexOf(",",secondComma); 
    int endOfMessage = data_received.indexOf(",,", thirdComma+1); // End of the message part
    int endOfpayload = data_received.indexOf(",,", endOfMessage+6); // End of the message part
    String messag = data_received.substring(thirdComma+1, endOfMessage);


 for (uint i = 0; i<messag.length(); i++) 
 {
  encoded[i]=messag[i];

  }
rs.Decode(encoded, repaired);
//for(uint i = 0; i < sizeof(repaired); i++) { Serial.print(repaired[i]);}Serial.println(""); 
 
  int payload_length = 2 * data_received.length();
   memset(message_frame, 0,sizeof(repaired)); 
  rs.Encode(repaired, encoded);

  String payload="";
     for (int i=0;i<sizeof(encoded);i++)
{
  payload+=((encoded[i]));

  }
  String cmd2 = "AT+SEND=4," +String( 2*payload.length())+ ",+RCV=2,"+payload+",,,,,\n\n" ;
  LoRa.println(cmd2);
  delay(random(2000) + 1000);
//  Serial.println(cmd2);
}
