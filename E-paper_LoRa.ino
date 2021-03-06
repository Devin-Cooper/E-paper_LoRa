
#include <SPI.h>
#include "epd2in9b.h"
#include "imagedata.h"
#include "epdpaint.h"
#include <LoRa.h>


#define COLORED     0
#define UNCOLORED   1

  //LoR32u4II 868MHz or 915MHz (black board)
  #define SCK     15
  #define MISO    14
  #define MOSI    16
  #define SS      8
  #define RST     4
  #define DI0     7
  #define BAND    915E6 //868E6 
  #define PABOOST true 
  
String outgoing;              // outgoing message
int msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends
int i = 0;
byte displayflag = 1;
unsigned char REC_IMAGE_BLACK[592];
int incomingMsgId = 0;
//const unsigned char IMAGE_test[128];


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);                   // initialize serial
  //while (!Serial);



//Init the display 
 Epd epd;

  if (epd.Init() != 0) {
    Serial.print("e-Paper init failed");
    return;
  }

  /* This clears the SRAM of the e-paper display */
  epd.ClearFrame();

  /**
    * Due to RAM not enough in Arduino UNO, a frame buffer is not allowed.
    * In this case, a smaller image buffer is allocated and you have to 
    * update a partial display several times.
    * 1 byte = 8 pixels, therefore you have to set 8*N pixels at a time.
    */
  unsigned char image[1024];
  Paint paint(image, 128, 18);    //width should be the multiple of 8 

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(0, 0, "e-Paper Demo", &Font12, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 24, 32, paint.GetWidth(), paint.GetHeight());
/*
  paint.Clear(COLORED);
  paint.DrawStringAt(2, 2, "Hello world", &Font16, UNCOLORED);
  epd.SetPartialWindowRed(paint.GetImage(), 0, 64, paint.GetWidth(), paint.GetHeight());
  
  paint.SetWidth(64);
  paint.SetHeight(64);

  paint.Clear(UNCOLORED);
  paint.DrawRectangle(0, 0, 40, 50, COLORED);
  paint.DrawLine(0, 0, 40, 50, COLORED);
  paint.DrawLine(40, 0, 0, 50, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 8, 120, paint.GetWidth(), paint.GetHeight());
  
  paint.Clear(UNCOLORED);
  paint.DrawCircle(32, 32, 30, COLORED);
  epd.SetPartialWindowBlack(paint.GetImage(), 64, 120, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawFilledRectangle(0, 0, 40, 50, COLORED);
  epd.SetPartialWindowRed(paint.GetImage(), 8, 200, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawFilledCircle(32, 32, 30, COLORED);
  epd.SetPartialWindowRed(paint.GetImage(), 64, 200, paint.GetWidth(), paint.GetHeight());

 

  /* This displays the data from the SRAM in e-Paper module */
  epd.DisplayFrame();

  /* This displays an image 
  epd.DisplayFrame(IMAGE_BLACK, IMAGE_RED);

  /* Deep sleep */
  epd.Sleep();

  Serial.println("LoRa Duplex with callback");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(SS,RST,DI0);

  if (!LoRa.begin(915E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  LoRa.setSpreadingFactor(7);           // ranges from 6-12,default 7 see API docs
  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa init succeeded.");
  //LoRa.sleep();

  
}

String payload;
byte payloadByte;

void loop() {
for(int i = 0; i < 128; i++)
{
  Serial.println (String((IMAGE_test[i]), HEX));
}



  
  for(i=0; i<591; ){
  if (displayflag == 1) {

    
  //Init the display 
  Epd epd;

  /* This displays the data from the SRAM in e-Paper module */
  epd.DisplayFrame();

  // This displays an image 
  epd.DisplayFrame(REC_IMAGE_BLACK, IMAGE_RED);

  /* Deep sleep */
  epd.Sleep();
    
  }
  if ((millis() - lastSendTime > interval) && (i<590)) {   
    int inc = i;
    String payload = String((IMAGE_BLACK[i]), HEX);
    sendByte(IMAGE_BLACK[i]);
    Serial.println("inc " + String(inc));
    Serial.println("Sending " + payload);
    lastSendTime = millis();            // timestamp the message
    interval = 1000;     //random(2000) + 1000;     // 2-3 seconds
    LoRa.receive();                     // go back into receive mode
    i++;
    } 
  }
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void sendByte(byte payloadByte) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(highByte(msgCount));                 // add message ID 
  LoRa.write(lowByte(msgCount));                 // add message ID 
  LoRa.write(1);                        // add payload length
  LoRa.write(payloadByte);              // add payload byte
 // LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}


void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  else if (packetSize == 6) {
    Serial.println("Single byte Payload");

// read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingHigh = LoRa.read();     // incoming msg ID
  byte incomingLow = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length
  byte incomingByte = LoRa.read();                // payload of packet
//  byte incomingByte2 = LoRa.read();
  incomingMsgId = word(incomingHigh, incomingLow);

    if (incomingLength != 1) {   // check length for error
    Serial.println("error: message length does not match length");
    Serial.println("Message length: " + String(incomingLength));
    return;                             // skip rest of function
    }
  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }
  
  // Store the received byte into array
  REC_IMAGE_BLACK[incomingMsgId] = incomingByte;

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + String(incomingByte, HEX));
 // Serial.println("Message: " + String(incomingByte2, HEX));
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
  }

  //if packet is non-zero and is greater than one byte then assume it is a string
  
  else {
        Serial.println("String Payload");
        Serial.println(packetSize);

                                         // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length
  String incoming = "";                 // payload of packet

  
  while (LoRa.available()) {            // can't use readString() in callback, so
    incoming += (char)LoRa.read();      // add bytes one by one
  }
      // if the recipient isn't this device or broadcast, 
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    Serial.println("Message length: " + String(incomingLength));
    return;                             // skip rest of function
  }
  
    // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println("Packet Size: " + String(packetSize));
  Serial.println();


  }
  }
