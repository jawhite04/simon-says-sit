/*
Sketch Name:  Simon_Says_Sit
Created By:   John White
Created Date: 20150301
Purpose:      Play "Simon", a pattern recognition game, with switch inputs and LED outputs and then use a motor to dispense a reward
Hardware:     TI LaunchPad MSP430, stepper motor 28BYJ-48, controller chip ULN2003APG; http://artists.sci-toys.com/node/49

Pin assignments:
01 none
02 Red LED +
03 Green LED +
04 White LED +
05 none
06 Red switch +
07 Green switch +
08 White switch +
09 none
10 speaker +
11 stepper motor controller pin 1N1
12 stepper motor controller pin 1N2
13 stepper motor controller pin 1N3
14 stepper motor controller pin 1N4
15 none
16 none
17 none
18 none
19 none
20 none
TP1 (immediately in front of the Mini USB connector) - controller chip power + 
*/

#include "pitches.h"
#include "Stepper.h"    //https://github.com/energia/Energia/blob/master/libraries/Stepper/Stepper.h

// Change # of LEDs and switches here (hardware must match this value and be added in pin assignments below)
#define PINS 3

// Change # of rounds per game here
#define ROUNDS 3

// Change how long the player has to pick during a round
int pickTimeout = 6000;  //6 seconds to enter the next pick

// LED Pin assignments
int aLEDs[PINS] = {2, 3, 4};

// Switch pin assignments
int aSwitches[PINS] = {6, 7, 8};

// Speaker tones to accompany LED and switch behavior
int aTones[PINS] = {NOTE_CS4, NOTE_F4, NOTE_GS4};
int soundDuration = 1500;

// Speaker pin assignment
int speakerPin = 10;

// stepper motor controller pin assignment
int motorOut1N1 = 11;
int motorOut1N2 = 12;
int motorOut1N3 = 13;
int motorOut1N4 = 14;

// stepper motor configuration
const int STEPS_PER_REVOLUTION = 64;
Stepper motor(STEPS_PER_REVOLUTION, motorOut1N1, motorOut1N3, motorOut1N2, motorOut1N4);

/*
  The setup routine runs once when you press reset
*/
void setup()
{

  // initialize LED pins and states, and switch pins
  for(int i = 0; i < PINS; i++) 
  {
    pinMode(aLEDs[i], OUTPUT);           //  initialize the LED digital pins as outputs
    digitalWrite(aLEDs[i], LOW);         //  initialize the LED state as "off"
    pinMode(aSwitches[i], INPUT_PULLUP); //  initialize switch pins as inputs
  }
  
  //set motor RPMs
  motor.setSpeed(400);

/*++SANITY TEST++

  int testTime = 200;
  ToneOut(speakerPin, aTones[0], testTime);
  BlinkPin(aLEDs[0], testTime, 0);
  ToneOut(speakerPin, aTones[1], testTime);
  BlinkPin(aLEDs[1], testTime, 0);
  ToneOut(speakerPin, aTones[2], testTime);
  BlinkPin(aLEDs[2], testTime, 0);

  motor.step(64);
  delay(testTime);
  motor.step(-64);
  
/*--SANITY TEST--*/
 
}

void loop()
{
  
  //  Signal the beginning of the game
  int beginningTime = 100;
  ToneOut(speakerPin, aTones[0], beginningTime);
  BlinkPin(aLEDs[0], beginningTime, 0);
  ToneOut(speakerPin, aTones[1], beginningTime);
  BlinkPin(aLEDs[1], beginningTime, 0);
  ToneOut(speakerPin, aTones[2], beginningTime);
  BlinkPin(aLEDs[2], beginningTime, 0);  
  ToneOut(speakerPin, aTones[0], beginningTime);
  BlinkPin(aLEDs[0], beginningTime, 0);
  ToneOut(speakerPin, aTones[1], beginningTime);
  BlinkPin(aLEDs[1], beginningTime, 0);
  ToneOut(speakerPin, aTones[2], beginningTime);
  BlinkPin(aLEDs[2], beginningTime, 0);
  
  //  Delay between reset and start of game
  delay(3000);
  
  //  Play the game!
  PlaySimon(aLEDs, aSwitches, aTones, soundDuration, speakerPin, pickTimeout);
}

// -----------------
// The Main Event
// -----------------
void PlaySimon(int aLEDs[], int aSwitches[], int aTones[], int duration, int speakerPin, int pickTimeout)
{

  // ------------------
  /*++ SANITY TEST ++ * /
  for(int i = 0; i < PINS; i++)
  {
    ToneOut(speakerPin, aTones[i], duration);
    BlinkPin(aLEDs[i], duration, 0);
  }
  motor.step(1024);
  // ------------------
  /*-- SANITY TEST --*/
  
  //blinky waiting waiter
  //for(int i = 0; i < 10; i++){BlinkPin(GREEN_LED, 500,500);}

  // ------------------

  // create the game array
  int RoundPick[ROUNDS];

  // initialize winning condition, which can be changed by PlayerInput negative outcome
  int recVal = 0;

  // initialize the game array with impossible values
  for(int i = 0; i < ROUNDS; i++){RoundPick[i] = -1;}

  /* How SIMON works
  1. select a random PICK (ex: 1, 2, or 3) 
  2. Add the selected PICK to the list of all PICKs for the game so far 
  3. Show the player all PICKs for the game so far
  4. Wait for the player to input the same PICKs that they were shown
  5. Loop this process until the max number of rounds is accomplished successfully
  6. If the player completed the last round, they win
  */

  // this loops through the pick array
  for(int i = 0; i < ROUNDS; i++)
  {
    // select a new PICK for this round and add that PICK to the list of all PICKs for the game
    RoundPick[i] = NewPick(0,PINS); // returns a zero-based index, so 0 to pins-1 is the necessary range (instead of 1->PINS+1)

    // display all PICKs
    DisplayAllPicks(aLEDs, aTones, RoundPick, speakerPin, duration);

    // listen for interrupts on switches in PICK order
    recVal = PlayerInput(RoundPick, pickTimeout, aLEDs, aSwitches, aTones, speakerPin, duration);

    // if PlayerInput received bad input, get out of this instance of the game
    if(recVal == -1)
    {
      break;
    }

    // delay between rounds 
    delay(3000);
  } // i = 0 to ROUNDS

  if(recVal == 0)
  {
    DispenseReward();
  }
} // PlaySimon

// -----------------
// Game Functions
// -----------------
int NewPick(int start, int end)
{
  int pick;
  randomSeed(random(1,2048));
  pick = random(start,end);
  return pick;
}

void DisplayAllPicks(int aLEDs[], int aTones[], int RoundPick[], int speakerPin, int duration)
{
  int thisRound;

  for(int i = 0; i < ROUNDS; i++)
  {
    thisRound = RoundPick[i];
    if(thisRound > -1)
    {
      DisplayPick(aLEDs[thisRound], aTones[thisRound], speakerPin, duration);  
    }
  }
}

void DisplayPick(int pin, int itone, int speakerPin, int duration)
{
    ToneOut(speakerPin, itone, duration);
    BlinkPin(pin, duration, 100);
}

int PlayerInput(int RoundPick[], int pickTimeout, int aLEDs[], int aSwitches[], int aTones[], int speakerPin, int duration)
{
  int thisRound;
  int actualButton;
  int retVal;

  // initialize passing condition
  retVal = 0;

  for(int j = 0; j < ROUNDS; j++)
  {
    thisRound = RoundPick[j];

    if(thisRound > -1)
    {
      actualButton = ButtonListener(pickTimeout, aSwitches, aLEDs, aTones, speakerPin);
      
      if(actualButton == thisRound)
      {
        DisplayPick(aLEDs[actualButton], aTones[actualButton], speakerPin, duration);
      }
      else
      {
        // set failing condition
        retVal = -1;
        break;
      }
    } // if this round has a valid PICK
  } // for each value in RoundPick

  if(retVal == -1)
  {
    WrongPick();
  }

  // pass retVal back to PlaySimon
  return retVal;
}

void WrongPick()
{
  // BUZZZZZZZZ
  for(int i = 0; i < 200; i++)
  {
    ToneOut(speakerPin, 200, 50);
    ToneOut(speakerPin, 203, 50);
  }
}

void DispenseReward()
{
  MotorControl(1);
}

// -----------------
// Utility Functions
// -----------------
void BlinkPin(int ledPin, int onTime, int offTime)
{
  digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level 3V)
  delay(onTime);                // wait
  digitalWrite(ledPin, LOW);    // turn the LED off (LOW is the voltage level 0V)
  delay(offTime);               // wait
}

void ToneOut(int pin, int note, int duration)
{
  noTone(pin);
  tone(pin, note, duration);
}

int ButtonListener(int pickTimeout, int aSwitches[], int aLEDs[], int aTones[], int speakerPin)
{
  int state = -1;
  int switchPin;
  volatile long lngStartTime = 0;
  volatile long lngEndTime = 0;
  int btnPin;
  int ledPin;
  int itone;
  int recVal = 0;
 
 // "start timer" by saving current uptime of the LaunchPad
  lngStartTime = millis();
  while(state == -1)
  {

    for(int i = 0; i < PINS; i++)
    {
      btnPin = aSwitches[i];
      ledPin = aLEDs[i];
      itone = aTones[i];
      recVal = SyncIO(btnPin, ledPin, itone, speakerPin);

      if(recVal == 1)
      {
        state = i;
      }
    }
      
    lngEndTime = millis();
    if(lngEndTime - lngStartTime > pickTimeout){break;}
  }
  // 
  return state;
}

int SyncIO(int btnPin, int ledPin, int speakerPin, int itone)
{
  int btn;
  btn = digitalRead(btnPin);

  // check if the pushbutton is pressed.
  // if it is, set the button state to HIGH:
  if (btn == LOW) 
  {     
    // turn LED on    
    digitalWrite(ledPin, HIGH);  
    ToneOut(speakerPin, itone, 200);
    return 1;
  } 
  else 
  {
    // turn LED off
    digitalWrite(ledPin, LOW); 
    return 0;
  }
}

void MotorControl(int bins)
{
  int ratio;
  int rotate;

  ratio = 2.0;  //ratio of little wheel to big lazy susan, eyeball this and patch it later (or be a perfectionist and use math)
  rotate = bins * (2048/6) * ratio;
  motor.step(rotate);
}
