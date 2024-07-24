#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include "RS-FEC.h"
#include <Bonezegei_DHT22.h>
#include <string.h>
#include <Bonezegei_DS1307.h>
#include <time.h>
#include <EEPROM.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_task_wdt.h>
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define CONFIG_ESP32_ENABLE_COREDUMP 0

// Initialize watchdog with a longer timeout (in seconds)

TaskHandle_t displayTaskHandle;
TaskHandle_t dataTaskHandle;

File myFile;
// .cpp file
int outputA = 27;
int outputB = 26;
int button = 5;
int counter = -1;

#include "display_functions.h"
#include "helper_functions.h"

const int maxLevel = 100;  // Maximum water level in cm
const int minLevel = 0;    // Minimum water level in cm
int currentLevel = 50;     // Starting water level in cm
volatile int interruptCounter;
const int CS = 4;
String file_name = "/test.txt";
String logpath= "/logs.txt";
long prev_duration = 0;
bool motor_state = false;
bool exitMenu=false;
bool passwordCorrectFlag=false;
SoftwareSerial LoRa(33, 32);
const int trigPin = 12;
const int echoPin = 13;
const int add3 = 5;
const int add1 = 35;
const int add2 = 34;
Bonezegei_DHT22 dht(25);
const int relayPin = 27;

const int msglen = 30;
const uint8_t ECC_LENGTH = 10;  // Max message length, and "guardian bytes", Max corrected bytes ECC_LENGTH/2
char message_frame[msglen];      // The message size would be different, so need a container
char repaired[msglen];
char encoded[msglen + ECC_LENGTH];
char password[5]="    ";
char Band[5]="    ";
char Address[5]="    ";
char CurrentData[10][8];

RS::ReedSolomon<msglen, ECC_LENGTH> rs;

hw_timer_t *My_timer = NULL;

void save_data_csv(String *dataString);
int INTERVAL = 20;
int INTERVAL2 = 10000;

int address = 5;
int current_distance;
int changed_in_distance = 0;
Bonezegei_DS1307 rtc(0x68);
String motor_state_4 = "OFF";

const float SOUND_SPEED = 0.034;

unsigned long previousMillis = 0;
long duration;
float distanceCm;
float tempDeg, hum;
volatile bool sendDataFlag = false;
volatile bool sendDataFlag2 = false;
String payload_data;

void IRAM_ATTR onTimer() {
  sendDataFlag = true;
}

void setup() {
  esp_task_wdt_init(50, true);
  Serial.begin(9600);
  EEPROM.begin(32);
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  pinMode(button, INPUT_PULLUP);
   display.begin(i2c_Address, true);
      delay(1000);
  display.display(); 
Address[3]=EEPROM.read(0); 
for (int i=0;i<=3;i++){Band[i]=EEPROM.read(i+1);  CurrentData[7][i]=EEPROM.read(i+1);}


  
  delay(250);

  
  delay(1000);


  LoRa.begin(115200);
  delay(1000);
 
  if (!SD.begin(CS)) {
    Serial.println("SD card initialization failed!");
    
     WriteFile(logpath.c_str(),  String(time_now() + ", ER1").c_str());
  
  }
  delay(2000);

  // ReadFile("/test.txt");

  // pinMode(relayPin, OUTPUT);
  // digitalWrite(relayPin, HIGH);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  LoRa.println("AT");
  delay(1000);
  if (!LoRa.available()) {
    Serial.println("LoRa initialization failed");
    WriteFile( logpath.c_str(), String(time_now() + ", ER2").c_str());
    return;
  }

  LoRa.println("AT+ADDRESS="+ String(Address[3]));
   delay(3000);
   LoRa.println("AT+BAND="+String(Band[0]) +String( Band[1] )+ String(Band[2])  +String(Band[3])  + "00000");
    delay(1000);
   LoRa.println("AT+PARAMETER=10,7,4,7");
   delay(100);
    LoRa.println("AT+ADDRESS?");
    delay(1000);
    LoRa.println("AT+BAND?");
    delay(1000);
  // LoRa.println("AT+CPIN=12345678912345678912345678912345");
   dht.begin(); 


  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 1000000 * INTERVAL, true);
  timerAlarmEnable(My_timer);

  Serial.println(time_now());
// /while(!exitMenu) { Oledmenu();}
  Serial.println("exit menu");
  
   rtc.begin();

  xTaskCreatePinnedToCore (
    displayTask,     // Function to implement the task
    "displayTask",   // Name of the task
    4096,      // Stack size in bytes
    NULL,      // Task input parameter
    10,         // Priority of the task
    &displayTaskHandle,      // Task handle.
    1          // Core where the task should run
  );
    xTaskCreatePinnedToCore (
    dataTask,     // Function to implement the task
    "dataTask",   // Name of the task
    4096,      // Stack size in bytes
    NULL,      // Task input parameter
    10,         // Priority of the task
    &dataTaskHandle,      // Task handle.
    1          // Core where the task should run
  );
//  if (xTaskCreate(dataTask, "DataTask", 4096*4, NULL, 1, &dataTaskHandle) != pdPASS) {
//    Serial.println("Failed to create DataTask");
//  }

vTaskStartScheduler();
}
//void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
//{
//    static StaticTask_t xIdleTaskTCB;
//    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];
//
//    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
//    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
//    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
//}

void checkHeap() {
  UBaseType_t freeHeap = xPortGetFreeHeapSize();
  Serial.print("Free heap: ");
  Serial.println(freeHeap);
}
void displayTask(void *pvParameters) {
Serial.println("in display");
  while (1) {
    Serial.println("Display task running");
     Serial.println (xTaskGetTickCount());


       // lock the mutex (busy waiting)

      displayloopfunciton();
     
      counter = -1;
      delay(300);
      display.clearDisplay();
      exitMenu = false;
      display.clearDisplay();
      exitMenu = false;

//    portEXIT_CRITICAL (&taskMux); // unlock the mutex
    vTaskDelay(pdMS_TO_TICKS(10)); // Add a small delay to yield control to other tasks
  }
}


void loop() {


  //chill
}
void dataTask(void *pvParameters) {
 

  Serial.println("data task desu");
  for (;;) {
  char value;
 for(int i=0;i<=3;i++) {  Serial.println(EEPROM.get(1+i,value));}
Serial.println(EEPROM.read(0));

  Serial.println("in loop");
  String incomingString;
  if (LoRa.available()) {
    incomingString = LoRa.readString();
    Serial.println(incomingString);

    if (incomingString.indexOf("+RCV=6") != -1) {
      if (incomingString.indexOf("+RCV=3") != -1) {
        payload_data = incomingString;
        String data_to_send = payload_data.substring(incomingString.indexOf("+RCV=3"));
        Serial.println(data_to_send);
        save_data_csv(&data_to_send);
      } else if (incomingString.indexOf("+RCV=2") != -1) {
        payload_data = incomingString;
        String data_to_send = payload_data.substring(incomingString.indexOf("+RCV=2"));
        Serial.println(data_to_send);
        save_data_csv(&data_to_send);
      } else if (incomingString.indexOf("+RCV=1") != -1) {
        payload_data = incomingString;
        String data_to_send = payload_data.substring(incomingString.indexOf("+RCV=1"));
        Serial.println(data_to_send);
        save_data_csv(&data_to_send);
      } else {
        payload_data = incomingString;
        Serial.println(payload_data);
        save_data_csv(&payload_data);
        exit;
      }
    } else {
      exit;
    }
  } else {
 
//     WriteFile(logpath.c_str(),"ER3");
    exit;
  }

  current_distance = calculate_distance();
  delay(300);

  long duration_passed = millis();
  if (current_distance <= 18) {
    if ((duration_passed - prev_duration > INTERVAL2) && (motor_state_4 == "ON")) {
      prev_duration = duration_passed;
      Serial.print("OFF request sent");
      LoRa.println("AT+SEND=" + String(address - 1) + ",16,OFFOFFOFFOFFOFFOFF\n\n\n");
      delay(300);
    }
  }

  if (current_distance >= 19) {
    if ((duration_passed - prev_duration > INTERVAL2) && (motor_state_4 == "OFF")) {
      prev_duration = duration_passed;
      Serial.print("ON request sent");
      LoRa.println("AT+SEND=" + String(address - 1) + ",16,ONONONONONONONONON\n\n\n");
      delay(300);
    }

    if ((duration_passed - prev_duration > INTERVAL2) && (motor_state_4 == "SUS")) {
      Serial.println("motor state sus");
      prev_duration = duration_passed;
      delay(100);
    }
  }
  delay(50);
  // send_data();
  saveData();
   vTaskDelay(pdMS_TO_TICKS(10)); 
}

}



void save_data_csv(String *dataString) {
  String data_received = *dataString;
  int startIdx, endIdx;
  String distanceStr, motor_state, sender_uid, tempStr, humidityStr;

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
  if (data_received.indexOf("+RCV=4") != -1) {
    int firstCommaIndex = data_received.indexOf("+RCV=4");  // Find the index of the first comma
    int startOfMessage = firstCommaIndex + 7;  // Start of the message part
    int secondComma = data_received.indexOf(",", startOfMessage);
    int thirdComma = data_received.indexOf(",", secondComma);
    int endOfMessage = data_received.indexOf(",,", thirdComma + 1);  // End of the message part
    int endOfpayload = data_received.indexOf(",,", endOfMessage + 6);  // End of the message part
    messag = data_received.substring(thirdComma + 1, endOfMessage);
  } else {
    int firstCommaIndex = data_received.indexOf("CV=");  // Find the index of the first comma
    int startOfMessage = firstCommaIndex + 6;  // Start of the message part
    int endOfMessage = data_received.indexOf(",,", startOfMessage + 1);  // End of the message part
    int endOfpayload = data_received.indexOf(",,", endOfMessage + 6);  // End of the message part
    messag = data_received.substring(startOfMessage + 1, endOfpayload);
  }

  for (uint i = 0; i < messag.length(); i++) {
    encoded[i] = messag[i];
  }

  rs.Decode(encoded, repaired);

  String payload = "";
  for (int i = 0; i < sizeof(repaired); i++) {
    payload += ((repaired[i]));
  }

  for (int i = 0; i < dataString->length(); i++) {
    Serial.print(payload[i]);
  }
  Serial.println("");

  dataString = &payload;

  // Extract distance
  startIdx = dataString->indexOf("D:") + 2;
  endIdx = dataString->indexOf(",", startIdx);
  if (startIdx != -1 && endIdx != -1) {
    distanceStr = dataString->substring(startIdx, endIdx);
  } else {
    distanceStr = "N/A";
  }

  // Extract motor state
  startIdx = dataString->indexOf("M:");
  if (startIdx != -1) {
    motor_state = dataString->substring(startIdx + 2);  // Extract from tempo, not dataString
  } else {
    motor_state = "N/A";
  }

  // Extract temperature value
  startIdx = dataString->indexOf("t:") + 2;
  endIdx = dataString->indexOf(",", startIdx);
  if (startIdx != -1 && endIdx != -1) {
    tempStr = dataString->substring(startIdx, endIdx);
  } else {
    tempStr = "N/A";
  }

  // Extract humidity value
  startIdx = dataString->indexOf("H:") + 2;
  endIdx = dataString->indexOf(",", startIdx);
  if (startIdx != -1 && endIdx != -1) {
    humidityStr = dataString->substring(startIdx, endIdx);
  } else {
    humidityStr = "N/A";
  }
  if (humidityStr.toInt() > 100) {
    humidityStr = "N/A";
  }

  Serial.println((time_now() + "," + distanceStr + "," + tempStr + "," + humidityStr + "," + motor_state + "," + "\n"));
  if (distanceStr != "N/A" && motor_state != "N/A") {
    WriteFile(file_name.c_str(), (sender_uid + "," + time_now() + "," + distanceStr + "," + tempStr + "," + humidityStr + "," + motor_state + "," + sender_uid).c_str());

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

    if (sender_uid == "4" && (motor_state == "OFF" || motor_state == "ON" || motor_state == "SUS")) {
      motor_state_4 = motor_state;
      Serial.println("The motor state of 4 is " + String(motor_state_4));
    }

    Serial.println("time_now");
    Serial.println(time_now());
    Serial.println("------");
  } else {
    Serial.println("Invalid_data");
  }

 
}
