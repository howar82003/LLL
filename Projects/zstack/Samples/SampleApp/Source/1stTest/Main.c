#include "Sensor.h"
#include "Fan.h"

main()
{
  Init_Sensor();
  while(1){
    SensorFunction();
  }
}