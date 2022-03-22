// Author: Marine CAMBA 
// DM6001 Seamus GORDON
// MEng Mechatronics 2021-2022
// Assignement 2

// Brief: This code takes G-Code values as location parameters (inputs) and trace the design using coordinates. 
// The coordinates will be put in excel and trace using a graph to show the design. 

//------------------------------------------------------------------------------------

/* I used TinkerCAD simulator as I had problem with Arduino simulator, the only changes are that one must make '/n'
 *  to send the value. And that is was difficult to copy paste the output to excel (max 150 output and it auto scrolls
 *  down). 
 */

//--------------------MOVES---------------------------------------
/*
TO DO LINE BY LINE
IF USING THE SERIAL MONITOR ON ARDUINO IDE, NEED TO DO AN ENTER AT THE END OF EACH LINE AND NO '#'
IF USING THE ARDUINO SIMULATOR, PUT AN '#' AT THE END OF EVERY LINE AND USE THE CASE '#' LINE 101
MOVE DEPENDS ON THE PREVIOUS POSITION

G00 X10.0 Y0.0 S50 #          // Increase the speed of the rapid move
G01 X30.0 Y0.0 F100 #         
G03 X10.0 Y10.0 I0.0 J10.0 #  // First arc counterclokwise
G01 X0.0 Y35.0.0 F100 #
G03 X-5.0 Y5.0 I-6.0 J0.0 #   // Second arc, also counterclockwise
G01 X-40.0 Y0.0 F100 #
G01 X-5.0 Y-5.0 F100 #
G01 X0.0 Y-35.0 F100 #
G01 X10.0 Y-10.0 F100 #
G00 X15.0 Y39.5 S50 #         // Placing the target at the bottom of the central circle
G03 X0.0 Y5.0 I0.0 J2.5 #     // Do the first demi-circle using counterclockwise motion
G00 X0.0 Y-5.0 S50 #          // Return to bottom of the circle
G02 X0.0 Y5.0 I0.0 J2.5 #     // Do the second demi-circle using clockwise motion
G00 X-25.0 Y-44.5 S50 #       // Go back to (0,0) position
 */
 
//--------------------PRE PROCESSOR INSTRUCTIONS-------------------
#include<Stepper.h>   // This librairy will control the steppers
#include<math.h>      // Here to calculate arcs

#define RESOLUTION 0.01   // Needed to know how much we can approximate the arcs, took 0.01 to increase speed of the code

// Name of the different modes to be used
#define RAPID 0
#define LINE 1
#define ARC_CLW 2
#define ARC_CCLW 3

//--------------------GLOBAL VARIABLES-----------------------------
int movemode;   // What mode is going to be used for a movement
const int stepsPerRevolution = 200;   
float xpos, ypos; // Actual positions
int xprevious = 0, yprevious = 0; // Previous positions, to help calculate relative displacement  
int xsteps, ysteps; // Steps to make to reach the wanted position
int speed, init_speed =30; // Speed of the stepper motors, in rpm
int feedRate; // Speed at which the pen will write in line mode, in mm per minute
float ival, jval; // Position of arc centres

//--------------------CREATION OF STEPPERS------------------------
Stepper X_Axis(stepsPerRevolution, 2, 3, 5, 6); // Create an instance of Stepper for axis X, connected to pin 2,3,5 and 6 to the Arduino
Stepper Y_Axis(stepsPerRevolution, 7, 8, 9, 10); // Create an instance of Stepper for axis Y, connected to pin 7,8,9 and 10 to the Arduino

//--------------------SETUP---------------------------------------
void setup() {
  Serial.begin(9600);   // Launch the dialogue between arduino and the laptop (through the Serial Monitor)
  X_Axis.setSpeed(init_speed);    // Set initial speed of the stepper
  Y_Axis.setSpeed(init_speed);
  Serial.println("Enter G-code coordinates: "); // Tell user to input values from keyboard, don't forget to do it line by line 
}

//--------------------MAIN LOOP------------------------------------
void loop() {
  if (Serial.available()) {  // Wait for something to be typed
    // Beware! Always write in float format (10.0 instead of 10) as we use parsefloat and that putting an 'int' may cause trouble
    char ch = Serial.read();  // Read the input character

    // Depending on the character inputed, the program will behave differently
    switch (ch) {

      case'G': movemode = Serial.parseInt(); // After the G and before the next "not int" character, store the number found and use it for the chosemode function
        Serial.print("Move mode:");
        Serial.println(movemode);
        break;

      case 'X':
      case 'x': xpos = Serial.parseFloat();  // If it is X then expect a float to follow
        Serial.print("X move:");
        Serial.println(xpos);
        break; // Values are stored

      case 'Y':
      case 'y': ypos = Serial.parseFloat();  // If it is X then expect a float to follow
        Serial.print("Y move:");
        Serial.println(ypos);
        break; // Values are stored

      case 'S': // Speed at which the stepper in rapid mode should go, in rpm
      case 's': 
        speed = Serial.parseInt();   // Get value for speed and store, used only in rapidmove mode to define a max speed
        Serial.print("Speed:");
        Serial.println(speed); 
        break;

      case 'F': 
      case 'f': 
        feedRate = Serial.parseInt();   // Get value for feedrate, used only in linemove mode to define speed in mm/minute
        Serial.print("Feedrate:");
        Serial.println(feedRate);
        break;
        
      case 'I':
      case 'i': ival = Serial.parseFloat();  // If it is X then expect a float to follow
        break; // Values are stored

      case 'J': 
      case 'j': jval    = Serial.parseFloat();  // If it is X then expect a float to follow
        break; // Values are stored

      case  '#':    // Comment if using Arduino IDE otherwise it does each movement twice
      case '\n':   // Return key has been hit
        chosemode(movemode);    // Select the move to do depending on the G number inputed
        break;

      default:
      if (ch != ' '){
        Serial.print("Not recognised:"); // A non-valid input so ignore
        Serial.print(ch);
      }
        break;
    }
  }
}

//--------------------DIFFERENT WAYS OF MOVING - functions-------------------
void chosemode (int mode){
   /* Choose the move mode depending on the Gxx input.
    I created a function outside the loop because my Arduino simulator did not wanted to 
    enter in rapidmove/linemove/etc if it was in the case '/n' of the switch. But doing so is 
    not mandatory one could simply copy paste the content of the function and remplace "chosemode(movemode)" with that.*/
   if (mode == RAPID) {
    rapidmove(xpos, ypos, speed);
   }
   if (mode == LINE){
    linemove(xpos, ypos, feedRate);
   }
   if (mode == ARC_CLW) {
    arcmove_CLW(xpos, ypos, ival, jval);
   }
   if (mode == ARC_CCLW) {
    arcmove_CCLW(xpos, ypos, ival, jval);
   }
}

void rapidmove(float x, float y, int maxSpeed)
{
  int xsteps = x* 100;    // To do 1mm we need 100 steps so we have to multiply by 100 (as a revolution is 2mm and we choose stepPerRevolution to be 200)
  int ysteps = y* 100;    // the number of iteration our while loop will do
  int xcount=0, ycount=0;   // Increase or decrease until it is equal to xsteps and/or ysteps
  
  // Set speed to value given
  X_Axis.setSpeed(maxSpeed);
  Y_Axis.setSpeed(maxSpeed);

  // 3 cases to work on : horizontal, vertical and diagonal
  if (xsteps==0 && ysteps != 0){    // Vertical movement with X0.0, only Y will move
    while (ycount!=ysteps){         // We increase/decrease ycount until it is equal to ysteps, then y destination is reached
      if (ysteps>=0){               // If Ysteps>=0 so Ycount has to increases
        if (ycount < ysteps){       // Until the number of step (counted by ycount) is not reached, '<' so that is doesn't do an overstep
          Y_Axis.step(1);           // Y stepper increases by one step
          ycount++;                 // Ycount increases and the while loop continues until ycount=ystep so the Y position is reached
        }
      }
        else{                       // If Ysteps<=0 so Ycount has to decrease 
          if (ycount > ysteps){     // '>' so that is doesn't do an overstep
            Y_Axis.step(-1);        // Y stepper decreases by one step
            ycount--;               // Ycount decreases and the while loop continues until ycount=ystep so the Y position is reached
          }
        }
    // Just for testing purpose, these steps are not supposed to be used in the drawing as we are in rapid mode and not line mode
    /*Serial.print(xcount+xprevious);
    Serial.write(9);  
    Serial.println(ycount+yprevious);*/
    }
  } 
      
  if (ysteps==0 & xsteps != 0){     // Vertical movement with Y0.0, only X will move
      while (xcount!=xsteps){       // Same methodology as before
        if (xsteps>=0){            
          if (xcount < xsteps){
            X_Axis.step(1);   
            xcount++;
            
          }
        }
        else{
          if (xcount > xsteps){
            X_Axis.step(-1);
            xcount--;
          }
        }    
      // Same as previous
      /*Serial.print(xcount+xprevious);
      Serial.write(9);  
      Serial.println(ycount+yprevious);*/
      }
  }

  if (xsteps !=0 && ysteps!=0){   // Diagonal movement 
  while (xcount != xsteps || ycount != ysteps)  // Needs to reach X AND Y given positions, so continue until the 2 conditions are false
  {
    if (xsteps>=0){   // We increase/decrease xcount until it is equal to xsteps, then x destination is reached
      if (xcount < xsteps){
        X_Axis.step(1);
        xcount++;
      }
    }
    else{
      if (xcount > xsteps){
        X_Axis.step(-1);
        xcount--;
      }
    }

    if (ysteps>=0){   // Same for y
      if (ycount < ysteps){
        Y_Axis.step(1);
        ycount++;
      }
    }
    else{
      if (ycount > ysteps){
        Y_Axis.step(-1);
        ycount--; 
      }
    }
    // Same as previous
    /*Serial.print(xcount+xprevious); 
    Serial.write(9);  
    Serial.println(ycount+yprevious);*/
    }
  }
  // Set speed to initial value 
  X_Axis.setSpeed(init_speed);
  Y_Axis.setSpeed(init_speed);

  // Set the new actual position as our displacement depends on the previous movement (so when the movement is finished the last know position becomes
  // the new actual position and the next movement will be based on that
  xprevious+=xcount;
  yprevious+=ycount;
  // This print allowed me to know if the rapidmove went to the good position as rapidmove is not printing each position the stepper makes
  Serial.print("Actual position : X = ");
  Serial.print(xprevious);
  Serial.print("; Y = ");
  Serial.println(yprevious);
  }

void linemove(float x, float y, int feedRate)
{
  // The reasoning is exactly the same except that here we have to print the step we are in to mimic the pen "drawing" whereas with 
  // rapid move the pen does not draw it just changes position
  int xsteps = x* 100;
  int ysteps = y* 100;
  int xcount=0, ycount=0;
  
  // For the feedrate we know that feedrate is in mm/min and speed in rpm, 100steps = 1mm and 200 steps = 1 revolution, hence 1rpm=200steps/min so:
  // -> a feedrate of 50mm/min=5000steps/min=25*200steps/min=25rpm
  X_Axis.setSpeed(feedRate/2);
  Y_Axis.setSpeed(feedRate/2);
  
  if (xsteps==0 && ysteps != 0){
    while (ycount!=ysteps){
      if (ysteps>=0){
        if (ycount < ysteps){
          Y_Axis.step(1);
          ycount++;
        }
      }
        else{
          if (ycount > ysteps){
            Y_Axis.step(-1);
            ycount--; 
          }
        }
    // Here we have to print xcount=the actual number of steps the stepper has done since the last G-code instruction and ADD
    // the previous number of step because our movement depends on the precedent one !
    Serial.print(xcount+xprevious);
    Serial.write(9);  // ...enables us to separate xposition and ypositions using csv format with ',' character and tab as a separator
    Serial.println(ycount+yprevious);
     }
  } 
      
  if (ysteps==0 & xsteps != 0){
      while (xcount!=xsteps){
        if (xsteps>=0){   
          if (xcount < xsteps){
            X_Axis.step(1);
            xcount++;
          }
        }
        else{
          if (xcount > xsteps){
            X_Axis.step(-1);
            xcount--;
          }
        }   
    Serial.print(xcount+xprevious);
    Serial.write(9);  
    Serial.println(ycount+yprevious);
    }
  }

  if (xsteps !=0 && ysteps!=0){
  while (xcount != xsteps || ycount != ysteps)  
  {
    if (xsteps>=0){   
      if (xcount < xsteps){
        X_Axis.step(1);
        xcount++;
      }
    }
    else{
      if (xcount > xsteps){
        X_Axis.step(-1);
        xcount--;
      }
    }

    if (ysteps>=0){
      if (ycount < ysteps){
        //Y_Axis.step(1);
        ycount++;
      }
    }
    else{
      if (ycount > ysteps){
        //Y_Axis.step(-1);
        ycount--; 
      }
    }
   Serial.print(xcount+xprevious);
   Serial.write(9);  
   Serial.println(ycount+yprevious);
}
  }
  //Set the new actual position 
  xprevious+=xcount;
  yprevious+=ycount;
  // Set speed to initial value 
  X_Axis.setSpeed(init_speed);
  Y_Axis.setSpeed(init_speed);
}

void arcmove_CCLW(float x, float y, float i, float j)
{
  // This movement will go to x and y position making a COUNTER clock wise move centered in (i,j)
  // It's maximum angle is from -pi/2 to pi/2 and it can do arc from the range : [-pi/2, 0], [0, pi/2], [-pi/2, pi/2]
  // To draw an arc we will approximate it using small rectangles so that it can be drawn using linemove function already implemented

  float deltax, deltay; // This is the variation between the x (or y) position to reach and the center i (or j) of the circle, this will help us calculate the radius
  float xpos, ypos;     // Intermediaries positions taken by x and y to be calculated using line move
  float radius;

  deltax = abs(x - i); 
  deltay = abs(y - j); 
  radius = sqrt((deltax * deltax) + (deltay * deltay)); // Pythagoras to find the radius
  Serial.print(" Rad = : ");
  Serial.println(radius);
  float tangent_slope = atan(y / x);         // Angle of the rectangle to make the approximation with
  float included_angle = tangent_slope * 2;  //Angle between precedent point and the next one, in the circle
  Serial.println("Angle");
  Serial.println(included_angle);
   
  if (included_angle<0){    // If we want an arc to be from 0 to pi/2, the included angle will be negative (as we are in counterclockwise)
    for (float inc = 0; inc < abs(included_angle); inc += RESOLUTION) { 
    // We divide the angle we need in smaller angles to calculate small rectangles to create the arc (approximation by rectangles)
    // For each iteration, the angle is going to increase by RESOLUTION in degree
    // The condition is '<' so the last for iteration will not be bigger than the included angle
    ypos = radius * sin(inc); // This is the y cordinate of the line we'll have to trace
    xpos = -(radius-(radius * cos(inc))); // This is it's x coordinate
    // As you can see the y and the x have been "exchange" as when we are between -pi/2 and 0 the calculus of xpos will be the one of ypos in between 0 and pi/2 (rotation of 90 of the whole arc)
    
    // This prints the positions to reach for x and y in relative coordinates (regarding the previous position given by G-Code), this is for testing purpose and readability
    // We then need to draw the line between the two points using linemove
    /*Serial.print(xpos+xprevious);   
    Serial.write(9);  
    Serial.println(ypos+yprevious);*/
    linemove(xpos,ypos, init_speed); // We call line move to trace the line in between the x and y coordinate we found using the good stepper position
    // There is no need to add here "previous positions" as it is already done in the linemove function
    
    // Unfortunately I encountered a problem when using line move:
    // When using only the prints line 413, we saw that when asked to do G03 X1.0 Y1.0 I0.0 J1.0, the end position is correct as it is (1.0, 1.0) and when ploting in excel
    // we see the arc done properly, however when using line move, the movement stops at Xcount+Xprevious= 9951 and Ycount+Yprevious= 5671, so the steps to go to 10 000 and 10 000 are not done
    // This can be due to an iteration not done inside a for or a while. I tried to add an iteration in the for but it did oversteps, and I cannot change linemove function as it works for proper line moves.
    // Hence I am just plotting the arcs in excel using the prints line 413 and not line move
    }
  }
  
  else { 
  for (float inc = 0; inc < included_angle; inc +=RESOLUTION) {
    xpos = radius * sin(inc);   // Calculus given in the video
    ypos = radius-(radius * cos(inc)); 
    /*Serial.print(xpos+xprevious); 
    Serial.write(9);  
    Serial.println(ypos+yprevious);*/
    // Same problem regarding line move
    linemove(xpos,ypos, init_speed); 
  }
  }
}

void arcmove_CLW(float x, float y, float i, float j)
{
  // To do clock wise arc I simply use arcmove-CCLW and mirrored it as the main difference was how X was moving (decrease instead of increasing)
  float deltax, deltay;
  float xpos, ypos;
  float radius;

  deltax = abs(x - i); 
  deltay = abs(y - j); 
  radius = sqrt((deltax * deltax) + (deltay * deltay)); 
  Serial.print(" Rad = : ");
  Serial.println(radius);
  float tangent_slope = atan(y / x);
  float included_angle = tangent_slope * 2;
  Serial.print("Angle: ");
  Serial.println(included_angle);

  if (included_angle>=3.14){  // If we are doing an arc from -pi/2 to pi/2, I had to put >= because == wouldn't work
    for (float inc = 0; inc < included_angle; inc += RESOLUTION) {    // The condition remains the same
      xpos = -(radius * sin(inc));  // X should increase in the negatives to go clockwise so I simply added a '-' to "mirror" the counterclockwise movement
      ypos = radius-(radius * cos(inc)); // Remains the same as with CCLW movement
      // Like with CCLW I printed the values here to see if it was working or not
     /* Serial.print(xpos+xprevious);
      Serial.write(9);  
      Serial.println(ypos+yprevious);*/
      // Like with CCLW I had troubles using linemove as the last steps were not the correct ones
      linemove(xpos,ypos, init_speed); 
    }   
  }
  else {
    for (float inc = 0; inc > included_angle; inc -= RESOLUTION) {  // If we are doing an angle between -pi/2 and 0, this one I didn't use in my G-code but still implemented it
      // To go to the opposite way of CCLW I could not only use a negative X I had to transform the for loop so that the iterations in X would go in the negatives (with sin) 
      // but will remain the same in the positives with Y (as it is a cos)
      xpos = radius * sin(inc); 
      ypos = radius-(radius * cos(inc)); 
      /*Serial.print(xpos+xprevious);
      Serial.write(9);  
      Serial.println(ypos+yprevious);*/
      linemove(xpos,ypos, init_speed); // use initial speed
    }  
  }
}
