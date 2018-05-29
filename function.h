#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <pic18.h>
#include <stdint.h>
#include "can.h"



#define TRAF_LIGHT  (0<<7)  // command 0
#define PED_LIGHT   (1<<7) // command 1
#define WARN_LIGHT (2<<7) // command 2
#define REQ_SENSOR_CAR (3<<7) // command 3
#define REQ_SENSOR_PED (4<<7) // command 4
#define REQ_CROSS_CONFIG (8<<7) // command 8
#define REQ_CROSS_ID    ((10<<7) | 0xF) // command 10
#define SYSTEM_RESET (11<<7) // command 11
#define NUMBER_USER 12 // number of path
#define CONFLICTSIZE 3 // number of byte in each path

// crossroad ID
uint8_t id = 0;

//Existing path
uint8_t bottomRight = 0;
uint8_t topRight = 0;
uint8_t topLeft = 0;
uint8_t bottomLeft = 0;


struct CANFILTER filter = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0};
struct CANMESSAGE message = { 0 , 0 , 0 , 0 , 0 , 0 , 0};
struct CANMESSAGE messageTx = { 0 , 0 , 0 , 0 , 0 , 0 , 0};

/*
 * store the conflicts in a 12x12 array
 * first index means the car/person concerned, the second index is all the car persons to look at for this car/person
 * for example : [1][4] encode the conflict of the car 4 compared to the car 1 (Is the reference).
 */
uint8_t conflicts[NUMBER_USER][NUMBER_USER];

void init(void);
uint8_t readID(void);
void readConflictInfo(void);
void resetData(void);
void sendTrafficLight(uint8_t , uint8_t );
void sendPedestrianLight(uint8_t , uint8_t);
void sendWarningLight(uint8_t trafficLight , uint8_t state);
uint8_t requestPed(uint8_t ped);

#endif
