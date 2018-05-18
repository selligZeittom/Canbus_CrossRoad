#include "can.h"

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
}
