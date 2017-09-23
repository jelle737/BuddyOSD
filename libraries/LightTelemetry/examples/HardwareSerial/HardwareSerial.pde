#include <Lighttelemetry.h>

LightTelemetry ltmHardwarerSerial;

void setup() 
{
    Serial.begin(2400);

    ltmHardwarerSerial.init(&Serial);
}

void loop(){
      ltmHardwarerSerial.read();
}
