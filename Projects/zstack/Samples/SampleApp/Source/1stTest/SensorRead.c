#include <iocc2530.h>
#include "Sensor.h"

void SensorRead(void)
{
  ReadDHT22();
  Read_TGS4161();
  Read_PM25();
}