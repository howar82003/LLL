#include <iocc2530.h>
#include "UART.H"
#include "DHT22.H"
#include "math.h"

#define __MS1100__
#define __TGS4161__
#define __DHT22__
#define __MQ7__
//#define __TGS813__
#define __PM25__
/*************************************************************
                          PIN_sitting
output:
P1_0 --> Fan PWM
P1_1 --> Send PWM to PM2.5

input:
P0_7 --> TGS813         P0_6 --> MQ7
P0_5 --> PM2.5          P0_4 --> MS1100 
P0_2¡BP0_3 can't work                  
P0_1 --> DHT22          P0_0 --> TGS4161
*************************************************************/


unsigned char SensorsData[23] = {0};

void SendData(unsigned int i, unsigned char Data)
{
  SensorsData[i] = Data;
}

unsigned char ReadData(unsigned int i)
{
  return SensorsData[i];
}

void delay_25us(void)
{
  int i;
  for(i=0;i<60;i++)asm("nop");
}

#ifdef __DHT22__

#define DATA_PIN P0_1

void SendData_DHT22(void)
{
  uchar temp[6]; 
  uchar humidity[5];  
  
  wendu_shi = wendu /100;
  wendu_ge = wendu /10 %10;
  wendu_dian = wendu %10;
  
  shidu_shi = shidu /100;
  shidu_ge = shidu /10 %10;
  shidu_dian = shidu %10;
  
  temp[0]=wendu_shi+0x30;
  temp[1]=wendu_ge+0x30;
  temp[2]='.';
  temp[3]=wendu_dian+0x30;
  temp[4] = '*';
  temp[5] = 'C';
        
  SendData(1,temp[0]);
  SendData(2,temp[1]);
  SendData(3,temp[3]);
  
  humidity[0]=shidu_shi+0x30;
  humidity[1]=shidu_ge+0x30;
  humidity[2]='.';
  humidity[3]=shidu_dian+0x30;
  humidity[4]='%';
  
  SendData(4,humidity[0]);
  SendData(5,humidity[1]);
  SendData(6,humidity[3]);
}

void COM(void)   
{     
    uchar i;       
    for(i=0;i<8;i++)    
    {
        ErrFLAG = 2;
        while(!DATA_PIN && ErrFLAG++);
        delay_25us();
        delay_25us();
        uchartemp=0;
        if(DATA_PIN)uchartemp=1;
        ErrFLAG = 2;
        while(DATA_PIN && ErrFLAG++);   
        ucharcomdata<<=1;
        ucharcomdata|=uchartemp; 
    }   
}
void DHT22(void)   
{
    int x = 0;
    P0DIR |= 0x02;
    DATA_PIN = 0;
    
    for (x = 0;x < 20;x++)delay_25us();
    
    DATA_PIN = 1; 
    
    P0DIR &= ~0x02; //P0_1-->Inp

    delay_25us();
    delay_25us(); 
    
        ErrFLAG = 2;
        while(!DATA_PIN && ErrFLAG++);        //80us low
        ErrFLAG = 2;
        while(DATA_PIN && ErrFLAG++);         //80us high
        
        COM();
        ucharRH_data_H_temp=ucharcomdata;
        COM();
        ucharRH_data_L_temp=ucharcomdata;
        COM();
        ucharT_data_H_temp=ucharcomdata;
        COM();
        ucharT_data_L_temp=ucharcomdata;
        COM();
        ucharcheckdata_temp=ucharcomdata;
        DATA_PIN = 1; 
        uchartemp=(ucharT_data_H_temp+ucharT_data_L_temp+ucharRH_data_H_temp+ucharRH_data_L_temp);
        
        if(uchartemp==ucharcheckdata_temp)
        {
            ucharRH_data_H=ucharRH_data_H_temp;
            ucharRH_data_L=ucharRH_data_L_temp;
            ucharT_data_H=ucharT_data_H_temp;
            ucharT_data_L=ucharT_data_L_temp;
            ucharcheckdata=ucharcheckdata_temp;
        }
        
        wendu = ucharT_data_H*256 + ucharT_data_L;
        shidu = ucharRH_data_H*256 + ucharRH_data_L;
        
    P0DIR |= 0x02; 
}
void DHT22_background(void)
{
  unsigned char j;
  DHT22();
  /*wenduTemp[DHT22_num] = wendu;
  shiduTemp[DHT22_num] = shidu;
  if(++DHT22_num >= 10)DHT22_num = 0;
  for(j=0;j<10;j++){
    wendu = wendu + wenduTemp[j];
    shidu = shidu + shiduTemp[j];
  }
  wendu = wendu/10;
  shidu = shidu/10;*/
}

unsigned int Shiwen(void)
{
  return wendu;
}

#endif

#ifdef __MS1100__
#define Rs_air 27037

int Temp_MS1100[10] = {0};
void Init_MS1100(void)
{
  P0DIR &= ~0x10;                   //P0_4-->Inp
  ADCCFG |= 0x10;                   //Set P0_4 Analog Periferal I/O
}
void background_MS1100(void)
{
  unsigned int  value = 0, MSD = 0, LSD = 0;
  static int i_MS1100 = 0;
  
  ADCCON1 = 0x30;
  ADCCON3 = 0xB4;
                   
  ADCCON1 |= 0x40;                  //Start conversion
  while(!(ADCCON1 & 0x80));        

  LSD = ADCL;
  LSD = LSD >>4;
  MSD = ADCH;
  MSD = MSD <<4;
  value = LSD + MSD;                

  Temp_MS1100[i_MS1100++] = value;
  if(i_MS1100 == 10)i_MS1100=0;
}
void SendData_MS1100(void)
{
  unsigned char i = 0,str_MS1100[3] = {0};
  unsigned int  temp = 0;
  float X = 0;
  
  while(i<10)temp = temp + Temp_MS1100[i++];
  temp = temp/10;
  X = (float)temp*3.3/2048;
  X = (5/X-1)*10000;     
  X = (0.71-(X/Rs_air))/2.96;
  X = pow(10,X);
  str_MS1100[0] = (int)X/100 +0x30;
  str_MS1100[1] = (int)X/10%10 +0x30;
  str_MS1100[2] = (int)X%10 +0x30;
  
  SendData(18,str_MS1100[0]);
  SendData(19,str_MS1100[1]);
  SendData(20,str_MS1100[2]);
}  
#endif

#ifdef __TGS4161__

unsigned int Temp_TGS4161[10] = {0};

void Init_TGS4161(void)
{
  P0DIR &= ~0x01;                   //P0_0-->Inp
  ADCCFG |= 0x01;                   //Set P0_0 Analog Periferal I/O
}
void background_TGS4161(void)
{
  unsigned int  value = 0, MSD = 0, LSD = 0;
  static int i_TGS4161 = 0;
  
  ADCCON1 = 0x30;
  ADCCON3 = 0xB0;
                   
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

void SendData_TGS4161(void)
{  
/*************************************************/  
  unsigned char i =0;
  unsigned int  EMF2 = 0, Delta_EMF = 0, EMF1 = 0, temp = 0;
  float X = 0;
  EMF1 = TGS4161_EMF_350ppm(); 
  while(i < 10)temp = temp + Temp_TGS4161[i++];
  temp = temp /10;
  X = (float)temp*1000/4096;
  EMF2 = (int)X;
  
/*************************************************/  
  unsigned char str_TGS4161[4] = {0};
  
  if( EMF2 > EMF1)
  {
    SendData(10,0x30);
    SendData(11,0x30);
    SendData(12,0x30);
    SendData(13,0x30);
  }
  else
  {
    Delta_EMF = EMF1 - EMF2;
    if(Delta_EMF > 0 && Delta_EMF <= 12)
      X = (float)Delta_EMF*50/3+350;
    if(Delta_EMF >12 && Delta_EMF <= 25)
      X = (float)Delta_EMF*25+250;
    if(Delta_EMF >27 && Delta_EMF <= 71){
      X = ((float)Delta_EMF+156.7)/61.57;
      X = pow(10,X);
    }
    
    str_TGS4161[0] = (int)X/1000 +0x30;
    str_TGS4161[1] = (int)X/100%10 +0x30;
    str_TGS4161[2] = (int)X%100/10 +0x30;
    str_TGS4161[3] = (int)X%10 +0x30;
    
    SendData(10,str_TGS4161[0]);
    SendData(11,str_TGS4161[1]);
    SendData(12,str_TGS4161[2]);
    SendData(13,str_TGS4161[3]);
  }
}

#endif

#ifdef __MQ7__

float R0_MQ7 = 1000;
unsigned int Temp_MQ7[10] = {0};

void Init_MQ7(void)
{
  P0DIR &= ~0x40;                   //P0_6-->Inp
  ADCCFG |= 0x40;                   //Set P0_6 Analog Periferal I/O
}

void background_MQ7(void)
{
  unsigned int  value = 0, MSD = 0, LSD = 0;
  static int i = 0;

  ADCCON1 = 0x30;
  ADCCON3 = 0xB6;
  
  ADCCON1 |= 0x40;                  //Start a conversion sequence if ADCCON1.STSEL = 11 and no sequence is running.
  while(!(ADCCON1 & 0x80));        

  LSD = ADCL;
  LSD = LSD >>4;
  MSD = ADCH;
  MSD = MSD <<4;
  value = LSD + MSD;                
    
  Temp_MQ7[i++] = value;
  if(i == 10)i=0;
}
void SendData_MQ7(void)
{  
  unsigned char i,str_MQ7[4] = {0};
  unsigned int temp = 0;
  float X = 0,Rs_MQ7 = 0, GasCon;
  for(i=0;i<10;i++)
    temp = temp + Temp_MQ7[i];
  X = (float)temp /10;
  X = X*3.3/2048;                         
  Rs_MQ7 = 50000/X - 10000;

  if( Rs_MQ7/R0_MQ7 > 1.65)
  {
    SendData(14,0x30);
    SendData(15,0x30);
    SendData(16,0x30);
    SendData(17,0x30);
  }
  else
  {
    if(Rs_MQ7/R0_MQ7 > 1 && Rs_MQ7/R0_MQ7 <= 1.65)
      GasCon = -(Rs_MQ7/R0_MQ7*50/0.65) +50;
    if(Rs_MQ7/R0_MQ7 > 0.38 && Rs_MQ7/R0_MQ7 <= 1)
      GasCon = -(Rs_MQ7/R0_MQ7*300/0.62) +400;
    if(Rs_MQ7/R0_MQ7 >0.22 && Rs_MQ7/R0_MQ7 <= 0.38)
      GasCon = -(Rs_MQ7/R0_MQ7*600/0.16) +1000;
    if(Rs_MQ7/R0_MQ7 >0.095 && Rs_MQ7/R0_MQ7 <= 0.22)
      GasCon = -(Rs_MQ7/R0_MQ7*3000/0.125) +4000;
    
    str_MQ7[0] = (int)GasCon/1000 +0x30;
    str_MQ7[1] = (int)GasCon/100%10 +0x30; 
    str_MQ7[2] = (int)GasCon%100/10 +0x30;
    str_MQ7[3] = (int)GasCon%10 +0x30;
    
    SendData(14,str_MQ7[0]);
    SendData(15,str_MQ7[1]);
    SendData(16,str_MQ7[2]);
    SendData(17,str_MQ7[3]);
  }
}

#endif

#ifdef __PM25__

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
  
  ADCCON1 = 0x30;
  ADCCON3 = 0xB5;
  
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
void SendData_PM25(void)
{
  unsigned char i, PM25uchar[3]={0};
  unsigned int Sum = 0;
  float X = 0;
  
  for(i = 0; i <10; i++)Sum = Temp_PM25[i] + Sum;
  Sum = Sum /10;
  if(Sum < 435)
  {
    SendData(7,0x30);
    SendData(8,0x30);
    SendData(9,0x30);;
  }
  else
  {
    X = (float)Sum*33/2048;                 //x10
    X = (X-7)/56*1000;                      //change the unit(mg/m3) to um/m3
  
    PM25uchar[0] = (int)X/100 +0x30;
    PM25uchar[1] = (int)X/10%10 +0x30;
    PM25uchar[2] = (int)X%10 +0x30;
  
    SendData(7,PM25uchar[0]);
    SendData(8,PM25uchar[1]);
    SendData(9,PM25uchar[2]);
  }
}
#endif

#ifdef __TGS813__

float R0_TGS813 = 1000;             //R0 = Sensor resistance in 1000 ppm Methane
unsigned int Temp_TGS813[10] = {0};

void Init_TGS813(void)
{
  P0DIR &= ~0x04;                   //P0_3 -->Inp
  ADCCFG |= 0x04;                   //Set P0_3 Analog Periferal I/O
}
void background_TGS813(void)
{
  unsigned int  value = 0, MSD = 0, LSD = 0;
  static int i_TGS813 = 0;
  
  ADCCON1 = 0x30;
  ADCCON3 = 0xB3;
  
  ADCCON1 |= 0x40;                  //Start conversion
  while(!(ADCCON1 & 0x80));        
  
  LSD = ADCL;
  LSD = LSD >>4;
  MSD = ADCH;
  MSD = MSD <<4;
  value = LSD + MSD;                

  Temp_TGS813[i_TGS813++] = value;
  if(i_TGS813 == 10)i_TGS813=0;
}
void SendData_TGS813(void)
{  
  unsigned char i,str_TGS813[4] = {0};
  unsigned int temp = 0;
  float X = 0,Rs_TGS813 = 0, GasCon_TGS813 = 0;
  for(i=0;i<10;i++)temp = temp + Temp_TGS813[i];
  X = (float)temp /10;
  X = X*3.3/2048;                         
  Rs_TGS813 = (50000/X-1)*10000;

  if( Rs_TGS813/R0_TGS813 > 1.8)
  {
    str_TGS813[0] = 0x30;
    str_TGS813[1] = 0x30;
    str_TGS813[2] = 0x30;
    str_TGS813[3] = 0x30;
  }
  if( Rs_TGS813/R0_TGS813 >= 0.84 && Rs_TGS813/R0_TGS813 < 1.8)
  {
    GasCon_TGS813 = (2.953-Rs_TGS813/R0_TGS813)*1000/1.92;
  
    str_TGS813[0] = (int)GasCon_TGS813/1000 +0x30;
    str_TGS813[1] = (int)GasCon_TGS813/100%10 +0x30; 
    str_TGS813[2] = (int)GasCon_TGS813%100/10 +0x30;
    str_TGS813[3] = (int)GasCon_TGS813%10 +0x30;
  } 
  if( Rs_TGS813/R0_TGS813 >= 0.29 && Rs_TGS813/R0_TGS813 < 0.84)
  {
    GasCon_TGS813 = (1.5121-Rs_TGS813/R0_TGS813)*10000/6.11;
  
    str_TGS813[0] = (int)GasCon_TGS813/1000 +0x30;
    str_TGS813[1] = (int)GasCon_TGS813/100%10 +0x30; 
    str_TGS813[2] = (int)GasCon_TGS813%100/10 +0x30;
    str_TGS813[3] = (int)GasCon_TGS813%10 +0x30;
  }
  
  SendData(20,str_TGS813[0]);
  SendData(21,str_TGS813[1]);
  SendData(22,str_TGS813[2]);
  SendData(23,str_TGS813[3]);
}

#endif

void Init_Sensor(void)
{
#ifdef __TGS4161__
  Init_TGS4161();
#endif
  
#ifdef __TGS813__
  Init_TGS813();
#endif
  
#ifdef __MQ7__
  Init_MQ7();
#endif
  
#ifdef __PM25__  
  Init_PM25();
#endif
  
#ifdef __MS1100__  
  Init_MS1100();
#endif
}
void SensorFunction(void)
{
#ifdef __DHT22__
  DHT22_background();
  SendData_DHT22();
#endif
  
#ifdef __TGS4161__
  background_TGS4161();
  SendData_TGS4161();
#endif
  
#ifdef __TGS813__
  background_TGS813();
  SendData_TGS813();
#endif
  
#ifdef __PM25__
  background_PM25();
  SendData_PM25();
#endif
  
#ifdef __MQ7__
  background_MQ7();
  SendData_MQ7();
#endif
  

  
#ifdef __MS1100__
  background_MS1100();
  SendData_MS1100();
#endif
}