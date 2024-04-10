//import cue drawing header file
#include "cueDrawing.h"
//import graphics library
#include <UTFT.h>
//import packages for touch code
#include <stdint.h>
#include <SPI.h>
#include <Wire.h>

uint8_t flag;
bool correct = false;
uint8_t testNum;
float percent = 0;
long increment = 0;
//BEGIN VAR SETUP

//vars used for time taking (using millis())
long tic;
long toc;
long maxTime = 5000;
long currentTime;

//note that the screen object myGLCD is defined in the header file

//Trial Variables
const int numCues = 4;
const int numTrials = 15;
int correctCues[numTrials];
int correctCueForTrial;
int trialNum = -1;

// array for average time calculation
float timeTaken[numTrials];
//array for correctIncorrect breakdown per trial (0 if incorrect, 1 if correct)
int correctIncorrect[numTrials];
//array for which cue was supposed to be the correct cue for the trial
int correctCueArray[numTrials];

//randomization vars
int numPos = 4;
//a pointer that will reference the randomly generated cuePosArray
int *cuePosArray;

//A stuct used to store starting x and ending x values
struct xPoint {
  int x1;
  int x2;
};

//struct used to store the x values for each cue when the cues are drawn in the center
struct cuesCenter {
  struct xPoint horizontal;
  struct xPoint vertical;
  struct xPoint box;
  struct xPoint boxInverse;
};
cuesCenter centerPos = {50, 200, 225, 375, 400, 550, 575, 725};

//vars used to keep track of the x coordinates the script needs to look for a correct touch
int xCorStart;
int xCorEnd;
const int yCorStart = 140;
const int yCorEnd = 340;

// Define states (for the program to switch over)
typedef enum {

  menu_screen,
  wait_for_start,
  select_trial,
  draw_stuff,
  show_results,

} state_defs;

state_defs programState;

//END VAR SETUP
void setup() {
  //init random seed for use throughout the script
  randomSeed(analogRead(A0));
  Serial.begin(9600);
  Wire.begin(); // join i2c bus (address optional for master)
  // init the LCD
  myGLCD.InitLCD();
  myGLCD.setFont(BigFont);
  pinMode(8, OUTPUT);  //backlight
  digitalWrite(8, HIGH);//on
  // -------------------------------------------------------------
  //define width and height of the screen
  int xSize = myGLCD.getDisplayXSize();
  int ySize = myGLCD.getDisplayYSize();

  //determine the center point of the LCD and save in a center variable
  center[0] = xSize / 2;
  center[1] = ySize / 2;

  //generates array that defines which cue is going to be correct ([0,1]
  //                                                               [2,3])
  for (int cT = 0; cT < numTrials; cT++) {
    correctCues[cT] = random(numCues);
  }
  // -------------------------------------------------------------
  //code we grabbed from example scripts that allow touch sensing on a capacitative screen
  readOriginValues();
  pinMode     (FT5206_WAKE, INPUT);
  digitalWrite(FT5206_WAKE, HIGH );
  writeFT5206TouchRegister(0, eNORMAL); // device mode = Normal
  uint8_t periodMonitor = readFT5206TouchRegister(0x89);
  uint8_t  lenLibVersion = readFT5206TouchAddr(0x0a1, buf, 2 );
  if (lenLibVersion) {
    uint16_t libVersion = (buf[0] << 8) | buf[1];
  }
  else {
    Serial.println("lib version length is zero");
  }

  uint8_t firmwareId = readFT5206TouchRegister( 0xa6 );
  pinMode     (FT5206_INT, INPUT);
  //digitalWrite(FT5206_INT, HIGH );
  // -------------------------------------------------------------
  //set program state to the beginning menu screen
  programState = menu_screen;
}

void loop() {
  //more code that is needed to use the touch sensing capabilities of the screen, AGAIN we did not write this
  int buf[798];
  int x, x2;
  int y, y2;
  int r;
  uint8_t flag = 1; // flag initializer
  static uint16_t w = 800;
  static uint16_t h = 480;
  float xScale = 1024.0F / w;
  float yScale = 1024.0F / h;
  uint8_t attention = digitalRead(FT5206_INT);
  static uint8_t oldAttention = 1;
  uint32_t thisTouchTime = millis();
  uint8_t i = 0;

  static
  uint32_t lastTouchTime = thisTouchTime;

  switch (programState) {

    case menu_screen: {
        //pick test (whether arrows show up with cues or separately)
        myGLCD.clrScr();
        //will keep looking for a touch until flag var is set to 0
        flag = 1;
        //init trial num as -1, so the first trial will be 0
        trialNum = -1;
        //initialize the vars that will be used to calculate percent correct, and average selection time
        percent = 0;
        increment = 0;

        //coordinates to draw rectangles
        int trial1[4] = {130, 200, 390, 260};
        int trial2[4] = {410, 200, 670, 260};

        myGLCD.setColor(0, 255, 255);
        myGLCD.fillRect(trial1[0], trial1[1], trial1[2], trial1[3]);
        myGLCD.setColor(0, 0, 0);
        myGLCD.setBackColor(0, 255, 255);
        //prints to the LCD
        myGLCD.print("Simultaneous", 160, 225, 0);

        myGLCD.setColor(255, 255, 0);
        myGLCD.fillRect(trial2[0], trial2[1], trial2[2], trial2[3]);
        myGLCD.setColor(0, 0, 0);
        myGLCD.setBackColor(255, 255, 0);
        myGLCD.print("Sequential", 445, 225, 0);


        while (flag) {

          attention = digitalRead(FT5206_INT);

          /* Wait around for touch events */
          if (!attention && oldAttention ) {
            uint8_t count = readFT5206TouchLocation( touchLocations, 5 );

            if (count) {
              static TouchLocation lastTouch = touchLocations[0];

              if (((thisTouchTime - lastTouchTime) > 10000) && sameLoc( touchLocations[0], lastTouch ) ) {
                myGLCD.setColor(0, 0, 0);
                myGLCD.fillRect(0, 0, 799, 479);
                lastTouchTime = thisTouchTime;
              }

              //section of the touch code fragment we needed to manipulate, you can just change the xstart, ystart, xend, and y end values for wherever your button is
              for (i = 0; i < count; i++) {
                if (touchLocations[i].x >= trial1[0] && touchLocations[i].y >= trial1[1] && touchLocations[i].x <= trial1[2] && touchLocations[i].y <= trial1[3]) {
                  //simultaneous test was chosen
                  testNum = 0;
                  Serial.println("Simultaneous");
                  flag = 0;
                  programState = wait_for_start;
                }
                else if (touchLocations[i].x >= trial2[0] && touchLocations[i].y >= trial2[1] && touchLocations[i].x <= trial2[2] && touchLocations[i].y <= trial2[3]) {
                  //Sequential test was chosen
                  testNum = 1;
                  Serial.println("Sequential");
                  flag = 0;
                  programState = wait_for_start;
                }
              }
            }
          }

          else {
          }
          oldAttention = attention;
        }
        //once a task is selected we break from this state and proceed to the next
        break;
      }

    case wait_for_start: {
        //clear LCD
        myGLCD.clrScr();

        if (testNum == 0) {
          myGLCD.setColor(0, 255, 255);
          myGLCD.setBackColor(0, 0, 0);
          myGLCD.print("Simulataneous", 300, 100, 0);
          myGLCD.print("Cues and Arrows appear together", 150, 150, 0);
        }
        else if (testNum == 1) {
          myGLCD.setColor(255, 255, 0);
          myGLCD.setBackColor(0, 0, 0);
          myGLCD.print("Sequential", 300, 100, 0);
          myGLCD.print("Arrows appear shortly after cues disappear", 70, 150, 0);
        }

        myGLCD.setColor(0, 255, 0);
        myGLCD.fillRect(300, 300, 500, 380);
        myGLCD.setColor(0, 0, 0);
        myGLCD.setBackColor(0, 255, 0);
        myGLCD.print("START", 360, 330, 0);

        myGLCD.setColor(255, 0, 0);
        myGLCD.fillRect(50, 410, 150, 470);
        myGLCD.setColor(0, 0, 0);
        myGLCD.setBackColor(255, 0, 0);
        myGLCD.print("BACK", 70, 430, 0);
        flag = 1;

        //wait for touch inside of the start button to begin the experiment
        while (flag) {

          attention = digitalRead(FT5206_INT);

          /* Wait around for touch events */
          if (!attention && oldAttention ) {
            uint8_t count = readFT5206TouchLocation( touchLocations, 5 );

            if (count) {
              static TouchLocation lastTouch = touchLocations[0];

              if (((thisTouchTime - lastTouchTime) > 10000) && sameLoc( touchLocations[0], lastTouch ) ) {
                myGLCD.setColor(0, 0, 0);
                myGLCD.fillRect(0, 0, 799, 479);
                lastTouchTime = thisTouchTime;
              }

              for (i = 0; i < count; i++) {
                if (touchLocations[i].x >= 300 && touchLocations[i].y >= 300 && touchLocations[i].x <= 500 && touchLocations[i].y <= 380) {
                  flag = 0;
                  programState = select_trial;
                }
                else if (touchLocations[i].x >= 50 && touchLocations[i].y >= 410 && touchLocations[i].x <= 150 && touchLocations[i].y <= 470) {
                  flag = 0;
                  programState = menu_screen;
                }
              }
            }
          }

          else {
          }

          oldAttention = attention;
        }

        break;
      }

    case select_trial: {

        //decide if there are more trials, if there are no more
        //programState should jump to show_results
        if (trialNum >= numTrials - 1) {
          programState = show_results;
          break;
        }

        //generate the random positions for the cues for the trial
        for (int i = 0; i < numPos; i++) {
          cuePosArray[i] = -1;
        }
        cuePosArray = getRandArray();
        myGLCD.clrScr();
        //ITI of 1.5 seconds
        delay(1500);

        //sets which position should be the correct cue for the trial (0,1,2,3)
        correctCueForTrial = correctCues[trialNum];
        //iterate to next trial
        trialNum += 1;
        programState = draw_stuff;
        break;
      }

    case draw_stuff: {

        //Display stuff for synchronous task
        if (testNum == 0) {

          drawVerticalNew(cuePosArray[0]);
          if (cuePosArray[0] == correctCueForTrial) {
            xCorStart = centerPos.vertical.x1;
            xCorEnd = centerPos.vertical.x2;
          }
          drawHorizontal(cuePosArray[1]);
          if (cuePosArray[1] == correctCueForTrial) {
            xCorStart = centerPos.horizontal.x1;
            xCorEnd = centerPos.horizontal.x2;
          }
          drawBoxWith(cuePosArray[2]);
          if (cuePosArray[2] == correctCueForTrial) {
            xCorStart = centerPos.box.x1;
            xCorEnd = centerPos.box.x2;
          }

          drawBoxWithInverse(cuePosArray[3]);
          if (cuePosArray[3] == correctCueForTrial) {
            xCorStart = centerPos.boxInverse.x1;
            xCorEnd = centerPos.boxInverse.x2;
          }
          drawArrowPos(correctCueForTrial);
          //subject gets 3 seconds on to see cues and arrows before going to selection screen
          delay(3000);
          //clear the screen and draw the cues in the center for participant to pick a cue.
          myGLCD.clrScr();
          drawCuesCenter();

          //take the start time, so we can check how long it has been since the beginning of the selection period
          tic = millis();

          flag = 1;
          //look for touch 
          while (flag) {

            //take a current time, and check if the current time minus tic (start time) is > than 5 seconds, if it is trial is wrong, go to next trial
            currentTime = millis();
            if (currentTime - tic > maxTime) {
              myGLCD.fillScr(255, 0, 0);
              timeTaken[trialNum] = maxTime;
              correctCueArray[trialNum] = correctCueForTrial;
              correctIncorrect[trialNum] = 0;
              //red fills screen for 500 ms
              delay(500);
              flag = 0;
            }

            attention = digitalRead(FT5206_INT);

            /* Wait around for touch events */
            if (!attention && oldAttention ) {
              uint8_t count = readFT5206TouchLocation( touchLocations, 5 );

              //static uint8_t lastCount = count;

              if (count) {
                static TouchLocation lastTouch = touchLocations[0];

                if (((thisTouchTime - lastTouchTime) > 10000) && sameLoc( touchLocations[0], lastTouch ) ) {
                  myGLCD.setColor(0, 0, 0);
                  myGLCD.fillRect(0, 0, 799, 479);
                  lastTouchTime = thisTouchTime;
                }

                //if there is a touch inside of the correct cue, defied by xCorStartm yCorStart, xCorEnd, and yCorEnd
                //then the trial is correct, the time is taken by subtracting the current time - the start time and stored in a time array
                for (i = 0; i < count; i++) {
                  if (touchLocations[i].x >= xCorStart && touchLocations[i].y >= yCorStart && touchLocations[i].x <= xCorEnd && touchLocations[i].y <= yCorEnd) {
                    myGLCD.fillScr(0, 255, 0);
                    percent += 1;
                    toc = millis();
                    timeTaken[trialNum] = toc - tic;
                    correctCueArray[trialNum] = correctCueForTrial;
                    correctIncorrect[trialNum] = 1;
                    //we had a touch stop looking for one
                    flag = 0;
                    //see the green correct screen for 500ms
                    delay(500);
                  }
                  //there was a touch BUT it was not inside the correct bounds, trial is incorrect
                  else {
                    myGLCD.fillScr(255, 0, 0);
                    toc = millis();
                    timeTaken[trialNum] = toc - tic;
                    correctCueArray[trialNum] = correctCueForTrial;
                    correctIncorrect[trialNum] = 0;
                    //stop looking for touch
                    flag = 0;
                    delay(500);
                  }
                }
              }
            }

            else {
            }
            oldAttention = attention;
          }

        }

        //sequential task display stuff (draw arrows and cues separately)
        else {
          drawVerticalNew(cuePosArray[0]);
          if (cuePosArray[0] == correctCueForTrial) {
            xCorStart = centerPos.vertical.x1;
            xCorEnd = centerPos.vertical.x2;
          }
          drawHorizontal(cuePosArray[1]);
          if (cuePosArray[1] == correctCueForTrial) {
            xCorStart = centerPos.horizontal.x1;
            xCorEnd = centerPos.horizontal.x2;
          }
          drawBoxWith(cuePosArray[2]);
          if (cuePosArray[2] == correctCueForTrial) {
            xCorStart = centerPos.box.x1;
            xCorEnd = centerPos.box.x2;
          }

          drawBoxWithInverse(cuePosArray[3]);
          if (cuePosArray[3] == correctCueForTrial) {
            xCorStart = centerPos.boxInverse.x1;
            xCorEnd = centerPos.boxInverse.x2;
          }

          //participant sees the cues for 3 seconds
          delay(3000);
          //clear cues off the screen
          myGLCD.clrScr();
          //draw arrows without cues
          drawArrowPos(correctCueForTrial);
          //particpant sees the arrows for 2.5 seconds
          delay(2500);
          myGLCD.clrScr();
          //draw selection screen
          drawCuesCenter();

          tic = millis();

          flag = 1;
          while (flag) {
            currentTime = millis();
            if (currentTime - tic > maxTime) {
              myGLCD.fillScr(255, 0, 0);
              timeTaken[trialNum] = maxTime;
              correctCueArray[trialNum] = correctCueForTrial;
              correctIncorrect[trialNum] = 0;
              delay(500);
              flag = 0;
            }

            attention = digitalRead(FT5206_INT);

            /* Wait around for touch events */
            if (!attention && oldAttention ) {
              uint8_t count = readFT5206TouchLocation( touchLocations, 5 );

              //static uint8_t lastCount = count;

              if (count) {
                static TouchLocation lastTouch = touchLocations[0];

                if (((thisTouchTime - lastTouchTime) > 10000) && sameLoc( touchLocations[0], lastTouch ) ) {
                  myGLCD.setColor(0, 0, 0);
                  myGLCD.fillRect(0, 0, 799, 479);
                  lastTouchTime = thisTouchTime;
                }

                for (i = 0; i < count; i++) {
                  if (touchLocations[i].x >= xCorStart && touchLocations[i].y >= yCorStart && touchLocations[i].x <= xCorEnd && touchLocations[i].y <= yCorEnd) {
                    myGLCD.fillScr(0, 255, 0);
                    percent += 1;
                    toc = millis();
                    timeTaken[trialNum] = toc - tic;
                    correctCueArray[trialNum] = correctCueForTrial;
                    correctIncorrect[trialNum] = 1;
                    flag = 0;
                    delay(500);
                  }
                  else {
                    myGLCD.fillScr(255, 0, 0);
                    toc = millis();
                    timeTaken[trialNum] = toc - tic;
                    correctCueArray[trialNum] = correctCueForTrial;
                    correctIncorrect[trialNum] = 0;
                    flag = 0;
                    delay(500);
                  }
                }
              }
            }

            else {
            }
            oldAttention = attention;
          }
        }

        //programState back to select trial (figure out next trial)
        programState = select_trial;
        break;
      }
      
    //print the time array, correct/incorrect array, and the correct cue array to the serial monitor
    //display stats for the session on the screen: percent correct and average time taken
    case show_results: {

        Serial.print("Time taken for each trial: ");
        Serial.print("[");
        for (i = 0; i < numTrials; i++) {
          increment += timeTaken[i];
          Serial.print(timeTaken[i]);
          Serial.print(",");
        }
        Serial.println("]");

        Serial.println("Correct or incorrect for each trial: ");
        Serial.print("[");
        for (i = 0; i < numTrials; i++) {
          Serial.print(correctIncorrect[i]);
          Serial.print(",");
        }
        Serial.println("]");


        Serial.println("Correct cue for each trial: ");
        Serial.print("[");
        for (i = 0; i < numTrials; i++) {
          Serial.print(correctCueArray[i]);
          Serial.print(",");
        }
        Serial.println("]");

        //calc average selection time
        increment /= numTrials;
        myGLCD.clrScr();

        if (testNum == 0) {
          myGLCD.setColor(0, 255, 255);
          myGLCD.setBackColor(0, 0, 0);
        }
        if (testNum == 1) {
          myGLCD.setColor(255, 255, 0);
          myGLCD.setBackColor(0, 0, 0);
        }
        //calc percent correct
        float percentFinal = percent / numTrials * 100.0;
        //display stats to the screen
        myGLCD.print("Percentage: ", 160, 100, 0);
        myGLCD.print("Average Selection Time: ", 160, 150, 0);
        myGLCD.printNumF(percentFinal, 2, 350, 100, 0);
        myGLCD.printNumF(increment, 2, 550, 150, 0);


        myGLCD.setColor(255, 0, 0);
        myGLCD.fillRect(300, 300, 500, 380);
        myGLCD.setColor(0, 0, 0);
        myGLCD.setBackColor(255, 0, 0);
        //display an exit button that brings the user back to the menu screen if pushed
        myGLCD.print("EXIT", 370, 330, 0);
        flag = 1;

        while (flag) {

          attention = digitalRead(FT5206_INT);

          /* Wait around for touch events */
          if (!attention && oldAttention ) {
            uint8_t count = readFT5206TouchLocation( touchLocations, 5 );

            //static uint8_t lastCount = count;

            if (count) {
              static TouchLocation lastTouch = touchLocations[0];

              if (((thisTouchTime - lastTouchTime) > 10000) && sameLoc( touchLocations[0], lastTouch ) ) {
                myGLCD.setColor(0, 0, 0);
                myGLCD.fillRect(0, 0, 799, 479);
                lastTouchTime = thisTouchTime;
              }

              for (i = 0; i < count; i++) {
                if (touchLocations[i].x >= 300 && touchLocations[i].y >= 300 && touchLocations[i].x <= 500 && touchLocations[i].y <= 380) {
                  flag = 0;
                  programState = menu_screen;
                }
              }
            }
          }

          else {
          }
          oldAttention = attention;
        }

        break;
      }
  } // end switch
} // end loop

// function returns pointer that references a randomly generated array from 0-3
int *getRandArray() {
  int posArray[4] = {0, 1, 2, 3};
  static int cuePosArray[4] = { -1, -1, -1, -1};
  randomSeed(analogRead(A1));
  int ridx;

  for (int cP = 0; cP < numPos; cP++) {
    ridx = random(numPos);
    if (cuePosArray[ridx] == -1) {
      cuePosArray[ridx] = posArray[cP];
    }
    else {
      while (cuePosArray[ridx] != -1) {
        ridx = random(numPos);
      }
      cuePosArray[ridx] = posArray[cP];
    }
  }
  // returns a pointer to cuePosArray
  return cuePosArray;
}
