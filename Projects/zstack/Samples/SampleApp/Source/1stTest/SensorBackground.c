#include <iocc2530.h>
#include "Sensor.h"

void SensorBackground(void)
{
  DHT22_background();
  background_TGS4161();
  background_PM25();
}