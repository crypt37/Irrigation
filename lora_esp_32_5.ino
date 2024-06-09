#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include "RS-FEC.h"
#include <Bonezegei_DHT22.h>
#include <string.h>
#include <Bonezegei_DS1307.h>
#include <time.h>
File myFile;

const int CS = 4;
String file_name="/test.txt";

bool motor_state =false;
SoftwareSerial LoRa(33, 32);
const int trigPin = 26;
const int echoPin = 14;

Bonezegei_DHT22 dht(25);
const int relayPin = 27;


 
const int msglen = 30;  
const uint8_t ECC_LENGTH = 10;  //Max message lenght, and "gurdian bytes", Max corrected bytes ECC_LENGTH/2
char message_frame[msglen]; // The message size would be different, so need a container
char repaired[msglen];    char encoded[msglen + ECC_LENGTH];

RS::ReedSolomon<msglen, ECC_LENGTH> rs;



hw_timer_t *My_timer=NULL;

void save_data_csv(String *dataString);
int INTERVAL=60;



const int address=5;
int current_distance;

Bonezegei_DS1307 rtc(0x68);
bool motor_state_4=false;
#define SOUND_SPEED 0.034
unsigned long previousMillis = 0;
long duration;
float distanceCm;
float tempDeg,hum;
volatile bool sendDataFlag = false;
String payload_data;
void IRAM_ATTR onTimer() {
  
sendDataFlag=true;

}
void WriteFile(const char * path, const char * message){
  myFile = SD.open(path, FILE_APPEND);
  
  if (myFile) {
    Serial.printf("Writing to %s ", path);
    myFile.println(message);
    myFile.close(); 
    Serial.println("completed.");
  } 
 
  else {
    Serial.println("error opening file ");
    Serial.println(path);
  }
}
void ReadFile(const char * path){
  myFile = SD.open(path);
  if (myFile) {
     Serial.printf("Reading file from %s\n", path);
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close(); // close the file:
  } 
  else {
    Serial.println(F("error opening test.txt"));
  }
}

int calculate_distance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED / 2;
  return int(distanceCm);
}
 
void setup() {
    Serial.begin(9600);
  LoRa.begin(115200);
  delay(5000);
  rtc.begin();
  if (!SD.begin(CS)) {
    Serial.println("initialization failed!");
    return;
  }
  delay(2000);
  
ReadFile("/test2.txt");

//    pinMode(relayPin, OUTPUT);
//  digitalWrite(relayPin, HIGH);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  
    LoRa.println("AT");
  delay(1000);
  LoRa.println("AT+FACTORY");
  delay(1000);
  LoRa.println("AT+ADDRESS=5");
  delay(2000);
//   LoRa.println("AT+BAND=868500000");
//  
//  LoRa.println("AT+PARAMETER=10,7,4,7");
delay(1000);
////  LoRa.println("AT+CPIN=12345678912345678912345678912345");



  dht.begin();
  My_timer = timerBegin(0, 80, true);
timerAttachInterrupt(My_timer, &onTimer, true);
timerAlarmWrite(My_timer, 1000000*INTERVAL, true);
timerAlarmEnable(My_timer);

}


void loop() {
   
  delay(3000);



  
  String incomingString;
    if (LoRa.available())
    {
        incomingString = LoRa.readString();
       
        Serial.println(incomingString);
        
        if (incomingString.indexOf("+RCV=4") != -1 ) 
        {
         
        if (incomingString.indexOf("+RCV=3") != -1 ) 
        {
          payload_data=incomingString;
          String data_to_send=payload_data.substring(incomingString.indexOf("+RCV=3"));
          Serial.println(data_to_send);
          save_data_csv(&data_to_send);
         }

      else if (incomingString.indexOf("+RCV=2") != -1 )
        {
  
         payload_data=incomingString;
          String data_to_send=payload_data.substring(incomingString.indexOf("+RCV=2"));
          Serial.println(data_to_send);
          save_data_csv(&data_to_send);
  
          }

          
      else if (incomingString.indexOf("+RCV=1") != -1 )
        {
  
         payload_data=incomingString;
          String data_to_send=payload_data.substring(incomingString.indexOf("+RCV=1"));
          Serial.println(data_to_send);
          save_data_csv(&data_to_send);
  
          }
          
     
         else {
              
               payload_data=incomingString;
               Serial.println(payload_data);
              save_data_csv(&payload_data);
         exit;
         }
     
        }
        else {
          exit;
        }
      
    }
    else {
      exit;
    }
   

current_distance= calculate_distance();

 if (current_distance <18 && sendDataFlag && (motor_state_4==true) ){
  Serial.println(current_distance);
  LoRa.println("AT+SEND=4,16,OFFOFFOFFOFFOFFOFF\n\n\n");
  delay(300);
  sendDataFlag=false;
}

 if ( current_distance>=23 &&  sendDataFlag && (motor_state_4==false) )
 {
  Serial.print(current_distance+"state");
  Serial.println(motor_state_4);
  LoRa.println("AT+SEND=4,16,ONONONONONONONONON\n\n\n");
  delay(300);
  sendDataFlag=false;
}

delay(50);
//   send_data(); 
}



String time_now(){

  if (rtc.getTime()) {
 String time_ima= String(rtc.getMonth()) +"/" +String(rtc.getDate())   +"/" +String(rtc.getYear()) +","+ String(rtc.getHour())  +":"+ String(rtc.getMinute())  +":"+ String( rtc.getSeconds());
return time_ima;
}
 
}



void send_data() {

  current_distance = calculate_distance();
  if (!dht.getData()) {                         
    Serial.println(F("Problem in DH22"));
    delay(200);
  }

String state_of_motor=String(motor_state ? "ON" : "OFF");
  
Serial.println(time_now() + ","+current_distance +","+ dht.getTemperature() +"," +  dht.getHumidity() +"," +state_of_motor  + ","+ address);


  WriteFile(file_name.c_str(), ( time_now() + ","+current_distance +","+ dht.getTemperature() +"," +  dht.getHumidity() +","+ state_of_motor+","+address ).c_str() );

}

void save_data_csv(String *dataString) {

String data_received=*dataString;



int startIdx, endIdx;
String distanceStr, motor_state,sender_uid ,tempStr, humidityStr;

// Extract sender UID
startIdx = dataString->indexOf("+RCV=") + 5;
endIdx = dataString->indexOf(",", startIdx);
if (startIdx != -1 && endIdx != -1) {
    sender_uid = dataString->substring(startIdx, endIdx);
} else {
    // Invalid or incomplete data
    sender_uid = "N/A"; 
}

String messag;
if (data_received.indexOf("+RCV=4")!=-1){
   int firstCommaIndex = data_received.indexOf("+RCV=4"); // Find the index of the first comma
      
     int startOfMessage = firstCommaIndex + 7; // Start of the message part
    int secondComma = data_received.indexOf(",",startOfMessage); 
    int thirdComma = data_received.indexOf(",",secondComma); 
    int endOfMessage = data_received.indexOf(",,", thirdComma+1); // End of the message part
    int endOfpayload = data_received.indexOf(",,", endOfMessage+6); // End of the message part
     messag = data_received.substring(thirdComma+1, endOfMessage);

  }
else {
  
     int firstCommaIndex = data_received.indexOf("+RCV="); // Find the index of the first comma
      
     int startOfMessage = firstCommaIndex + 6; // Start of the message part

    int endOfMessage = data_received.indexOf(",,", startOfMessage+1); // End of the message part
    int endOfpayload = data_received.indexOf(",,", endOfMessage+6); // End of the message part
     messag = data_received.substring(startOfMessage+1, endOfpayload);
  
  }



 for (uint i = 0; i<messag.length(); i++) 
 {
  encoded[i]=messag[i];
  }




rs.Decode(encoded, repaired);


  String payload="";
     for (int i=0;i<sizeof(repaired);i++)
{
  payload+=((repaired[i]));

  }
  payload+=",--\0";


for(int i =0;i<dataString->length();i++){
  Serial.print(payload[i]);
  
  }
  Serial.println("");

 
dataString=&payload;

startIdx = dataString->indexOf("D:") + 2;
endIdx = dataString->indexOf(",", startIdx);
if (startIdx != -1 && endIdx != -1 ) {
    distanceStr = dataString->substring(startIdx, endIdx);
} else 
{
    
    distanceStr = "N/A"; 
}

// Extract motor state
startIdx = dataString->indexOf("M:") +2;
endIdx =data_received.indexOf(',', startIdx+1); 
if (startIdx != -1 && endIdx != -1) {
    motor_state = dataString->substring(startIdx, endIdx+3);

} else {
    
    motor_state ="N/A"; 
}




// Extract temperature value
startIdx = dataString->indexOf("t:") + 2;
endIdx = dataString->indexOf(",", startIdx);
if (startIdx != -1 && endIdx != -1 ) {
    tempStr = dataString->substring(startIdx, endIdx);
} else {
  
    tempStr = "N/A"; 
}

// Extract humidity value
startIdx = dataString->indexOf("H:") + 2;
endIdx = dataString->indexOf(",", startIdx);
if (startIdx != -1 && endIdx != -1) {
    humidityStr = dataString->substring(startIdx, endIdx);
} 

else {
   
    humidityStr = "N/A"; 
}
if(humidityStr.toInt()>100) {
  humidityStr = "N/A"; 
}


Serial.print("Distance: ");
Serial.println(distanceStr);
Serial.print("Temperature: ");
Serial.println(tempStr);
Serial.print("Humidity: ");
Serial.println(humidityStr);
Serial.print("motor_state:");
Serial.println(motor_state);
Serial.print("sender_uid:");
Serial.println(sender_uid);

if (distanceStr != "N/A" && motor_state != "N/A" ) {
    WriteFile(file_name.c_str(), (time_now() + "," + distanceStr +","+ tempStr +"," +  humidityStr +","+ motor_state+"," +sender_uid ).c_str() );
Serial.print("Distance: ");
Serial.println(distanceStr);
Serial.print("Temperature: ");
Serial.println(tempStr);
Serial.print("Humidity: ");
Serial.println(humidityStr);
Serial.print("motor_state:");
Serial.println(motor_state);
Serial.print("sender_uid:");
Serial.println(sender_uid);
if ( sender_uid=="4" && (motor_state=="OFF"|| motor_state=="ON") )
{
  motor_state_4= (motor_state =="ON")? true :false ;
  
}


Serial.println("time_now");
Serial.println(time_now());
Serial.println("------");
  
} else {
    
Serial.println("Invalid_data");
}
send_data();

}
