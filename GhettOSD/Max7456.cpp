#include "Max7456.h"

Max7456::Max7456(){/*Nothing to do here*/}

Max7456::~Max7456(){/*Nothing to destruct*/}

void Max7456::init(){
    setHardwarePorts();
    uint8_t MAX7456_reset;
    uint8_t MAX_screen_rows;
    disable();
    hwReset();

    // SPCR = 01010000
    //interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
    //sample on leading edge of clk,system clock/4 rate (4 meg)
    //SPI2X will double the rate (8 meg)
    //setting up SPI the DIY way https://www.arduino.cc/en/Tutorial/SPIEEPROM
    
    SPCR = (1<<SPE)|(1<<MSTR); //setting spi enable and spi as master
    SPSR = (1<<SPI2X); //setting double the spi speed
    uint8_t clr;
    clr=SPSR; //clearing spurious data in status register
    clr=SPDR; //clearing spurious data in data regsiter
    delay(10);

    enable();

    uint8_t srdata;
    spi_transfer(MAX7456ADD_STAT);
    srdata = spi_transfer(0xFF); 
    srdata &= B00000011;
    if (srdata == B00000001){      // PAL
        //Settings[S_VIDEOSIGNALTYPE]=1; 
        //flags.signaltype = 1;
        MAX7456_reset = 0x4C;
        MAX_screen_size = 480;
        MAX_screen_rows = 16;
    }else if (srdata == B00000010){ // NTSC
        //Settings[S_VIDEOSIGNALTYPE]=0;
        //flags.signaltype = 0;
        MAX7456_reset=0x0C;
        MAX_screen_size = 390;
        MAX_screen_rows=13;
    }else{
        //flags.signaltype = 2 + Settings[S_VIDEOSIGNALTYPE]; // NOT DETECTED    
        MAX7456_reset=0x0C;
        MAX_screen_size = 390;
        MAX_screen_rows=13;
    }

    send(MAX7456ADD_VM0, MAX7456_reset);
    disable();
    //readEEPROM_screenlayout();
}

uint8_t Max7456::spi_transfer(uint8_t data){
    SPDR = data;                    // Start the transmission
    while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
    ;
    return SPDR;                    // return the received byte
}


void Max7456::writeString(const char *string, int addr){


}

void Max7456::writeString_P(const char *string, int Adresse){


}

void Max7456::drawScreen(){


}

void Max7456::send(uint8_t add, uint8_t data){
  spi_transfer(add);
  spi_transfer(data);
}

void Max7456::writeNVM(uint8_t char_address){


}

void Max7456::checkStatus(void){


}

void Max7456::displayFont(void){


}

void Max7456::updateFont(void){


}


void Max7456::setHardwarePorts(){
    pinMode(MAX7456RESET,OUTPUT);
    pinMode(MAX7456SELECT,OUTPUT);
    pinMode(DATAOUT, OUTPUT);
    pinMode(DATAIN, INPUT);
    pinMode(SPICLOCK,OUTPUT);
    pinMode(VSYNC, INPUT);
}

void Max7456::hwReset(){
    digitalWrite(MAX7456RESET,LOW);
    delay(60);
    digitalWrite(MAX7456RESET,HIGH);
    delay(40);
}

void Max7456::enable(){
    digitalWrite(MAX7456SELECT,LOW);
}

void Max7456::disable(){
    digitalWrite(MAX7456SELECT,HIGH);
}
