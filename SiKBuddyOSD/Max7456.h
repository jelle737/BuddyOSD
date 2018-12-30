#ifndef Max7456_h
#define Max7456_h

#include "Arduino.h"

#include "Max7456font.h"

#define DATAOUT          11 // MOSI
#define DATAIN           12 // MISO
#define SPICLOCK         13 // sck
#define VSYNC             2 // INT0
#define MAX7456SELECT     6 // ss
#define MAX7456RESET     10 // RESET

#define MAX7456ADD_VM0          0x00  //0b0011100// 00 // 00             ,0011100
#define MAX7456ADD_VM1          0x01
#define MAX7456ADD_HOS          0x02
#define MAX7456ADD_VOS          0x03
#define MAX7456ADD_DMM          0x04
#define MAX7456ADD_DMAH         0x05
#define MAX7456ADD_DMAL         0x06
#define MAX7456ADD_DMDI         0x07
#define MAX7456ADD_CMM          0x08
#define MAX7456ADD_CMAH         0x09
#define MAX7456ADD_CMAL         0x0a
#define MAX7456ADD_CMDI         0x0b
#define MAX7456ADD_OSDM         0x0c
#define MAX7456ADD_RB0          0x10
#define MAX7456ADD_RB1          0x11
#define MAX7456ADD_RB2          0x12
#define MAX7456ADD_RB3          0x13
#define MAX7456ADD_RB4          0x14
#define MAX7456ADD_RB5          0x15
#define MAX7456ADD_RB6          0x16
#define MAX7456ADD_RB7          0x17
#define MAX7456ADD_RB8          0x18
#define MAX7456ADD_RB9          0x19
#define MAX7456ADD_RB10         0x1a
#define MAX7456ADD_RB11         0x1b
#define MAX7456ADD_RB12         0x1c
#define MAX7456ADD_RB13         0x1d
#define MAX7456ADD_RB14         0x1e
#define MAX7456ADD_RB15         0x1f
#define MAX7456ADD_OSDBL        0x6c
#define MAX7456ADD_STAT         0xA0

//MAX7456 commands

#define WRITE_TO_MAX7456
#define NVM_ram_size 54

#define CLEAR_display 0x04
#define CLEAR_display_vert 0x06
#define END_string 0xff
#define WRITE_nvr 0xa0
// with NTSC
#define ENABLE_display 0x08
#define ENABLE_display_vert 0x0c
//#define MAX7456_reset 0x02
#define DISABLE_display 0x00
#define STATUS_reg_nvr_busy 0x20

// video mode register 0 bits
#define VIDEO_BUFFER_DISABLE 0x01
//#define MAX7456_RESET 0x02
#define VERTICAL_SYNC_NEXT_VSYNC 0x04
#define OSD_ENABLE 0x08
#define SYNC_MODE_AUTO 0x00
#define SYNC_MODE_INTERNAL 0x30
#define SYNC_MODE_EXTERNAL 0x20
#define VIDEO_MODE_PAL 0x40
#define VIDEO_MODE_NTSC 0x00

//Symbols
#define SYM_BLANK 0x20
#define SYM_KMH 0xA1
#define SYM_ALT 0xB1
#define SYM_DST 0xB5
#define SYM_ARROW 0x60 //+16
#define SYM_M 185
#define SYM_KM 186
#define SYM_SCALE 175
#define SYM_RSSI 0x01
#define SYM_SAT_L 0x1E
#define SYM_SAT_R 0x1F
#define SYM_BATT_FULL 0x90
#define SYM_COLON 0x2D
#define SYM_ZERO_HALF_TRAILING_DOT 192
#define SYM_ZERO_HALF_LEADING_DOT 208
#define SYM_ALT_M       177
#define SYM_ALT_KM      178
#define SYM_DIST_M      181
#define SYM_DIST_KM     182


class Max7456{
    public:
        Max7456();
        ~Max7456();
        void init();
        uint8_t spi_transfer(uint8_t data);
        void writeString(const char *string, int addr);
        void writeChar(uint8_t x, uint8_t y, uint8_t c);
        void Max7456::writeString2(uint8_t x, uint8_t y, char *s);
        void writeString_P(const char *string, int Adresse);
        void drawScreen();
        void send(uint8_t add, uint8_t data);
        void writeNVM(uint8_t char_address, uint8_t* fontData);
        void checkStatus();
        void displayFont();
        void updateFont();
        char screen[480];
    private:
        void setHardwarePorts();
        void hwReset();
        void enable();
        void disable();
        uint16_t MAX_screen_size;
};



#endif
