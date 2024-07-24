extern const int trigPin;
extern const int echoPin;
extern float distanceCm;
extern const float SOUND_SPEED;
extern int current_distance;
extern long  duration;
extern bool  motor_state;
extern String file_name ;
extern int address;
extern Bonezegei_DS1307 rtc;
extern Bonezegei_DHT22 dht;
extern char CurrentData[10][8];
extern String logpath;
extern String time_now();
extern char Address[5];
void WriteFile(const char * path, const char * message) {
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
void ReadFile(const char * path) {
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
    WriteFile(logpath.c_str(),  String(time_now() + ", ER4").c_str());
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
  return distanceCm;
}
String time_now() {

  if (rtc.getTime()) {
    String time_ima = String(rtc.getMonth()) + "/" + String(rtc.getDate())   + "/" + String(rtc.getYear()) + "," + String(rtc.getHour())  + ":" + String(rtc.getMinute())  + ":" + String( rtc.getSeconds());
    return time_ima;
  }

}
void saveData() {

  current_distance = calculate_distance();
  if (!dht.getData()) {
    Serial.println(F("Problem in DH22"));
    delay(200);
  }

String state_of_motor = String(motor_state ? "ON" : "OFF");

const char* temptime = String(time_now()).c_str();

const  char*  tempMotorState=String(state_of_motor).c_str();
 dtostrf(float(dht.getTemperature()), 6, 2, CurrentData[0]); // 6 is the minimum width, 2 is the precision
 dtostrf(float(dht.getHumidity()), 6, 2, CurrentData[1]); // 6 is the minimum width, 2 is the precision

strcpy(CurrentData[1], state_of_motor.c_str());
//strcpy(CurrentData[3],String(rtc.getMonth() + "/" + String(rtc.getDate())   + "/" + String(rtc.getYear())).c_str() );
strcpy(CurrentData[0], String(String(rtc.getHour())  + ":" + String(rtc.getMinute())).c_str() );
strcpy(CurrentData[6], String(Address[3]).c_str() );
//CurrentData[4]=String(current_distance);


for (int i=0;i<5;i++){
  Serial.println(CurrentData[i]);
  }
  

  Serial.println(time_now() + "," + current_distance + "," + dht.getTemperature() + "," +  dht.getHumidity() + "," + state_of_motor  + "," + address);


char buffer[128];
snprintf(buffer, sizeof(buffer), "%d,%d,%.2f,%.2f,%.2f,%d",
         address, current_distance, dht.getTemperature(), dht.getHumidity(), state_of_motor);
WriteFile(file_name.c_str(), buffer);

  
 
return;
}
