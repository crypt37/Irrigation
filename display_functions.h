#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>


extern int outputA;
extern int outputB;
extern int button;
extern int counter;
extern String file_name;
extern bool motor_state;
int aState;
int aLastState;
bool butState;
bool menuexit = false;
extern bool passwordCorrectFlag;
extern bool exitMenu;
bool submenuexit=false;
int currentparentid;
 int currentMenuIndex = 0;
#define i2c_Address 0x3c // Initialize with the I2C addr 0x3C Typically eBay OLED's
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   // QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
char CorrectPass[5]="4444";

extern char password[5];
extern char Band[5];
extern char Address[5];
extern char CurrentData[10][8];
char *Currentda[] = { "Data1", "Data2", "Data3" };

char *parameterVal[]={Band,Address};

/////////////////menu item char array //////////////////

char *MainMenuItems[] = {
  " STATS",
  " SETUP",
  " LOGS",
  " REBOOT",
  " RESTORE DEAULTS",
  " EXIT MENU"
  };

char *STATsubmenu[]={
    "CURRENT DATA",
    " EXIT"
  
  };
  char *PARAMsubmenu[] {
    "BAND",
    "ADDRESS",
       " EXIT",
       "SAVE AND REBOOT" 
    
    };
  char *SETUPsubmenu[] ={
      " CHANGE PARAMS", 
      " ADD SENSOR",
      " Motor ON/OFF ",
       " EXIT" 

    };

    char *LOGSsubmenu[] ={
       " ERRORLOGS",
       " DATA LOGS",
       " EXIT" 
      };



 
char *SubMenuData[] = {
  " TIME: ",
  " DATE: ",
  " TEMPERATURE: ",
  " HUMIDITY: ",
  " VOLT: ",
  " CURRENT: ",
  " ADDRESS: ",
  " BAND: ",  
  "EXIT"
};


char *SubMenuSensors[] = {
  " DHT22",
  " PRESSURE",
  " TEMPERATURE",
  " EXIT"
};
char *SubMenuData2[] = {
  " TIME: ",
  " DATE: ",
  " TEMPERATURE: ",
  " HUMIDITY: ",
  " VOLT: ",
  " CURRENT: ",
  " ADDRESS: ",
  " BAND: ",  
  "EXIT"
};

char *submenutitles[4];
/////////////Function prototypes ////////////////////

void handleMenuSelection(char* menu[], int menuSize);
void navigatedataMenu( char *Title , char *placeholder ); 

void  ViewLogs(const char *);
void TurnMotorON();
void RebootESP();
void mainMenuExit();
void ChangeBand();
void ChangeAddress();
void ViewDATABASELogs();
void ViewErrorLogs();
void ShowCurrentData();
void RestoreDefaults();
typedef void (*MenuAction)();


////////Schema////////////////////////////////


struct MenuItem {
    char* name;
    MenuAction action;
    int key;
    bool showSubMenu;
    char **submenuitems;
    int size;
    MenuItem *submenu;
};

/////////////menu tables with link////////////////////////




MenuItem LOGsubitemmenu[]{
   {" DATABASE LOG", ViewDATABASELogs, 1, false, nullptr, 0,nullptr },
    {" ERROR LOG", ViewErrorLogs, 1, false, nullptr, 0,nullptr},
    {"EXIT", nullptr, 1, false, nullptr, 0,nullptr}
  
  };
  
 MenuItem PARAMETERsubmenu[] ={
  {"FREQUENCY BAND",ChangeBand,1,false,nullptr,0,nullptr},
  {"ADDRESS",ChangeAddress,1,false,nullptr,0,nullptr},
     {"EXIT", nullptr, 1, false, nullptr, 0,nullptr},
    {"SAVE AND REBOOT" , RebootESP, 1, false, nullptr, 0,nullptr}
  };




MenuItem Currentdatamenu[] = {
    {"Time", nullptr, 2,false, nullptr, 0,nullptr},
    {"State",nullptr, 2,false, nullptr, 0,nullptr},
   {"Temperature", nullptr, 2,false, nullptr, 0,nullptr},
    {"Humidity", nullptr, 2,false, nullptr, 0,nullptr},
    {"Volt",nullptr, 2,false, nullptr, 0,nullptr},
     {"Current",nullptr, 2,false, nullptr, 0,nullptr},
     {"Address",nullptr, 2,false, nullptr, 0,nullptr},
     {"Band",nullptr, 2,false, nullptr, 0,nullptr},
     {"EXIT",nullptr, 2,false, nullptr, 0,nullptr}
};
  
   
MenuItem SETUPsubitemmenu[] = {
    {" CHANGE PARAMS", nullptr, 1, true, PARAMsubmenu,  sizeof(PARAMsubmenu) / sizeof(PARAMsubmenu[0]), PARAMETERsubmenu},
    {" ADD SENSOR", nullptr, 1, true, nullptr, 0,nullptr },
    {" MOTOR ON/OFF", TurnMotorON, 1, true, nullptr, 0,nullptr },
    {"EXIT", mainMenuExit, 1, false, nullptr, 0,nullptr},
 
};


MenuItem STATsubitemmenu[] = {
    {" CURRENT DATA", ShowCurrentData, 1, false,nullptr,0,Currentdatamenu },
    {"EXIT", nullptr, 1, false, nullptr, 0,nullptr}
};



MenuItem mainMenu[] = {
    {"STATS", nullptr, 0,true, STATsubmenu,  sizeof(STATsubmenu) / sizeof(STATsubmenu[0]),STATsubitemmenu},
    {"SETUP", nullptr, 0,true, SETUPsubmenu, sizeof(SETUPsubmenu) / sizeof(SETUPsubmenu[0]),SETUPsubitemmenu},
   {"LOGS", nullptr, 0,  true, LOGSsubmenu, sizeof(LOGSsubmenu)/sizeof(LOGSsubmenu[0]),LOGsubitemmenu},
    {"REBOOT", RebootESP, 0, false, nullptr, 0,nullptr },
    {"RESTORE DEFAULTS", RestoreDefaults, 0, false, nullptr, 0,nullptr },
     {"EXIT", mainMenuExit, 0, false, nullptr, 0,nullptr},
};


/////////////////////Funciton definition /////////////////////
void navigateMenu(char* menu[], int menuSize, MenuItem main_Menu[]);
void displayMenu(char* menu[], int menuSize, int beg);
void updateMenu(char* menu[], int menuSize);

void RestoreDefaults(){
Serial.println("in restore defaults");
  
                   EEPROM.put(0,'7');
                    EEPROM.put(1,'9');
                EEPROM.put(2,'6');
                 EEPROM.put(3,'3');
                  EEPROM.put(4,'0');
                  EEPROM.commit();
  delay(500);
  ESP.restart();
  }
void ShowCurrentData(){
 
for (int i = 0; i < sizeof(SubMenuData) / sizeof(SubMenuData[0])-1; i++) {
    // Calculate the size needed for the new string
    int newSize = strlen(SubMenuData[i]) + strlen(CurrentData[i]) + 1; // +1 for the null terminator

    // Allocate memory for the new string
    char *temp = (char *)malloc(newSize);

    if (temp) {
      // Copy and concatenate strings
      strcpy(temp, SubMenuData[i]);
      strcat(temp, CurrentData[i]);

      // Update SubMenuData to point to the new concatenated string
      SubMenuData2[i] = temp;
    }
  } 

   displayMenu(SubMenuData2,sizeof(SubMenuData2)/sizeof(SubMenuData2[0]),0 );
   delay(250);
   navigateMenu(SubMenuData2, sizeof(SubMenuData2)/sizeof(SubMenuData2[0]) , Currentdatamenu);
   return;
  
  }
void navigateMenu(char* menu[], int menuSize, MenuItem Main_menu[] );
void mainMenuExit(){return;}
  void ChangeAddress(){
     Serial.println("Change Address");
     delay(300);
     navigatedataMenu("Password",password);
     if(passwordCorrectFlag){   delay(300);navigatedataMenu( "  Frequency Band", Band);}
     passwordCorrectFlag=false;
     currentMenuIndex--;
    }

    void ChangeBand(){
      Serial.println("Change band");
      delay(300);
         navigatedataMenu("Password",password);
       if(passwordCorrectFlag) {   delay(300);navigatedataMenu( " Address" ,Address );}
       currentMenuIndex--;
       passwordCorrectFlag=false;
      }
void RebootESP(){
    ESP.restart();
  }
  void ViewDATABASELogs(){  ViewLogs("/test.txt");}
void ViewErrorLogs(){  ViewLogs("/logs.txt");}
int centerText(const char* text, int textSize) {
  int16_t x1, y1;
  uint16_t w, h;
  display.setTextSize(textSize);
  display.getTextBounds((char*)text, 0, 0, &x1, &y1, &w, &h);
  return (SCREEN_WIDTH - w) / 2;
}



void parameterMenu(){
                    display.clearDisplay();
                  display.setTextSize(1);
                  display.setTextColor(SH110X_WHITE);
                  display.setCursor(0,64/2);
                  for(int i=0;i<2;i++){
                    display.print(PARAMsubmenu[i]);
                    display.print(": ");
                   display.println((parameterVal[i]));
                    }                     
                    display.display();
                    
  delay(5000);   
  }

  void displayloopfunciton(){
//        while(!exitMenu ) {
        
         submenutitles[0]="MENU";
         Serial.println("insidethedisplayloopfunciton");
       
        displayMenu(MainMenuItems, 6, 0);
        if( digitalRead(button)){ 
     
   
    delay(500);
    navigateMenu(MainMenuItems, 6,mainMenu);
//      }
    }
    
    }

        
void  passwordMenu( char *Title, char *valarray) {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(16, 0);
  display.print("Enter ");
  display.println(Title);

  for (int i = 0; i < 4; i++) {
    display.setCursor(32 * i + 8, 64 / 2);
    display.print(valarray[i]);
    display.setCursor(32 * i + 5, 64 / 2 + 5);
    display.print("__");
  }
  
  display.display();
  delay(50);

  return;
  }

int displaycounter=0;
int itemsToDisplay=6;
int displayOffset=0;
String menu_title="MENU";

void displayMenu(char* menu[], int menuSize, int offset) {

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  Serial.println( submenutitles[currentMenuIndex]);
  menu_title=submenutitles[currentMenuIndex];

  Serial.println(" the array");
  for (int i=0;i<2;i++) {Serial.println( submenutitles[i]); }
    Serial.println(" the array");
  if(offset>0){  display.setCursor(0, 0); display.println(menu_title.substring(0,10));     }
  else if (menu_title.length()>9) {   display.setCursor(0, 0); display.println(menu_title.substring(0,10)); }
  else if ((menu_title.length())>9  && offset > 0 ){ display.setCursor(centerText(menu_title.c_str(), 2), 0); display.println(menu_title.substring(0,9)+"^");   }
 
  else  { display.setCursor(centerText( "MENU", 2), 0); display.println(menu_title); }
  
   Serial.println("end of display function");
  display.setTextSize(1);

  for (int i = 0; i < itemsToDisplay && (i + offset) < menuSize; i++) {
    display.setCursor(centerText(menu[i + offset], 1), (i + 1) * 8 + 8);
    display.println(menu[i + offset]);
  }
  vTaskDelay(pdMS_TO_TICKS(20));
  display.display();
 
      if(submenuexit) {   submenuexit=false; counter=0;
      updateMenu( menu, menuSize); 
      }
      
}



void updateMenu(char* menu[], int menuSize) {

  Serial.println("here in update");
  if (counter >= displayOffset + itemsToDisplay) {
    displayOffset += itemsToDisplay;
  } else if (counter < displayOffset) {
    displayOffset -= itemsToDisplay;
  }

  displayMenu(menu, menuSize, displayOffset);

  display.setTextColor(SH110X_BLACK, SH110X_WHITE);
  display.setCursor(centerText(menu[counter], 1), (counter - displayOffset + 1) * 8 + 8);
  display.println(menu[counter]);
  display.display();
}

            void handleMenuSelection(MenuItem menu[], int menuSize) {
                menu_title=menu[counter].name;
                     currentMenuIndex++;
                    submenutitles[currentMenuIndex]=menu[counter].name; 


                   
                if (1) {
                  if(menu[counter].showSubMenu == true)
                  {  

                     displayMenu(menu[counter].submenuitems, menu[counter].size, 0);
                        vTaskDelay(pdMS_TO_TICKS(200));
                      navigateMenu(menu[counter].submenuitems, menu[counter].size, menu[counter].submenu);
                      Serial.println("returned here " );
                     
                    }
                 else {

                  Serial.println("else"); menu[counter].action();}
           
                } 
                return;
            }

void navigateMenu(char* menu[], int menuSize, MenuItem main_Menu[]) {
 Serial.println("here in navigate menu");
  menuexit = false;
  while (!menuexit) {
    aState = digitalRead(outputA); // Reads the "current" state of the outputA
    butState = digitalRead(button);
    
    // If the previous and the current state of the outputA are different, that means a Pulse has occurred
    if (aState != aLastState) {
      // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
      if (digitalRead(outputB) != aState) {
        counter--;
       
        if (counter < 0) {
          counter = menuSize - 1;
        }
      } else {
      
        counter++;
        if (counter >= menuSize) {
          counter = 0;
        }
      }
      Serial.println("the counter is "  + String (counter));
      updateMenu(menu, menuSize);
       
    }

    if (!butState) {
          vTaskDelay(pdMS_TO_TICKS(200));
          
      if(main_Menu[counter].name=="EXIT") { Serial.println("exit found");  
        currentMenuIndex--;
      submenuexit=true;
      delay(200);
      return;
      
      }
       handleMenuSelection(main_Menu, sizeof(main_Menu));
      updateMenu(menu, menuSize);
      delay(200);

      
    }

    aLastState = aState; // Updates the previous state of the outputA with the current state
  }
}






void navigatedataMenu( char *Title , char *placeholder ) {
  for(int i=0;i<=3;i++){ placeholder[i]=' ';}
  passwordMenu(Title,placeholder);
  menuexit = false;
  int size = 9;
  int menuSize=4;
  int Menucounter=0;
int counter=0;
  while (!menuexit) {
    aState = digitalRead(outputA); // Reads the "current" state of the outputA
    butState = digitalRead(button);
      
    // If the previous and the current state of the outputA are different, that means a Pulse has occurred
    if (aState != aLastState) {
       Serial.println(counter);
      // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
      if (digitalRead(outputB) != aState) {
        counter++;
             if (counter < 0) { 
          counter = 9;
        }
             if (counter > 9) { 
          counter = 0;
        }
         display.clearDisplay();
           passwordMenu(Title,placeholder);
          display.setTextColor(SH110X_WHITE);
           display.setCursor(Menucounter*32+8,64/2+2);
            Serial.println(counter);
          display.print(counter);
          display.display();
          
                
      } else {
        counter--;       
           if (counter > 9) { 
          counter = 0;
        }
        if (counter < 0) 
        { 
          counter = 9;
        }
        delay(100);
               if (Menucounter < 0) {
          Menucounter=3;
        }
         display.clearDisplay();
          passwordMenu(Title,placeholder);
          display.setTextColor(SH110X_WHITE);    
          display.setCursor(Menucounter*32+8,64/2+2);
          display.print(counter);
          display.display();
      }
     
    }

    if (!butState) {
       
      placeholder[Menucounter] ='0'+ counter ;
      Menucounter++;  
      if(strcmp(Title, "Password") == 0 && password[0]==CorrectPass[0] &&password[1]==CorrectPass[1] && password[2]==CorrectPass[2] && password[3]==CorrectPass[3] )
      { Serial.println("password correct"); passwordCorrectFlag=true; counter=0; for(int i=0;i<5;i++){placeholder[i]=' ';} return;}
     
              if (Menucounter >= menuSize) {
                  if (strcmp(Title, " Frequency Band") == 0 ){ 
                   
                  display.clearDisplay();
                  display.setTextSize(1.5);
                  display.setTextColor(SH110X_WHITE);
                  display.setCursor(0,64/2+2);
                  for(int i=0;i<=3;i++){EEPROM.put(1+i,placeholder[i]);  }
                  // Stores the value at the given address
                  EEPROM.commit();
                  display.print("Band set to :");

                  display.println(placeholder );
                  display.display();
                   delay(200);
                  return;}
                   if (strcmp(Title, " Address") == 0 )
                   {
                  display.clearDisplay();
                  display.setTextSize(1.5);
                  display.setTextColor(SH110X_WHITE);
                  display.setCursor(0,64/2+2);
                  EEPROM.put(0,placeholder[3]); // Stores the value at the given address
                  EEPROM.commit();
                  display.println("Address set to : ");
                  delay(200);
                    return;
                    }
     
                    
                 for(int i=0;i<5;i++){placeholder[i]=' ';}
                Menucounter = 0;
                 delay(300);
                if (strcmp(Title, "Password") == 0 ){
                display.clearDisplay();
                display.setTextSize(1.5);
                display.setTextColor(SH110X_WHITE);
                display.setCursor(0,64/2+2);
                display.println("Password Incorrect :(");
                delay(4000);
                Serial.println("password INcorrect");

                  return;}
          
        }

        
      counter=int(placeholder[Menucounter]);
              passwordMenu( Title,placeholder );  
             
             
     //      handleMenuSelection(menu, menuSize);
      delay(200);
    }

    aLastState = aState; // Updates the previous state of the outputA with the current state
  }
}

void  ViewLogs(const char *path) {


  myFile = SD.open(path);
  if (myFile) {
    Serial.printf("Reading file from %s\n", path);
    char buffer[128];
    int lineCount = 0;
    int pageCount = 0;
    int aState = digitalRead(outputA);
    int aLastState = aState;
    int linesPerPage = 4;
    long filePosition = 0;
    bool flag=false;
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);

    Serial.println(butState);
    delay(200);

    while (digitalRead(button)) {
      myFile.seek(filePosition);

      lineCount = 0;
      display.clearDisplay();

      while (lineCount < linesPerPage && myFile.available()) 
      {

        if(!digitalRead(button)){return;}
        int len = myFile.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
        buffer[len] = 0; // Null-terminate the string

        display.setCursor(0, lineCount * 16); // Adjust Y position based on font size
        
        if(flag){ lineCount--;flag=false; }
        else{display.println(buffer);}

        lineCount++;
      }

      display.display();

      while (aState == aLastState) {
        aState = digitalRead(outputA);
        
      }
     
      if (aState != aLastState) {
        if (digitalRead(outputB) != aState) {
          filePosition = myFile.position();
          pageCount++;
        } else {
          // Move back in the file lines
          for (int i = 0; i < linesPerPage && filePosition > 0; i++) {
            filePosition--;
//            while (filePosition > 0 && myFile.peek() != '\n') {
//              filePosition--;
//            }
              flag=true;

          }
          
           filePosition = max(0L, filePosition);
          pageCount--;
          
        }
      }
      
      aLastState = aState;
    }

    myFile.close(); // close the file
  } else {
    Serial.println(F("error opening test.txt"));
  }
}
   void TurnMotorON(){
    // to do code to turn motor on 
     if(motor_state) { 
      display.clearDisplay();
      display.setTextColor(SH110X_WHITE);
      display.setCursor(0,32);
      display.setTextSize(2);
        display.display();
      display.println("Motor turned OFF ");
      delay(5000);
      }
else {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,32);
  display.setTextColor(SH110X_WHITE);
  display.println("Motor Turned ON check Motor");
  display.display();
  delay(3000);
  
  }
      
    }


  
