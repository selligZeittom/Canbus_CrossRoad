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
    for(uint8_t i = 0; i < NUMBERLIGHTS_CAR_PERSON; i++)
    {
        uint8_t color = colorLights[i]; //get the color of the light
        uint8_t occupancy = occupancyPaths[i]; //look if there are still cars
        uint8_t duration = durationLights[i];
        
        //depending on the color of the light switching off the lights
        switch (color)
        {
            case RED :

                if(i > 7 && warningLights[i-8] == 1){
                    setWarningLight(i-8, ORANGE_BLINKING_OFF);
                    warningLights[i-8] = 0;
                }
                break;
                
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
                    }
                }
                break;
                
            case GREEN:

                if(i > 7 && duration >= GREEN_TIME)
                {
                    occupancyPaths[i] = 0;
                    //decrPriority(i);
                } 
                if(duration >= GREEN_TIME && occupancy == 0) //if the green light stayed enough long and nobody is waiting anymore
                {
                    //go to orange
                    setLight(i,ORANGE);
                }
                break;
                
            case RED_ORANGE:
                if(duration >= RED_ORANGE_TIME) //if the red_orange light stayed enough long
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
     uint8_t firstUserIsGone = 0;
     
    //look for every car/person waiting if there is a solution 
    for(uint8_t i = 0; i < numberWaitingUsers; i++)
    {
        //store the potentials conflicts : 0 = no conflict, 1 = conflict
        uint8_t conflictUnresolved = 0;
       
        //store the potential warning users
        uint8_t storedWarningPedestrian = 255;
        
        //is there a warning with all the others users
        uint8_t warningUnresolved = 0;
     
       //the car/person which is waiting
        uint8_t waitingUser = priorityUser[i];
        
        //check all the other car/persons which may cause a conflict with the waiting car
        for(uint8_t j = 0; j < NUMBERLIGHTS_CAR_PERSON; j++)
        {
            //get the code for the conflict between thoses 2 users
            uint8_t conflict = getConflict(waitingUser, j);
            
            //warning for pedestrian
            if(conflict == WARNING)
            { 
                if(colorLights[waitingUser] == RED && durationLights[waitingUser] >= RED_TIME && occupancyPaths[j] == 1)
                {
                    /*
                    //set the warning light for the user j-8, because person are from 8 to 11 in the array
                    setWarningLight((waitingUser-8), ORANGE_BLINKING_ON);
                    warningLights[waitingUser-8] = 1;
                     * */
                    
                    warningUnresolved = 1;
                    storedWarningPedestrian = waitingUser-8;
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
                    shiftPriority(); //set this user at the end of the priority array
                }
            }
        } 
        
        //only if there isn't a conflict and the color is red and the duration is enough long we can let the car go...
        if(conflictUnresolved == 0 && colorLights[waitingUser] == RED && durationLights[waitingUser] >= RED_TIME)
        {
            if(i == 0)
            {
                firstUserIsGone = 1;
            }
            if(colorLights[waitingUser] == GREEN)
            {
                firstUserIsGone = 1;
            }
            if(colorLights[priorityUser[0]] == GREEN)
            {
                firstUserIsGone = 1;
            }
            if(firstUserIsGone == 1)
            {
                if(warningUnresolved == 1)
                {
                    setWarningLight((storedWarningPedestrian), ORANGE_BLINKING_ON);
                    warningLights[storedWarningPedestrian] = 1;
                }
                if(waitingUser < 8 ){
                    setLight(waitingUser, RED_ORANGE);
                }
                else{
                    setLight(waitingUser, GREEN);
                    decrPriority(waitingUser);
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

void shiftPriority()
{
    uint8_t temp = priorityUser[0]; //store the first priority that was stopped
    for(uint8_t i = 0; i < numberWaitingUsers-1; i++)
    {
        priorityUser[i] = priorityUser[i+1]; //shift the array
    }
    priorityUser[numberWaitingUsers-1] = temp; //put the old priority at the first place
}

uint8_t getConflict(uint8_t carPersonConcerned, uint8_t carPersonToLookAt)
{
    return conflicts[carPersonConcerned][carPersonToLookAt];
}