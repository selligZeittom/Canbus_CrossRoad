#include "function.h"
#include "picebs2.h"
#include "logic.h"

void init(void){
    enable_high_speed();
    Can_Init( &canSpeed125k , &filter);
    PEIE = 1;
    GIE = 1;
    id = readID();
    filter.mask0 = 0X78F;
    filter.filter0 = REQ_SENSOR_CAR | id;
    filter.filter1 = REQ_SENSOR_PED | id;
    filter.ext = 0;
    Can_Init( &canSpeed125k , &filter);
    readConflictInfo();
    
    filter.mask1 = 0X7FF;
    filter.filter2 = SYSTEM_RESET | id;
    filter.filter3 = SYSTEM_RESET | id;
    filter.filter4 = SYSTEM_RESET | id;
    filter.filter5 = SYSTEM_RESET | id;
    Can_Init( &canSpeed125k , &filter);

    // init timer 0 to make an interrupt each 0.1s
    T0CONbits.T08BIT = 0; // configured as an 16 bit timer
    T0CONbits.T0CS = 0; // Fosc/4
    T0CONbits.PSA = 0;// use prescaler
    T0CONbits.T0PS = 0x7; // prescaler @ 256
    TMR0IE = 1;
    TMR0IF = 0; // clean flag
    TMR0 = 0xe796; // 10Hz interrupt
    T0CONbits.TMR0ON = 1;// enable timer
}

uint8_t readID(void){
    uint8_t retVal = 0;
    
    // create message
    messageTx.identifier = REQ_CROSS_ID ;
    resetData();
    messageTx.rtr = 1;
    
    // send message
    Can_PutMessage(&messageTx);
    
    // wait until the message is received
    while(Can_GetMessage(&message) != 0){};
    
    retVal = message.dta[0];
    // reset data on the message
    resetData();
  
    return retVal;
}

void readConflictInfo(void){
    
    // create message
    resetData();
    messageTx.rtr = 1;
    
    // ask 12 path on the EEPROM
    int i;
    int j;
    for (i = 0 ; i < NUMBER_USER ; i++){
       
        messageTx.identifier = (REQ_CROSS_CONFIG | (i << 4) | id);
        
        // send message
        Can_PutMessage(&messageTx);
        // wait until the message is received
        while(Can_GetMessage(&message) != 0){};
        
        // read message and save data
        for ( j = 0 ; j < CONFLICTSIZE ; j ++){
            conflicts[i][(4*j) + 3] = (message.dta[j] & 0x03); 
            conflicts[i][(4*j) + 2] = (message.dta[j] & 0x0c) >> 2; 
            conflicts[i][(4*j) + 1] = (message.dta[j] & 0x30) >> 4; 
            conflicts[i][(4*j) + 0] = (message.dta[j] & 0xc0) >> 6; 
        }
        // reset data in the message
        resetData();
    }
}
/*
 * @ param trafficLight : traffic light number ( 0-7 )
 * @ param color : color of the light => 0 : red
 *                                       1 : orange
 *                                       2 : green
 *                                       3 : red and orange
 */
void sendTrafficLight(uint8_t trafficLight, uint8_t color){
    
    // create message
    messageTx.identifier = (TRAF_LIGHT | (trafficLight << 4) | id );
    messageTx.rtr = 0;
    messageTx.dta[0] = color;
    messageTx.dlc = 1;
    
    switch(color)
    {
        case RED:
            messageTx.txPrio = 3;
            break;
        case RED_ORANGE:
            messageTx.txPrio = 1;
            break;
        case GREEN:
            messageTx.txPrio = 1;
            break;
        case ORANGE:
            messageTx.txPrio = 2;
            break;
    }
    
    // send message
    Can_PutMessage(&messageTx);
    
}
/*
 * @ param trafficLight : traffic light number ( 0-3 )
 * @ param color : color of the light => 0 : red
 *                                       1 : orange
 *                                       2 : green
 */
void sendPedestrianLight(uint8_t trafficLight , uint8_t color){
    // create message
    messageTx.identifier = (PED_LIGHT | (trafficLight << 4) | id );
    messageTx.rtr = 0;
    messageTx.dta[0] = color;
    messageTx.dlc = 1;
    
    switch(color)
    {
        case RED:
            messageTx.txPrio = 3;
            break;
        case GREEN:
            messageTx.txPrio = 1;
            break;
        case ORANGE:
            messageTx.txPrio = 2;
            break;
    }
    

    // send message
    Can_PutMessage(&messageTx);
}
/*
 * @ param trafficLight : traffic light number ( 0-3 )
 * @ param state : state of the light => 0 : off
 *                                       1 : warning light blink
 */
void sendWarningLight(uint8_t trafficLight , uint8_t state){
    // create message 
    messageTx.identifier = (WARN_LIGHT | (trafficLight << 4) | id );
    messageTx.rtr = 0;
    messageTx.dta[0] = state;
    messageTx.dlc = 1;
    
    switch(state)
    {
        case ORANGE_BLINKING_ON:
            messageTx.txPrio = 3;
            break;
        case ORANGE_BLINKING_OFF:
            messageTx.txPrio = 0;
            break;
    }
    
    // send message
    Can_PutMessage(&messageTx);
}

uint8_t requestPed(uint8_t ped){
     // create message 
    messageTx.identifier = (REQ_SENSOR_PED | ((ped-8) << 4) | id );
    messageTx.dlc = 0;
    messageTx.rtr = 1;
    
    // send message
    Can_PutMessage(&messageTx);
    
    uint8_t nMessages = Can_InBufferStatus();
    
     // wait until the message is received
     while(nMessages == 0){
         nMessages = Can_InBufferStatus();
     };
     
     Can_GetMessage(&message);
     
     return message.dta[0];
}
void resetData(void){
    int i; 
    for (i = 0 ; i < 4 ; i++){
        message.dta[i] = 0;
    }
}

