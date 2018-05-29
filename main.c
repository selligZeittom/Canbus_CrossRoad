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
    init();
    initLogic();
 
    /*test
    sendTrafficLight(1,3);
    sendWarningLight(0,1);
    sendPedestrianLight(0,2);
     */
    while(1){
    
        if(Can_InBufferStatus() != 0){
            // Handle the message 
            Can_GetMessage(&message);
            
            //use the message informations
            uint16_t cmd = (message.identifier & 0b11110000000);
            uint8_t user = (message.identifier & 0b00001110000) >> 4; 
            
            switch(cmd)
            {
                case REQ_SENSOR_CAR : 
                    //nobody is waiting
                    if(occupancyPaths[user] == 1 && message.dta[0] == 0)
                    {
                        decrPriority(user);
                        occupancyPaths[user] = message.dta[0];
                        break;
                    }
                    occupancyPaths[user] = message.dta[0];
                    
                    uint8_t isAlreadyWaiting = 0;
                    for(uint8_t i = 0; i < NUMBER_USER;i++)
                    {
                        if(priorityUser[i] == user)
                        {
                            isAlreadyWaiting = 1;
                        }
                    }
                    
                    if(isAlreadyWaiting == 0)
                    {
                        setPriority(user);
                    }
                    
                    break;
                case REQ_SENSOR_PED : 
                    user+=8;
                    if(occupancyPaths[user] == 1 && message.dta[0] == 0)
                    {
                        decrPriority(user);
                    }
                    occupancyPaths[user] = message.dta[0];
                    
                    uint8_t isAlreadyWaiting = 0;
                    for(uint8_t i = 0; i < NUMBER_USER;i++)
                    {
                        if(priorityUser[i] == user)
                        {
                            isAlreadyWaiting = 1;
                        }
                    }
                    
                    if(isAlreadyWaiting == 0)
                    {
                        setPriority(user);
                    }
                    break;
                    
                case SYSTEM_RESET:
                    initLogic();
                    break;
                default:
                    break;
                
            }
        }
        
        
    };
    return (EXIT_SUCCESS);
}

