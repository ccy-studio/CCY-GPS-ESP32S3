# MAX17048

Arduino library for [MAX17048/MAX17049](https://www.maximintegrated.com/en/products/power/battery-management/MAX17048.html) 1-Cell/2-Cell Fuel Gauge with ModelGauge


## Usage

```C++
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
```

## License

MIT
