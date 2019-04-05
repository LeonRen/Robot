#include <Servo.h>
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(6, 5, NEO_GRB + NEO_KHZ800); //note the 5 is PIN 5
class JointMotor
{
  Servo servo;              //the servo
  int pos;                  //current servo position
  int increment;            //increment to move at each update, can be -,0,or +
  int updateInterval;       //interval between updates
  unsigned long lastUpdate; //last update time

  public:
  JointMotor()
  {
    lastUpdate = millis();
    increment = 0; //on construction, the motor should not move
  }
  
  JointMotor(int interval)
  {
    lastUpdate = millis();
    updateInterval = interval;
    increment = 0; //on construction, the motor should not move
  }



  void reConstruct(int interval)
  {
    lastUpdate = millis();
    updateInterval = interval;
    increment = 0; //on construction, the motor should not move
  }

  void linkServo(int pin)
  {
    servo.attach(pin);
  }

  void unlinkServo()
  {
    servo.detach();
  }

  void setPos(int newPos)
  {
    pos = newPos;
  }

  int getPos()
  {
    return pos;
  }

  void moveNow()
  {
    servo.write(pos);
  }

  void moveNow(int newPos)
  {
    pos = newPos;
    servo.write(pos);
  }

  void setDirection(int dir)
  {
    increment = dir;
  }

  void Update()
  {
    if((millis() - lastUpdate) > updateInterval)  // time to update
    {
      lastUpdate = millis();
      /*
      if ((pos > 180) || (pos < 0)) // end of sweep
      {
        // reverse direction
        increment = -1 * increment;
      }
      */
      if ((pos >= 180) && increment == 1)
      {
        //Do nothing for now
      } else if ((pos <= 0) && increment == -1)
      {
         //Do nothing for now
      } else {
        pos += increment;
        servo.write(pos);
      }
    }
  }

  void Sweep()
  {
    if((millis() - lastUpdate) > updateInterval)  // time to update
    {
      lastUpdate = millis();
      if ((pos > 180) || (pos < 0)) // end of sweep
      {
        // reverse direction
        increment = -1 * increment;
      }
      pos += increment;
      servo.write(pos);
    }
  }

  //For now start is not used
  void sweepTo(int start, int endPoint, int inc)
  {
    if((millis() - lastUpdate) > updateInterval)  // time to update
    {
      increment = inc;
      lastUpdate = millis();
      if (inc > 0)
      {
        if (pos < endPoint) // not end of sweep
        {
          pos += increment;
          servo.write(pos);
        }
      } else if (inc < 0)
      {
        if (pos > endPoint) // not end of sweep
        {
          pos += increment;
          servo.write(pos);
        }
      }
      
    }
  }

  //Sweep to a point
  void sweepTo(int endPoint)
  {
    if((millis() - lastUpdate) > updateInterval)  // time to update
    {
      lastUpdate = millis();
      if (pos < endPoint) {
        increment = 1;
      } else if (pos > endPoint) {
        increment = -1;
      } else {
        increment = 0;
      }
      if (increment > 0)
      {
        if (pos < endPoint) // not end of sweep
        {
          pos += increment;
          servo.write(pos);
        }
      } else if (increment < 0)
      {
        if (pos > endPoint) // not end of sweep
        {
          pos += increment;
          servo.write(pos);
        }
      }
    }
  }
  
};

class Leg
{
  JointMotor m1; //Tibia motor
  JointMotor m2; //Femur motor
  JointMotor m3; //Coxa motor
  int legPos; //1:rF 2:rM 3:rR 4:lF 5:lM 6:lR
  int motion; //-1: backwards    0: don't move     1: forwards
  int state; //0:stop 1:fUpswing 2:fDownswing 3:fBackswing 4:bUpswing 5:bDownswing 6:bBackswing
  int updateInterval;       //interval between updates
  unsigned long lastUpdate; //last update time

  public:
  Leg(int legNum, int interval)
  {
    lastUpdate = millis();
    m1.reConstruct(interval);
    m2.reConstruct(interval);
    m3.reConstruct(interval);
    legPos = legNum;
    updateInterval = interval;
    motion = 0; //on construction, the leg should not move
  }

  void linkJoints(int pin1, int pin2, int pin3)
  {
    m1.linkServo(pin1);
    m2.linkServo(pin2);
    m3.linkServo(pin3);
  }

  void unlinkJoints()
  {
    m1.unlinkServo();
    m2.unlinkServo();
    m3.unlinkServo();
  }

  void setPos(int new1, int new2, int new3)
  {
    m1.setPos(new1);
    m2.setPos(new2);
    m3.setPos(new3);
  }

  void setMotion(int dir)
  {
    motion = dir;
  }

  void moveNow()
  {
    m1.moveNow();
    m2.moveNow();
    m3.moveNow();
  }

  void moveNow(int p1, int p2, int p3)
  {
    m1.moveNow(p1);
    m2.moveNow(p2);
    m3.moveNow(p3);
  }

  void setState(int newState)
  {
    state = newState;
  }

  //"Dispatch Table"
  void moveLeg()
  {
    //if((millis() - lastUpdate) > updateInterval)  // time to update
    //{
    //  lastUpdate = millis();
      switch (state) {
          case 0: //Not moving
            //Do nothing for now
            break;
            
          case 1: //forward upswing
            fUpswing();
            break;
            
          case 2: //forward downswing
            fDownswing();
            break;
            
          case 3: //forward backswing
            fBackswing();
            break;
            
          case 4: //backward upswing
            bUpswing();
            break;
          case 5: //backward downswing
            bDownswing();
            break;
          case 6: //backward backswing
            bBackswing();
            break;
          case 7: //Reset
            reset();
            break;
      }
      updateState();
    //}
  }

  //Move to the next state
  void updateState()
  {
    switch (legPos) {
        case 1: //rF
            switch (state) {
              case 0: //Not moving
                //Do nothing for now
                break;
              case 1: //forward upswing
                if ((m1.getPos() == 125) && (m2.getPos() == 0) && (m3.getPos() == 60)) {
                  state = 2;
                }
                break;
              case 2: //forward downswing
                if ((m1.getPos() == 135) && (m2.getPos() == 10) && (m3.getPos() == 50)) {
                  state = 3;
                }
                break;
              case 3: //forward backswing
                if (m3.getPos() == 70) {
                  state = 1;
                }
                break;
              case 4: //backward upswing
                 if ((m1.getPos() == 125) && (m2.getPos() == 0) && (m3.getPos() == 60)) {
                  state = 5;
                }
                break;
              case 5: //backward downswing
                if ((m1.getPos() == 135) && (m2.getPos() == 10) && (m3.getPos() == 70)) {
                  state = 6;
                }
                break;
              case 6: //backward backswing
                if (m3.getPos() == 50) {
                  state = 4;
                }
                break;
              case 7: //Reset
                if ((m1.getPos() == 135) && (m2.getPos() == 10) && (m3.getPos() == 60)) {
                  state = 0;
                }
                break;
            }
            break;
        case 2: //rM
            switch (state) {
              case 0: //Not moving
                //Do nothing for now
                break;
              case 1: //forward upswing
                if ((m1.getPos() == 95) && (m2.getPos() == 0) && (m3.getPos() == 20)) {
                  state = 2;
                }
                break;
              case 2: //forward downswing
                if ((m1.getPos() == 105) && (m2.getPos() == 10) && (m3.getPos() == 10)) {
                  state = 3;
                }
                break;
              case 3: //forward backswing
                if (m3.getPos() == 30) {
                  state = 1;
                }
                break;
              case 4: //backward upswing
                if ((m1.getPos() == 95) && (m2.getPos() == 0) && (m3.getPos() == 20)) {
                  state = 5;
                }
                break;
              case 5: //backward downswing
                if ((m1.getPos() == 105) && (m2.getPos() == 10) && (m3.getPos() == 30)) {
                  state = 6;
                }
                break;
              case 6: //backward backswing
                if (m3.getPos() == 10) {
                  state = 4;
                }
                break;
              case 7: //Reset
                if ((m1.getPos() == 105) && (m2.getPos() == 10) && (m3.getPos() == 20)) {
                  state = 0;
                }
                break;
            }
            break;
        case 3: //rR
           switch (state) {
              case 0: //Not moving
                //Do nothing for now
                break;
              case 1: //forward upswing
                if ((m1.getPos() == 105) && (m2.getPos() == 0) && (m3.getPos() == 70)) {
                  state = 2;
                }
                break;
              case 2: //forward downswing
                if ((m1.getPos() == 115) && (m2.getPos() == 10) && (m3.getPos() == 60)) {
                  state = 3;
                }
                break;
              case 3: //forward backswing
                if (m3.getPos() == 80) {
                  state = 1;
                }
                break;
              case 4: //backward upswing
                if ((m1.getPos() == 105) && (m2.getPos() == 0) && (m3.getPos() == 70)) {
                  state = 5;
                }
                break;
              case 5: //backward downswing
                if ((m1.getPos() == 115) && (m2.getPos() == 10) && (m3.getPos() == 80)) {
                  state = 6;
                }
                break;
              case 6: //backward backswing
                if (m3.getPos() == 60) {
                  state = 4;
                }
                break;
              case 7: //Reset
                if ((m1.getPos() == 115) && (m2.getPos() == 10) && (m3.getPos() == 70)) {
                  state = 0;
                }
                break;
            }
          break;
        case 4: //lF
          switch (state) {
              case 0: //Not moving
                //Do nothing for now
                break;
              case 1: //forward upswing
                if ((m1.getPos() == 55) && (m2.getPos() == 170) && (m3.getPos() == 105)) {
                  state = 2;
                }
                break;
              case 2: //forward downswing
                if ((m1.getPos() == 45) && (m2.getPos() == 160) && (m3.getPos() == 115)) {
                  state = 3;
                }
                break;
              case 3: //forward backswing
                if (m3.getPos() == 95) {
                  state = 1;
                }
                break;
              case 4: //backward upswing
                if ((m1.getPos() == 55) && (m2.getPos() == 170) && (m3.getPos() == 105)) {
                  state = 5;
                }
                break;
              case 5: //backward downswing
                if ((m1.getPos() == 45) && (m2.getPos() == 160) && (m3.getPos() == 95)) {
                  state = 6;
                }
                break;
              case 6: //backward backswing
                if (m3.getPos() == 115) {
                  state = 4;
                }
                break;
              case 7: //Reset
                if ((m1.getPos() == 45) && (m2.getPos() == 160) && (m3.getPos() == 105)) {
                  state = 0;
                }
                break;
          }
          break;
        case 5: //lM
          switch (state) {
              case 0: //Not moving
                //Do nothing for now
                break;
              case 1: //forward upswing
                if ((m1.getPos() == 50) && (m2.getPos() == 170) && (m3.getPos() == 125)) {
                  state = 2;
                }
                break;
              case 2: //forward downswing
                if ((m1.getPos() == 40) && (m2.getPos() == 160) && (m3.getPos() == 135)) {
                  state = 3;
                }
                break;
              case 3: //forward backswing
                if (m3.getPos() == 115) {
                  state = 1;
                }
                break;
              case 4: //backward upswing
                if ((m1.getPos() == 50) && (m2.getPos() == 170) && (m3.getPos() == 125)) {
                  state = 5;
                }
                break;
              case 5: //backward downswing
                if ((m1.getPos() == 40) && (m2.getPos() == 160) && (m3.getPos() == 115)) {
                  state = 6;
                }
                break;
              case 6: //backward backswing
                if (m3.getPos() == 135) {
                  state = 4;
                }
                break;
              case 7: //Reset
                if ((m1.getPos() == 40) && (m2.getPos() == 160) && (m3.getPos() == 125)) {
                  state = 0;
                }
                break;
          }
          break;
        case 6: //lR
          switch (state) {
              case 0: //Not moving
                //Do nothing for now
                break;
              case 1: //forward upswing
                if ((m1.getPos() == 55) && (m2.getPos() == 170) && (m3.getPos() == 100)) {
                  state = 2;
                }
                break;
              case 2: //forward downswing
                if ((m1.getPos() == 45) && (m2.getPos() == 160) && (m3.getPos() == 110)) {
                  state = 3;
                }
                break;
              case 3: //forward backswing
                if (m3.getPos() == 90) {
                  state = 1;
                }
                break;
              case 4: //backward upswing
                if ((m1.getPos() == 55) && (m2.getPos() == 170) && (m3.getPos() == 100)) {
                  state = 5;
                }
                break;
              case 5: //backward downswing
                if ((m1.getPos() == 45) && (m2.getPos() == 160) && (m3.getPos() == 90)) {
                  state = 6;
                }
                break;
              case 6: //backward backswing
                if (m3.getPos() == 110) {
                  state = 4;
                }
                break;
              case 7: //Reset
                if ((m1.getPos() == 45) && (m2.getPos() == 160) && (m3.getPos() == 100)) {
                  state = 0;
                }
                break;
          }
          break;
    }
  }

  //Dispatch table for forward upswing state based on leg location
  void fUpswing()
  {
    switch (legPos) {
          case 1: //rF
            m1.sweepTo(125);      //m1 45 to 0
            m2.sweepTo(0);      //m2 45 to 0
            m3.sweepTo(60);   //m3 135 to 112
            break;
            
          case 2: //rM
            m1.sweepTo(95);      //m1 45 to 0
            m2.sweepTo(0);      //m2 45 to 0
            m3.sweepTo(20);   //m3 135 to 112
            break;
          case 3: //rR
            m1.sweepTo(105);      //m1 45 to 0
            m2.sweepTo(0);      //m2 45 to 0
            m3.sweepTo(70);   //m3 135 to 112
            break;
          case 4: //lF
            m1.sweepTo(55);    //m1 to 80
            m2.sweepTo(170);    //m2 to 135
            m3.sweepTo(105);   //m3 to 105
            break;
          case 5: //lM
            m1.sweepTo(50);    //m1 to 80
            m2.sweepTo(170);    //m2 to 135
            m3.sweepTo(125);   //m3 to 105
            break;
          case 6: //lR
            m1.sweepTo(55);    //m1 to 80
            m2.sweepTo(170);    //m2 to 135
            m3.sweepTo(100);   //m3 to 105
            break;
    }
  }

  //Dispatch table for forward downswing state based on leg location
  void fDownswing()
  {
    switch (legPos) {
          case 1: //rF
            m1.sweepTo(135);    //m1 to 70
            m2.sweepTo(10);    //m2 to 20
            m3.sweepTo(50);    //m3 to 70
            break;
          case 2: //rM
            m1.sweepTo(105);    //m1 to 70
            m2.sweepTo(10);    //m2 to 20
            m3.sweepTo(10);    //m3 to 20
            break;
          case 3: //rR
            m1.sweepTo(115);    //m1 to 70
            m2.sweepTo(10);    //m2 to 20
            m3.sweepTo(60);    //m3 to 70
            break;
          case 4: //lF
            m1.sweepTo(45);    //m1 to 80
            m2.sweepTo(160);    //m2 to 135
            m3.sweepTo(115);   //m3 to 105
            break;
          case 5: //lM
            m1.sweepTo(40);    //m1 to 80
            m2.sweepTo(160);    //m2 to 135
            m3.sweepTo(135);   //m3 to 105
            break;
          case 6: //lR
            m1.sweepTo(45);    //m1 to 80
            m2.sweepTo(160);    //m2 to 135
            m3.sweepTo(110);   //m3 to 105
            break;
    }
  }

  //Dispatch table for forward backswing state based on leg location
  void fBackswing()
  {
    switch (legPos) {
          case 1: //rF
            m3.sweepTo(70);   //m3 45 to 90
            break;
          case 2: //rM
            m3.sweepTo(30);    //m3 to 20
            break;
          case 3: //rR
            m3.sweepTo(80);   //m3 45 to 90
            break;
          case 4: //lF
            m3.sweepTo(95);   //m3 45 to 90
            break;
          case 5: //lM
            m3.sweepTo(115);   //m3 to 105
            break;
          case 6: //lR
            m3.sweepTo(90);   //m3 to 105
            break;
    }
  }

  //Dispatch table for forward upswing state based on leg location
  void bUpswing()
  {
    switch (legPos) {
          case 1: //rF
            m1.sweepTo(125);      //m1 45 to 0
            m2.sweepTo(0);      //m2 45 to 0
            m3.sweepTo(60);   //m3 135 to 112
            break;
            
          case 2: //rM
            m1.sweepTo(95);      //m1 45 to 0
            m2.sweepTo(0);      //m2 45 to 0
            m3.sweepTo(20);   //m3 135 to 112
            break;
          case 3: //rR
            m1.sweepTo(105);      //m1 45 to 0
            m2.sweepTo(0);      //m2 45 to 0
            m3.sweepTo(70);   //m3 135 to 112
            break;
          case 4: //lF
            m1.sweepTo(55);    //m1 to 80
            m2.sweepTo(170);    //m2 to 135
            m3.sweepTo(105);   //m3 to 105
            break;
          case 5: //lM
            m1.sweepTo(50);    //m1 to 80
            m2.sweepTo(170);    //m2 to 135
            m3.sweepTo(125);   //m3 to 105
            break;
          case 6: //lR
            m1.sweepTo(55);    //m1 to 80
            m2.sweepTo(170);    //m2 to 135
            m3.sweepTo(100);   //m3 to 105
            break;
    }
  }

  //Dispatch table for forward downswing state based on leg location
  void bDownswing()
  {
    switch (legPos) {
          case 1: //rF
            m1.sweepTo(135);    //m1 to 70
            m2.sweepTo(10);    //m2 to 20
            m3.sweepTo(70);    //m3 to 70
            break;
          case 2: //rM
            m1.sweepTo(105);    //m1 to 70
            m2.sweepTo(10);    //m2 to 20
            m3.sweepTo(30);    //m3 to 20
            break;
          case 3: //rR
            m1.sweepTo(115);    //m1 to 70
            m2.sweepTo(10);    //m2 to 20
            m3.sweepTo(80);    //m3 to 70
            break;
          case 4: //lF
            m1.sweepTo(45);    //m1 to 80
            m2.sweepTo(160);    //m2 to 135
            m3.sweepTo(95);   //m3 to 105
            break;
          case 5: //lM
            m1.sweepTo(40);    //m1 to 80
            m2.sweepTo(160);    //m2 to 135
            m3.sweepTo(115);   //m3 to 105
            break;
          case 6: //lR
            m1.sweepTo(45);    //m1 to 80
            m2.sweepTo(160);    //m2 to 135
            m3.sweepTo(90);   //m3 to 105
            break;
    }
  }

  //Dispatch table for forward backswing state based on leg location
  void bBackswing()
  {
    switch (legPos) {
          case 1: //rF
            m3.sweepTo(50);   //m3 45 to 90
            break;
          case 2: //rM
            m3.sweepTo(10);    //m3 to 20
            break;
          case 3: //rR
            m3.sweepTo(60);   //m3 45 to 90
            break;
          case 4: //lF
            m3.sweepTo(115);   //m3 45 to 90
            break;
          case 5: //lM
            m3.sweepTo(135);   //m3 to 105
            break;
          case 6: //lR
            m3.sweepTo(110);   //m3 to 105
            break;
    }
  }

  //Dispatch table for reset state based on leg location
  void reset()
  {
    switch (legPos) {
          case 1: //rF
            m1.sweepTo(135);    //m1 to 70
            m2.sweepTo(10);    //m2 to 20
            m3.sweepTo(60);   //m3 to 60
            break;
          case 2: //rM
            m1.sweepTo(105);    //m1 to 70
            m2.sweepTo(10);    //m2 to 20
            m3.sweepTo(20);    //m3 to 20
            break;
          case 3: //rR
            m1.sweepTo(115);    //m1 to 70
            m2.sweepTo(10);    //m2 to 20
            m3.sweepTo(70);    //m3 to 70
            break;
          case 4: //lF
            m1.sweepTo(45);    //m1 to 80
            m2.sweepTo(160);    //m2 to 135
            m3.sweepTo(105);   //m3 to 105
            break;
          case 5: //lM
            m1.sweepTo(40);    //m1 to 80
            m2.sweepTo(160);    //m2 to 135
            m3.sweepTo(125);   //m3 to 105
            break;
          case 6: //lR
            m1.sweepTo(45);    //m1 to 80
            m2.sweepTo(160);    //m2 to 135
            m3.sweepTo(100);   //m3 to 105
            break;
    }
  }

};

/*GLOBALS*/
Leg l1(1, 30);
Leg l2(2, 30);
Leg l3(3, 30);
Leg l4(4, 30);
Leg l5(5, 30);
Leg l6(6, 30);
int incomingByte; // a variable to read incoming serial data into
int inputState; //Input state;  0:M  1:H 2:L 3:U
int i; //Used for debugging

void setup() {
  // initialize serial communication:
  Serial.begin(9600);
  delay(1000);
  
  //LED Initializing
  strip.begin();
  strip.show();//sets all pixels to off
  for(int i=0;i<strip.numPixels();i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    strip.setPixelColor(i, strip.Color(255,0,255)); // purple
    strip.show(); // This sends the updated pixel color to the hardware.
    delay(240); // Delay for a period of time (in milliseconds).

  }
  


  
  l1.linkJoints(41,43,45);
  l1.moveNow(135, 10, 60);
  l1.setState(7); //Reset
  
  l2.linkJoints(35,37,39);
  l2.moveNow(105, 10, 20); //m3 = 20
  l2.setState(7); //Reset

  l3.linkJoints(34,36,38);
  l3.moveNow(115, 10, 70);
  l3.setState(7); //Reset

  l4.linkJoints(28,30,32);
  l4.moveNow(45, 160, 105);
  l4.setState(7); //Reset
  
  l5.linkJoints(29,31,33);
  l5.moveNow(40, 160, 125); //m3 = 125
  l5.setState(7); //Reset

  l6.linkJoints(40,42,44);
  l6.moveNow(45, 160, 100);
  l6.setState(7); //Reset
  
  inputState = 0;
  i = 0;
  
}

void colorWipe(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
  }
}

void loop() {

  //Get user input to change states
  if (Serial.available() > 0) {
    // read the oldest byte in the serial buffer:
    incomingByte = Serial.read();
    
    // if it's an H sweep forward:
    if (incomingByte == 'H' && inputState != 1) {
      Serial.println("MOVING FORWARD");
      colorWipe(strip.Color(0, 255, 0)); // Green
      inputState = 1;
      l1.setState(1);
      l2.setState(3);
      l3.setState(1);
      l4.setState(3);
      l5.setState(1);
      l6.setState(3);
    } 
    
    // if it's an L sweep back:
    if (incomingByte == 'L' && inputState != 2) {
      Serial.println("MOVING BACKWARD");
      colorWipe(strip.Color(255, 200,0)); // Yellow
      inputState = 2;
      l1.setState(4);
      l2.setState(6);
      l3.setState(4);
      l4.setState(6);
      l5.setState(4);
      l6.setState(6);
    }
    
    // if it's an M stop moving:
    if (incomingByte == 'M' && inputState != 0) {
      Serial.println("Stopping");
      colorWipe(strip.Color(255, 0,0)); // Red
      inputState = 0;
      l1.setState(0);
      l2.setState(0);
      l3.setState(0);
      l4.setState(0);
      l5.setState(0);
      l6.setState(0);
    }

    // if it's a U reset:
    if (incomingByte == 'U' && inputState != 3) {
      Serial.println("Reseting");
      colorWipe(strip.Color(255, 0,255)); // Purple
      inputState = 3;
      l1.setState(7);
      l2.setState(7);
      l3.setState(7);
      l4.setState(7);
      l5.setState(7);
      l6.setState(7);
    }
  }
  l1.moveLeg();
  l2.moveLeg();
  l3.moveLeg();
  l4.moveLeg();
  l5.moveLeg();
  l6.moveLeg();
}
