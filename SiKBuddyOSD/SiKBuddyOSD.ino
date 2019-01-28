/*


Program  : BuddyOSD
Version  : V1.0
Author(s): Jelle L

BuddyOSD is forked from GhettOSD. Mavlink support has been removed, it supports LTM telemetry.
some code highly based on iNav.

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
#define TELEMETRY_SPEED_MASTER  2400  // How fast our master LTM telemetry is coming to Serial port
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
    Serial.begin(TELEMETRY_SPEED_SIK);
    masterSerial.begin(TELEMETRY_SPEED_MASTER); 
    ltmMaster.init(&masterSerial);
    ltmSiK.init(&Serial);

} // END of setup();



/* ***********************************************/
/* ***************** MAIN LOOP *******************/

// Mother of all happenings, The loop()
// As simple as possible.

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
bool newLtmMaster = false;
bool newLtmSiK = false;
uint8_t statusMaster = 0;
uint8_t statusSiK = 0;

void loop(){
    #ifdef LOADFONT
        OSD.writeString_P(PSTR("DO NOT POWER OFF"), 32);
        OSD.drawScreen();
        previousMillis = millis();
        while(millis() - previousMillis < 3000){
        }
        OSD.displayFont();
        OSD.writeString_P(PSTR("SCREEN WILL GO BLANK"), 32);
        OSD.drawScreen();
        previousMillis = millis();
        while(millis() - previousMillis < 3000){
        }
        OSD.updateFont();
        OSD.init();
        OSD.displayFont();
        OSD.writeString_P(PSTR("UPDATE COMPLETE"), 32);
        OSD.drawScreen();
        while(true){
        }
    #else //LOADFONT
        newLtmMaster = ltmMaster.read();
        newLtmSiK = ltmSiK.read();
        if (newLtmMaster || newLtmSiK){
            currentMillis = millis();
            if(newLtmSiK){
                //new message from slave, update time
                previousMillis = currentMillis;
            }

            if(currentMillis - previousMillis < 5000){
                //only show the radar if last new message from buddy was < 5s ago. After +-5 seconds of inactivity the radar disapears, if there was an update from the Master.
                displayBuddyRadar();
            }

            //status icons
            statusMaster = displayStatus(statusMaster, newLtmMaster, 27);
            statusSiK = displayStatus(statusSiK, newLtmSiK, 28);

            OSD.drawScreen();
            if(newLtmMaster && ltmMaster.uav_satellites_visible > 5){
              //retransmit relevant message to slave only if GPS>=6.
              ltmMaster.transmit(&Serial);
            }
            newLtmMaster = false;
            newLtmSiK = false;
        }
    #endif //LOADFONT

}

//char screenBuffer[20];
char buff[32];
char statusSymbol[4] = {'-','\\','|','/'};

uint8_t displayStatus(uint8_t numberStatus, bool newStatus,int locationStatus){
  numberStatus = (newStatus?(numberStatus+1)%4:numberStatus);
  OSD.writeChar(locationStatus, 13, statusSymbol[numberStatus]);
  return numberStatus;
}

/*
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
                *//*
            }
        }
    }
}*/

/*
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
*/


void displayBuddyRadar(void){
    const uint8_t minX = 1;
    const uint8_t midX = 14;
    const uint8_t maxX = 28;
    const uint8_t minY = 1;
    const uint8_t maxY = 14;
    const uint8_t midY = 7;

    const int charWidth = 12;
    const int charHeight = 18;

    float rads = fabs(ltmMaster.uav_lat/ 10000000.0) * 0.0174532925;
    double GPS_scaleLonDown = cos(rads);
    float dLat = (ltmSiK.uav_lat  - ltmMaster.uav_lat);
    float dLon = (float)(ltmSiK.uav_lon  - ltmMaster.uav_lon) * GPS_scaleLonDown;
    uint32_t buddyDistance = sqrt(sq(dLat) + sq(dLon)) * 1.113195 / 100; //[m]
    int32_t dir = (int32_t)(360 + 90 + (atan2(-dLat, dLon)) / 3.14159265358979323846 * 180.0)%360; //[°]      
    int32_t buddyDirection = (360 + dir - ltmMaster.uav_heading + 180)%360;

    OSD.writeChar(minX, minY, SYM_SCALE); //175
    OSD.writeChar(midX, midY, SYM_ARROW); //0x60 = 96

    uint32_t scale = 10;//[m] // 10m as initial scale
    if (buddyDistance > scale) {
        float poiAngle = buddyDirection*3.14159265358979323846/180.0;
        float poiSin = sin(poiAngle);
        float poiCos = cos(poiAngle);

        // Now start looking for a valid scale that lets us draw everything
        for (int ii = 0; ii < 50; ii++, scale *= 2) {
            // Calculate location of the aircraft in map
            int points = buddyDistance / (1.0 * scale / charHeight);
        
            float pointsX = 1.0 * points * poiSin;
            int poiX = midX - round(pointsX / charWidth);
            if (poiX < minX || poiX > maxX) {
                continue;
            }

            float pointsY = 1.0 * points * poiCos;
            int poiY = midY + round(pointsY / charHeight);
            if (poiY < minY || poiY > maxY) {
                continue;
            }

            if (poiX == midX && poiY == midY) {
                // We're over the map center symbol, so we would be drawing
                // over it even if we increased the scale. No reason to run
                // this loop 50 times.
                uint16_t buddyRelativeDirection = (360 + ltmSiK.uav_heading - ltmMaster.uav_heading)%360;
                OSD.writeChar(poiX, poiY, SYM_ARROW+((uint8_t)((buddyRelativeDirection+11)/22.5))%16);
                break;
            }

            uint16_t buddyRelativeDirection = (360 + ltmSiK.uav_heading - ltmMaster.uav_heading)%360;
            OSD.writeChar(poiX, poiY, SYM_ARROW+((uint8_t)((buddyRelativeDirection+11)/22.5))%16);
            break;
        }
    }

    // Draw the used scale
    if(osdFormatCentiNumber(buff, scale * 100, 1000, 0, 2, 3)){
        buff[3] = SYM_KM;
    }else{
        buff[3] = SYM_M;
    }
    buff[4] = '\0';
    OSD.writeString2(minX + 1, minY, buff);

    //Draw distance
    if (osdFormatCentiNumber(buff + 1, buddyDistance*100, 1000, 0, 2, 3)) {
        buff[0] = SYM_DIST_KM;
    } else {
        buff[0] = SYM_DIST_M;
    }
    OSD.writeString2(4,13,buff);

    //Draw altitudedifference
    int buddyAltitudeDifference = ltmSiK.uav_alt - ltmMaster.uav_alt;
    if (osdFormatCentiNumber(buff+1, buddyAltitudeDifference, 1000, 0, 2, 3)) {
        // Scaled to km
        buff[0] = SYM_ALT_KM;
    } else {
        // Formatted in m
        buff[0] = SYM_ALT_M;
    }
    OSD.writeString2(9,13,buff);

    //Draw speed
    osdFormatCentiNumber(buff, ltmSiK.uav_groundspeed * 100, 0, 0, 3, 3);
    buff[3] = SYM_KMH;
    buff[4] = '\0';
    OSD.writeString2(14,13,buff);

}

/**
 * Formats a number given in cents, to support non integer values
 * without using floating point math. Value is always right aligned
 * and spaces are inserted before the number to always yield a string
 * of the same length. If the value doesn't fit into the provided length
 * it will be divided by scale and true will be returned.
 */
static bool osdFormatCentiNumber(char *buff, int32_t centivalue, uint32_t scale, int maxDecimals, int maxScaledDecimals, int length){
    char *ptr = buff;
    char *dec;
    int decimals = maxDecimals;
    bool negative = false;
    bool scaled = false;

    buff[length] = '0';

    if (centivalue < 0) {
        negative = true;
        centivalue = -centivalue;
        length--;
    }

    int32_t integerPart = centivalue / 100;
    // 3 decimal digits
    int32_t millis = (centivalue % 100) * 10;

    int digits = digitCount(integerPart);
    int remaining = length - digits;

    if (remaining < 0 && scale > 0) {
        // Reduce by scale
        scaled = true;
        decimals = maxScaledDecimals;
        integerPart = integerPart / scale;
        // Multiply by 10 to get 3 decimal digits
        millis = ((centivalue % (100 * scale)) * 10) / scale;
        digits = digitCount(integerPart);
        remaining = length - digits;
    }

    // 3 decimals at most
    decimals = (remaining < (decimals < 3?decimals:3)?remaining:(decimals < 3?decimals:3));
    remaining -= decimals;

    // Done counting. Time to write the characters.

    // Write spaces at the start
    while (remaining > 0) {
        *ptr = SYM_BLANK;
        ptr++;
        remaining--;
    }

    // Write the minus sign if required
    if (negative) {
        *ptr = '-';
        ptr++;
    }
    // Now write the digits.
    ui2a(integerPart, 10, 0, ptr);
    ptr += digits;
    if (decimals > 0) {
        *(ptr-1) += SYM_ZERO_HALF_TRAILING_DOT - '0';
        dec = ptr;
        int factor = 3; // we're getting the decimal part in millis first
        while (decimals < factor) {
            factor--;
            millis /= 10;
        }
        int decimalDigits = digitCount(millis);
        while (decimalDigits < decimals) {
            decimalDigits++;
            *ptr = '0';
            ptr++;
        }
        ui2a(millis, 10, 0, ptr);
        *dec += SYM_ZERO_HALF_LEADING_DOT - '0';
    }
    return scaled;
}

int digitCount(int32_t value){
    int digits = 1;
    value/=10;
    while(value){
        digits++;
        value/=10;
    }
    return digits;
}

void ui2a(unsigned int num, unsigned int base, int uc, char *bf){
    int n = 0;
    unsigned int d = 1;
    while (num / d >= base)
        d *= base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= base;
        if (n || dgt > 0 || d == 0) {
            *bf++ = dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *bf = 0;
}


//==master==
/*
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
    ItoaPadded(ltmMaster.uav_lat, buff,10,3);
    OSD.writeString2(3, 4, buff);
    ItoaPadded(ltmMaster.uav_lon, buff,10,3);
    OSD.writeString2(3, 5, buff);
}

void displaySats(void){
    buff[0] = SYM_SAT_L;
    buff[1] = SYM_SAT_R;
    ItoaPadded(ltmMaster.uav_satellites_visible, buff+2,2,0);
    OSD.writeString2(10,6,buff);
}

void displayBat(void){
    buff[0] = SYM_BATT_FULL + constrain((ltmMaster.uav_bat - 144) * 100 / (168 - 144),0,100);
    buff[1] = '\0';
    OSD.writeString2(18, 6, buff);
    ItoaPadded(ltmMaster.uav_bat/100, buff,4,3);
    OSD.writeString2(19, 6, buff);
}

void displayRSSI(void){
    buff[0] = SYM_RSSI;
    ItoaPadded(ltmSiK.uav_rssi, buff+1,2,0);
    OSD.writeString2(2,6,buff);
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
*/
