#include <Arduino.h>
#include "WiFi.h"
#include "esp_wifi.h"
#include <FirebaseESP32.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

const char* ssid     = "BubbleO W";
const char* password = "bubbleo123";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

const uint16_t port1 = 5555;
const char * host;
WiFiClient client;

#define FIREBASE_HOST "https://sterilizer.firebaseio.com/"
#define FIREBASE_AUTH "pnUwQVHcBf5AJ0fObO7d5oVDmnYIS87E1VQgiNQT" 

FirebaseData firebaseDataUv;
FirebaseData firebaseDataSchedule;
FirebaseData Lifespan;

FirebaseJson json;

TaskHandle_t Task1;

String lifePath = "/IBO211222/lifespan";
String UvPath = "/IBO211222/uv";
String schedulePath1 = "/IBO211222/schedules";

unsigned long timer1;
const int uv_light = 5; 
const int indication = 26;  
const int ACin = 27;
const int sensor1 = 21;
const int sensor1_power = 25;

String line, newdata = "", updatenewdata, lifedata, hepaelements, uvelements, updatehepaelements, updateuvelements;
byte l, j, o, len, day, hotspot_selector;
bool uvLight = false;
bool humanDetected = false;
bool indicationLed = false;
bool motionfalg = false;
bool checkflag = false;
bool endflag = false;
bool lifecounter = false;
bool wifi_selector = false;
int tempDay, x, y, hepasplitarray[6], uvsplitarray[6], updatehepasplitarray[6], updateuvsplitarray[6], startTimearray[4], endTimearray[4], startTime = 0, endTime = 0, motioncounter = 0;
bool flag = false;
int a0, b0, d0, f0, l0, n0, a01, b01, s0, t0, i, m, shifter = 2, mm = 0, dc = 20000, aaa, falseflag = 0;
unsigned long checker = 0, hepaTimearray[6],  uvTimearray[6], hepaTime = 0, uvTime = 0;
char hepastringarray[6], uvstringarray[6], updatehepastringarray[6], updateuvstringarray[6], schedulearray[8];

void  systemControl(){
  
if(uvLight){

  digitalWrite(uv_light,LOW);
  digitalWrite(sensor1_power,LOW);
  
    if(motioncounter <= 2500){
     motioncounter++;
    }
    
    if(motioncounter >= 2000){
     motionfalg = true;
    }

}else{

  digitalWrite(uv_light,HIGH);
  digitalWrite(sensor1_power,HIGH);
  motionfalg = false;
  motioncounter = 0;

}

if(indicationLed){
  digitalWrite(indication,LOW);
}else{
  digitalWrite(indication,HIGH);
}

}

void Task1code(void * parameter){
  int h = 1;
  for (;;) {
   for(aaa = 59; aaa >= 0; aaa--){

    for(int bbb = 0; bbb < 100; bbb++){
     
     if(motionfalg){
      h =  digitalRead(sensor1);
      if(!h){
       uvLight = false;
       checkflag = true;
       humanDetected = true;
       h = 1;
      }
     } 
     systemControl();
    
     vTaskDelay(10);

    }
   }
   lifecounter = true;
   aaa = 59;
  }
}

void checkUvStateFromDb(String uv){
  if(uv == "ON"){
    uvLight = true;
    }
  else if(uv == "OFF"){
    uvLight = false;
    checkflag = false;
  }
  else if(uv == "OFF1"){
    uvLight = false;
  }
}

void putDataToDb(String location,String dataToWrite){
  Firebase.setString(firebaseDataUv, location,  dataToWrite);
}

void connection() {

 timer1 = millis();
  
  while(1){
    
    if (millis() - timer1 >= 500UL){
      wifi_sta_list_t wifi_sta_list;
      tcpip_adapter_sta_list_t adapter_sta_list;
     
      memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
      memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));
     
      esp_wifi_ap_get_sta_list(&wifi_sta_list);
      tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);
     
      for (int i = 0; i < adapter_sta_list.num; i++) {
     
        tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
        host = ip4addr_ntoa(&(station.ip));
        if (client.connect(host, port1)) {
         return;
        }

      }
     
      timer1 = millis(); 
      }    
   }
}

void Credential_listener(){

 byte flag1 = 0;
 
  while(1){
   
      while(client.connected() < 1){
       connection();
      }

      while(client.available() > 0){
       line = client.readStringUntil('\r');
       len = line.length();
       o = len;
       flag1++;
       if(flag1 == 1){
        l = 0;
        len = (len + 0);
       }
       else if(flag1 == 2){
        l = 32;
        len = (len + 32);
       }

         if ((line.length() > 0) && (flag1 == 1)) {

          for (int k = 0; k < 500; ++k) {
            EEPROM.write(k, 0);
          }
          EEPROM.commit();
         }

          for((l, j=0); (l < len) && (j < o); (++l, ++j)){
           EEPROM.write(l, line[j]);
          }
          EEPROM.commit();
          while(flag1 == 2){

           flag1 = 0;

           EEPROM.write(511,1);
           EEPROM.commit();
           ESP.restart();
          }

      }
     
   }  
 
}

void hotspot(){

EEPROM.write(511,0);
EEPROM.commit();
ESP.restart();

}

void setup() {

  pinMode(uv_light, OUTPUT);
  digitalWrite(uv_light, HIGH);

  pinMode(indication, OUTPUT);
  digitalWrite(indication, HIGH);
  
  pinMode(ACin, OUTPUT);
  digitalWrite(ACin, HIGH);

  pinMode(sensor1, INPUT_PULLUP);

  pinMode(sensor1_power, OUTPUT);
  digitalWrite(sensor1_power, HIGH);

  digitalWrite(ACin, LOW);

  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);

      String esid;
      for (int f = 0; f < 32; ++f)
      {
        esid += char(EEPROM.read(f));
      }
   
      String epass = "";
      for (int g = 32; g < 96; ++g)
      {
        epass += char(EEPROM.read(g));
      }

      if((epass[0] == '?') && (epass[1] == '@') && (epass[2] == '!')){
       wifi_selector = true;
      }
      else{
       wifi_selector = false; 
      }
 
      hotspot_selector = EEPROM.read(511);

      if(hotspot_selector){

       if(wifi_selector){
        WiFi.begin(esid.c_str());
       }
       else{
        WiFi.begin(esid.c_str(), epass.c_str());
       }
     
        while((WiFi.status() != WL_CONNECTED)){
        
         delay(1);    
         checker++;
        
         if(checker < 5000 && checker > 3000 ){
          if(wifi_selector){
           WiFi.begin(esid.c_str());
          }
          else{
           WiFi.begin(esid.c_str(), epass.c_str());
          }
         }

         if(checker > 60000){
          hotspot();           
         }

        }

      }
      else{

        WiFi.softAP(ssid, password);
        while(1){       
         while(!(client.connected())){
          connection();
         }
         while((client.connected())){
          Credential_listener(); 
         }
        }  

      }

      Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
      Firebase.reconnectWiFi(true);

  if (!Firebase.beginStream(firebaseDataUv, UvPath))
  {
    falseflag = 1;
  }

  Firebase.setReadTimeout(firebaseDataUv, 1000 * 60);
  Firebase.setwriteSizeLimit(firebaseDataUv, "tiny");

  indicationLed = true;

  timeClient.begin();
  timeClient.setTimeOffset(32400);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    0,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  

if(Firebase.get(Lifespan, lifePath)){
 lifedata = Lifespan.stringData();
}

  for(int u0 = 0; u0 < 6; u0++){ 
    if(u0 == 0){a01 = 100000;} 
    if(u0 == 1){a01 = 10000;}
    if(u0 == 2){a01 = 1000;}
    if(u0 == 3){a01 = 100;}
    if(u0 == 4){a01 = 10;}
    if(u0 == 5){a01 = 1;}
    hepaTimearray[u0] = (int(lifedata[u0]) - 48) * a01 ;
    hepaTime = hepaTime + hepaTimearray[u0]; 
  }
  for(int z0 = 0; z0 < 6; z0++){ 
    if(z0 == 0){b01 = 100000;}
    if(z0 == 1){b01 = 10000;}
    if(z0 == 2){b01 = 1000;}
    if(z0 == 3){b01 = 100;}
    if(z0 == 4){b01 = 10;}
    if(z0 == 5){b01 = 1;}
    uvTimearray[z0] = (int(lifedata[7 + z0]) - 48) * b01 ;
    uvTime = uvTime + uvTimearray[z0]; 
  }

}

int starttimecalculator(String newdata3, int shifterr){
  startTime = 0;
  for(s0 = 0 ; s0 < 8 ; s0++){
    schedulearray[s0] = newdata3[s0 + 1 + shifterr];
  }

  for(int u = 0; u < 4; u++){ 
    if(u == 0){a0 = 1000;}
    if(u == 1){a0 = 100;}
    if(u == 2){a0 = 10;}
    if(u == 3){a0 = 1;}
    startTimearray[u] = (int(schedulearray[u]) - 48) * a0 ;
    startTime = startTime + startTimearray[u]; 
  }

  return startTime;

}

int endtimecalculator(String newdata4, int shifterr){
  endTime = 0;
  for(s0 = 0 ; s0 < 8 ; s0++){
    schedulearray[s0] = newdata4[s0 + 1 + shifterr];
  }

  for(int z = 0; z < 4; z++){ 
    if(z == 0){b0 = 1000;}
    if(z == 1){b0 = 100;}
    if(z == 2){b0 = 10;}
    if(z == 3){b0 = 1;}
    endTimearray[z] = (int(schedulearray[4 + z]) - 48) * b0 ;
    endTime = endTime + endTimearray[z]; 
  } 

  return endTime;

}

void loop() { 
 
while(!timeClient.update()){
 timeClient.forceUpdate();
}

day = timeClient.getDay();

if(day != tempDay){
    schedulePath1 = "/IBO211222/schedules/" + String(day);
  if (!Firebase.beginMultiPathStream(firebaseDataSchedule, schedulePath1)){
    falseflag = 6;
  }else{
    tempDay =  day;
  }
  Firebase.setReadTimeout(firebaseDataSchedule, 1000 * 60);
  Firebase.setwriteSizeLimit(firebaseDataSchedule, "tiny");
}

if (!Firebase.readStream(firebaseDataUv))
    {
      falseflag = 2;
    }

if (firebaseDataUv.streamTimeout())
    {
      falseflag = 3;
    }

if (firebaseDataUv.streamAvailable())
    {
      checkUvStateFromDb(firebaseDataUv.stringData());
    }

if (!Firebase.readStream(firebaseDataSchedule))
    {
      falseflag = 4;
    }
    
if (firebaseDataSchedule.streamTimeout())
    {
      falseflag = 5;
    }

if (firebaseDataSchedule.streamAvailable()){

      newdata = firebaseDataSchedule.stringData();
      startTime = 0;
      endTime = 0;
      shifter = 2;       

    }

if(dc != newdata.length()){  // tells if schedule is deleted 
  dc = newdata.length();
  shifter = 2;
}   

startTime = starttimecalculator(newdata, shifter);  
endTime = endtimecalculator(newdata, shifter);

int shours = startTime / 100;
int sminutes = startTime - (shours * 100);
int dbstartminutes = (shours * 60) + sminutes;

int ehours = endTime / 100;
int eminutes = endTime - (ehours * 100);
int dbendminutes = (ehours * 60) + eminutes;

int currentTime = ((timeClient.getHours()) * 60) + (timeClient.getMinutes());

int duration = dbendminutes - dbstartminutes;
int limit = duration / 2;
int start_offset = (dbstartminutes + limit);

if((dbstartminutes <= currentTime) && (currentTime <= start_offset) && (checkflag == false)){
   uvLight = true;
   endflag = true;
  }

if((dbendminutes <= currentTime) && (endflag == true)){
   uvLight = false;
   endflag = false;
  }

if((dbendminutes <= currentTime) && (shifter < (newdata.length()-14)) && (newdata.length() != 4)){ // this loop should work only if current time exceeds end time and there is more than one schedule and the schedule space is not empty
 shifter = shifter + 10;
} 

if(humanDetected){
  putDataToDb(UvPath, "OFF1");
  humanDetected = false;  
}

if(lifecounter){
  if(uvLight){
   hepaTime = (hepaTime + 1); 
   uvTime = (uvTime + 1);
   updatehepaelements = "";
   updateuvelements = "";
   for(int k0 = 0; k0 < 6; k0++){
    if(k0 == 0){l0 = 100000;} 
    if(k0 == 1){l0 = 10000;}
    if(k0 == 2){l0 = 1000;}
    if(k0 == 3){l0 = 100;}
    if(k0 == 4){l0 = 10;}
    if(k0 == 5){l0 = 1;}
    updatehepasplitarray[k0] = (hepaTime/l0)%10;
    updatehepastringarray[k0] = char(updatehepasplitarray[k0] + 48); 
    updatehepaelements = updatehepaelements +  updatehepastringarray[k0];
   }
   for(int m0 = 0; m0 < 6; m0++){
    if(m0 == 0){n0 = 100000;}
    if(m0 == 1){n0 = 10000;}
    if(m0 == 2){n0 = 1000;}
    if(m0 == 3){n0 = 100;}
    if(m0 == 4){n0 = 10;}
    if(m0 == 5){n0 = 1;}
    updateuvsplitarray[m0] = (uvTime/n0)%10;
    updateuvstringarray[m0] = char(updateuvsplitarray[m0] + 48); 
    updateuvelements = updateuvelements +  updateuvstringarray[m0];
   }
  }
  else{
   hepaTime = (hepaTime + 1);
   updatehepaelements = "";
   updateuvelements = "";
   for(int k0 = 0; k0 < 6; k0++){
    if(k0 == 0){l0 = 100000;} 
    if(k0 == 1){l0 = 10000;}
    if(k0 == 2){l0 = 1000;}
    if(k0 == 3){l0 = 100;}
    if(k0 == 4){l0 = 10;}
    if(k0 == 5){l0 = 1;}
    updatehepasplitarray[k0] = (hepaTime/l0)%10;
    updatehepastringarray[k0] = char(updatehepasplitarray[k0] + 48); 
    updatehepaelements = updatehepaelements +  updatehepastringarray[k0];
   }
   for(int m0 = 0; m0 < 6; m0++){
    if(m0 == 0){n0 = 100000;}
    if(m0 == 1){n0 = 10000;}
    if(m0 == 2){n0 = 1000;}
    if(m0 == 3){n0 = 100;}
    if(m0 == 4){n0 = 10;}
    if(m0 == 5){n0 = 1;}
    updateuvsplitarray[m0] = (uvTime/n0)%10;
    updateuvstringarray[m0] = char(updateuvsplitarray[m0] + 48); 
    updateuvelements = updateuvelements +  updateuvstringarray[m0];
   }
  }  
  Firebase.setString(Lifespan, lifePath,  updatehepaelements + "," + updateuvelements);
  lifecounter = false;
}

}