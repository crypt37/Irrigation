#include <SoftwareSerial.h>

#include <SPI.h>

#include <SD.h>
#include "RS-FEC.h"
#include <Bonezegei_DHT22.h>

char message[] = "";
const int msglen = 30;  
const uint8_t ECC_LENGTH = 10;  //Max message lenght, and "gurdian bytes", Max corrected bytes ECC_LENGTH/2
char message_frame[msglen]; // The message size would be different, so need a container
char repaired[msglen];    char encoded[msglen + ECC_LENGTH];

RS::ReedSolomon<msglen, ECC_LENGTH> rs;

const int trigPin = 14;
const int echoPin = 26;
const int address = 2;
const int relaypin = 23;
int current_distance;
SoftwareSerial LoRa(33, 32);
Bonezegei_DHT22 dht(25);
bool motor_state_3 = false;
#define SOUND_SPEED 0.034
#define INTERVAL 30

unsigned long previousMillis = 0;
bool sendDataFlag = false;
bool flag = false;
String data_received;
bool motor_state = false;
hw_timer_t * My_timer = NULL;

void IRAM_ATTR onTimer() {
  sendDataFlag = true;
}

void setup() {
  Serial.begin(9600);
  LoRa.begin(115200);
  //  delay(5000);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relaypin, OUTPUT);
  digitalWrite(relaypin, HIGH);
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, & onTimer, true);
  timerAlarmWrite(My_timer, 1000000 * INTERVAL, true);
  timerAlarmEnable(My_timer);

  LoRa.println("AT");
  delay(1000);
  LoRa.println("AT+FACTORY");
  delay(1000);
  LoRa.println("AT+ADDRESS=4");
  delay(2000);
//  LoRa.println("AT+BAND=868500000");

  //  LoRa.println("AT+PARAMETER=10,7,4,7");
  delay(2000);
  dht.begin();
}

void loop() {  

  if (current_distance >= 20 && sendDataFlag && (motor_state_3 == false)) {
    Serial.println(current_distance);
    LoRa.println("AT+SEND=3,16,ONONONONONONONONON");
    delay(500);
  }

  if (sendDataFlag) 
  {
    send_data();
    sendDataFlag = false;
  }

  String incomingString;
  if (LoRa.available()) {
    incomingString = LoRa.readString();
    Serial.println(incomingString);

 

      if (incomingString.indexOf("ONON") != -1 && incomingString.indexOf("+RCV=5") != -1) 
      {
        motor_state = true;
        Serial.print("on request received");
        digitalWrite(relaypin, LOW);
        delay(1000);

      } 
      else if (incomingString.indexOf("OFFOFF") != -1 && incomingString.indexOf("+RCV=5") != -1) 
      {
        Serial.print("off request received");
        motor_state = false;
        digitalWrite(relaypin, HIGH);
        delay(1000);
      } 
      else {

        if ((incomingString.length() > 20) && (incomingString.indexOf("+RCV=3") != -1)) 
        {
          

          if (incomingString.indexOf("+RCV=2") != -1 )
{
  
        
          String data_to_send=incomingString.substring(incomingString.indexOf("+RCV=2"));
           data_received=data_to_send;
           
          Serial.println(data_to_send);
          send_received_data();
          
  
  }

  else if ( (incomingString.length() > 20) && (incomingString.indexOf("+RCV=1") != -1))
  {
    
      String data_to_send=incomingString.substring(incomingString.indexOf("+RCV=1"));
           data_received=data_to_send;
          Serial.println(data_to_send);
          send_received_data();
    
    }
    else {
       data_received = incomingString;
      send_received_data();
      
        
          int startIdx = data_received.indexOf("M:") + 2;
          int endIdx = data_received.indexOf(",", startIdx);
          
          if (startIdx != -1 && endIdx != -1) 
          
          {
            String motor_state = data_received.substring(startIdx, endIdx);
            motor_state_3 = (motor_state == "ON") ? true : false;
            
            
          }
      }
    
        }
else {
  Serial.println(incomingString);

          }
        }
      }

  
  delay(50);

}

int calculate_distance() {
//  digitalWrite(trigPin, LOW);
//  delayMicroseconds(2);
//  digitalWrite(trigPin, HIGH);
//  delayMicroseconds(10);
//  digitalWrite(trigPin, LOW);
//  long duration = pulseIn(echoPin, HIGH);
//  float distanceCm = duration * SOUND_SPEED / 2;

 int randomNumber = random(1, 101); 
  
  return int(randomNumber);
}

String time_now() {
  unsigned long int currentTime = millis();
  return String(currentTime);
}

int MAX_DISTANCE = 5000;


void send_data() {

  int distance = calculate_distance();


float temperature,humidity;
  if (dht.getData()) {    
      temperature = dht.getTemperature();
  
   humidity = dht.getHumidity();

    Serial.printf("Temperature: %0.1lf°C  %0.1lf°F Humidity:%d \n", temperature, humidity);
  }
  delay(2000);

 
  String payload = "D:" + String(distance) +
    ",t:" + String(int(temperature)) +
    ",H:" + String(int(humidity)) +
    ",M:" + String(motor_state ? "ON" : "OFF");


  int payload_length = 2 * payload.length();


    memset(message_frame, 0,payload.length()); 
  rs.Encode(payload.c_str(), encoded);  

payload="";
for (int i=0;i<sizeof(encoded);i++)
{
  payload+=encoded[i];
}

  
  String cmd = "AT+SEND=5," + String(payload.length()) + "," + payload + ",,,,,,,\n";

  LoRa.println(cmd);
  Serial.println(cmd);
  delay(random(2000)+1000);

}

void send_received_data() {
int startIdx,endIdx;
String sender_uid;
startIdx = data_received.indexOf("+RCV=") + 5;
endIdx = data_received.indexOf(",", startIdx);
if (startIdx != -1 && endIdx != -1) {
    sender_uid = data_received.substring(startIdx, endIdx);
} 

    
    int firstCommaIndex = data_received.indexOf("+RCV=,"); // Find the index of the first comma
      
     int startOfMessage = firstCommaIndex + 10; // Start of the message part
    int secondComma = data_received.indexOf(",",startOfMessage); 
    int thirdComma = data_received.indexOf(",",secondComma); 
    int endOfMessage = data_received.indexOf(",,", thirdComma+1); // End of the message part
    int endOfpayload = data_received.indexOf(",,", endOfMessage+6); // End of the message part
    String messag = data_received.substring(thirdComma+1, endOfMessage);



 for (uint i = 0; i<messag.length(); i++) 
 {
  encoded[i]=messag[i];
    
  Serial.print(encoded[i]);
  }

rs.Decode(encoded, repaired);
 memset(message_frame, 0,sizeof(repaired)); 
 rs.Encode(repaired, encoded);
 
  String payload="";
     for (int i=0;i<sizeof(encoded);i++)
{
payload+=((encoded[i]));

  }
  Serial.println("encoded message");

  String cmd2 = "AT+SEND=5," + String ( 2*payload.length()) +",+RCV=" +sender_uid+"," + payload + ",,,,,,,,,\n";

  LoRa.println(cmd2);
  delay(random(2000) + 1000);
  Serial.println(cmd2);
  
}
