#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include <OneWire.h>
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

  int readServo()
  {
    return servo.read();
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

  void setInterval(int interval)
  {
    updateInterval = interval;
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
  int state; //0:stop 1:fUpswing 2:fDownswing 3:fBackswing 4:bUpswing 5:bDownswing 6:bBackswing
  int updateInterval;       //interval between updates
  unsigned long lastUpdate; //last update time
  int mdsDone; //a flag that the metachronal downswing finished
  int rotaionDir; //Determines clockwise vs counter-clockwise for metachronal rotaion (1= clockwise   -1=counter-clockwise)

  public:
  Leg()
  {
    lastUpdate = millis();
    updateInterval = 15;
  }
  
  Leg(int legNum, int interval)
  {
    lastUpdate = millis();
    m1.reConstruct(interval);
    m2.reConstruct(interval);
    m3.reConstruct(interval);
    legPos = legNum;
    updateInterval = interval;
    mdsDone = 0;
  }

  void reConstruct(int legNum, int interval)
  {
    lastUpdate = millis();
    m1.reConstruct(interval);
    m2.reConstruct(interval);
    m3.reConstruct(interval);
    legPos = legNum;
    updateInterval = interval;
    mdsDone = 0;
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

  void setInterval(int interval)
  {
    updateInterval = interval;
    m1.setInterval(interval);
    m2.setInterval(interval);
    m3.setInterval(interval);
  }

  void setJointInterval(int joint, int interval)
  {
    switch (joint) {
      case 1:
        m1.setInterval(interval);
        break;
      case 2:
        m2.setInterval(interval);
        break;
      case 3:
        m3.setInterval(interval);
        break;
    }
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

  int checkMDS()
  {
    return mdsDone;
  }

  void clearMDS()
  {
    mdsDone = 0;
  }

  void setRotationDirection(int dir)
  {
    rotaionDir = dir; //1 = clockwise   -1= counter-clockwise
  }
  
  //"Dispatch Table"
  void moveLeg()
  {
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
          resetLeg();
          break;
        case 8: //Metachronal upswing
          mdsDone = 0;
          mus();
          break;
        case 9: //Metachronal downswing
          mds();
          break;
        case 10: //Metachronal backswing
          mbs();
          break;
        case 11: //Metachronal upswing
          mdsDone = 0;
          rmus();
          break;
        case 12: //Metachronal downswing
          rmds();
          break;
        case 13: //Metachronal backswing
          rmbs();
          break;
    }
    updateState();
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
                if ((m1.getPos() == 110) && (m2.getPos() == 10) && (m3.getPos() == 60)) {
                  state = 2;
                }
                break;
              case 2: //forward downswing
                if ((m1.getPos() == 120) && (m2.getPos() == 20) && (m3.getPos() == 50)) {
                  state = 3;
                }
                break;
              case 3: //forward backswing
                if (m3.getPos() == 70) {
                  state = 1;
                }
                break;
              case 4: //backward upswing
                 if ((m1.getPos() == 110) && (m2.getPos() == 10) && (m3.getPos() == 60)) {
                  state = 5;
                }
                break;
              case 5: //backward downswing
                if ((m1.getPos() == 120) && (m2.getPos() == 20) && (m3.getPos() == 70)) {
                  state = 6;
                }
                break;
              case 6: //backward backswing
                if (m3.getPos() == 50) {
                  state = 4;
                }
                break;
              case 7: //Reset
                if ((m1.getPos() == 120) && (m2.getPos() == 20) && (m3.getPos() == 60)) {
                  state = 0;
                }
                break;
              case 8: //Metachronal upswing
                if ((m1.readServo() == 110) && (m2.readServo() == 5) && (m3.readServo() == 60)) {
                  state = 9;
                }
                break;
              case 9: //Metachronal downswing
                if ((m1.readServo() == 120) && (m2.readServo() == 20) && (m3.readServo() == 50)) {
                  state = 10;
                  mdsDone = 1; //finished the downsiwng
                }
                break;
              case 10: //Metachronal backswing
                if (m3.readServo() == 75) {
                  state = 0;
                }
                break;
              case 11: //Rotation Metachronal upswing
                if ((m1.readServo() == 110) && (m2.readServo() == 5) && (m3.readServo() == 60)) {
                  state = 12;
                }
                break;
              case 12: //Rotation Metachronal downswing
                if ((m1.readServo() == 120) && (m2.readServo() == 20) && (m3.readServo() == (60 + (rotaionDir * 10)))) {
                  state = 13;
                  mdsDone = 1; //finished the downsiwng
                }
                break;
              case 13: //Rotation Metachronal backswing
                if (m3.readServo() == (60 - (rotaionDir * 15))) {
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
                if ((m1.getPos() == 110) && (m2.getPos() == 10) && (m3.getPos() == 20)) {
                  state = 2;
                }
                break;
              case 2: //forward downswing
                if ((m1.getPos() == 120) && (m2.getPos() == 20) && (m3.getPos() == 10)) {
                  state = 3;
                }
                break;
              case 3: //forward backswing
                if (m3.getPos() == 30) {
                  state = 1;
                }
                break;
              case 4: //backward upswing
                if ((m1.getPos() == 110) && (m2.getPos() == 10) && (m3.getPos() == 20)) {
                  state = 5;
                }
                break;
              case 5: //backward downswing
                if ((m1.getPos() == 120) && (m2.getPos() == 20) && (m3.getPos() == 30)) {
                  state = 6;
                }
                break;
              case 6: //backward backswing
                if (m3.getPos() == 10) {
                  state = 4;
                }
                break;
              case 7: //Reset
                if ((m1.getPos() == 120) && (m2.getPos() == 20) && (m3.getPos() == 20)) {
                  state = 0;
                }
                break;
              case 8: //Metachronal upswing
                if ((m1.readServo() == 110) && (m2.readServo() == 5) && (m3.readServo() == 20)) {
                  state = 9;
                }
                break;
              case 9: //Metachronal downswing
                if ((m1.readServo() == 120) && (m2.readServo() == 20) && (m3.readServo() == 10)) {
                  state = 10;
                  mdsDone = 1; //finished the downsiwng
                }
                break;
              case 10: //Metachronal backswing
                if (m3.readServo() == 35) {
                  state = 0;
                }
                break;
              case 11: //Rotation Metachronal upswing
                if ((m1.readServo() == 110) && (m2.readServo() == 5) && (m3.readServo() == 20)) {
                  state = 12;
                }
                break;
              case 12: //Rotation Metachronal downswing
                if ((m1.readServo() == 120) && (m2.readServo() == 20) && (m3.readServo() == (20 + (rotaionDir * 10)))) {
                  state = 13;
                  mdsDone = 1; //finished the downsiwng
                }
                break;
              case 13: //Rotation Metachronal backswing
                if (m3.readServo() == (20 - (rotaionDir * 15))) {
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
                if ((m1.getPos() == 110) && (m2.getPos() == 10) && (m3.getPos() == 70)) {
                  state = 2;
                }
                break;
              case 2: //forward downswing
                if ((m1.getPos() == 120) && (m2.getPos() == 20) && (m3.getPos() == 60)) {
                  state = 3;
                }
                break;
              case 3: //forward backswing
                if (m3.getPos() == 80) {
                  state = 1;
                }
                break;
              case 4: //backward upswing
                if ((m1.getPos() == 110) && (m2.getPos() == 10) && (m3.getPos() == 70)) {
                  state = 5;
                }
                break;
              case 5: //backward downswing
                if ((m1.getPos() == 120) && (m2.getPos() == 20) && (m3.getPos() == 80)) {
                  state = 6;
                }
                break;
              case 6: //backward backswing
                if (m3.getPos() == 60) {
                  state = 4;
                }
                break;
              case 7: //Reset
                if ((m1.getPos() == 120) && (m2.getPos() == 20) && (m3.getPos() == 70)) {
                  state = 0;
                }
                break;
              case 8: //Metachronal upswing
                if ((m1.readServo() == 110) && (m2.readServo() == 5) && (m3.readServo() == 70)) {
                  state = 9;
                }
                break;
              case 9: //Metachronal downswing
                if ((m1.readServo() == 120) && (m2.readServo() == 20) && (m3.readServo() == 60)) {
                  state = 10;
                  mdsDone = 1; //finished the downsiwng
                }
                break;
              case 10: //Metachronal backswing
                if (m3.readServo() == 85) {
                  state = 0;
                }
                break;
              case 11: //Rotation Metachronal upswing
                if ((m1.readServo() == 110) && (m2.readServo() == 5) && (m3.readServo() == 70)) {
                  state = 12;
                }
                break;
              case 12: //Rotation Metachronal downswing
                if ((m1.readServo() == 120) && (m2.readServo() == 20) && (m3.readServo() == (70 + (rotaionDir * 10)))) {
                  state = 13;
                  mdsDone = 1; //finished the downsiwng
                }
                break;
              case 13: //Rotation Metachronal backswing
                if (m3.readServo() == (70 - (rotaionDir * 15))) {
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
                if ((m1.getPos() == 70) && (m2.getPos() == 170) && (m3.getPos() == 105)) {
                  state = 2;
                }
                break;
              case 2: //forward downswing
                if ((m1.getPos() == 60) && (m2.getPos() == 160) && (m3.getPos() == 115)) {
                  state = 3;
                }
                break;
              case 3: //forward backswing
                if (m3.getPos() == 95) {
                  state = 1;
                }
                break;
              case 4: //backward upswing
                if ((m1.getPos() == 70) && (m2.getPos() == 170) && (m3.getPos() == 105)) {
                  state = 5;
                }
                break;
              case 5: //backward downswing
                if ((m1.getPos() == 60) && (m2.getPos() == 160) && (m3.getPos() == 95)) {
                  state = 6;
                }
                break;
              case 6: //backward backswing
                if (m3.getPos() == 115) {
                  state = 4;
                }
                break;
              case 7: //Reset
                if ((m1.getPos() == 60) && (m2.getPos() == 160) && (m3.getPos() == 105)) {
                  state = 0;
                }
                break;
              case 8: //Metachronal upswing
                if ((m1.readServo() == 70) && (m2.readServo() == 175) && (m3.readServo() == 105)) {
                  state = 9;
                }
                break;
              case 9: //Metachronal downswing
                if ((m1.readServo() == 60) && (m2.readServo() == 160) && (m3.readServo() == 115)) {
                  state = 10;
                  mdsDone = 1; //finished the downsiwng
                }
                break;
              case 10: //Metachronal backswing
                if (m3.readServo() == 90) {
                  state = 0;
                }
                break;
              case 11: //Rotation Metachronal upswing
                if ((m1.readServo() == 70) && (m2.readServo() == 175) && (m3.readServo() == 105)) {
                  state = 12;
                }
                break;
              case 12: //Rotation Metachronal downswing
                if ((m1.readServo() == 60) && (m2.readServo() == 160) && (m3.readServo() == (105 + (rotaionDir * 10)))) {
                  state = 13;
                  mdsDone = 1; //finished the downsiwng
                }
                break;
              case 13: //Rotation Metachronal backswing
                if (m3.readServo() == (105 - (rotaionDir * 15))) {
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
                if ((m1.getPos() == 70) && (m2.getPos() == 170) && (m3.getPos() == 125)) {
                  state = 2;
                }
                break;
              case 2: //forward downswing
                if ((m1.getPos() == 60) && (m2.getPos() == 160) && (m3.getPos() == 135)) {
                  state = 3;
                }
                break;
              case 3: //forward backswing
                if (m3.getPos() == 115) {
                  state = 1;
                }
                break;
              case 4: //backward upswing
                if ((m1.getPos() == 70) && (m2.getPos() == 170) && (m3.getPos() == 125)) {
                  state = 5;
                }
                break;
              case 5: //backward downswing
                if ((m1.getPos() == 60) && (m2.getPos() == 160) && (m3.getPos() == 115)) {
                  state = 6;
                }
                break;
              case 6: //backward backswing
                if (m3.getPos() == 135) {
                  state = 4;
                }
                break;
              case 7: //Reset
                if ((m1.getPos() == 60) && (m2.getPos() == 160) && (m3.getPos() == 125)) {
                  state = 0;
                }
                break;
              case 8: //Metachronal upswing
                if ((m1.readServo() == 70) && (m2.readServo() == 175) && (m3.readServo() == 125)) {
                  state = 9;
                }
                break;
              case 9: //Metachronal downswing
                if ((m1.readServo() == 60) && (m2.readServo() == 160) && (m3.readServo() == 135)) {
                  state = 10;
                  mdsDone = 1; //finished the downsiwng
                }
                break;
              case 10: //Metachronal backswing
                if (m3.readServo() == 110) {
                  state = 0;
                }
                break;
              case 11: //Rotation Metachronal upswing
                if ((m1.readServo() == 70) && (m2.readServo() == 175) && (m3.readServo() == 125)) {
                  state = 12;
                }
                break;
              case 12: //Rotation Metachronal downswing
                if ((m1.readServo() == 60) && (m2.readServo() == 160) && (m3.readServo() == (125 + (rotaionDir * 10)))) {
                  state = 13;
                  mdsDone = 1; //finished the downsiwng
                }
                break;
              case 13: //Rotation Metachronal backswing
                if (m3.readServo() == (125 - (rotaionDir * 15))) {
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
                if ((m1.getPos() == 70) && (m2.getPos() == 170) && (m3.getPos() == 100)) {
                  state = 2;
                }
                break;
              case 2: //forward downswing
                if ((m1.getPos() == 60) && (m2.getPos() == 160) && (m3.getPos() == 110)) {
                  state = 3;
                }
                break;
              case 3: //forward backswing
                if (m3.getPos() == 90) {
                  state = 1;
                }
                break;
              case 4: //backward upswing
                if ((m1.getPos() == 70) && (m2.getPos() == 170) && (m3.getPos() == 100)) {
                  state = 5;
                }
                break;
              case 5: //backward downswing
                if ((m1.getPos() == 60) && (m2.getPos() == 160) && (m3.getPos() == 90)) {
                  state = 6;
                }
                break;
              case 6: //backward backswing
                if (m3.getPos() == 110) {
                  state = 4;
                }
                break;
              case 7: //Reset
                if ((m1.getPos() == 60) && (m2.getPos() == 160) && (m3.getPos() == 100)) {
                  state = 0;
                }
                break;
              case 8: //Metachronal upswing
                if ((m1.readServo() == 70) && (m2.readServo() == 175) && (m3.readServo() == 100)) {
                  state = 9;
                }
                break;
              case 9: //Metachronal downswing
                if ((m1.readServo() == 60) && (m2.readServo() == 160) && (m3.readServo() == 110)) {
                  state = 10;
                  mdsDone = 1; //finished the downsiwng
                }
                break;
              case 10: //Metachronal backswing
                if (m3.readServo() == 85) {
                  state = 0;
                }
                break;
              case 11: //Rotation Metachronal upswing
                if ((m1.readServo() == 70) && (m2.readServo() == 175) && (m3.readServo() == 100)) {
                  state = 12;
                }
                break;
              case 12: //Rotation Metachronal downswing
                if ((m1.readServo() == 60) && (m2.readServo() == 160) && (m3.readServo() == (100 + (rotaionDir * 10)))) {
                  state = 13;
                  mdsDone = 1; //finished the downsiwng
                }
                break;
              case 13: //Rotation Metachronal backswing
                if (m3.readServo() == (100 - (rotaionDir * 15))) {
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
            m1.sweepTo(110);      
            m2.sweepTo(10);     
            m3.sweepTo(60);   
            break;
          case 2: //rM
            m1.sweepTo(110);     
            m2.sweepTo(10);      
            m3.sweepTo(20);   
            break;
          case 3: //rR
            m1.sweepTo(110);      
            m2.sweepTo(10);     
            m3.sweepTo(70);   
            break;
          case 4: //lF
            m1.sweepTo(70);    
            m2.sweepTo(170);    
            m3.sweepTo(105);   
            break;
          case 5: //lM
            m1.sweepTo(70);    
            m2.sweepTo(170);    
            m3.sweepTo(125);   
            break;
          case 6: //lR
            m1.sweepTo(70);   
            m2.sweepTo(170);    
            m3.sweepTo(100);  
            break;
    }
  }

  //Dispatch table for forward downswing state based on leg location
  void fDownswing()
  {
    switch (legPos) {
          case 1: //rF
            m1.sweepTo(120);   
            m2.sweepTo(20);    
            m3.sweepTo(50);    
            break;
          case 2: //rM
            m1.sweepTo(120);    
            m2.sweepTo(20);    
            m3.sweepTo(10);    
            break;
          case 3: //rR
            m1.sweepTo(120);    
            m2.sweepTo(20);   
            m3.sweepTo(60);    
            break;
          case 4: //lF
            m1.sweepTo(60);   
            m2.sweepTo(160);   
            m3.sweepTo(115);   
            break;
          case 5: //lM
            m1.sweepTo(60);    
            m2.sweepTo(160);   
            m3.sweepTo(135);   
            break;
          case 6: //lR
            m1.sweepTo(60);    
            m2.sweepTo(160);    
            m3.sweepTo(110);   
            break;
    }
  }

  //Dispatch table for forward backswing state based on leg location
  void fBackswing()
  {
    switch (legPos) {
          case 1: //rF
            m3.sweepTo(70);   
            break;
          case 2: //rM
            m3.sweepTo(30);   
            break;
          case 3: //rR
            m3.sweepTo(80);  
            break;
          case 4: //lF
            m3.sweepTo(95);   
            break;
          case 5: //lM
            m3.sweepTo(115);  
            break;
          case 6: //lR
            m3.sweepTo(90);   
            break;
    }
  }

  //Dispatch table for forward upswing state based on leg location
  void bUpswing()
  {
    switch (legPos) {
          case 1: //rF
            m1.sweepTo(110);      
            m2.sweepTo(10);      
            m3.sweepTo(60);   
            break;
            
          case 2: //rM
            m1.sweepTo(110);      
            m2.sweepTo(10);      
            m3.sweepTo(20);   
            break;
          case 3: //rR
            m1.sweepTo(110);     
            m2.sweepTo(10);      
            m3.sweepTo(70);   
            break;
          case 4: //lF
            m1.sweepTo(70);    
            m2.sweepTo(170);    
            m3.sweepTo(105);   
            break;
          case 5: //lM
            m1.sweepTo(70);    
            m2.sweepTo(170);    
            m3.sweepTo(125);   
            break;
          case 6: //lR
            m1.sweepTo(70);    
            m2.sweepTo(170);    
            m3.sweepTo(100);   
            break;
    }
  }

  //Dispatch table for forward downswing state based on leg location
  void bDownswing()
  {
    switch (legPos) {
          case 1: //rF
            m1.sweepTo(120);    
            m2.sweepTo(20);    
            m3.sweepTo(70);    
            break;
          case 2: //rM
            m1.sweepTo(120);  
            m2.sweepTo(20);   
            m3.sweepTo(30);    
            break;
          case 3: //rR
            m1.sweepTo(120);    
            m2.sweepTo(20);    
            m3.sweepTo(80);    
            break;
          case 4: //lF
            m1.sweepTo(60);    
            m2.sweepTo(160);    
            m3.sweepTo(95);   
            break;
          case 5: //lM
            m1.sweepTo(60);    
            m2.sweepTo(160);   
            m3.sweepTo(115);   
            break;
          case 6: //lR
            m1.sweepTo(60);    
            m2.sweepTo(160);    
            m3.sweepTo(90);  
            break;
    }
  }

  //Dispatch table for forward backswing state based on leg location
  void bBackswing()
  {
    switch (legPos) {
          case 1: //rF
            m3.sweepTo(50);   
            break;
          case 2: //rM
            m3.sweepTo(10);    
            break;
          case 3: //rR
            m3.sweepTo(60);   
            break;
          case 4: //lF
            m3.sweepTo(115);  
            break;
          case 5: //lM
            m3.sweepTo(135);   
            break;
          case 6: //lR
            m3.sweepTo(110);   
            break;
    }
  }

  //Dispatch table for metachronal upswing state based on leg location
  void mus()
  {
    switch (legPos) {
          case 1: //rF
            m1.sweepTo(110);    
            m2.sweepTo(5);      
            m3.sweepTo(60);   
            break;
          case 2: //rM
            m1.sweepTo(110);     
            m2.sweepTo(5);      
            m3.sweepTo(20);   
            break;
          case 3: //rR
            m1.sweepTo(110);      
            m2.sweepTo(5);     
            m3.sweepTo(70);   
            break;
          case 4: //lF
            m1.sweepTo(70);    
            m2.sweepTo(175);    
            m3.sweepTo(105);   
            break;
          case 5: //lM
            m1.sweepTo(70);    
            m2.sweepTo(175);    
            m3.sweepTo(125);   
            break;
          case 6: //lR
            m1.sweepTo(70);   
            m2.sweepTo(175);    
            m3.sweepTo(100);  
            break;
    }
  }

  //Dispatch table for metachronal downswing state based on leg location
  //the metachronal downswing sets a done flag for when it finished
  void mds()
  {
    switch (legPos) {
          case 1: //rF
            m1.sweepTo(120);   
            m2.sweepTo(20);      
            m3.sweepTo(50);
          case 2: //rM
            m1.sweepTo(120);    
            m2.sweepTo(20);    
            m3.sweepTo(10);    
            break;
          case 3: //rR
            m1.sweepTo(120);    
            m2.sweepTo(20);   
            m3.sweepTo(60);    
            break;
          case 4: //lF
            m1.sweepTo(60);   
            m2.sweepTo(160);   
            m3.sweepTo(115);   
            break;
          case 5: //lM
            m1.sweepTo(60);    
            m2.sweepTo(160);   
            m3.sweepTo(135);   
            break;
          case 6: //lR
            m1.sweepTo(60);    
            m2.sweepTo(160);    
            m3.sweepTo(110);   
            break;
    }
  }

  //Dispatch table for metachronal backswing state based on leg location
  void mbs()
  {
    switch (legPos) {
          case 1: //rF
            m3.sweepTo(75);
            break;
          case 2: //rM
            m3.sweepTo(35);   
            break;
          case 3: //rR
            m3.sweepTo(85);  
            break;
          case 4: //lF
            m3.sweepTo(90);   
            break;
          case 5: //lM
            m3.sweepTo(110);  
            break;
          case 6: //lR
            m3.sweepTo(85);   
            break;
    }
  }

  //Dispatch table for rotation metachronal upswing state based on leg location
  void rmus()
  {
    switch (legPos) {
          case 1: //rF
            m1.sweepTo(110);    
            m2.sweepTo(5);      
            m3.sweepTo(60);   
            break;
          case 2: //rM
            m1.sweepTo(110);     
            m2.sweepTo(5);      
            m3.sweepTo(20);   
            break;
          case 3: //rR
            m1.sweepTo(110);      
            m2.sweepTo(5);     
            m3.sweepTo(70);   
            break;
          case 4: //lF
            m1.sweepTo(70);    
            m2.sweepTo(175);    
            m3.sweepTo(105);   
            break;
          case 5: //lM
            m1.sweepTo(70);    
            m2.sweepTo(175);    
            m3.sweepTo(125);   
            break;
          case 6: //lR
            m1.sweepTo(70);   
            m2.sweepTo(175);    
            m3.sweepTo(100);  
            break;
    }
  }

  //Dispatch table for rotation metachronal downswing state based on leg location
  void rmds()
  {
    switch (legPos) {
          case 1: //rF
            m1.sweepTo(120);    
            m2.sweepTo(20);    
            m3.sweepTo(60 + (rotaionDir * 10));   
            break;
          case 2: //rM
            m1.sweepTo(120);    
            m2.sweepTo(20);    
            m3.sweepTo(20 + (rotaionDir * 10));    
            break;
          case 3: //rR
            m1.sweepTo(120);    
            m2.sweepTo(20);    
            m3.sweepTo(70 + (rotaionDir * 10));    
            break;
          case 4: //lF
            m1.sweepTo(60);   
            m2.sweepTo(160);    
            m3.sweepTo(105 + (rotaionDir * 10));   
            break;
          case 5: //lM
            m1.sweepTo(60);    
            m2.sweepTo(160);    
            m3.sweepTo(125 + (rotaionDir * 10));   
            break;
          case 6: //lR
            m1.sweepTo(60);    
            m2.sweepTo(160);    
            m3.sweepTo(100 + (rotaionDir * 10));   
            break;
    }
  }

  //Dispatch table for rotational metachronal backswing state based on leg location
  void rmbs()
  {
    switch (legPos) {
          case 1: //rF
            m3.sweepTo(60 - (rotaionDir * 15));
            break;
          case 2: //rM
            m3.sweepTo(20 - (rotaionDir * 15));       
            break;
          case 3: //rR
            m3.sweepTo(70 - (rotaionDir * 15));    
            break;
          case 4: //lF
            m3.sweepTo(105 - (rotaionDir * 15));   
            break;
          case 5: //lM
            m3.sweepTo(125 - (rotaionDir * 15));
            break;
          case 6: //lR
            m3.sweepTo(100 - (rotaionDir * 15)); 
            break;
    }
  }

  //Dispatch table for reset state based on leg location
  void resetLeg()
  {
    switch (legPos) {
          case 1: //rF
            m1.sweepTo(120);    
            m2.sweepTo(20);   
            m3.sweepTo(60);  
            break;
          case 2: //rM
            m1.sweepTo(120);    
            m2.sweepTo(20);   
            m3.sweepTo(20);   
            break;
          case 3: //rR
            m1.sweepTo(120);    
            m2.sweepTo(20);   
            m3.sweepTo(70);    
            break;
          case 4: //lF
            m1.sweepTo(60);    
            m2.sweepTo(160);   
            m3.sweepTo(105);   
            break;
          case 5: //lM
            m1.sweepTo(60);    
            m2.sweepTo(160);    
            m3.sweepTo(125);  
            break;
          case 6: //lR
            m1.sweepTo(60);    
            m2.sweepTo(160);    
            m3.sweepTo(100);   
            break;
    }
    mdsDone = 0;
  }

};

//The hexapod class, has 6 legs
class Hexapod
{
  Leg l1;
  Leg l2;
  Leg l3;
  Leg l4;
  Leg l5;
  Leg l6;
  int updateInterval;       //interval between updates
  unsigned long lastUpdate; //last update time
  int state;  //0:stop   1:forward tripod   2:backwards tripod  3:reset  4:soil reading  5:planting seed
  int mFastInterval = 10;     //The interval speed of the leg that lifts during metachronal gait
  int mSlowInterval = 50;     //The interval speed of the leg that pushes during metachronal gait
  int metachronalUpswingState; //8 for forward, 11 for rotation

  public:
  Hexapod(int interval) 
  {
    updateInterval = interval;
    l1.reConstruct(1, interval);
    l2.reConstruct(2, interval);
    l3.reConstruct(3, interval);
    l4.reConstruct(4, interval);
    l5.reConstruct(5, interval);
    l6.reConstruct(6, interval);
    lastUpdate = millis();
  }

  void linkLeg(int leg, int pin1, int pin2, int pin3)
  {
    switch (leg) {
          case 1: //rF
            l1.linkJoints(pin1, pin2, pin3);
            break;
          case 2: //rM
            l2.linkJoints(pin1, pin2, pin3);
            break;
          case 3: //rR
            l3.linkJoints(pin1, pin2, pin3);
            break;
          case 4: //lF
            l4.linkJoints(pin1, pin2, pin3);
            break;
          case 5: //lM
            l5.linkJoints(pin1, pin2, pin3);
            break;
          case 6: //lR
            l6.linkJoints(pin1, pin2, pin3);
            break;
    }
  }

  void setState(int newState)
  {
    state = newState;
  }

  void setAllLegStates(int newState)
  {
    l1.setState(newState);
    l2.setState(newState);
    l3.setState(newState);
    l4.setState(newState);
    l5.setState(newState);
    l6.setState(newState);
  }

  void setLegState(int leg, int newState)
  {
    switch (leg) {
          case 1: //rF
            l1.setState(newState);
            break;
          case 2: //rM
            l2.setState(newState);
            break;
          case 3: //rR
            l3.setState(newState);
            break;
          case 4: //lF
            l4.setState(newState);
            break;
          case 5: //lM
            l5.setState(newState);
            break;
          case 6: //lR
            l6.setState(newState);
            break;
    }
  }

  void moveLegNow(int leg, int p1, int p2, int p3)
  {
    switch (leg) {
          case 1: //rF
            l1.moveNow(p1, p2, p3);
            break;
          case 2: //rM
            l2.moveNow(p1, p2, p3);
            break;
          case 3: //rR
            l3.moveNow(p1, p2, p3);
            break;
          case 4: //lF
            l4.moveNow(p1, p2, p3);
            break;
          case 5: //lM
            l5.moveNow(p1, p2, p3);
            break;
          case 6: //lR
            l6.moveNow(p1, p2, p3);
            break;
    }
  }

  void setAllLegsRotation(int dir)
  {
    l1.setRotationDirection(dir);
    l2.setRotationDirection(dir);
    l3.setRotationDirection(dir);
    l4.setRotationDirection(dir);
    l5.setRotationDirection(dir);
    l6.setRotationDirection(dir);
  }

  void setInterval(int interval)
  {
    updateInterval = interval;
    l1.setInterval(interval);
    l2.setInterval(interval);
    l3.setInterval(interval);
    l4.setInterval(interval);
    l5.setInterval(interval);
    l6.setInterval(interval);
  }

  void setStartingStates()
  {
    switch (state) {
        case 0: //Not moving
          undoAllMetachronalIntervals();
          clearAllMDS();
          l1.setState(0);
          l2.setState(0);
          l3.setState(0);
          l4.setState(0);
          l5.setState(0);
          l6.setState(0);
          break;
          
        case 1: //forward tripod gait
          undoAllMetachronalIntervals();
          clearAllMDS();
          l1.setState(1);
          l2.setState(3);
          l3.setState(1);
          l4.setState(3);
          l5.setState(1);
          l6.setState(3);
          break;
          
        case 2: //backward tripod gait
          undoAllMetachronalIntervals();
          clearAllMDS();
          l1.setState(4);
          l2.setState(6);
          l3.setState(4);
          l4.setState(6);
          l5.setState(4);
          l6.setState(6);
          break;
          
        case 3: //reset
          undoAllMetachronalIntervals();
          clearAllMDS();
          l1.setState(7);
          l2.setState(7);
          l3.setState(7);
          l4.setState(7);
          l5.setState(7);
          l6.setState(7);
          break;
          
        case 4: //soil reading
          
          break;
        case 5: //planting seed
          
          break;

        case 6: //forward metachronal gait
          metachronalUpswingState = 8;
          undoAllMetachronalIntervals();
          clearAllMDS();
          setMetachronalIntervals(1);
          l1.setState(8);
          l2.setState(10);
          l3.setState(10);
          l4.setState(10);
          l5.setState(10);
          l6.setState(10);
          break;

        case 7: //clockwise rotation metachronal gait
          metachronalUpswingState = 11;
          undoAllMetachronalIntervals();
          clearAllMDS();
          setAllLegsRotation(1); //1 = clockwise
          setMetachronalIntervals(1);
          l1.setState(11);
          l2.setState(13);
          l3.setState(13);
          l4.setState(13);
          l5.setState(13);
          l6.setState(13);
          break;

        case 8: //counter-clockwise rotation metachronal gait
          metachronalUpswingState = 11;
          undoAllMetachronalIntervals();
          clearAllMDS();
          setAllLegsRotation(-1); //-1 = counter-clockwise
          setMetachronalIntervals(1);
          l1.setState(11);
          l2.setState(13);
          l3.setState(13);
          l4.setState(13);
          l5.setState(13);
          l6.setState(13);
          break;
    }
  }

  void clearAllMDS()
  {
    l1.clearMDS();
    l2.clearMDS();
    l3.clearMDS();
    l4.clearMDS();
    l5.clearMDS();
    l6.clearMDS();
  }

  void setMetachronalIntervals(int leg)
  {
    switch (leg) {
          case 1: //rF
            l1.setInterval(mFastInterval);
            break;
          case 2: //rM
            l2.setInterval(mFastInterval);
            break;
          case 3: //rR
            l3.setInterval(mFastInterval);
            break;
          case 4: //lF
            l4.setInterval(mFastInterval);
            break;
          case 5: //lM
            l5.setInterval(mFastInterval);
            break;
          case 6: //lR
            l6.setInterval(mFastInterval);
            break;
    }
  }

  void undoMetachronalIntervals(int leg)
  {
    switch (leg) {
          case 1: //rF
            l1.setInterval(updateInterval);
            break;
          case 2: //rM
            l2.setInterval(updateInterval);
            break;
          case 3: //rR
            l3.setInterval(updateInterval);
            break;
          case 4: //lF
            l4.setInterval(updateInterval);
            break;
          case 5: //lM
            l5.setInterval(updateInterval);
            break;
          case 6: //lR
            l6.setInterval(updateInterval);
            break;
    }
  }

  void undoAllMetachronalIntervals()
  {
    l1.setInterval(updateInterval);
    l2.setInterval(updateInterval);
    l3.setInterval(updateInterval);
    l4.setInterval(updateInterval);
    l5.setInterval(updateInterval);
    l6.setInterval(updateInterval);
  }

  int moveLegs()
  {
    l1.moveLeg();
    l2.moveLeg();
    l3.moveLeg();
    l4.moveLeg();
    l5.moveLeg();
    l6.moveLeg();
    if (l1.checkMDS() > 0) {
      l1.clearMDS();
      undoMetachronalIntervals(1);
      setMetachronalIntervals(5);
      l5.setState(metachronalUpswingState);
      return 1;
    } else if (l5.checkMDS() > 0) {
      l5.clearMDS();
      undoMetachronalIntervals(5);
      setMetachronalIntervals(3);
      l3.setState(metachronalUpswingState);
      return 1;
    } else if (l3.checkMDS() > 0) {
      l3.clearMDS();
      undoMetachronalIntervals(3);
      setMetachronalIntervals(6);
      l6.setState(metachronalUpswingState);
      return 1;
    } else if (l6.checkMDS() > 0) {
      l6.clearMDS();
      undoMetachronalIntervals(6);
      setMetachronalIntervals(2);
      l2.setState(metachronalUpswingState);
      return 1;
    } else if (l2.checkMDS() > 0) {
      l2.clearMDS();
      undoMetachronalIntervals(2);
      setMetachronalIntervals(4);
      l4.setState(metachronalUpswingState);
      return 1;
    } else if (l4.checkMDS() > 0) {
      l4.clearMDS();
      undoMetachronalIntervals(4);
      setMetachronalIntervals(1);
      l1.setState(metachronalUpswingState);
      return 1;
    }
    return 0;
  }
  
};

/*GLOBALS*/
Hexapod charollete(50);
int incomingByte; // a variable to read incoming serial data into
int inputState; //Input state;  0:M  1:H 2:L 3:U

//Servo
Servo Seed_servo;  // create servo object to control a servo
int Delay=305; //specified delay (change for seeds)


//Sensors
int Moisture_thresholdUp = 400;
int Moisture_thresholdDown = 250;
int Temp_thresholdUp = 35;//in Celsius
int Temp_thresholdDown = 15;//in Celsius
int sensorPin = A15; //moisture sensor
OneWire  ds(34);  // on pin 34 (a 4.7K resistor is necessary)

void plantSeed() {
      Seed_servo.write(110); //open
      delay(Delay);
      Seed_servo.write(116); //closed
      delay(Delay);
      Seed_servo.write(100); //open
      delay(Delay);
      Seed_servo.write(116); //closed
}

int readSoil() {
  //ADD IN CODE
  return thermo();
}

void setup() {
  // initialize serial communication:
  Serial.begin(9600);
  delay(1000); //remove later
  
  //Seed Servo
  Seed_servo.attach(9);  // attaches the servo on pin 9 to the servo object
  Seed_servo.write(116); //closed on startup

  //LED Initializing
  strip.begin();
  strip.show();//sets all pixels to off
  for(int i=0;i<strip.numPixels();i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    strip.setPixelColor(i, strip.Color(255,0,255)); // purple
    strip.show(); // This sends the updated pixel color to the hardware.
    delay(240); // Delay for a period of time (in milliseconds).
      }
  
    
  charollete.linkLeg(1,43,41,39);
  charollete.moveLegNow(1, 120, 20, 60);

  charollete.linkLeg(2,37,35,33);
  charollete.moveLegNow(2, 120, 20, 20); 

  charollete.linkLeg(3,30,28,26);
  charollete.moveLegNow(3, 120, 20, 70);

  charollete.linkLeg(4,31,29,27);
  charollete.moveLegNow(4, 60, 160, 105);

  charollete.linkLeg(5,46,44,42);
  charollete.moveLegNow(5, 60, 160, 125); 

  charollete.linkLeg(6,52,50,48);
  charollete.moveLegNow(6, 60, 160, 100);

  charollete.setState(3);
  charollete.setStartingStates();
  
  inputState = 0;
  
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
    
    // if it's an H tripod forward:
    if (incomingByte == 'H' && inputState != 1) {
      Serial.println("MOVING FORWARD");
      colorWipe(strip.Color(0, 255, 0)); // Green
      inputState = 1;

      charollete.setState(1);
      charollete.setStartingStates();
    } 
    
    // if it's an L tripod back:
    if (incomingByte == 'L' && inputState != 2) {
      Serial.println("MOVING BACKWARD");
      colorWipe(strip.Color(255, 200,0)); // Yellow
      inputState = 2;

      charollete.setState(2);
      charollete.setStartingStates();
    }
    
    // if it's an M stop moving:
    if (incomingByte == 'M' && inputState != 0) {
      Serial.println("Stopping");
      colorWipe(strip.Color(255, 0,0)); // Red
      inputState = 0;

      charollete.setState(0);
      charollete.setStartingStates();
    }

    // if it's a U reset:
    if (incomingByte == 'U' && inputState != 3) {
      Serial.println("Reseting");
      colorWipe(strip.Color(255, 0,255)); // Purple
      inputState = 3;

      charollete.setState(3);
      charollete.setStartingStates();
    }
    
    if (incomingByte == 'S') {
      Serial.println("Checking Soil:");
      Serial.println("Soil Moisture: ");
      Serial.print(soil_Moisture());
      Serial.println("Soil Temperature: ");
      Serial.print(readSoil());
      readSoil();
    }
    
    if (incomingByte == 'P') {
      Serial.println("Planting");
      plantSeed();
    }

    // if it's a G metachronal forward:
    if (incomingByte == 'G' && inputState != 6) {
      Serial.println("Metachronal Forward");
      colorWipe(strip.Color(0, 255, 0)); // Green
      inputState = 6;

      charollete.setState(6);
      charollete.setStartingStates();
    }

    // if it's a C rotate metachronal clockwise:
    if (incomingByte == 'C' && inputState != 7) {
      Serial.println("Metachronal Clockwise");
      colorWipe(strip.Color(0, 255, 255)); // Cyan
      inputState = 7;

      charollete.setState(7);
      charollete.setStartingStates();
    }

    // if it's an R rotate metachronal clounter-clockwise:
    if (incomingByte == 'R' && inputState != 8) {
      Serial.println("Metachronal Counter-Clockwise");
      colorWipe(strip.Color(0, 255, 255)); // Cyan
      inputState = 8;

      charollete.setState(8);
      charollete.setStartingStates();
    }
  }
  int mdsStatus = charollete.moveLegs();
  if (mdsStatus > 0) {
    //Serial.println("Metachronal Downswing Finished");
  }
}


int soil_Moisture() {
  int sensorValue; //int to store Moisture Data
  sensorValue = analogRead(sensorPin);
  if (sensorValue <= Moisture_thresholdDown){
   return 0;// Too dry
  } else if (sensorValue >= Moisture_thresholdUp){
    return 2;//Too wet 
  } else {
   return 1; //good
  }
}

int thermo() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;

  if (OneWire::crc8(addr, 7) != addr[7]) {
      return 0;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  //delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  if (celsius <= Temp_thresholdDown) {
    return 2;// Too cold
  }
  else if(celsius >= Temp_thresholdUp) {
    return 0; // Too hot
  }
  else {
    return 1; // good
  }
}



