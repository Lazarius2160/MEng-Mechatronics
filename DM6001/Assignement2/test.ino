// Author: Marine CAMBA 
// DM6001 Seamus GORDON
// MEng Mechatronics 2021-2022
// Assignement 2
//------------------------------------------------------------------------------------


//--------------------MOVES---------------------------------------
/*
TO DO LINE BY LINE
IF USING THE SERIAL MONITOR ON ARDUINO IDE, NEED TO DO AN ENTER AT THE END OF EACH LINE
IF USING THE ARDUINO SIMULATOR, PUT AN '#' AT THE END OF EVERY LINE AND UNCOMMENT CASE '#' LINE 101
MOVE DEPENDS ON THE PREVIOUS POSITION

G00 X10.0 Y0.0 S50 #          // Increase the speed of the rapid move
G01 X30.0 Y0.0 F100 #         
G03 X10.0 Y10.0 I0.0 J10.0 #  // First arc counterclokwise
G01 X0.0 Y34.0.0 F100 #
G03 X-6.0 Y6.0 I-6.0 J0.0 #   // Second arc, also counterclockwise
G01 X-39.0 Y0.0 F100 #
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
Stepper Y_Axis(stepsPerRevolution, 7, 8, 9, 10); // Create an instance of Stepper for axis Y

//--------------------SETUP---------------------------------------
void setup() {
  Serial.begin(9600);
  X_Axis.setSpeed(init_speed);
  Y_Axis.setSpeed(init_speed);
  Serial.println("Enter G-code coordinates: "); // Tell user to input values from keyboard, don't forget to do it line by line 
}

//--------------------MAIN LOOP------------------------------------
void loop() {
  if (Serial.available()) {  // Wait for something to be typed
    // Beware! Always write in float format (10.0 instead of 10) as we use parsefloat and that putting an 'int' may cause trouble
    char ch = Serial.read();  // Read the input character
    
    switch (ch) {

      case'G': movemode = Serial.parseInt(); // After the G and before the next not int character, store the number found and use it for the chosemode function
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

      case  '#':    // If use '#' in arduino IDE does each movement twice
      case '\n':   // Return key has been hit. 
        chosemode(movemode);    // Select the move to do depending on the G input
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
  int xsteps = x* 100;
  int ysteps = y* 100;
  int xcount=0, ycount=0;
  
  // Set speed to value given
  X_Axis.setSpeed(maxSpeed);
  Y_Axis.setSpeed(maxSpeed);

  // 3 cases to work on : horizontal, vertical and diagonal
  if (xsteps==0 && ysteps != 0){    // Horizontal movement with X0.0
    while (ycount!=ysteps){         // We increase/decrease ycount until it is equal to ysteps, then y destination is reached
      if (ysteps>=0){               // If Y increases so Yx.x > 0
        if (ycount < ysteps){       // '<' so that is doesn't do an overstep
          Y_Axis.step(1);           // Y stepper increases by one
          ycount++;
        }
      }
        else{                       // If Y decreases so Yx.x < 0
          if (ycount > ysteps){     // '>' so that is doesn't do an overstep
            Y_Axis.step(-1);        // Y stepper decreases by one
            ycount--; 
          }
        }
    // Just for testing purpose, these steps are not supposed to be used in the drawing as we are in rapid mode and not line mode
    /*Serial.print(xcount+xprevious);
    Serial.write(9);  
    Serial.println(ycount+yprevious);*/
    }
  } 
      
  if (ysteps==0 & xsteps != 0){     // Vertical movement with Y0.0 
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
  while (xcount != xsteps || ycount != ysteps)  // Needs to reach X AND Y given positions
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

  //Set the new actual position 
  xprevious+=xcount;
  yprevious+=ycount;
  Serial.print("Actual position : X = ");
  Serial.print(xprevious);
  Serial.print("; Y = ");
  Serial.println(yprevious);
  }

void linemove(float x, float y, int feedRate)
{
  // ADD FEEDRATE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  int xsteps = x* 100;
  int ysteps = y* 100;
  int xcount=0, ycount=0;

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
    Serial.print(xcount+xprevious);
    Serial.write(9);  // ...enables us to separate xposition and ypositions using csv format with ',' character and tab as a separator
    Serial.println(ycount+yprevious);
     }
  } 
      
  if (ysteps==0 & xsteps != 0){
      while (xcount!=xsteps){
        if (xsteps>=0){   //we increase xcount until it is equal to xsteps, then x destination is reached
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
  while (xcount != xsteps || ycount != ysteps)  // theses cases are due to incremental way of moving
  {
    if (xsteps>=0){   //we increase xcount until it is equal to xsteps, then x destination is reached
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
}

void arcmove_CCLW(float x, float y, float i, float j)
{
  float deltax, deltay;
  float xpos, ypos;
  float radius;

  deltax = abs(x - i); // general case if x does not corresponds to i 
  deltay = abs(y - j); // pareil!!
  radius = sqrt((deltax * deltax) + (deltay * deltay)); //pythagoras to find the radius
  Serial.print( deltax);
  Serial.print(" ");
  Serial.println(deltay);
  Serial.print(" Rad = : ");
  Serial.println(radius);
  float tangent_slope = atan(y / x);
  float included_angle = tangent_slope * 2;
  Serial.println("Angle");
  Serial.println(included_angle);
   
  if (included_angle<0){
    for (float inc = 0; inc < abs(included_angle); inc += RESOLUTION) {
    ypos = radius * sin(inc);
    xpos = -(radius-(radius * cos(inc))); // radius-radius*cos(inc
    Serial.print(xpos+xprevious);
    Serial.write(9);  
    Serial.println(ypos+yprevious);
    // NEED TO EXPLAIN WHY DO NOT USE LINE MOVE AND WHAT MOVE DOES THE STEPPER HAS TO DO !!!!!!!!!!!!!!!!!
    //linemove(xpos,ypos, init_speed); // use initial speed4
  }
  }
  
  else { 
  for (float inc = 0; inc < included_angle; inc +=RESOLUTION) {
    xpos = radius * sin(inc);
    ypos = radius-(radius * cos(inc)); // radius-radius*cos(inc)
    Serial.print(xpos+xprevious); 
    Serial.write(9);  
    Serial.println(ypos+yprevious);
    //linemove(xpos,ypos, init_speed); // use initial speed4
  }
  }
}
  

void arcmove_CLW(float x, float y, float i, float j)
{
  float deltax, deltay;
  float xpos, ypos;
  float radius;

  deltax = abs(x - i); // general case if x does not corresponds to i 
  deltay = abs(y - j); // pareil!!
  radius = sqrt((deltax * deltax) + (deltay * deltay)); //pythagoras to find the radius
  Serial.print( deltax);
  Serial.print(" ");
  Serial.println(deltay);
  Serial.print(" Rad = : ");
  Serial.println(radius);
  float tangent_slope = atan(y / x);
  float included_angle = tangent_slope * 2;
  Serial.print("Angle: ");
  Serial.println(included_angle);

  if (included_angle>=3.14){  //had to put >= because == wouldn't work
    Serial.println("angle =3.14");
  for (float inc = 0; inc < included_angle; inc += RESOLUTION) {
    xpos = -(radius * sin(inc));
    ypos = j-(radius * cos(inc)); // radius-radius*cos(inc)
        Serial.print(xpos+xprevious);
    Serial.write(9);  
    Serial.println(ypos+yprevious);
    //linemove(xpos,ypos, init_speed); // use initial speed
  }   
  }
  else {
        Serial.println("angle !=3.14");
  for (float inc = 0; inc > included_angle; inc -= RESOLUTION) {
    xpos = radius * sin(inc);
    ypos = j-(radius * cos(inc)); // radius-radius*cos(inc)
    Serial.print(xpos+xprevious);
    Serial.write(9);  
    Serial.println(ypos+yprevious);
    //linemove(xpos,ypos, init_speed); // use initial speed
  }  
  }
}
