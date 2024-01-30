// Los numeros tienen un tamaño de 27 x 47 px
// 1123549B
// 1123373B
// 1123217B   if-else
// 1123225B   switch
// 1112785B


#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <XPT2046_Touchscreen.h>  // para el tactil
#include <BluetoothSerial.h>
#include "ELMduino.h"

#define ELM_PORT   SerialBT
#define T_MOSI 32
#define T_MISO 39
#define T_CLK 25
#define T_CS 33
#define LOOP_DELAY 75 // This controls how frequently the meter is updated
#define color_background TFT_BLACK

BluetoothSerial SerialBT;
ELM327 myELM327;
SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(T_CS);
TFT_eSPI tft = TFT_eSPI();            // Invoke custom library with default width and height

int runTime = 0;       // time for next update
int last_angle = 45;
int color_rpm[3]={TFT_GREEN,TFT_YELLOW,TFT_RED};
int rpm = 0;
int vel = 0;

String device_name = "ESP32-BT-Slave";


int updte_rpm(int old_rpm);
void ringMeter(int x, int y, int r, int val);
void numberMeter(int xpos, int ypos, int reading, int tamanio);
int updte_vel(int old_vel);

void setup(void) {
  Serial.begin(115200);

  mySpi.begin(T_CLK, T_MISO, T_MOSI, T_CS);
  ts.begin(mySpi);
  ts.setRotation(1);
  
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(color_background);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);


// Protocolos para probar conexion
// const char ISO_9141_5_BAUD_INIT       = '3';
// const char ISO_14230_5_BAUD_INIT      = '4';
// const char ISO_14230_FAST_INIT        = '5';
  
  SerialBT.setPin("1234");
  SerialBT.begin("ArduHUD", true);    // ArduHUD es el nombre del BT que pone el elm32
//  SerialBT.begin("ESP-32");

  if (!ELM_PORT.connect("OBDII"))
  {
    Serial.println(F("Error conexion bluetooth - Fase 1"));
    while(1);
  }

  if (!myELM327.begin(ELM_PORT, true, 2000,5))
  {
    Serial.println(F("Error conexion OBD - Fase 2"));
    while (1);
  }

  Serial.println(F("Conexion correcta"));

}


void loop() {

  static int radius = 110;
  static int xpos = tft.width() / 2;
  static int ypos = tft.height() / 2;
  static int pantalla = 1;
  static int pantalla_old=1;
  bool tocado=false;
  static bool hundido=false;
//  static float volts = myELM327.batteryVoltage();
//  float ambientAirTemp();
//  float fuelRate();
//  float fuelLevel();
  
 

  if (millis() - runTime >= LOOP_DELAY) {
    
  
    if (ts.tirqTouched() && ts.touched()) {
      tocado=true;
    }
    if(tocado==true && hundido==false){
      TS_Point p = ts.getPoint();
      if(p.x>1798){
        pantalla++;
      }else{
        pantalla--;
      }
      if(pantalla>3){
        pantalla=1;
      }
      if(pantalla<1){
        pantalla=3;
      }
    }
    /*
    Serial.println(tocado);
    Serial.println(pantalla);
    Serial.println(pantalla_old);
*/

    switch(pantalla){
      case 1:{
        if(pantalla_old!=1){
          tft.fillScreen(TFT_BLACK);
          last_angle=45;
          ringMeter(xpos-40, ypos, radius, rpm); // Draw analogue meter
          numberMeter(xpos+20,ypos,rpm,6);
        }  
        
        rpm = updte_rpm(rpm);
        vel = updte_vel(vel); 

        ringMeter(xpos-40, ypos, radius, rpm); // Draw analogue meter
        numberMeter(xpos+20,ypos,rpm,6);

        numberMeter(xpos+160,ypos+40, vel,8);
        break;
      }
      case 2:{
        if(pantalla_old!=2){
          tft.fillScreen(TFT_WHITE);
        } 
        tft.fillScreen(TFT_WHITE);        
        break;
      }
      case 3:{
        if(pantalla!=3){
          tft.fillScreen(TFT_GREEN);
        }
        tft.fillScreen(TFT_GREEN);
        break;
      }

    }

    runTime = millis();
    pantalla_old=pantalla;
    hundido=tocado;
  
  }
  
}

// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
// x,y is centre of meter, r the radius, val a number in range 0-100

void ringMeter(int x, int y, int r, int val){

  int thickness = 40;
  static int last_i=1;
  int i;
//  int color_arco = color_rpm[0];
//  int color_quito=color_background;
  
  if(val>6000){
    i=2;
  }else{
    if(val>4500){
      i=1;
    }else{
      i=0;
    }
  }
  
  

  // Range here is 0-100 so value is scaled to an angle 45-270
//  int val_angle = map(val, 0, 70, 45, 270);       // Modo test
  int val_angle = map(val, 0, 7000, 45, 270);     // Modo coche
  if(val>7000) val_angle=270;

  if (last_angle != val_angle) {      
    // Update the arc, only the zone between last_angle and new val_angle is updated
    if (val_angle > last_angle) {
      if(i != last_i){
        last_angle=45;
      }
        tft.drawArc(x, y, r, r - thickness, last_angle, val_angle, color_rpm[i], color_background); // TFT_SKYBLUE random(0x10000)
    }
    else {
      if(i != last_i){
        last_angle=45;
        tft.drawArc(x, y, r, r - thickness, last_angle, val_angle, color_rpm[i], color_background);
      }
        tft.drawArc(x, y, r, r - thickness, val_angle, last_angle, color_background, color_background);
    }
    last_i = i;
    last_angle = val_angle; // Store meter arc position for next redraw
  }
  
}

  /***********************************************************************************
    Imprime por pantalla un numero en una posicion, con tamaño de letra 6
    Lo uso para escribir las rpms
  ************************************************************************************/

void numberMeter(int xpos, int ypos, int reading, int tamanio){

  char buf[8]; 
  dtostrf(reading, 1, 0, buf);
  tft.drawRightString(buf, xpos, ypos, tamanio);

}

int updte_rpm(int old_rpm){
  int rpm=(int)myELM327.rpm();
//  char leido;
//  int rpm=0;
//  int tmp;

  if(myELM327.nb_rx_state == ELM_SUCCESS){
    if(rpm!=old_rpm){
      return(rpm);      
    }
  }else{
    if (myELM327.nb_rx_state != ELM_GETTING_MSG){
      myELM327.printError();
    }    
  }

  return(old_rpm);


// 13 \r
// 10 \n
/*
  if(SerialBT.available()){
    leido=SerialBT.read();
    Serial.print((int)leido);
    Serial.println();
      
    while( ((int)leido) != 13 ){
      rpm=rpm*10+leido-48;
      Serial.print(rpm);
      Serial.println();
      leido=SerialBT.read();
      Serial.print((int)leido);
      Serial.println();
      
    }
    while(SerialBT.available()){
      SerialBT.read();
    }
    return(rpm);
  }
  return old_rpm;
*/

}

int updte_vel(int old_vel){
  int vel=(int)myELM327.kph();

  if(myELM327.nb_rx_state == ELM_SUCCESS){
    if( vel != old_vel ){
      return( vel );      
    }
  }else{
    if (myELM327.nb_rx_state != ELM_GETTING_MSG){
      myELM327.printError();
    }
  }

  return(old_vel);

}
