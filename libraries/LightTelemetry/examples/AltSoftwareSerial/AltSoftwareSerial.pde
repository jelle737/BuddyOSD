#include <AltSoftSerial.h>  
#include <Lighttelemetry.h>

AltSoftSerial softwareSerial(8, 9);

LightTelemetry ltmSoftwareSerial;

void setup() 
{
    softwareSerial.begin(2400); 

    ltmSoftwareSerial.init(&softwareSerial);
}

void loop(){
      ltmSoftwareSerial.read();
}
