/* 
 * File:   Protocol2.c
 * Author: maryx
 *
 * Created on March 14, 2024, 12:34 PM
 */

#include <stdio.h>  
#include <xc.h> 
#include <stdint.h>
#include <string.h>
#include "BOARD.h" 
#include "uart.h" 
#include "MessageIDs.h" 
#include "Protocol2.h"
#include "sys/attribs.h" 

#define BAUD_RATE 115200

// Define header and tail bytes
#define HEADER_BYTE 0xCC
#define TAIL_BYTE 0xB9
//typedef struct {
//    uint8_t ID;
//    uint8_t len;
//    uint8_t checkSum;
//    unsigned char payLoad[MAXPAYLOADLENGTH];
//} rxpADT;
//
//typedef struct Circular_Buffer {
//    int head;
//    int tail;
//    rxpADT data[PACKETBUFFERSIZE];
//} packBuff;

// Define baud rate

// Define states for packet building
typedef enum {
    STANDBY_STATE,
    READ_LEN_STATE,
    READ_ID_STATE,
    READ_PAYLOAD_STATE,
    READ_CHECKSUM_STATE,
    BUILD_STATE
} State;

// Define global variables
static int PacketReady = 0;
static RxPacket ProtocolPacket;
static int StateMachineState = STANDBY_STATE;
static PacketBuffer Packets;
static unsigned char Checksum;

// Function prototypes
static int PacketBufferState(const PacketBuffer *buffer);

// Function to initialize the protocol
void Protocol_Init(unsigned long baudrate) {
    UART_Init(baudrate);
    Packets.head = 0;
    Packets.tail = 0;
    LEDS_INIT();
    LEDS_SET(0x00);
    StateMachineState = STANDBY_STATE;
    Checksum = 0;
    char message[MAX_PAYLOAD_LENGTH];
    sprintf(message, "Mary's Protocol.c");
    Protocol_SendDebugMessage(message); // Initialize debug message
}

// Function to calculate checksum
static uint8_t Protocol_CalculateChecksum(unsigned char charIn, unsigned char curChecksum) {
    curChecksum = (curChecksum >> 1) + (curChecksum << 7);
    curChecksum += charIn;
    return curChecksum;
}

// Function to send debug message
int Protocol_SendDebugMessage(const char *Message) {
    if (Message == NULL) {
        return ERROR;
    }
    size_t messageLength = strnlen(Message, MAX_PAYLOAD_LENGTH);
    if (messageLength == MAX_PAYLOAD_LENGTH) {
        messageLength = MAX_PAYLOAD_LENGTH - 1;
    }
    char debugArray[MAX_PAYLOAD_LENGTH];
    snprintf(debugArray, sizeof(debugArray), "%.*s  -- Compiled on %s %s\n",
            (int) messageLength, Message, __DATE__, __TIME__);
    size_t formattedLength = strlen(debugArray);
    if (Protocol_SendPacket(formattedLength, ID_DEBUG, debugArray) == ERROR) {
        return ERROR;
    }
    return SUCCESS;
}

// Function to build incoming packets
uint8_t BuildRxPacket(RxPacket *rxPacket, unsigned char ch, unsigned char reset) {
    static int i = 0;
    
    switch (StateMachineState) {
        case STANDBY_STATE:
            i = 0;
            if (ch == HEADER_BYTE) {
                StateMachineState = READ_LEN_STATE; // Transition to read length state
            }
            break;
        case READ_LEN_STATE:
            rxPacket->len = ch;
            StateMachineState = READ_ID_STATE; // Transition to read ID state
            break;
        case READ_ID_STATE:
            rxPacket->ID = ch;
            Checksum = Protocol_CalculateChecksum(Checksum, ch); // Update checksum
            StateMachineState = READ_PAYLOAD_STATE; // Transition to read payload state
            break;
        case READ_PAYLOAD_STATE:
            if (i < rxPacket->len) {
                rxPacket->payLoad[i++] = ch;
                Checksum = Protocol_CalculateChecksum(Checksum, ch); // Update checksum
                if (i == rxPacket->len) {
                    StateMachineState = READ_CHECKSUM_STATE; // Transition to read checksum state
                }
            }
            break;
        case READ_CHECKSUM_STATE:
            rxPacket->checkSum = ch;
            if (rxPacket->checkSum == Checksum) {
                StateMachineState = BUILD_STATE; // Transition to build state
            } else {
                StateMachineState = STANDBY_STATE; // Reset state machine
            }
            Checksum = 0; // Clear checksum
            break;
        case BUILD_STATE:
            if (PacketBufferState(&Packets) != ERROR) {
                Packets.packItems[Packets.head] = *rxPacket;
                Packets.head = (Packets.head + 1) % PACKET_BUFFER_SIZE;
                StateMachineState = STANDBY_STATE; // Reset state machine
                PacketReady = 1; // Set packet ready flag
            }
            break;
    }
}

// Function to process parsed packets
uint8_t Packet_Message(void) {
    if (Packets.tail == Packets.head) {
        return ERROR; // No packets in buffer
    }
    RxPacket parsePack = Packets.packItems[Packets.tail];
    Packets.tail = (Packets.tail + 1) % PACKET_BUFFER_SIZE;
    uint8_t LEDS_ARE;
    unsigned int preEndLoad;
    
    switch (parsePack.ID) {
        // Add cases for different message IDs
        case ID_INVALID:
            PacketReady = 0; // Reset packet ready flag
            break;
        case ID_LEDS_SET:
            LEDS_SET(parsePack.payLoad[0]); // Set LEDs
            PacketReady = 0; // Reset packet ready flag
            break;
        case ID_LEDS_GET:
            LEDS_ARE = LEDS_GET();
            uint8_t LED_array[1];
            LED_array[0] = LEDS_ARE;
            Protocol_SendPacket(0x02, ID_LEDS_STATE, LED_array); // Send LED state
            PacketReady = 0; // Reset packet ready flag
            break;
        case ID_PING:
            for (int k = 0; k < (parsePack.len - 1); k++) {
                preEndLoad *= 256;
                preEndLoad += parsePack.payLoad[k];
            }
            unsigned int postEndLoad = preEndLoad;
            unsigned char pongArray[4];
            pongArray[0] = postEndLoad >> 24;
            pongArray[1] = postEndLoad >> 16;
            pongArray[2] = postEndLoad >> 8;
            pongArray[3] = postEndLoad;
            Protocol_SendPacket(0x05, ID_PONG, pongArray); // Send PONG packet
            PacketReady = 0; // Reset packet ready flag
            break;
    }
}