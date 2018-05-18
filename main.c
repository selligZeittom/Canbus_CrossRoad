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


uint8_t id = 0;
/*
 * 
 */
struct CANFILTER filter = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0};
struct CANMESSAGE message = { 0 , 0 , 0 , 0 , 0 , 0 , 0};

uint8_t readID(void);

void init(void){
    Can_Init( &canSpeed125k , &filter);
    PEIE = 1;
    GIE = 1;
    id = readID();
    filter.mask0 = 0X00F;
    filter.filter0 = id;
    Can_Init( &canSpeed125k , &filter);

}

uint8_t readID(void){
    uint8_t retVal = 0;
    
    // creat message
    message.identifier = REQ_CROSS_ID ;
    message.dta[0] = 0;
    message.rtr = 1;
    
    // send message
    Can_PutMessage(&message);
    
    // wait until the message is received
    while(Can_GetMessage(&message) != 0){};
    
    retVal = message.dta[0];
  
    return retVal;
}
int main(int argc, char** argv) {
    init();
    return (EXIT_SUCCESS);
}

