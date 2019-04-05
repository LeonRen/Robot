#include <Stepper.h>

int output1 = 2;
int output2 = 3;
int output3 = 4;
int output4 = 5; 
void setup() {
 
 pinMode(output1,OUTPUT);
 pinMode(output2,OUTPUT);
 pinMode(output3,OUTPUT);
 pinMode(output4,OUTPUT);
 digitalWrite(output1, LOW);   
 digitalWrite(output2, LOW);   
 digitalWrite(output3, LOW);   
 digitalWrite(output4, LOW);  
}
void step1(){
  digitalWrite(output1,HIGH);
  digitalWrite(output2,LOW);
  digitalWrite(output3,LOW);
  digitalWrite(output4,LOW);

}
void step2(){
  digitalWrite(output1,LOW);
  digitalWrite(output2,LOW);
  digitalWrite(output3,HIGH);
  digitalWrite(output4,LOW);


}
void step3(){
  digitalWrite(output1,LOW);
  digitalWrite(output2,HIGH);
  digitalWrite(output3,LOW);
  digitalWrite(output4,LOW);
 
}

void step4(){
  digitalWrite(output1,LOW);
  digitalWrite(output2,LOW);
  digitalWrite(output3,LOW);
  digitalWrite(output4,HIGH);
  
}

void loop() {
  // put your main code here, to run repeatedly: 
 step1();
 delay(10);
 step2();
 delay(10);
 step3();
 delay(10);
 step4();
 delay(10);
}
