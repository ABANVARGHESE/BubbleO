#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

const int indication = 26;
const int ACin = 27;
const int uv_light = 5;

const int sensor1 = 21;
const int sensor1_power = 25;

int asc = 0, num = 0;
int seconds;
unsigned long trigger;
String line;

void setup() {
  
  SerialBT.begin("BubbleO BT 2"); //Bluetooth device name
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
  digitalWrite(indication, LOW);
  mainloop();
    
}

void loop() {

}

void mainloop(){

 while(1){
  
   while(SerialBT.available() > 0){
    line = SerialBT.readStringUntil('\r');
    if(line.length() == 1){
     num = 0; 
     asc = line[0];
     num = asc - 48; 
    }
    else if(line.length() == 2){
     num = 0; 
     asc = ((10*line[0]) + line[1]); 
     num = asc - 480 - 48;
    }    
   }
   if((num == 1)||(num == 2)||(num == 3)||(num == 4)||(num == 5)||(num == 6)||(num == 7)||(num == 8)||(num == 9)||(num == 10)||(num == 15)||(num == 20)||(num == 25)||(num == 30)||(num == 35)||(num == 40)||(num == 45)||(num == 50)||(num == 55)||(num == 60)){
    seconds = (240 * num);
    trigger = 0;
    time_loop(seconds, trigger);
   }
   else if((num == 65)){
    num = 0;
    digitalWrite(uv_light, HIGH);
    digitalWrite(indication, HIGH);
    digitalWrite(ACin, HIGH);
    ESP.restart();
   }
   else {
    num = 0; 
   }
   if(line == "s"){
    digitalWrite(uv_light, HIGH);
    digitalWrite(sensor1_power, HIGH);  
   }
   
  } 
 
 }

void time_loop(int count, unsigned long checker){
  
 while(1){

  for(int a = count-1; a >= 0; a--){
   for(int b = 0; b < 250; b++){

    digitalWrite(uv_light, LOW);
    digitalWrite(sensor1_power, LOW);
    while(SerialBT.available()) {
          
     line = SerialBT.readStringUntil('\r'); 
     if(line == "s"){
      digitalWrite(uv_light, HIGH);
      digitalWrite(sensor1_power, HIGH);   
      num = 0;
      mainloop();
     }

     if(line == "h"){ 
      digitalWrite(uv_light, HIGH);
      digitalWrite(sensor1_power, HIGH); 
      num = 0;
      int sub = a;
      halt_loop(sub, checker);
     }
          
    }

    delay(1);

    if(checker <= 40000){
       checker++;
    }

    if(checker >= 20000){

       int val = digitalRead(sensor1);

       if(val  == 0) {

            SerialBT.write('m');             
            digitalWrite(uv_light, HIGH);
            digitalWrite(sensor1_power, HIGH); 
            num = 0;       
            mainloop();
                     
       }
    
    }
   
  }

 }

  checker = 0;
  count = 0;
    
  digitalWrite(uv_light, HIGH); 
  digitalWrite(sensor1_power, HIGH); 
  num = 0;  
  mainloop();

 }

}

void halt_loop(int loop_count, unsigned long loop_checker){

 while(1){

  while(SerialBT.available()) {

   line = SerialBT.readStringUntil('\r');
   if(line == "p"){ 
    loop_checker = 0;
    time_loop(loop_count, loop_checker);
   }

   if(line == "s"){
    digitalWrite(uv_light, HIGH);
    digitalWrite(sensor1_power, HIGH);   
    num = 0;
    mainloop();
   }
    
  }
  
 }

}
