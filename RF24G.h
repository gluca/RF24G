
/**
 * @file RF24G.h
 * @author Caio Motta
 * @date 19 Sep 2016
 * @brief A simple interface for the RF24 radio that abstracts thmr20's Driver.
 *
 * This library provides a simple way for up to 6 nRF24L01 radios to communicate with each other.
 *
 * @see http://tmrh20.github.io/RF24/
 */


#ifndef __RF24G_H__
#define __RF24G_H__

#include "RF24.h"

#define PACKET_CNTER 32

#define MAX_NODES 6

#define BASE_ADDRESS 0xDEADBEEF00LL


#define TIMEOUT 5

class packet {
	/**
   * @name Packet object that is sent via the radios
   *
   *  This is the object that is passed to the radio to be sent.  It has the source/destination address, a counter, and the payload inside.
   */
  /**@{*/
private:
	uint8_t address;
	uint8_t cnt;
	byte buffer[30];
public:
/**
   * @name Packet public interface
   *
   *  These are the main methods you need to set, modify, and retrieve data from packets.
   */
  /**@{*/
   /**
	   * Default  Constructor
	   *
	   * Creates a new instance of the packet object.  The packet is blank and will need to be modified with the methods below. 
	   */

	RF24_G();

	/**
	   * Sets the address of a packet.  
	   *
	   * If you are sending a packet, set this to set the destination of the packet.
	   * 
	   */
	void setAddress(uint8_t _address);

	/**
	   * Gets the address of a packet.  
	   *
	   * If you receive a packet, call this on the packet to get what address the packet came from.
	   *
	   * @return Current packet address.
	   */
	uint8_t getAddress();

	/**
	   * Gets the counter of a packet.  
	   *
	   * This is used internally by the library to set the packet counter.  This is used to detect duplicate packets.
	   *
	   * The user does not need to use this method.
	   *
	   * @return Current packet counter.
	   */
	uint8_t getCnt();

	/**
	   * Sets the counter of a packet.  
	   *
	   * This is used internally by the library to set the packet counter.  This is used to detect duplicate packets.
	   *
	   * The user does not need to use this method.
	   *
	   */
	void setCnt(uint8_t _cnt);

	/**
	   * Adds any datatype smaller than 31 bytes to the packet. 
	   * @note There is no way to determine what kind of datatype is in this packet.
	   * @note If you want to send multiple values, use a struct or class similar to this packet within the payload.
	   *
	   * This needs the address of an object and it's size to work correctly.
	   * @code
	   *	//addPayload() example:
	   *
	   *			int var = 23;
	   *			if (packet.addPayload( &value, sizeof(var)) == false) {
	   *				Serial.println("Datatype is too large!")
	   *			}
	   * @endcode 
	   *
	   * @return True if the size is within 31 bytes,  false if it is not. 
	   *
	   * @warning This does not allow for you to overwrite the packet.  But it is possible to overread from locations in memory that are adjacent to an object!  Always use sizeof(yourObject) to prevent this.
	   */

	bool addPayload(const void * data, const uint8_t size);

	/**
	   * Retrieves any datatype smaller than 31 bytes from the packet. 
	   * @note There is no way to determine what kind of datatype is in this packet.
	   * @note If you want to send multiple values, use a struct or class similar to this packet within the payload.
	   *
	   * This needs the address of an object and it's size to work correctly.
	   *
	   * @code
	   *	//readPayload() example:
	   *
	   *			int var;
	   *			if (packet.readPayload( &var, sizeof(var)) == false) {
	   *				Serial.println("Datatype is too large!")
	   *			}
	   * @endcode 
	   *
	   * @note The variable \a var will have a new value from the packet.
	   *
	   * @return True if the size is within 31 bytes,  false if it is not.
	   *
	   * @warning If you specify a size that is larger than the object you wish to write to, you can write into adjacent memory!  
	   * @warning This \a probably will crash your program and/or give you junk data in other parts of your code!  Always use sizeof(yourObject) to prevent this.
	   */
	bool readPayload(void * data, const uint8_t size);
};




class RF24_G {
private:
	int myAddress;
	uint8_t TXpacketCounters[MAX_NODES];
	uint8_t RXpacketCounters[MAX_NODES];
	RF24 radio{8,9};
public:
/**
   * @name Primary public interface
   *
   *  These are the main methods you need to send and receive data.
   */
  /**@{*/

	/**
	   * Default Constructor
	   *
	   * Creates a new instance of the radio object.  This configures tmrh20's driver to default settings.  
	   * Use this if you want to instantiate the radio class, but initialize it later.
	   */
	RF24_G();

	/**
	   * Constructor
	   *
	   * Creates a new instance of the radio object.  This configures tmrh20's driver.  Before using, you create an instance
	   * and send in the unique pins that this chip is connected to.
	   *
	   * @param address The address of tis radio instance
	   * @param _cepin The pin attached to Chip Enable on the RF module
	   * @param _cspin The pin attached to Chip Select
	   */	
	RF24_G(uint8_t address, uint8_t _cepin, uint8_t _cspin);

	/**
	   * Checks if there is a packet received packet to be read
	   *
	   * 
	   * @return True if a packet is available, false if not. 
	   * 
	   */
	bool available();

	/**
	   * Writes a packet.
	   * This needs the address of an object to work correctly.
	   *
	   * @code
	   *	//write() example:
	   *
	   *			int var;
	   *			if (radio.write( &packet) == false) {
	   *				Serial.println("Transmission failed!")
	   *			}
	   * @endcode 
	   * 
	   * @return True if a packet was sent successfully, false if not. 
	   * 
	   */
	bool write(const packet* _packet);
	/**
	   * Reads a packet.
	   * This needs the address of an object to work correctly.
	   *
	   * @code
	   *	//read() example:
	   *
	   *			int var;
	   *			if (radio.read( &packet) == false) {
	   *				Serial.println("Receive failed!")
	   *			}
	   * @endcode 
	   * 
	   * @return True if a packet was read successfully, false if not. 
	   * 
	   */
	bool read(packet* _packet);
	/**
	   * Sets the channel to use
	   * @note The available channels are 0-125, but channels 108+ are out of the wifi band and recommended.
	   * 
	   * @return True if the channel was set successfully, false if not. 
	   * 
	   */
	bool setChannel(uint8_t channel);

};
#endif