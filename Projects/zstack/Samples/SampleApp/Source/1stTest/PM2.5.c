#include <iocc2530.h>
#include "Sensor.h"
#include "UART.H"
extern void delay_25us(void);
extern void SendData(unsigned int i, unsigned char Data);

unsigned int Temp_PM25[10] = {0};

void Init_PM25(void)
{
  P0DIR &= ~0x20;                   //P0_5-->Inp
  P1DIR |= 0x02;                    //P1_1-->Output
  ADCCFG |= 0x20;                   //Set P0_5 Analog Periferal I/O
}
void background_PM25_ADC(void)
{
  unsigned int  value = 0, MSD = 0, LSD = 0;
  static int i = 0;
  
  ADCCON1 = 0x33;
  ADCCON3 = 0x00;
  
  ADCCON1 |= 0x30;                  //Start a conversion sequence if ADCCON1.STSEL = 11 and no sequence is running.
                                    
  ADCCON3 |= 0x80;                  //AVDD5 pin
  ADCCON3 |= 0x30;                  //512 decimation rate (12 bits ENOB)
  ADCCON3 |= 0x05;                  //AIN5
                   
  ADCCON1 |= 0x40;                  //Start conversion
  while(!(ADCCON1 & 0x80));        

  LSD = ADCL;
  LSD = LSD >>4;
  MSD = ADCH;
  MSD = MSD <<4;
  value = LSD + MSD;                
  
  Temp_PM25[i++] = value;
  if(i == 10)i=0;
}
void background_PM25(void)
{
  char i = 0;
  P1_1 = 1;
  while(i++ <14)delay_25us();
  background_PM25_ADC();
  P1_1 = 0;
}
void Read_PM25(void)
{
  unsigned char i ,PM25uchar[21] = "Dust Density:   ug/m3",
                UnderThreshold[31] = "Dust Density is under threshold";
  unsigned int Sum = 0;
  float X = 0;
  
  for(i = 0; i <10; i++)
    Sum = Temp_PM25[i] + Sum;
  Sum = Sum /10;
  if(Sum < 435)
    UartSendString(UnderThreshold, 31);
  else
  {
  X = (float)Sum*33/2048;                 //x10
  X = (X-7)/56*1000;                      //change the unit(mg/m3) to um/m3
  
  PM25uchar[13] = (int)X/100 +0x30;
  PM25uchar[14] = (int)X/10%10 +0x30;
  PM25uchar[15] = (int)X%10 +0x30;
  
  SendData(7,PM25uchar[13]);
  SendData(8,PM25uchar[14]);
  SendData(9,PM25uchar[15]);
  /*
  UartSendString(PM25uchar, 21);*/
  }
}
  