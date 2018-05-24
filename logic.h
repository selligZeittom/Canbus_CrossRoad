#ifndef LOGIC_H_
#define LOGIC_H_

#include <pic18.h>
#include <stdint.h>


//define the length of the arrays
#define NUMBERLIGHTS_CAR_PERSON 12

//define the colors of the lights
#define RED 0
#define ORANGE 1
#define GREEN 2
#define RED_ORANGE 3

//define the blinking lights state
#define ORANGE_BLINKING_OFF 0
#define ORANGE_BLINKING_ON 1


//the sequence of the color is the following one : RED, RED_ORANGE, GREEN, ORANGE, RED, ...

//define the T of the timer
#define T 1

//define the time to wait depending of the state of the light
#define RED_TIME 2
#define ORANGE_CAR_TIME 5
#define ORANGE_PERSON_TIME 8
#define GREEN_TIME 4
#define RED_ORANGE_TIME 2

//define the conflicts code
#define NO_CONFLICT 0
#define CONFLICT 1
#define WARNING 2

/*
	* the 4 following arrays are defined like that :
	* 0 ... 7  : car 0 to 7
	* 8 ... 11 : person 0 to 3
*/

//store the time in seconds that the ligths are set in their current color
uint8_t durationLights[NUMBERLIGHTS_CAR_PERSON];

//store the state of the lights	
uint8_t colorLights[NUMBERLIGHTS_CAR_PERSON];

//store the occupancy of a path or not : 0 = free, 1 = one or some cars
uint8_t occupancyPaths[NUMBERLIGHTS_CAR_PERSON];



/*
 * store the first car/person who arrived at the crossroad at the first index, the second car/person at the second index, ...
 * 255 means no one is waiting 
 * for example : 
 * priorityCarPerson contains : [6][2][9][1][255][255][255][255][255][255][255][255]
 * it means : car 6 is waiting since the longest time, followed by car 2, followed by person 2 , followed by car 1, and nobody else is waiting anymoroe then
 */
uint8_t priorityUser[NUMBERLIGHTS_CAR_PERSON];

// next priority for a new car/person in a path
uint8_t numberWaitingUsers = 0;

/*
 * init arrays
 */
void initLogic();

/*
 * used to check the first time if there are paths that aren't used anymore and that their 
 * duration is enough long : ->> take them to the next color
 */
void firstRound();

/*
 * used to set to off some lights even if there are still cars waiting but the duration reached his minimum
 * and others cars are waiting otherwhere
 */
void secondRound();

/*
 * this method is called by the timeout interrupt of the timer 
 * it increments every duration of the array by the period T of the timer
 */
void onTimeout();

/*
 * used to set a new state to a light
 * light : the light to set
 * color : the the new color
 */
void setLight(uint8_t light, uint8_t color);

/*
 * used to set a warning light
 * light : the light to set
 */
void setWarningLight(uint8_t light, uint8_t state);

/*
 * used to set the priority of a specific light
 * carPerson is the number of the car or person who needs to be pushed in the array
 */
void setPriority(uint8_t carPerson);

/*
 * used to decrement the priority of every light
 * used when a path is newly free
 * shifts the array to the left of one
 */ 
void decrPriority(uint8_t user);

/*
 * used to shift the priority 
 * for example : car 1 has been going GREEN the last 2 seconds, there are still car 1 waiting, 
 * but also car 3 which will create a conflict ->>
 * priority will be given to the car 3, and the car 1 will go at the last place of the priority array
 */ 
void shiftPriority();

/*
 * return the code conflict from a 12x12 array
 * for example : we're looking at the conflicts for car 1 compared to the person 2
 * uint8_t conflictBetweenCar1_Person2 = getConflict(1, 10);
 * attention : for a person : add 8 to the index
 */
uint8_t getConflict(uint8_t carPersonConcerned, uint8_t carPersonToLookAt);

/*
 * get the car or person which was waiting the first
 * return : the index from 0 to 11 if there are car waiting or 255 if nobody is waiting
 
uint8_t getHighestPriority();

/*
 * get the car or person which was waiting the first when a car was already crossing the road
 * return : the index from 0 to 11 if there are car waiting or 255 if nobody is waiting
 
uint8_t getSecondHighestPriority();
*/



#endif
