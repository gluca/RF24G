
/* This is a small sketch that listens 
 * for packets and forwards them back to the sender.
 */

#include <rf24g.h>
// we must instantiate the RF24_G object outside of the setup function so it is available in the loop function
RF24_G test;

void setup() {
  Serial.begin(9600);
  // create the RF24G object with an address of 1, using pins 7 and 8
  test = RF24_G(1, 9, 10, 115);
  // print out the details of the radio's configuration (useful for debug)
}

void loop() {
  // declare packet variable
  packet receiver;
  // declare variable to place the packet payload in
  uint16_t payload;
  // check if the radio has any packets in the receive queue
  if (test.available() == true) {
    Serial.println("packet received!");
    // read the data into the packet 
    test.read(&receiver);
    receiver.serialDumpHex();
    receiver.setAddress(4);
    // since the address in the packet object is already
    // set to the address of the receiver, it doesn't need to be changed
    // hence, we can write the packet back to the receiver
    // we may check to see if the transmission failed, if so we just drop the packet
    if (test.write(&receiver) == false) {
      Serial.println("transmit back failed!");
      Serial.println("dropping packet...");
    } else {
      receiver.serialDumpHex();
    }
  }
}

