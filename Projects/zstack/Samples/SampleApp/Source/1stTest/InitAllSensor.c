#include <iocc2530.h>
#include "Sensor.h"
#include "UART.H"
void initAllSensor(void)
{
  InitUart();    
  Init_PM25();
 // initFan();                                  //Initial Fan PWM
  Init_TGS4161();
  //Init_MQ7();  
}