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

void loop() 
{
    #ifdef LOADFONT
        uint8_t fontStatus = 0;
        switch(fontStatus) {
            case 0:
            //OSD.writeString_P("DO NOT POWER OFF", 32);
            OSD.drawScreen();      
            delay(3000);
            OSD.displayFont();  
            //OSD.writeString_P("SCREEN WILL GO BLANK", 32);
            OSD.drawScreen();
            fontStatus++;
            delay(3000);      
            break;
            case 1:
            OSD.updateFont();
            OSD.init(); 
            //OSD.writeString_P("UPDATE COMPLETE", 32);
            OSD.displayFont();  
            OSD.drawScreen();
            fontStatus++;
            break;
        }
//        digitalWrite(LEDPIN,LOW);
        while(true);
    #else //LOADFONT


      ltmMaster.read();
      ltmSlave.read();
 //     ltmMaster.uav_rssi+ltmMaster.uav_linkquality;
    #endif //LOADFONT

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
