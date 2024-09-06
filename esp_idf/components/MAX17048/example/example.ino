#include "MAX17048.h"

MAX17048 pwr_mgmt;

void setup()
{
    Serial.begin(115200);
    Wire.begin();
    pwr_mgmt.attatch(Wire);
}

void loop()
{
    Serial.print("VCELL ADC : ");
    Serial.println(pwr_mgmt.adc());
    Serial.print("VCELL V   : ");
    Serial.println(pwr_mgmt.voltage());
    Serial.print("VCELL SOC : ");
    Serial.print(pwr_mgmt.percent());
    Serial.println(" \%");
    Serial.print("VCELL SOC : ");
    Serial.print(pwr_mgmt.accuratePercent());
    Serial.println(" \%");
    Serial.println();
    delay(1000);
}
