

#if defined(ARDUINO_ARCH_NRF52) || defined(ARDUINO_ARCH_NRF52840)
#include <nrf_to_nrf.h>
nrf_to_nrf radio;
#else
#include <RF24.h>
#endif
#include "rf24g.h"


    
packet::packet(){
    memset(this->buffer,0,30);
    this->cnt = 0;
    this->address = 0;
    this->size=0;
}

   
void packet::setAddress(uint8_t _address) {
	address = _address;
}

uint8_t packet::getAddress() const {
	return address;
}

uint8_t packet::getCnt() const {
	return cnt;
}

void packet::setCnt(uint8_t _cnt) {
	cnt = _cnt;
}

bool packet::addPayload(const void * data, const uint8_t size){
	if (size > MAX_PAYLOAD_SIZE-2) {
		return false;
	}
	const uint8_t* current = reinterpret_cast<const uint8_t*>(data);
  memcpy(buffer,current,size);
  this->size=size;
  uint8_t left=MAX_PAYLOAD_SIZE-2-size;
  if(left>0) memset(this->buffer+size,0,left);
	return true;
}

bool packet::readPayload(void * data, const uint8_t size) {
	if (size > MAX_PAYLOAD_SIZE-2) {
		return false;
	}
	uint8_t* current = reinterpret_cast<uint8_t*>(data);
  memcpy(current,buffer,size);
	return true;
}

void packet::serialDumpHex() {
  Serial.print("Address:");Serial.println(this->address);
  Serial.print("  count:");Serial.println(this->cnt);
  uint8_t size = this->size>0 ? this->size : MAX_PAYLOAD_SIZE;
  Serial.print("Payload:");
  for(uint8_t i=0; i<size;i++) {
    Serial.print(this->buffer[i]<10?"0x0":"0x");
    Serial.print(this->buffer[i],HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void packet::serialDumpString() {
  Serial.print("Address:");Serial.println(this->address);
  Serial.print("  count:");Serial.println(this->cnt);
  uint8_t size = this->size>0 ? this->size : MAX_PAYLOAD_SIZE-2;
  char c=buffer[MAX_PAYLOAD_SIZE-3];
  buffer[MAX_PAYLOAD_SIZE-3]=0;
  Serial.print("Payload:");Serial.print((char *)this->buffer);if(size==MAX_PAYLOAD_SIZE-2)Serial.print(c);
  buffer[MAX_PAYLOAD_SIZE-3]=c;
  Serial.println();
}

RF24_G::RF24_G(){
#if defined NRF52_RADIO_LIBRARY
	radio = nrf_to_nrf();
#else
  radio = RF24(9,10);
#endif
  myAddress = 0;
}


RF24_G::RF24_G(uint8_t address){
	setup(address,9,10);
}

RF24_G::RF24_G(uint8_t address, uint8_t _cepin, uint8_t _cspin, uint8_t _channel) {
     setup(address,_cepin,_cspin,_channel);
}

void RF24_G::setup(uint8_t address, uint8_t _cepin, uint8_t _cspin, uint8_t _channel){
    	int popIterator;
	for (popIterator = 0; popIterator < (MAX_NODES); popIterator++) {
		TXpacketCounters[popIterator] = 0;
		RXpacketCounters[popIterator] = PACKET_CNTER+1;
	}
#if defined NRF52_RADIO_LIBRARY
	radio = nrf_to_nrf();
#else
  radio = RF24(_cepin,_cspin);
#endif
  myAddress = address;

	radio.begin();
  radio.setChannel(_channel>125 ? 108 : _channel);
	radio.setAutoAck(1);                    // Ensure autoACK is enabled
	radio.setRetries(5 + myAddress,15);                 // Smallest time between retries, max no. of retries
	radio.setPayloadSize(MAX_PAYLOAD_SIZE);
	int pipenum = 0;
	for (popIterator = 0; popIterator < (MAX_NODES); popIterator++) {
		if ((popIterator) != myAddress) {
			uint64_t rxAddress = (BASE_ADDRESS + ((popIterator) + ((MAX_NODES + 1)  *  ( myAddress))));
			radio.openReadingPipe(pipenum, rxAddress);
			//Serial.print("reading from to this address: ");
			//Serial.println((int)(((popIterator) + ((MAX_NODES + 1)  *  ( myAddress)))));
			pipenum++;
		}
	}
    
#if defined NRF52_RADIO_LIBRARY
  radio.setPALevel(NRF_PA_MAX);
#else
  radio.setPALevel(RF24_PA_MAX); 
#endif
  radio.startListening();                 // Start listening
}


bool RF24_G::available() {
	return radio.available();
}


bool RF24_G::write(packet* _packet) {

	radio.stopListening();
	bool success;
	int dest = _packet->getAddress();
	if (dest == myAddress){
		return false;
	}
	packet *TX = _packet;
	TX->setAddress(myAddress);
	TX->setCnt(TXpacketCounters[dest]);
    
	radio.openWritingPipe((BASE_ADDRESS + ((myAddress) + ((MAX_NODES + 1)  *  dest))));
	//Serial.print("writing to this address: ");
	//Serial.println((int)(((myAddress ) + ((MAX_NODES + 1)  *  dest))));
	success = radio.write(TX, MAX_PAYLOAD_SIZE);
	if (success == false) {
		delayMicroseconds(TIMEOUT*myAddress);
		success = radio.write(TX, MAX_PAYLOAD_SIZE);
	}
	radio.startListening();
	if (success == true) {
		TXpacketCounters[dest] = ((TXpacketCounters[dest] + 1) & (PACKET_CNTER - 1) );
	}

	return success;
}

bool RF24_G::read(packet* _packet) {

	while (radio.available()) {
		radio.read(&receive, MAX_PAYLOAD_SIZE);
		*_packet = receive;        
		if(receive.getCnt() != RXpacketCounters[receive.getAddress()]){
			RXpacketCounters[receive.getAddress()] = ((RXpacketCounters[receive.getAddress()] + 1) & (PACKET_CNTER - 1) );
			return true;
		}
	}
    
	return false;
}


bool RF24_G::setChannel(uint8_t channel) {
	if (channel > 125) {
		return false;
	}
	radio.stopListening();
	radio.setChannel(channel);
	radio.startListening();
	return true;
}