// RHGenericDriver.h
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2014 Mike McCauley
// $Id: RHGenericDriver.h,v 1.6 2014/04/14 08:37:11 mikem Exp $

#ifndef RHGenericDriver_h
#define RHGenericDriver_h

#include <RadioHead.h>

/////////////////////////////////////////////////////////////////////
/// \class RHGenericDriver RHGenericDriver.h <RHGenericDriver.h>
/// \brief Abstract base class for a RadioHead driver.
///
/// This class defines the functions that must be provided by any RadioHead driver.
/// Different types of driver wil implement all the abstract functions, and willperhaps override 
/// other functions in this subclass, or perhaps add new functions specifically required by that driver.
/// Do not directly instantiate this class: it is only to be subclassed by driver classes.
///
/// Subclasses are expected to implement a half-duplex, unreliable, error checked, unaddressed packet transport.
/// They are expected to carry a message payload with an appropriate maximum length for the transport hardware
/// and to also carry unaltered 4 message headers: TO, FROM, ID, FLAGS
class RHGenericDriver
{
public:
    /// \brief Defines different operating modes for the transport hardware
    ///
    /// These are the different values that can be adopted by the _mode variable and 
    /// returned by the mode() member function,
    typedef enum
    {
	RHModeInitialising = 0, ///< Transport is initialising. Initial default value until init() is called..
	RHModeIdle,             ///< Transport is idle.
	RHModeTx,               ///< Transport is in the process of transmitting a message.
	RHModeRx                ///< Transport is in the process of receiving a message.
    } RHMode;

    /// Constructor
    RHGenericDriver();

    /// Initialise the Driver transport hardware and software.
    /// Make sure the Driver is properly configured before calling init().
    /// \return true if initialisation succeeded.
    virtual bool init();

    /// Tests whether a new message is available
    /// from the Driver. 
    /// On most drivers, this will also put the Driver into RHModeRx mode until
    /// a message is actually received bythe transport, when it wil be returned to RHModeIdle.
    /// This can be called multiple times in a timeout loop
    /// \return true if a new, complete, error-free uncollected message is available to be retreived by recv()
    virtual bool available() = 0;

    /// Turns the receiver on if it not already on.
    /// If there is a valid message available, copy it to buf and return true
    /// else return false.
    /// If a message is copied, *len is set to the length (Caution, 0 length messages are permitted).
    /// You should be sure to call this function frequently enough to not miss any messages
    /// It is recommended that you call it in your main loop.
    /// \param[in] buf Location to copy the received message
    /// \param[in,out] len Pointer to available space in buf. Set to the actual number of octets copied.
    /// \return true if a valid message was copied to buf
    virtual bool recv(uint8_t* buf, uint8_t* len) = 0;

    /// Waits until any previous transmit packet is finished being transmitted with waitPacketSent().
    /// Then loads a message into the transmitter and starts the transmitter. Note that a message length
    /// of 0 is NOT permitted. 
    /// \param[in] data Array of data to be sent
    /// \param[in] len Number of bytes of data to send (> 0)
    /// \return true if the message length was valid and it was correctly queued for transmit
    virtual bool send(const uint8_t* data, uint8_t len) = 0;

    /// Returns the maximum message length 
    /// available in this Driver.
    /// \return The maximum legal message length
    virtual uint8_t maxMessageLength() = 0;

    /// Starts the receiver and blocks until a valid received 
    /// message is available.
    void            waitAvailable();

    /// Blocks until the transmitter 
    /// is no longer transmitting.
    void            waitPacketSent();

    /// Blocks until the transmitter is no longer transmitting.
    /// or until the timeout occuers, whichever happens first
    /// \param[in] timeout Maximum time to wait in milliseconds.
    /// \return true if the RF22 completed transmission within the timeout period. False if it timed out.
    bool            waitPacketSent(uint16_t timeout);

    /// Starts the receiver and blocks until a received message is available or a timeout
    /// \param[in] timeout Maximum time to wait in milliseconds.
    /// \return true if a message is available
    bool            waitAvailableTimeout(uint16_t timeout);

    /// Sets the address of this node. Defaults to 0xFF. Subclasses or the user may want to change this.
    /// This will be used to test the adddress in incoming messages. In non-promiscuous mode,
    /// only messages with a TO header the same as thisAddress or the broadcast addess (0xFF) will be accepted.
    /// In promiscuous mode, all messages will be accepted regardless of the TO header.
    /// In a conventional multinode system, all nodes will have a unique address 
    /// (which you could store in EEPROM).
    /// You would normally set the header FROM address to be the same as thisAddress (though you dont have to, 
    /// allowing the possibilty of address spoofing).
    /// \param[in] thisAddress The address of this node.
    virtual void setThisAddress(uint8_t thisAddress);

    /// Sets the TO header to be sent in all subsequent messages
    /// \param[in] to The new TO header value
    virtual void           setHeaderTo(uint8_t to);

    /// Sets the FROM header to be sent in all subsequent messages
    /// \param[in] from The new FROM header value
    virtual void           setHeaderFrom(uint8_t from);

    /// Sets the ID header to be sent in all subsequent messages
    /// \param[in] id The new ID header value
    virtual void           setHeaderId(uint8_t id);

    /// Sets the FLAGS header to be sent in all subsequent messages
    /// \param[in] flags The new FLAGS header value
    virtual void           setHeaderFlags(uint8_t flags);

    /// Tells the receiver to accept messages with any TO address, not just messages
    /// addressed to thisAddress or the broadcast address
    /// \param[in] promiscuous true if you wish to receive messages with any TO address
    virtual void           setPromiscuous(bool promiscuous);

    /// Returns the TO header of the last received message
    /// \return The TO header
    virtual uint8_t        headerTo();

    /// Returns the FROM header of the last received message
    /// \return The FROM header
    virtual uint8_t        headerFrom();

    /// Returns the ID header of the last received message
    /// \return The ID header
    virtual uint8_t        headerId();

    /// Returns the FLAGS header of the last received message
    /// \return The FLAGS header
    virtual uint8_t        headerFlags();

    /// Returns the most recent RSSI (Receiver Signal Strength Indicator).
    /// Usually it is the RSSI of the last received message, which is measured when the preamble is received.
    /// If you called readRssi() more recently, it will return that more recent value.
    /// \return The most recent RSSI measurement in dBm.
    int8_t        lastRssi();

    /// Returns the operating mode of the library.
    /// \return the current mode, one of RF69_MODE_*
    RHMode          mode();

    /// Sets the operating mode of the transport.
    void            setMode(RHMode mode);

    /// Prints a data buffer in HEX.
    /// For diagnostic use
    /// \param[in] prompt string to preface the print
    /// \param[in] buf Location of the buffer to print
    /// \param[in] len Length of the buffer in octets.
    static void    printBuffer(const char* prompt, const uint8_t* buf, uint8_t len);

protected:

    /// The current transport operating mode
    volatile RHMode       _mode;

    /// This node id
    uint8_t             _thisAddress;
    
    /// Whether the transport is in promiscuous mode
    bool                _promiscuous;

    /// TO header in the last received mesasge
    volatile uint8_t    _rxHeaderTo;

    /// FROM header in the last received mesasge
    volatile uint8_t    _rxHeaderFrom;

    /// ID header in the last received mesasge
    volatile uint8_t    _rxHeaderId;

    /// FLAGS header in the last received mesasge
    volatile uint8_t    _rxHeaderFlags;

    /// TO header to send in all messages
    uint8_t             _txHeaderTo;

    /// FROM header to send in all messages
    uint8_t             _txHeaderFrom;

    /// ID header to send in all messages
    uint8_t             _txHeaderId;

    /// FLAGS header to send in all messages
    uint8_t             _txHeaderFlags;

    /// The value of the last received RSSI value, in some transport specific units
    volatile int8_t     _lastRssi;

    /// Count of the number of bad messages (eg bad checksum etc) received
    volatile uint16_t   _rxBad;

    /// Count of the number of successfully transmitted messaged
    volatile uint16_t   _rxGood;

    /// Count of the number of bad messages (correct checksum etc) received
    volatile uint16_t   _txGood;
    
private:

};


#endif 
