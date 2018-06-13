/* 
 * File:   main.c
 * Author: yan.michello
 *
 * Created on 17. mai 2018, 13:11
 */

#include <stdio.h>
#include <stdlib.h>
#include "can.h"
#include "logic.h"
#include "function.h"

int main(int argc, char** argv) {
    
    //initialize the system
    init();
    initLogic();
        
    //main loop
    while(1){
    
        //if there is a message in the incoming buffer
        if(Can_InBufferStatus() != 0){
            // Handle the message 
            Can_GetMessage(&message);
            
            //use the message informations
            uint16_t cmd = (message.identifier & 0b11110000000);
            uint8_t user = (message.identifier & 0b00001110000) >> 4; 
            
            //do different actions depending on the cmd informations
            switch(cmd)
            {
                //if it's a car sensor sending a frame to the PIC 
                case REQ_SENSOR_CAR : 
                    //no more physical user at this path, and still a user in the program's arrays
                    if(occupancyPaths[user] == 1 && message.dta[0] == 0)
                    {
                        //delete the user from the waiting list
                        decrPriority(user);
                        occupancyPaths[user] = message.dta[0];
                        break;
                    }
                    
                    //update the occupancy in the program's array
                    occupancyPaths[user] = message.dta[0];
                    
                    //check before adding in the waiting list of the user of the request already exist or not
                    uint8_t isAlreadyWaiting = 0;
                    for(uint8_t i = 0; i < NUMBER_USER;i++)
                    {
                        if(priorityUser[i] == user)
                        {
                            isAlreadyWaiting = 1;
                        }
                    }
                    
                    //add only if the user of the request isn't waiting
                    if(isAlreadyWaiting == 0)
                    {
                        setPriority(user);
                    }
                    break;
                    
                //means it's about a request for a pedestrian 
                case REQ_SENSOR_PED : 
                    user+=8; //in our program, pedestrian are users from 8 to 11, but in request they're from 0 to 3
                    
                    //check that the pedestrian isn't already waiting 
                    uint8_t isAlreadyWaiting = 0;
                    for(uint8_t i = 0; i < NUMBER_USER;i++)
                    {
                        if(priorityUser[i] == user)
                        {
                            isAlreadyWaiting = 1;
                        }
                    }
                    
                    //only add if the user of the request isn't already waiting
                    if(isAlreadyWaiting == 0)
                    {
                        setPriority(user);
                    }
                    break;
                    
                //if we did a reset on the physical crossroad
                case SYSTEM_RESET:
                    initLogic(); //reinitialize all the arrays
                    // reset timer 0
                    TMR0IF = 0;
                    TMR0 = 0xe17b;
                    T0CONbits.TMR0ON = 1;
                    break;
                default:
                    break;
                
            }
        }
    
    //get into the logic functions and set the different lights
    firstRound();
    secondRound();
    };
    return (EXIT_SUCCESS);
}

