#include "Sensor.h"
unsigned char SensorsData[14] = {0};

void SendData(unsigned int i, unsigned char Data)
{ 
  SensorsData[i] = Data;
}


unsigned char ReadData(unsigned int i)
{
  return SensorsData[i];
}

/*
  for(i=0;i<13;i++)
{
  buffer[i]=ReadData(i);
}
*/