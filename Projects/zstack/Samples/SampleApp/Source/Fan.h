#ifndef __Fan__
#define __Fan__

void SetFanPin(void)
{
  P2SEL |= 0x10;
  P1SEL |= 0x01;
  P1DIR |= 0x01;
  PERCFG = 0x00;
}
void timer4_init(void)  //default Timer ticks 16MHz
{
  T4CTL |= 0xE0;        //Tick frequency/128
  T4CTL &= ~0x03;       //Free running, repeatedly count from 0x00 to 0xFF
  T4CCTL0 |= 0x28;      //Set output on compare, clear on 0xFF
  T4CCTL0 |= 0x04;      //Compare mode

  T4CC0 = 0x33;
  
  T4CTL |= 0x10;        //Start timer4
}

void On_Fan(void)
{
  SetFanPin();
  timer4_init();
}
void Off_Fan(void)
{
  T4CTL &= ~0x10;
  P1DIR &= ~0x01;
}

#endif
