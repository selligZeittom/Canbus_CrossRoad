#include "logic.h"
#include "can.h"
#include "function.h"

void initLogic()
{
    for(uint8_t i = 0; i < NUMBERLIGHTS_CAR_PERSON; i++)
    {
        durationLights[i] = 0; //set the durations to 0s
        colorLights[i] = RED; //set everything to red
        occupancyPaths[i] = 0; //nobody waits
        priorityUser[i] = 255; //nobody has the priority
        numberWaitingUsers = 0;
    }
    for(uint8_t i = 0 ; i < NUMBER_WARNING_LIGHTS ; i++){
        warningLights[i] = 0;
    }
}

void firstRound()
{
    //look for every light if there are some logic-less changes to do, like an orange-timeout
    for(uint8_t i = 0; i < NUMBERLIGHTS_CAR_PERSON; i++)
    {
        uint8_t color = colorLights[i]; //get the color of the light
        uint8_t occupancy = occupancyPaths[i]; //look if there are still cars
        uint8_t duration = durationLights[i]; //the duration
        
        //switching on the color of this light to do some changes
        switch (color)
        {
            //if the light is in orange, we can change to red if the time is over
            case ORANGE:
                if(i<8) //means it's about a car
                {
                    if(duration >= ORANGE_CAR_TIME) //if the orange light stayed enough long
                    {
                        //go to red
                        setLight(i,RED);
                    }
                }
                else //means it's about a person
                {
                    if(duration >= ORANGE_PERSON_TIME) //if the orange light stayed enough long and nobody is waiting anymore
                    {  
                        //go to red
                        setLight(i,RED);
                        
                        //check if there is a warning light
                        if(warningLights[i-8] == 1)
                        {
							//disable the warning lights when they aren't necessary anymore
                            setWarningLight(i-8, ORANGE_BLINKING_OFF);
                            warningLights[i-8] = 0;
                        }
						//delete the user i from the waiting users
                        decrPriority(i);
                    }
                }
                break;
                
            //we can set to orange if there isn't anybody waiting and the time is over    
            case GREEN:
                
                //no request sensor to say that there aren't anymore pedestrian : we use only the timer
                if(i > 7 && duration >= GREEN_TIME)
                {
					//means that all the pedestrian are gone
                    occupancyPaths[i] = 0;
                } 
                
                //if there isn't anybody waiting and the timer is over : go to orange
                if(duration >= GREEN_TIME && occupancy == 0) //if the green light stayed enough long and nobody is waiting anymore
                {
                    //go to orange
                    setLight(i,ORANGE);
                }
                break;
            
            //if the timer is over for a red orange, set the green light    
            case RED_ORANGE:
                if(duration >= RED_ORANGE_TIME && i < 8) //if the red_orange light stayed enough long
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
    //we can let a car go only if the first user is gone
     uint8_t firstUserIsGone = 0;
     
    //look for every user waiting if there is a solution 
    for(uint8_t i = 0; i < numberWaitingUsers; i++)
    {
        //store the potentials conflicts : 0 = no conflict, 1 = conflict
        uint8_t conflictUnresolved = 0;
       
        //store the potential warning users
        uint8_t storedWarningPedestrian = 255;
        
        //is there a warning with all the others users
        uint8_t warningUnresolved = 0;
     
       //the user which is waiting
        uint8_t waitingUser = priorityUser[i];
                
        //check all the other users which may cause a conflict with the waiting car
        for(uint8_t j = 0; j < NUMBERLIGHTS_CAR_PERSON; j++)
        {
            //get the code for the conflict between thoses 2 users
            uint8_t conflict = getConflict(waitingUser, j);
            
            //warning for pedestrian
            if(conflict == WARNING)
            { 
                //if the waiting user is a pedestrian and waited enough long at the red, and the potential car creating a warning is going to cross or is crossing
                if(colorLights[waitingUser] == RED && durationLights[waitingUser] >= RED_TIME && (colorLights[j] == GREEN || colorLights[j] == RED_ORANGE || colorLights[j] == ORANGE) && waitingUser > 7)
                {   
                    warningUnresolved = 1;
                    storedWarningPedestrian = waitingUser-8;
                }
                
                //if the waiting user is a car, and the potential pedestrian creating a warning is crossing or going to cross  
                else if(waitingUser < 8 && (colorLights[j] == GREEN || colorLights[j] == ORANGE))
                {
                    warningUnresolved = 1;
                    storedWarningPedestrian = j-8;

                }
            }
            
            //conflict : check if the user involved in the conflict has a green, orange or red_orange light
            else if(conflict == CONFLICT)
            {
                //set the unresolvedConflict to 1, because if the color is red-orange, orange or green there is a problem...
                if(colorLights[j] == GREEN || colorLights[j] == ORANGE || colorLights[j] == RED_ORANGE)
                {
                    conflictUnresolved = 1;    
                }
                
                //set to orange if the user got a green enough long
                if(colorLights[j] == GREEN && durationLights[j] >= GREEN_TIME)
                {
                    //set this user to orange to let the waiting user go soon
                    setLight(j, ORANGE);
                    //set this user at the end of the priority array
                    shiftPriorityUser(j);
                }
            }
        } 
        
        //only if there isn't a conflict and the color is red and the duration is enough long we can let the car go...
        if(conflictUnresolved == 0 && colorLights[waitingUser] == RED && durationLights[waitingUser] >= RED_TIME)
        {
			//if the first waiting user is going to be set to green
            if(i == 0)
            {
                firstUserIsGone = 1;
            }
			
			//if the first user is currently going
            if(colorLights[priorityUser[0]] == GREEN)
            {
                firstUserIsGone = 1;
            }
			
			//we can let other users going only if the first one is going too
            if(firstUserIsGone == 1)
            {   
                //if there are some warning 
                if(warningUnresolved == 1)
                {
                    setWarningLight((storedWarningPedestrian), ORANGE_BLINKING_ON);
                    warningLights[storedWarningPedestrian] = 1;
                }
                if(waitingUser < 8 ){                    
                    setLight(waitingUser, RED_ORANGE);
                }
                else
                {  
                    setLight(waitingUser, GREEN);
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
}

void setLight(uint8_t light, uint8_t color)
{
    durationLights[light] = 0; //reset the counter 
    colorLights[light] = color; //set the new state
    
    //CAN messages
    if(light < 8){
        sendTrafficLight(light,color);
    }
    else if(light > 7 && light < 12){
        sendPedestrianLight((light-8),color);
    }
    
    
}

void setWarningLight(uint8_t light, uint8_t state)
{
    //CAN message
    sendWarningLight(light,state);
}

void setPriority(uint8_t carPerson)
{
    if(numberWaitingUsers < NUMBERLIGHTS_CAR_PERSON)
    {
        priorityUser[numberWaitingUsers] = carPerson;
        numberWaitingUsers++;
    }
}

void decrPriority(uint8_t user)
{
    uint8_t modified = 0; //stores if this function modify the array or not
    
    //searching for the user to  remove
    for(uint8_t i = 0; i < NUMBERLIGHTS_CAR_PERSON; i++)
    {
        if(priorityUser[i] == user)
        {
            modified = 1;
        }
        
        //replace when it has been found
        if(modified == 1 && i < NUMBERLIGHTS_CAR_PERSON-1)
        {
            priorityUser[i] = priorityUser[i+1];
        }
        else if(modified == 1 && i == 11)
        {
            priorityUser[i] = 255;
        }   
    }

    if(modified==1)
    {
        numberWaitingUsers--; //decrement the index of the array if we shifted the array
    }
}

void shiftPriorityUser(uint8_t user)
{
    uint8_t temp = priorityUser[0]; //store the first priority that was stopped
    uint8_t foundUser = 0;
    for(uint8_t i = 0; i < numberWaitingUsers-1; i++)
    {
        if(user == priorityUser[i])
        {
            foundUser = 1;
            temp = priorityUser[i]; 
        }
        if(foundUser == 1)
        {
            priorityUser[i] = priorityUser[i+1]; //shift the array
        }
    }
    
    if(foundUser == 1)
    {
        priorityUser[numberWaitingUsers-1] = temp; //put the old priority at the first place    
    }
    
}

uint8_t getConflict(uint8_t carPersonConcerned, uint8_t carPersonToLookAt)
{
    return conflicts[carPersonConcerned][carPersonToLookAt];
}