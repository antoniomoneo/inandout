//  Add needed libraries )
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
/// Declare Constants and Pin Numbers
#define CE_PIN   9
#define CSN_PIN 10
#define IR1Pin 2 //IR sensor 1
#define IR2Pin 3 //IR sensor 2

const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };// Radio pipe addresses for the 2 nodes to communicate.

// Declare objects 
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio // Set up nRF24L01 radio on SPI bus pins 9 & 10 
// Declare Variables 
unsigned int dataSend[2];
unsigned int lastCount = 0;
unsigned int count = 0;
unsigned int  countOut = 0;
unsigned int  countIn = 0;
boolean flag1=0;
boolean flag2=0;

void setup()    // Setup runs once
{
  //begin radio
  radio.begin();
  radio.openWritingPipe(pipes[0]);//open writting pipe
  radio.openReadingPipe(1,pipes[1]);//open reading pipe
  //Define pin modes for sensors
  pinMode(IR1Pin, INPUT);
  pinMode(IR2Pin, INPUT);
  attachInterrupt(0, sensor1trig, RISING); //Pin2 is sensor 1
  attachInterrupt(1, sensor2trig, RISING); //Pin3 is sensor 2
}


void loop()   // Loop runs continuously
{
   count = countIn-countOut;//update count and if changed send it to other door
   if(count!=lastCount){
     lastCount = count;
     sendCount(count);
   }
}

void sensor1trig(){//Function that activate when sensor 1 triggered
  if(flag2){
    countOut++;
    flag2 = false;
  }else
    flag1 = true;
}

void sensor2trig(){//Function that activate when sensor 2 triggered
  if(flag1){
    countIn++;
    flag1 = false;
  }else
    flag2 = true;
}

void sendCount(int InOut){//this function send data to other door which has the server 
  dataSend[0] = 1;
  dataSend[1] = InOut;
  radio.stopListening();
  delay(5);
  radio.write( dataSend, sizeof(dataSend) ); // Send data set
}
