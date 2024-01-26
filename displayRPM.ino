// Los numeros tienen un tamaño de 27 x 47 px


#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <XPT2046_Touchscreen.h>  // para el tactil

#include "BluetoothSerial.h"
#include "ELMduino.h"

BluetoothSerial SerialBT;
ELM327 myELM327;

#define ELM_PORT   SerialBT

typedef enum {RPM, velocidad, voltaje } obd_pid;
obd_pid leer=RPM;


#define T_MOSI 32
#define T_MISO 39
#define T_CLK 25
#define T_CS 33

SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(T_CS);

#define LOOP_DELAY 35 // This controls how frequently the meter is updated

TFT_eSPI tft = TFT_eSPI();            // Invoke custom library with default width and height
TFT_eSprite spr = TFT_eSprite(&tft);  // Declare Sprite object "spr" with pointer to "tft" object

#define color_background TFT_BLACK
//#define color_rpm TFT_RED

uint32_t runTime = 0;       // time for next update

int reading = 0; // Value to be displayed
int d = 0; // Variable used for the sinewave test waveform
bool range_error = 0;
int8_t ramp = 1;
static uint16_t last_angle = 45;
int color_rpm[3]={TFT_GREEN,TFT_YELLOW,TFT_RED};
float rpm = 0;


float updte_rpm(float old_rpm);
void ringMeter(int x, int y, int r, int val);
void numberMeter(uint16_t xpos, uint16_t ypos, int reading,uint8_t tamanio);
float updte_vel(float old_vel);

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
  ELM_PORT.begin("ArduHUD", true);    // ArduHUD es el nombre del BT que pone el elm32
  
  if (!ELM_PORT.connect("OBDII"))
  {
    Serial.println("Error conexion bluetooth - Fase 1");
    while(1);
  }

  if (!myELM327.begin(ELM_PORT, true, 2000,5))
  {
    Serial.println("Error conexion OBD - Fase 2");
    while (1);
  }

  Serial.println("Conexion correcta");

}


void loop() {

  static uint8_t radius = 110;
  static int16_t xpos_rpm = tft.width() / 2 - 40;
  static int16_t ypos_rpm = tft.height() / 2;
//  static int16_t xpos = tft.width() / 2;
//  static int16_t ypos = tft.height() / 2;
//  static int8_t pantalla = 1;
//  static int8_t pantalla_old=1;
//  static int last_reading=0;
//  uint8_t tocado=0;
//  static uint8_t hundido=0;
  static int32_t vel;
//  static float volts = myELM327.batteryVoltage();
//  float ambientAirTemp();
//  float fuelRate();
//  float fuelLevel();
  
 

  if (millis() - runTime >= LOOP_DELAY) {
  
    switch (leer){
      case RPM:
      {
        rpm=updte_rpm(rpm);
        ringMeter(xpos_rpm, ypos_rpm, radius, rpm); // Draw analogue meter
        numberMeter(xpos_rpm+60,ypos_rpm,rpm,6);
        leer=velocidad;
        break;
        
      }
      case velocidad:
      {
        vel=updte_vel(vel);
        Serial.print(vel);
        Serial.println();
        leer=RPM;
        break;
      }
      case voltaje: 
      {
        break;
      }
    }
  /* 
    if (ts.tirqTouched() && ts.touched()) {
      tocado=1;
    }
    if(tocado==1 && hundido==0){
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
    Serial.println(tocado);
    Serial.println(pantalla);
    Serial.println(pantalla_old);

    if(pantalla==1){
      if(pantalla_old!=1){
        tft.fillScreen(TFT_BLACK);
        last_angle=45;
        ringMeter(xpos_rpm, ypos_rpm, radius, last_reading); // Draw analogue meter
        numberMeter(xpos_rpm+60,ypos_rpm,last_reading*100,6);
      }  
    

      reading += ramp;
      ringMeter(xpos_rpm, ypos_rpm, radius, reading); // Draw analogue meter
      numberMeter(xpos_rpm+60,ypos_rpm,reading*100,6);

      numberMeter(xpos+160,ypos+40, reading+1,8);

      last_reading=reading;
    }else{
      if(pantalla==2){
        if(pantalla_old!=2){
          tft.fillScreen(TFT_WHITE);
        } 
        tft.fillScreen(TFT_WHITE);
      }else{
        if(pantalla==3){
          if(pantalla!=3){
            tft.fillScreen(TFT_GREEN);
          }
          tft.fillScreen(TFT_GREEN);
        }
      }

    }

    if (reading > 69) ramp = -1;
    if (reading <=  0) ramp = 1;

    if (reading > 69) {
      delay(1000);
    }
    if (reading <= 0) {
      delay(1000);
    }

    runTime = millis();
    pantalla_old=pantalla;
    hundido=tocado;
  
  }
  */
  }

}

// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
// x,y is centre of meter, r the radius, val a number in range 0-100

void ringMeter(int x, int y, int r, int val){

  uint8_t thickness = 40;
  static uint8_t last_i=1;
  uint8_t i;
//  int color_arco = color_rpm[0];
//  int color_quito=color_background;

  if(val>60){
    i=2;
  }else{
    if(val>45){
      i=1;
    }else{
      i=0;
    }
  }

  // Range here is 0-100 so value is scaled to an angle 45-270
  int val_angle = map(val, 0, 70, 45, 270);       // Modo test
//  int val_angle = map(val, 0, 7000, 45, 270);     // Modo coche
    

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

void numberMeter(uint16_t xpos, uint16_t ypos, int reading,uint8_t tamanio){

  char buf[8]; 
  dtostrf(reading, 1, 0, buf);
  tft.drawRightString(buf, xpos, ypos, tamanio);

}

float updte_rpm(float old_rpm){
  float rpm=myELM327.rpm();

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

}

float updte_vel(float old_vel){
  float vel=myELM327.kph();

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
