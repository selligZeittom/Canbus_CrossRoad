#include "logic.h"
#include "can.h"

void initLogic()
{
    for(uint8_t i = 0; i < NUMBERLIGHTS_CAR_PERSON; i++)
    {
        durationLights[i] = 0; //set the durations to 0s
        colorLights[i] = RED; //set everything to red
        occupancyPaths[i] = 0; //nobody waits
        priorityCarPerson[i] = 255; //nobody has the priority
    }
}

void firstRound()
{
    for(uint8_t i = 0; i < NUMBERLIGHTS_CAR_PERSON; i++)
    {
        uint8_t color = colorLights[i]; //get the color of the light
        uint8_t occupancy = occupancyPaths[i]; //look if there are still cars
        uint8_t duration = durationLights[i];
        
        //depending on the color of the light switching off the lights
        switch (color)
        {
            case RED :
                //can't do anything right now...
                break;
                
            case ORANGE:
                if(i<8) //means it's about a car
                {
                    if(duration == ORANGE_CAR_TIME) //if the orange light stayed enough long
                    {
                        //go to red
                        setLight(i,RED);
                    }
                }
                else //means it's about a person
                {
                    if(duration == ORANGE_PERSON_TIME) //if the orange light stayed enough long and nobody is waiting anymore
                    {
                        //go to red
                        setLight(i,RED);
                    }
                }
                break;
                
            case GREEN:
                if(duration == GREEN_TIME && occupancy == 0) //if the green light stayed enough long and nobody is waiting anymore
                {
                    //go to orange
                    setLight(i,ORANGE);
                    //this user will be removed from the priority array by the can message from the crossroad
                }
                break;
                
            case RED_ORANGE:
                if(duration == RED_ORANGE_TIME) //if the red_orange light stayed enough long
                {
                    //go to green
                    setLight(i,GREEN);
                }
                break;
                
            default:
                break;
                
        }
    }
}

void secondRound()
{
    //look for every car/person waiting if there is a solution 
    for(uint8_t i = 0; i < priority; i++)
    {
        //the car/person which is waiting
        uint8_t waitingCarPerson = priorityCarPerson[i];
        
        //check all the other car/persons which may cause a conflict with the waiting car
        for(uint8_t j = 0; j < NUMBERLIGHTS_CAR_PERSON; j++)
        {
            //get the code for the conflict between thoses 2 users
            uint8_t conflict = getConflict(waitingCarPerson, j);
            
            //non conflict -> don't need to think more
            if(conflict == NO_CONFLICT)
            { 
                 if( colorLights[waitingCarPerson] == RED && durationLights[waitingCarPerson] == RED_TIME)
                {
                    //if the red light stayed long enough, let's go in red orange light
                    setLight(waitingCarPerson, RED_ORANGE);
                }    
            }
            
            //warning for pedestrian
            else if(conflict == WARNING)
            { 
                if(colorLights[waitingCarPerson] == RED && durationLights[waitingCarPerson] == RED_TIME)
                {
                    //if the red light stayed long enough, let's go in red orange light
                    setLight(waitingCarPerson, RED_ORANGE);
                    //set the warning light for the user j-8, beacuse person are from 8 to 11 in the array
                    setWarningLight(j-8, ORANGE_BLINKING_ON);
                }    
            }
            
            //conflict : check if the user involved in the conflict has a green, orange or red_orange light
            else if(conflict == CONFLICT)
            {
                //set to orange if the user got a green enough long
                if(colorLights[j] == GREEN && durationLights[j] == GREEN_TIME)
                {
                    //set this user to orange to let the waiting user go soon
                    setLight(j, ORANGE);
                    shiftPriority(); //set this user at the end of the priority array
                }
                //if both are red and their duration is over
                else if(colorLights[j] == RED && durationLights[j] == RED_TIME && colorLights[waitingCarPerson] == RED && durationLights[waitingCarPerson] == RED_TIME)
                {
                    //if the red light stayed long enough, let's go in red orange light
                    setLight(waitingCarPerson, RED_ORANGE);
                } 
            }
        }   
    }
}

void onTimeout()
{
	for(uint8_t i = 0; i < NUMBERLIGHTS_CAR_PERSON; i++)
    {
        durationLights[i] += T; //increment every duration of T period of the timer
    }
    firstRound();
    secondRound();
}

void setLight(uint8_t light, uint8_t color)
{
    durationLights[light] = 0; //reset the counter 
    colorLights[light] = color; //set the new state
    
    //CAN messages
  
    
}

void setWarningLight(uint8_t light, uint8_t state)
{
    //CAN message
}

void setPriority(uint8_t carPerson)
{
    if(priority < NUMBERLIGHTS_CAR_PERSON)
    {
        priorityCarPerson[priority] = carPerson;
        priority++;
    }
}

void decrPriority()
{
    uint8_t modified = 0; //stores if this function modify the array or not
    
    for(uint8_t i = 0; i < NUMBERLIGHTS_CAR_PERSON-1; i++)
    {
        if(priorityCarPerson[i] != 255)
        {
            priorityCarPerson[i] = priorityCarPerson[i+1]; //shift the array to the left
            modified = 1;
        }      
    }
    
    if(priorityCarPerson[NUMBERLIGHTS_CAR_PERSON-1] != 255)
    {
        priorityCarPerson[NUMBERLIGHTS_CAR_PERSON-1] = 255; //reset the last indexx of the array
    }
    
    if(modified==1)
    {
        priority--; //decrement the index of the array if we shifted the array
    }
}

void shiftPriority()
{
    uint8_t temp = priorityCarPerson[0]; //store the first priority that was stopped
    for(uint8_t i = 0; i < priority-1; i++)
    {
        priorityCarPerson[i] = priorityCarPerson[i+1]; //shift the array
    }
    priorityCarPerson[priority-1] = temp; //put the old priority at the first place
}

uint8_t getConflict(uint8_t carPersonConcerned, uint8_t carPersonToLookAt)
{
    return conflicts[carPersonConcerned][carPersonToLookAt];
}