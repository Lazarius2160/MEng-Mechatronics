//conversion factor 200 steps per rev
// = 200 steps per 2mm
// 100 steps per mm
// multiply mm by 100 to get steps
// then converto from float to int.

//--------------------PRE PROCESSOR INSTRUCTIONS-------------------
#include<Stepper.h>
#include<math.h>

#define RESOLUTION 0.001
#define LINE 1
#define ARC_CLW 2
#define ARC_CCLW 3
#define RAPID 0

//--------------------GLOBAL VARIABLES-----------------------------
int movemode;
const int stepsPerRevolution = 200;
float xpos, ypos; // actual position
int xsteps, ysteps; // steps to do to reach wanted position
int xprevious = 0, yprevious = 0; //previous position 
int speed, init_speed =30; // in rpm
int feedRate; // mm per minute,speed at which the pen will write in line mode
float ival, jval; // arc centres

//--------------------CREATION OF STEPPERS------------------------
Stepper X_Axis(stepsPerRevolution, 2, 3, 5, 6); //Create an instance of Stepper for axis X
Stepper Y_Axis(stepsPerRevolution, 7, 8, 9, 10); //Create an instance of Stepper for axis Y

//--------------------SETUP---------------------------------------
void setup() {
  Serial.begin(9600);
  X_Axis.setSpeed(init_speed);
  Y_Axis.setSpeed(init_speed);
  Serial.println("Enter G-code displacement and speed "); // Tell user to input values from keyboard 
}

//--------------------MAIN LOOP------------------------------------
void loop() {
  if (Serial.available()) {  // something has been typed
    char ch = Serial.read();  // read the input character
    // ALWAYS WRITE IN FLOAT NOT IN INT BECAUSE IT WILL DO PARSEFLOAT SO THE NUMBER WILL CHANGE (10 instead of 10.0 will give 88.0)
    switch (ch) {

      case'G': movemode = Serial.parseInt();
        Serial.print("Move mode:");
        Serial.println(movemode);
        break;

      case 'X':
      case 'x': xpos = Serial.parseFloat();  // if it is X then expect a float to follow
        Serial.print("X move:");
        Serial.println(xpos);
        break; // Values are stored.. Do nothing more for now.

      case 'Y':
      case 'y': ypos = Serial.parseFloat();  // if it is X then expect a float to follow
        Serial.print("Y move:");
        Serial.println(ypos);
        break; // Values are stored.. Do nothing more for now.

      case 'S': //speed in g code is written as F
      case 's': speed = Serial.parseInt();   // get value for speed and store. Value is for the next move so do nothing else right now.
        break;

      case 'I':
      case 'i': ival = Serial.parseFloat();  // if it is X then expect a float to follow
        break; // Values are stored.. Do nothing more for now. case 'X':

      case 'J': 
      case 'j': jval    = Serial.parseFloat();  // if it is X then expect a float to follow
        break; // Values are stored.. Do nothing more for now.

      case  '#':
      case '\n':   // return key has been hit. Output a move if the X value has changed since the last CR. 
        chosemode(movemode);
        break;

      default:
      if (ch != ' '){
        Serial.print("Not recognised:"); // A non-valid input so ignore.
        Serial.print(ch);
      }
        break;
    }
  }
}

//--------------------DIFFERENT WAYS OF MOVING - functions-------------------
void chosemode (int mode){
   if (mode ==RAPID) {
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
  //!!!!!!!!!! pas sure que doivent garder en mémoire les steps ici mais juste faire le mouvement du step (a voir sur tracé excel le rendu)
  int xsteps = x* 100;
  int ysteps = y* 100;
  int xcount=0, ycount=0;
  // Set speed to value given
  X_Axis.setSpeed(maxSpeed);
  Y_Axis.setSpeed(maxSpeed);

  
  if (xsteps==0 && ysteps != 0){
    while (xcount!=xsteps){
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
    }
  }
  // Set speed to initial value 
  X_Axis.setSpeed(init_speed);
  Y_Axis.setSpeed(init_speed);

  //Set the new actual position 
  xprevious+=xcount;
  yprevious+=ycount;
  Serial.print(xcount+xprevious);
  Serial.println(ycount+yprevious);
  }

void linemove(float x, float y, int feedRate)
{
  // ADD FEEDRATE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  int xsteps = x* 100;
  int ysteps = y* 100;
  int xcount=0, ycount=0;

  if (xsteps==0 && ysteps != 0){
    while (xcount!=xsteps){
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
    Serial.print(','); // These two lines...
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
    Serial.print(','); 
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
    Serial.print(','); 
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
    linemove(xpos,ypos, init_speed); // use initial speed4
  }
  }
  
  else { 
  for (float inc = 0; inc < included_angle; inc +=RESOLUTION) {
    xpos = radius * sin(inc);
    ypos = radius-(radius * cos(inc)); // radius-radius*cos(inc)
    linemove(xpos,ypos, init_speed); // use initial speed4
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
    linemove(xpos,ypos, init_speed); // use initial speed
  }   
  }
  else {
        Serial.println("angle !=3.14");
  for (float inc = 0; inc > included_angle; inc -= RESOLUTION) {
    xpos = radius * sin(inc);
    ypos = j-(radius * cos(inc)); // radius-radius*cos(inc)
    linemove(xpos,ypos, init_speed); // use initial speed
  }  
  }
}
