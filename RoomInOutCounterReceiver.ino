//https://learn.adafruit.com/adafruit-led-backpack/0-dot-56-seven-segment-backpack
//https://www.sparkfun.com/products/9026
//http://www.amazon.com/nRF24L01-Wireless-Transceiver-Arduino-Compatible/dp/B00E594ZX0

// Import needed libraries 
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

//  Declare Constants and Pin Numbers 
#define CE_PIN   9
#define CSN_PIN 8//10
#define IR1Pin 2 //IR sensor 1
#define IR2Pin 3 //IR sensor 2


// Define the transmit pipe 
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };// Radio pipe addresses for the 2 nodes to communicate.

//  Declare objects
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio // Set up nRF24L01 radio on SPI bus pins 9 & 10 
Adafruit_7segment segment = Adafruit_7segment();

//  Declare Variables  
int dataReceived[2];
unsigned int lastCount = 0;
unsigned int count = 0;
unsigned int count2 = 0;
unsigned int  countOut = 0;
unsigned int  countIn = 0;
unsigned int  total = 0;
boolean flag1=0;
boolean flag2=0;
boolean received = false;

byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0x6D, 0xFC}; //ethernet mac address
byte ip[] = {10,0,1,177}; //set IP of ethernet shield
byte gateway[] = {10,0,1,177}; //IP obtained from DhcpAddressPrinter example
byte subnet[] = {255,255,255,0}; //obtained from cmd->ipconfig
// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80); //initialize port
String readString;
void setup()   // Setup runs once
{
  //begin radio
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(pipes[1]);//open writting pipe
  radio.openReadingPipe(1,pipes[0]);//open reading pipe
  //Define pin modes for sensors
  pinMode(IR1Pin, INPUT);
  pinMode(IR2Pin, INPUT);
  attachInterrupt(0, sensor1trig, RISING); //Pin2 is sensor 1
  attachInterrupt(1, sensor2trig, RISING); //Pin3 is sensor 2
  radio.startListening();// Start listening 
  segment.begin(0x70);
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop(){   // Loop runs continuously
  updateCount();
  EthernetClient client = server.available();
  if(client){
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while(client.connected()){  
      if (client.available()){
        char c = client.read();
     
        //read char by char HTTP request
        if (readString.length() < 100) {
          //store characters to string
          readString += c;
          //Serial.print(c);
         }

         // if you've gotten to the end of the line (received a newline
         // character) and the line is blank, the http request has ended,
         // so you can send a reply
         if (c == '\n' && currentLineIsBlank) {
           client.println("HTTP/1.1 200 OK"); //send new page
           client.println("Content-Type: text/html");
           client.println("Connection: close");  // the connection will be closed after completion of the response
           client.println("Refresh: 10");  // refresh the page automatically every 5 sec     
           client.println();
           client.println("<HTML>");
           client.println("<HEAD>");
           client.println("<meta name='apple-mobile-web-app-capable' content='yes' />");
           client.println("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />");
           client.println("<link rel='stylesheet' type='text/css' href='http://randomnerdtutorials.com/ethernetcss.css' />");
           client.println("<TITLE>Room Occupancy Counter</TITLE>");
           client.println("</HEAD>");
           client.println("<BODY>");
           client.print("<H1>Room Occupancy is:  </H1>");
           client.println(total);
           client.println("<br />"); 
           client.println("<br />"); 
           //client.println("Refresh page to update room occupancy");
           client.println("</BODY>");
           client.println("</HTML>");
           client.println("</html>");
           updateCount();
           break;
         }
           if (c == '\n') {
            // you're starting a new line
            currentLineIsBlank = true;
           } else if (c != '\r') {
            // you've gotten a character on the current line
            currentLineIsBlank = false;
           }
           updateCount();
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    updateCount();
  }
}

void updateCount(){//this function update counter
  count = countIn-countOut;
   if(count!=lastCount){
     lastCount = count;
   }
   if(radio.available()){  //get data from other door
      while (!received)// Read all incoming data
      received = radio.read( dataReceived, sizeof(dataReceived) ); // read data 
      if(dataReceived[0] == 1){
        count2 = dataReceived[1];
      }else{
         // invalid  do nothing
 
      }
  }
  total = count2+count;
  segment.print(total, DEC);//update display
  received = false;
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
