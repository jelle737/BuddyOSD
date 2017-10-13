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
    char *screenp = &screen[addr];
    while (*string) {
        *screenp++ = *string++;
    }
}

void Max7456::writeString_P(const char *string, int addr){
    char c;
    char *screenp = &screen[addr];
    while((c = (char)pgm_read_byte(string++)) != 0){
        *screenp++ = c;
    }
}

void Max7456::drawScreen(){

    enable();

    send(MAX7456ADD_DMAH, 0);
    send(MAX7456ADD_DMAL, 0);
    send(MAX7456ADD_DMM, 1);

    //#ifdef USE_VSYNC
    //MAX7456_WaitVSYNC();
    //#endif

    for(uint16_t xx = 0; xx < MAX_screen_size; ++xx) {
    //#ifdef USE_VSYNC
    //    // We don't actually need this?
    //    if (xx == 240)
    //        MAX7456_WaitVSYNC(); // Don't need this?
    //#endif

    //#ifdef INVERTED_CHAR_SUPPORT
    //    bool invActive = false;
    //    if (!invActive && bitISSET(screenAttr, xx)) {
    //    MAX7456_Send(MAX7456ADD_DMM, 1|(1<<3));
    //    invActive = true;
    //    } else if (invActive && bitISCLR(screenAttr, xx)) {
    //    MAX7456_Send(MAX7456ADD_DMM, 1);
    //    invActive = false;
    //    }
    //#endif

        send(MAX7456ADD_DMDI, screen[xx]);

    //#ifdef CANVAS_SUPPORT
    //    if (!canvasMode) // Don't erase in canvas mode
    //#endif
        //{
        screen[xx] = ' ';  //Jelle: Needs clearing after putting on screen?
        //#ifdef INVERTED_CHAR_SUPPORT
        //bitCLR(screenAttr, xx);
        //#endif
        //}
    }

    send(MAX7456ADD_DMDI, END_string);
    send(MAX7456ADD_DMM, 0);

    disable();

}

void Max7456::send(uint8_t add, uint8_t data){
  spi_transfer(add);
  spi_transfer(data);
}

void Max7456::writeNVM(uint8_t char_address, uint8_t* fontData){
    enable();
    send(MAX7456ADD_VM0, (MAX_screen_size==480?VIDEO_MODE_PAL:VIDEO_MODE_NTSC));
    send(MAX7456ADD_CMAH, char_address); // set start address high

    for(uint8_t x = 0; x < 54; x++) {// write out 54 bytes of character to shadow ram // 54 was NVM_ram_size
        send(MAX7456ADD_CMAL, x); // set start address low
        send(MAX7456ADD_CMDI, fontData[x]);
    }

    // transfer 54 bytes from shadow ram to NVM
    send(MAX7456ADD_CMM, WRITE_nvr);

    // wait until bit 5 in the status register returns to 0 (12ms)
    while ((spi_transfer(MAX7456ADD_STAT) & STATUS_reg_nvr_busy) != 0x00);

    send(MAX7456ADD_VM0, OSD_ENABLE|VERTICAL_SYNC_NEXT_VSYNC|(MAX_screen_size==480?VIDEO_MODE_PAL:VIDEO_MODE_NTSC)); // turn on screen next vertical
    
    disable();
    //delay(20);

}

void Max7456::checkStatus(void){
    uint8_t srdata;
    enable();

    spi_transfer(MAX7456ADD_STAT);
    srdata = spi_transfer(0xFF);
    srdata &= B00000011;
    if ((MAX_screen_size == 480?B00000001:B00000010) != srdata) {
        init();
    }

}

void Max7456::displayFont(void){
    for(uint8_t x = 0; x < 255; x++) {
        screen[90+x] = x;
    }
}

void Max7456::updateFont(void){
    for(uint8_t x = 0; x < 255; x++){
        uint8_t fontData[54];
        for(uint8_t i = 0; i < 54; i++){
          
            fontData[i] = (uint8_t)pgm_read_byte(fontdata+(64*x)+i);
            
        }
        writeNVM(x, &fontData[0]);
        //ledstatus=!ledstatus;
        /*if (ledstatus==true){
            digitalWrite(LEDPIN,HIGH);
        }
        else{
            digitalWrite(LEDPIN,LOW);
        }*/
        delay(20); // Shouldn't be needed due to status reg wait. Jelle: Doesn't work without the wait
    }
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
