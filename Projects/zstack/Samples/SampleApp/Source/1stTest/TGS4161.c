#include <iocc2530.h>
#include "UART.H"
#include "math.h"
#include "Sensor.h"

extern unsigned int Shiwen(void);
extern void SendData(unsigned int i, unsigned char Data);

unsigned int Temp_TGS4161[10] = {0};

/*************************************************************
                  TGS4161 ,Set P0_3 input 
*************************************************************/
void Init_TGS4161(void)
{
  P0DIR &= ~0x01;                   //P0_0-->Inp
  ADCCFG |= 0x01;                   //Set P0_0 Analog Periferal I/O
}
void background_TGS4161(void)
{
  unsigned int  value = 0, MSD = 0, LSD = 0;
  static int i_TGS4161 = 0;
  
  ADCCON1 = 0x33;
  ADCCON3 = 0x00;
  
  ADCCON1 |= 0x30;                  //Start a conversion sequence if ADCCON1.STSEL = 11 and no sequence is running.
                                    
  ADCCON3 |= 0x80;                  //AVDD5 pin
  ADCCON3 |= 0x30;                  //512 decimation rate (12 bits ENOB)
  ADCCON3 |= 0x00;                  //AIN0
                   
  ADCCON1 |= 0x40;                  //Start conversion
  while(!(ADCCON1 & 0x80));        

  LSD = ADCL;
  LSD = LSD >>4;
  MSD = ADCH;
  MSD = MSD <<4;
  value = LSD + MSD;                

  Temp_TGS4161[i_TGS4161++] = value;
  if(i_TGS4161 == 10)i_TGS4161=0;
}

unsigned int TGS4161_EMF_350ppm(void)
{
  unsigned int EMF1,ShiWen;
  ShiWen = Shiwen();

  if(ShiWen <=400 && ShiWen >300)
  {
    ShiWen = ShiWen-300;
    EMF1 = 306+ShiWen*8/100;
    return EMF1;
  }
  
  if(ShiWen <=300 && ShiWen >200)
  {
    ShiWen = ShiWen-200;
    EMF1 = 300+ShiWen*6/100;
    return EMF1;
  }
  
  if(ShiWen <=200 && ShiWen >100)
  {
    ShiWen = ShiWen-100;
    EMF1 = 293+ShiWen*7/100;
    return EMF1;
  }
  return 0;
}

void Read_TGS4161(void)
{  
/*************************************************/  
  unsigned int i =0, EMF2 = 0, Delta_EMF = 0, EMF1 = 0, temp = 0;
  float X = 0;
  EMF1 = TGS4161_EMF_350ppm(); 
  while(i < 10)temp = temp + Temp_TGS4161[i++];
  temp = temp /10;
  X = (float)temp*1000/4096;
  EMF2 = (int)X;
  Delta_EMF = EMF1 - EMF2;
/*************************************************/  
  unsigned char UnderThreshold[37] = "Concentration of CO2 is under 350ppm.",
       str_TGS4161[11] = "CO2:    ppm";
  unsigned int GasCon = 0;
  
  if( EMF2 > EMF1)
  {
    UartSendString(UnderThreshold, 37);
    SendData(10,0xff);
    SendData(11,0xff);
    SendData(12,0xff);
    SendData(13,0xff);
  }
  else
  {
    if(Delta_EMF > 0 && Delta_EMF <= 15)
      GasCon = Delta_EMF*50/3+350;
    if(Delta_EMF >15 && Delta_EMF <= 27)
      GasCon = Delta_EMF*100/3+600;
    if(Delta_EMF >27 && Delta_EMF <= 75)
      GasCon = Delta_EMF*625/6+1000;
    
    str_TGS4161[4] = GasCon/1000 +0x30;
    str_TGS4161[5] = GasCon/100%10 +0x30;
    str_TGS4161[6] = GasCon%100/10 +0x30;
    str_TGS4161[7] = GasCon%10 +0x30;
    
    SendData(10,str_TGS4161[4]);
    SendData(11,str_TGS4161[5]);
    SendData(12,str_TGS4161[6]);
    SendData(13,str_TGS4161[7]);
    /*
    UartSendString(str_TGS4161, 11);*/
  }
}

