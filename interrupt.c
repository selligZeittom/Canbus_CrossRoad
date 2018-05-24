#include "can.h"
#include "logic.h"


void interrupt high_isr(void){
  if((CAN_INTF == 1)&&(CAN_INTE == 1)) // interrupt flag & active
  {
    CAN_INTF = 0;               // clear interrupt
    Can_Isr();                  // interrupt treatment
    if(CAN_INTPIN == 0)         // check pin is high again
    {
      CAN_INTF = 1;             // no -> re-create interrupt
    }
  }
  if(TMR0IF == 1){
    // disable timer 0
    T0CONbits.TMR0ON = 1;
        
    // action to do
        onTimeout();
        
    // reset timer 0
    TMR0IF = 0;
    TMR0 = 0xe17b;
    T0CONbits.TMR0ON = 1;
    
    }
}
