#include <AltSoftSerial.h>  
#include <Lighttelemetry.h>

AltSoftSerial softwareSerial(8, 9);

LightTelemetry ltmSoftwareSerial;
LightTelemetry ltmHardwarerSerial;

void setup() 
{
    Serial.begin(2400);
    softwareSerial.begin(2400); 

    ltmSoftwareSerial.init(&softwareSerial);
    ltmHardwarerSerial.init(&Serial);
}

void loop(){
      ltmSoftwareSerial.read();
      ltmHardwarerSerial.read();
}
