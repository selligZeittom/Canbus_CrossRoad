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

struct CANFILTER filter = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0};
struct CANMESSAGE message = { 0 , 0 , 0 , 0 , 0 , 0 , 0};
struct CANMESSAGE messageTx = { 0 , 0 , 0 , 0 , 0 , 0 , 0};

/*
 * store the conflicts in a 12x12 array
 * first index means the car/person concerned, the second index is all the car persons to look at for this car/person
 * for example : [1][4] encode the conflict of the car 4 compared to the car 1 (Is the reference).
 */
uint8_t conflicts[NUMBER_USER][NUMBER_USER];

/*
 * method to initialize filter and timer 0 
 * readID() and readConflictInfo() are called inside this method
 */
void init(void);

/*
 * used to read the ID of the crossroad
 */
uint8_t readID(void);

/*
 * use to read all the conflicts create by the crossroad
 */
void readConflictInfo(void);

/*
 * reset all data stored in struct message
 */
void resetData(void);

/*
 * use to change the color of the traffic light
 * @ param trafficLight : traffic light number ( 0-7 )
 * @ param color : color of the light => 0 : red
 *                                       1 : orange
 *                                       2 : green
 *                                       3 : red and orange
 */  

void sendTrafficLight(uint8_t trafficLight, uint8_t color);

/*
 * used to change the color of pedestrian light
 * @ param trafficLight : traffic light number ( 0-3 )
 * @ param color : color of the light => 0 : red
 *                                       1 : orange
 *                                       2 : green
 */
void sendPedestrianLight(uint8_t trafficLight , uint8_t color);

/*
 * use to set the warning light when a pedestrian cross the road
 * @ param trafficLight : traffic light number ( 0-3 )
 * @ param state : state of the light => 0 : off
 *                                       1 : warning light blink
 */
void sendWarningLight(uint8_t trafficLight , uint8_t state);


#endif
