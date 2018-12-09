/*


Program  : BuddyOSD
Version  : V1.0
Author(s): Jelle L

BuddyOSD is forked from GhettOSD. Mavlink support has been removed. 

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

//Upload once with LOADFONT to upload font, upload second without LOADFONT to use BuddyOSD
//#define LOADFONT

/* *************************************************/
/* ***************** DEFINITIONS *******************/

//OSD Hardware 
//#define MinimOSD
#define TELEMETRY_SPEED_MASTER  9600  // How fast our master LTM telemetry is coming to Serial port
#define TELEMETRY_SPEED_SIK 57600 // How fast our slave LTM telemetry is coming to Serial port


// Objects and Serial definitions
AltSoftSerial masterSerial(8, 9);

LightTelemetry ltmMaster;
LightTelemetry ltmSiK;
Max7456 OSD;

/* **********************************************/
/* ***************** SETUP() *******************/

void setup() 
{
    OSD.init();
//    pinMode(MAX7456_SELECT,  OUTPUT); // OSD CS

    Serial.begin(TELEMETRY_SPEED_SIK);
    masterSerial.begin(TELEMETRY_SPEED_MASTER); 

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
    ltmMaster.init(&masterSerial);
    ltmSiK.init(&Serial);

} // END of setup();



/* ***********************************************/
/* ***************** MAIN LOOP *******************/

// Mother of all happenings, The loop()
// As simple as possible.

uint8_t count = 0;
unsigned long previousMillis = 0;
bool newLtmMaster = false;
bool newLtmSiK = false;

void loop(){
    #ifdef LOADFONT
        OSD.writeString_P(PSTR("DO NOT POWER OFF"), 32);
        OSD.drawScreen();      
        delay(3000);
        OSD.displayFont();  
        OSD.writeString_P(PSTR("SCREEN WILL GO BLANK"), 32);
        OSD.drawScreen();
        delay(3000);
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
    #else //LOADFONT
        newLtmMaster = ltmMaster.read();
        newLtmSiK = ltmSiK.read();
        /*unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= 80) {
        previousMillis = currentMillis;*/
        if (newLtmMaster || newLtmSiK){
            displayBuddy();
            displayBuddyTelemetry();
            //displayCount(count++);
            //OSD.writeString_P(PSTR("JELLE LECOMTES BUDDY OSD"), 32);
            //testdata here:
            /*
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
            */
            OSD.drawScreen();
            if(newLtmMaster){
              //retransmit relevant message to slave
            }
            newLtmMaster = false;
            newLtmSiK = false;
        }
    #endif //LOADFONT

}

char screenBuffer[20];
//Symbols
#define SYM_KMH 0xA1
#define SYM_ALT 0xB1
#define SYM_DST 0xB5
#define SYM_ARROW 0x60 //+16


void displayBuddy(void){
    float dstlon, dstlat;
    uint16_t buddyDistance;
    uint16_t buddyDirectionX;
    int buddyAltitudeDiff;
    //float        osd_home_lon
    //osd_home_lon = (int32_t)ltmread_u32() / 10000000.0;
    //shrinking factor for longitude going to poles direction
    float rads = fabs(ltmSiK.uav_lat/ 10000000.0) * 0.0174532925;
    double scaleLongDown = cos(rads);
    double scaleLongUp   = 1.0f/cos(rads);
    dstlat = fabs(ltmSiK.uav_lat/ 10000000.0 - ltmMaster.uav_lat/ 10000000.0) * 111319.5;
    dstlon = fabs(ltmSiK.uav_lon/ 10000000.0 - ltmMaster.uav_lon/ 10000000.0) * 111319.5 * scaleLongDown;
    buddyDistance = sqrt(sq(dstlat) + sq(dstlon));
    //DIR to Home
    dstlon = (ltmSiK.uav_lon/ 10000000.0 - ltmMaster.uav_lon/ 10000000.0); //OffSet_X
    dstlat = (ltmSiK.uav_lat/ 10000000.0 - ltmMaster.uav_lat/ 10000000.0) * scaleLongUp; //OffSet Y
    buddyDirectionX = (630 + (atan2(dstlat, -dstlon) * 57.295775)- ltmMaster.uav_heading); //absolut home direction (rads to degrees)
    buddyDirectionX %= 360;
    buddyAltitudeDiff = (ltmSiK.uav_alt - ltmMaster.uav_alt)/10; //cm //should be signed
    
    screenBuffer[0]=SYM_ARROW+((int)((buddyDirectionX+11)/22.5))%16;
    screenBuffer[1]=0;
    OSD.writeString(screenBuffer,362);
    
    ItoaPadded(buddyDistance, screenBuffer,4,0);
    screenBuffer[4]=SYM_DST;
    screenBuffer[5]=0;
    OSD.writeString(screenBuffer,364);
    
    ItoaPadded(buddyAltitudeDiff, screenBuffer,6,5);
    screenBuffer[6]=SYM_ALT;
    screenBuffer[7]=0;
    OSD.writeString(screenBuffer,370);

    // < sqrt(2)*FOV/2 > (360 - sqrt(2)*FOV/2  FOV=150->107 FOV=180->130
    if(buddyDirectionX<107||buddyDirectionX>253){
        uint16_t buddyDirectionY;
        buddyDirectionY = (720 + (atan2(-buddyAltitudeDiff/10.0, buddyDistance) * 57.295775) - ltmMaster.uav_pitch);
        buddyDirectionY %= 360;
        uint16_t roll = ltmMaster.uav_roll+360;
        roll %= 360;
        uint16_t buddyDirectionXAfterRoll =  720 + 75 + (buddyDirectionX * cos((roll) * 0.0174532925) + buddyDirectionY * sin((roll) * 0.0174532925)); //+FOV/2 FOV=150->75 FOV=180->90
        buddyDirectionXAfterRoll %= 360;
        if(buddyDirectionXAfterRoll < 150){ // <FOV FOV=150->150 FOV=180->180
            uint16_t buddyDirectionYAfterRoll = 720 + 75 + (buddyDirectionY * cos((roll) * 0.0174532925) - buddyDirectionX * sin((roll) * 0.0174532925)); //+FOV/2 FOV=150->75 FOV=180->90
            buddyDirectionYAfterRoll %= 360;
            if(buddyDirectionYAfterRoll < 150){ // <FOV FOV=150->150 FOV=180->180// <FOV FOV=150->150 FOV=180->180
                uint16_t buddyMark = buddyDirectionXAfterRoll/5 + buddyDirectionYAfterRoll/10*30; 
                // /FOV/30 (only integers, round up) FOV=180->/6 FOV=150->/5                    // /FOV/16(=PAL=480/30) or /FOV/12(=NTSC=390/30) (only integers, round up) FOV=180->/12 FOV=150->/10
                OSD.writeString_P(PSTR("X"), buddyMark);
                //Testoverlay here:
                /*
                uint16_t temp = 720 + 75 + buddyDirectionX;
                temp %= 360;
                ItoaPadded(temp/6, screenBuffer,4,0);
                OSD.writeString(screenBuffer,31);
                temp = 720 + 75 + buddyDirectionY;
                temp %= 360;
                ItoaPadded(temp/12, screenBuffer,4,0);
                OSD.writeString(screenBuffer,61);
                ItoaPadded(buddyDirectionXAfterRoll/6, screenBuffer,4,0);
                OSD.writeString(screenBuffer,41);
                ItoaPadded(buddyDirectionYAfterRoll/12, screenBuffer,4,0);
                OSD.writeString(screenBuffer,71);
                */
            }
        }
    }
}

void displayBuddyTelemetry(void){
    uint16_t buddyRelDirection;
    buddyRelDirection = (360 + ltmSiK.uav_heading - ltmMaster.uav_heading)%360;
    screenBuffer[0]=SYM_ARROW+((int)((buddyRelDirection+11)/22.5))%16;
    screenBuffer[1]=0;
    OSD.writeString(screenBuffer,380);
    ItoaPadded(ltmSiK.uav_groundspeed, screenBuffer,3,0);
    screenBuffer[3]=SYM_KMH;
    screenBuffer[4]=0;
    OSD.writeString(screenBuffer,382);
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
    ItoaPadded(ltmSiK.uav_pitch, screenBuffer+2,3,0);
    OSD.writeString(screenBuffer,332);
    screenBuffer[0]='R';
    ItoaPadded(ltmSiK.uav_roll, screenBuffer+2,3,0);
    OSD.writeString(screenBuffer,338);
    screenBuffer[0]='H';
    ItoaPadded(ltmSiK.uav_heading, screenBuffer+2,3,0);
    OSD.writeString(screenBuffer,344);
}

void displayCoordsSl(void){
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    ItoaPadded(ltmSiK.uav_lat, screenBuffer,10,3);
    OSD.writeString(screenBuffer,243);
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    ItoaPadded(ltmSiK.uav_lon, screenBuffer,10,3);
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
    ItoaPadded(ltmSiK.uav_satellites_visible, screenBuffer+5,2,0);
    OSD.writeString(screenBuffer,310);
}

void displayBatSl(void){
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    screenBuffer[0]='B';
    screenBuffer[1]='A';
    screenBuffer[2]='T';
    ItoaPadded(ltmSiK.uav_bat/100, screenBuffer+4,4,3);
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
    ItoaPadded(ltmSiK.uav_rssi, screenBuffer+5,2,0);
    OSD.writeString(screenBuffer,302);
}







void displayCount(uint8_t number){
    for(int i=0;i<20; i++){
      screenBuffer[i]=' ';
    }
    ItoaPadded(number, screenBuffer,10,0);
    OSD.writeString(screenBuffer,100);
}

#define DECIMAL '.' //0X2E

char *ItoaPadded(int32_t val, char *str, uint8_t bytes, uint8_t decimalpos)  {
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
