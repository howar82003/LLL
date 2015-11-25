#include <ioCC2530.h>
#include "UART.H"
#include "DHT22.H"
#include "Sensor.h"
#define DATA_PIN P0_1

extern void delay_25us(void);
extern void SendData(unsigned int i, unsigned char Data);

unsigned int wendu = 0, shidu = 0;
int shiduTemp[10] = {0}, wenduTemp[10] = {0};






void ReadDHT22(void)
{
  uchar temp[6]; 
  uchar humidity[5];  
  uchar strTemp[13]="Temperature:";
  uchar strHumidity[11]="Humidity:";  
  uchar str_1[8]="DHT22:";
  
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
  /*
  UartSendString(str_1, 8);
  UartSendString(strTemp, 12);
  UartSendString(temp, 6);
  UartSendString(strHumidity, 9);
  UartSendString(humidity, 5);*/
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
    
    for (x = 0;x < 20;x++){delay_25us();}
    
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
        
    //P0DIR |= 0x02; 
}
void DHT22background(int i)
{
  
  int j = 0;
  DHT22();
  wenduTemp[i] = wendu;
  shiduTemp[i] = shidu;
  
  while(j<10){
    wendu = wendu + wenduTemp[j];
    shidu = shidu + shiduTemp[j];
    j++;
  }
  wendu = wendu/11;
  shidu = shidu/11;
}

void DHT22_background(void)
{
  static int i = 0;
  if(i == 10)i = 0;
  DHT22background(i++);
}

unsigned int Shiwen(void)
{
  return wendu;
}