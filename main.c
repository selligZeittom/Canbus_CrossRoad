/* 
 * File:   main.c
 * Author: yan.michello
 *
 * Created on 17. mai 2018, 13:11
 */

#include <stdio.h>
#include <stdlib.h>
#include "can.h"

#define TRAF_LIGHT  (0<<7)  // command 0
#define PED_LIGHT   (1<<7) // command 1
#define WARN_LIGHT (2<<7) // command 2
#define REQ_SENSOR_CAR (3<<7) // command 3
#define REQ_SENSOR_PED (4<<7) // command 4
#define REQ_CROSS_CONFIG (8<<7) // command 8
#define REQ_CROSS_ID    ((10<<7) | 0xF) // command 10
#define CONFLICT 12 // number of path
#define CONFLICTSIZE 3 // number of byte in each path

// crossroad ID
uint8_t id = 0;

//Existing path
uint8_t bottomRight = 0;
uint8_t topRight = 0;
uint8_t topLeft = 0;
uint8_t bottomLeft = 0;

// table of conflict
uint8_t conflictTable[12][12];
/*
 * 
 */
struct CANFILTER filter = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0};
struct CANMESSAGE message = { 0 , 0 , 0 , 0 , 0 , 0 , 0};
struct CANMESSAGE messageTx = { 0 , 0 , 0 , 0 , 0 , 0 , 0};

uint8_t readID(void);
void readConflictInfo(void);
void resetData(void);
void sendTrafficLight(uint8_t , uint8_t );
void sendPedestrianLight(uint8_t , uint8_t);

void init(void){
    Can_Init( &canSpeed125k , &filter);
    PEIE = 1;
    GIE = 1;
    id = readID();
    filter.mask0 = 0X00F;
    filter.filter0 = id;
    Can_Init( &canSpeed125k , &filter);
    readConflictInfo();
    
    // init timer 0 to make an interrupt each 0.5s
    T0CONbits.T08BIT = 0; // configured as an 16 bit timer
    T0CONbits.T0CS = 0; // Fosc/4
    T0CONbits.PSA = 0;// use prescaler
    T0CONbits.T0PS = 0x7; // prescaler @ 256
    TMR0 = 0xe17b;
    T0CONbits.TMR0ON = 1;
}

uint8_t readID(void){
    uint8_t retVal = 0;
    
    // create message
    messageTx.identifier = REQ_CROSS_ID ;
    resetData();
    message.rtr = 1;
    
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
    for (i = 0 ; i < CONFLICT ; i++){
       
        messageTx.identifier = (REQ_CROSS_CONFIG | (i << 4) | id);
        
        // send message
        Can_PutMessage(&messageTx);
        // wait until the message is received
        while(Can_GetMessage(&message) != 0){};
        
        // read message and save data
        for ( j = 0 ; j < CONFLICTSIZE ; j ++){
            conflictTable[i][(4*j) + 0] = (message.dta[j] & 0x03); 
            conflictTable[i][(4*j) + 1] = (message.dta[j] & 0x0c) >> 2; 
            conflictTable[i][(4*j) + 2] = (message.dta[j] & 0x30) >> 4; 
            conflictTable[i][(4*j) + 3] = (message.dta[j] & 0xc0) >> 6; 
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
    
    // send message
    Can_PutMessage(&messageTx);
}
void resetData(void){
    int i; 
    for (i = 0 ; i < 4 ; i++){
        message.dta[i] = 0;
    }
}

int main(int argc, char** argv) {
    init();
    sendTrafficLight(1,3);
    sendWarningLight(0,1);
    sendPedestrianLight(0,2);
    while(1){};
    return (EXIT_SUCCESS);
}

