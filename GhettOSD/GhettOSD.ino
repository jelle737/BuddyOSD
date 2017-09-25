/*


Program  : GhettOSD (Supports the variant: minimOSD)
Version  : V1.0
Author(s): Guillaume S

GhettOSD is forked from minimosd-extra. Mavlink support has been removed. 
GhettOSD use LightTelemetry protocol sent by Ghettostation: https://github.com/KipK/Ghettostation/wiki


Original minimod-extra credits:
Author(s): Sandro Benigno
Coauthor(s):
Jani Hirvinen   (All the EEPROM routines)
Michael Oborne  (OSD Configutator)
Mike Smith      (BetterStream and Fast Serial libraries)
Gábor Zoltán
Pedro Santos
Special Contribuitor:
Andrew Tridgell by all the support on MAVLink
Doug Weibel by his great orientation since the start of this project
Contributors: James Goppert, Max Levine, Burt Green, Eddie Furey
and all other members of DIY Drones Dev team
Thanks to: Chris Anderson, Jordi Munoz


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>

*/

/* ************************************************************ */
/* **************** MAIN PROGRAM - MODULES ******************** */
/* ************************************************************ */



/* **********************************************/
/* ***************** INCLUDES *******************/

//#define membug 
//#define FORCEINIT  // You should never use this unless you know what you are doing 

// AVR Includes
#include <AltSoftSerial.h>  
#include <math.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
// Get the common arduino functions
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "wiring.h"
#endif
#include <EEPROM.h>

#ifdef membug
#include <MemoryFree.h>
#endif


#include "Lighttelemetry.h"
#include "Max7456.h"

//#define LOADFONT

/* *************************************************/
/* ***************** DEFINITIONS *******************/

//OSD Hardware 
//#define MinimOSD
#define TELEMETRY_SPEED_MASTER  2400  // How fast our master LTM telemetry is coming to Serial port
#define TELEMETRY_SPEED_SLAVE 2400 // How fast our slave LTM telemetry is coming to Serial port


// Objects and Serial definitions
AltSoftSerial slaveSerial(8, 9);
//OSD osd; //OSD object 

//Metro osdMetro = Metro(20);  //( 50hz )
LightTelemetry ltmMaster;
LightTelemetry ltmSlave;
Max7456 OSD;

/* **********************************************/
/* ***************** SETUP() *******************/

void setup() 
{
    OSD.init();
//    pinMode(MAX7456_SELECT,  OUTPUT); // OSD CS

    Serial.begin(TELEMETRY_SPEED_MASTER);
    slaveSerial.begin(TELEMETRY_SPEED_SLAVE); 

    // Prepare OSD for displaying 
//    unplugSlaves();
 

    // Check EEPROM to see if we have initialized it already or not
    // also checks if we have new version that needs EEPROM reset
//    if(readEEPROM(CHK1) + readEEPROM(CHK2) != VER) {
//        osd.setPanel(6,9);
//        osd.openPanel();
//        osd.printf_P(PSTR("Missing/Old Config")); 
//        osd.closePanel();
        //InitializeOSD();
//    }

    // Get correct panel settings from EEPROM
//    readSettings();
//    for(panel = 0; panel < npanels; panel++) readPanelSettings();
//    panel = 0; //set panel to 0 to start in the first navigation screen
//    delay(2000);
//    Serial.flush();

    // House cleaning, clear display and enable timers
//    osd.clear();
    ltmMaster.init(&Serial);
    ltmSlave.init(&slaveSerial);

} // END of setup();



/* ***********************************************/
/* ***************** MAIN LOOP *******************/

// Mother of all happenings, The loop()
// As simple as possible.

uint8_t count = 0;

void loop(){
    #ifdef LOADFONT
        OSD.writeString_P(PSTR("DO NOT POWER OFF"), 32);
        OSD.drawScreen();      
        delay(3000);
        OSD.displayFont();  
        OSD.writeString_P(PSTR("SCREEN WILL GO BLANK"), 32);
        OSD.drawScreen();
        delay(3000);
    //    while(true){
    //        displayCount(count--);
    //        OSD.drawScreen();
    //        delay(200);
    //    } 
        OSD.updateFont();
        OSD.init(); 
        OSD.displayFont();
        OSD.writeString_P(PSTR("UPDATE COMPLETE"), 32);
        OSD.drawScreen();
        delay(10000);
        uint8_t count = 0;
        while(true){
            displayCount(count++);
            OSD.writeString_P(PSTR("UPDATE COMPLETE"), 32);
            OSD.drawScreen();
            delay(200);
        }
//        digitalWrite(LEDPIN,LOW);
        //    while(true){
        //        displayCount(count--);
        //        OSD.drawScreen();
        //        delay(200);
        //    } 
    #else //LOADFONT


      ltmMaster.read();
      ltmSlave.read();
      //displayBuddy();
 //     ltmMaster.uav_rssi+ltmMaster.uav_linkquality;
         
       //while(true){
          displayCount(count++);
          OSD.writeString_P(PSTR("JELLE LECOMTES BUDDY OSD"), 32);
          displayCoords();
          displaySats();
          displayRSSI();
          displayBat();
          displayCoordsSl();
          displaySatsSl();
          displayRSSISl();
          displayBatSl();
          displayPitchRollHeading();
          displayPitchRollHeadingSl();
          OSD.drawScreen();
          //delay(1000/30);
     // }  

    #endif //LOADFONT

}

char screenBuffer[20];
//Symbols
#define SYM_BUDDY 0X00


void displayBuddy(void){
    screenBuffer[0] = SYM_BUDDY; //immagine SYM_BUDDY is a wing
    OSD.writeString(screenBuffer,70);
}

void displayBuddyTelemetry(void){

}

//==master==

void displayPitchRollHeading(void){
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    screenBuffer[0]='P';
    ItoaPadded(ltmMaster.uav_pitch, screenBuffer+2,3,0);
    OSD.writeString(screenBuffer,212);
    screenBuffer[0]='R';
    ItoaPadded(ltmMaster.uav_roll, screenBuffer+2,3,0);
    OSD.writeString(screenBuffer,218);
    screenBuffer[0]='H';
    ItoaPadded(ltmMaster.uav_heading, screenBuffer+2,3,0);
    OSD.writeString(screenBuffer,224);
}

void displayCoords(void){
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    ItoaPadded(ltmMaster.uav_lat, screenBuffer,10,3);
    OSD.writeString(screenBuffer,123);
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    ItoaPadded(ltmMaster.uav_lon, screenBuffer,10,3);
    OSD.writeString(screenBuffer,153);
}

void displaySats(void){
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    screenBuffer[0]='S';
    screenBuffer[1]='A';
    screenBuffer[2]='T';
    screenBuffer[3]='S';
    ItoaPadded(ltmMaster.uav_satellites_visible, screenBuffer+5,2,0);
    OSD.writeString(screenBuffer,190);
}

void displayBat(void){
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    screenBuffer[0]='B';
    screenBuffer[1]='A';
    screenBuffer[2]='T';
    ItoaPadded(ltmMaster.uav_bat/100, screenBuffer+4,4,3);
    OSD.writeString(screenBuffer,198);  
}

void displayRSSI(void){
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    screenBuffer[0]='R';
    screenBuffer[1]='S';
    screenBuffer[2]='S';
    screenBuffer[3]='I';
    ItoaPadded(ltmMaster.uav_rssi, screenBuffer+5,2,0);
    OSD.writeString(screenBuffer,182);
}


//====slave/buddy====

void displayPitchRollHeadingSl(void){
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    screenBuffer[0]='P';
    ItoaPadded(ltmSlave.uav_pitch, screenBuffer+2,3,0);
    OSD.writeString(screenBuffer,332);
    screenBuffer[0]='R';
    ItoaPadded(ltmSlave.uav_roll, screenBuffer+2,3,0);
    OSD.writeString(screenBuffer,338);
    screenBuffer[0]='H';
    ItoaPadded(ltmSlave.uav_heading, screenBuffer+2,3,0);
    OSD.writeString(screenBuffer,344);
}

void displayCoordsSl(void){
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    ItoaPadded(ltmSlave.uav_lat, screenBuffer,10,3);
    OSD.writeString(screenBuffer,243);
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    ItoaPadded(ltmSlave.uav_lon, screenBuffer,10,3);
    OSD.writeString(screenBuffer,273);
}

void displaySatsSl(void){
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    screenBuffer[0]='S';
    screenBuffer[1]='A';
    screenBuffer[2]='T';
    screenBuffer[3]='S';
    ItoaPadded(ltmSlave.uav_satellites_visible, screenBuffer+5,2,0);
    OSD.writeString(screenBuffer,310);
}

void displayBatSl(void){
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    screenBuffer[0]='B';
    screenBuffer[1]='A';
    screenBuffer[2]='T';
    ItoaPadded(ltmSlave.uav_bat/100, screenBuffer+4,4,3);
    OSD.writeString(screenBuffer,318);  
}

void displayRSSISl(void){
      for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    screenBuffer[0]='R';
    screenBuffer[1]='S';
    screenBuffer[2]='S';
    screenBuffer[3]='I';
    ItoaPadded(ltmSlave.uav_rssi, screenBuffer+5,2,0);
    OSD.writeString(screenBuffer,302);
}







void displayCount(uint8_t number){
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    ItoaPadded(number, screenBuffer,10,0);
    OSD.writeString(screenBuffer,100);
}

#define DECIMAL '.'

char *ItoaPadded(uint32_t val, char *str, uint8_t bytes, uint8_t decimalpos)  {
  // Val to convert
  // Return String
  // Length
  // Decimal position
  uint8_t neg = 0;
  if(val < 0) {
    neg = 1;
    val = -val;
  } 

  str[bytes] = 0;
  for(;;) {
    if(bytes == decimalpos) {
      str[--bytes] = DECIMAL;
      decimalpos = 0;
    }
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    if(bytes == 0 || (decimalpos == 0 && val == 0))
      break;
  }

  if(neg && bytes > 0)
    str[--bytes] = '-';

  while(bytes != 0)
    str[--bytes] = ' ';
  return str;
}


/* *********************************************** */
/* ******** functions used in main loop() ******** */
/*
void osd_refresh()
{
    setHeadingPatern();  // generate the heading patern

    setVars(osd); 
    
    if (osd_enabled) {
        writePanels();       // writing enabled panels (check OSD_Panels Tab)
    }
    else if (osd_clear = 1) {
        osd.clear();
        osd_clear = 0;
    }
    
}


void unplugSlaves(){
    //Unplug list of SPI
    digitalWrite(MAX7456_SELECT,  HIGH); // unplug OSD
}*/
