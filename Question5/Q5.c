#include <stdio.h>

#define FALSE 0
#define TRUE 1
#define BITS_IN_BYTE 8

typedef unsigned char bool;
typedef unsigned char byte;

//The order of the sections of the message
typedef enum {
    MS_SOURCE_INDEX   = 0,
    MS_COMMAND_LENGTH = 1,
    MS_PAYLOAD_LENGTH = 2,
    MS_CHECKSUM       = 3,
    MS_COMMAND        = 4,
    MS_PAYLOAD        = 5,
    MI_MESSAGE_OVER   = 6,
} MessageSection;

//Length of each section in the message
static int MessageSectionLengths[] = { 1, 2, 2, 4, 0, 0, 0 };

//Strcut storing the message
typedef struct {
    byte sourceIndex;             //Index representing wich device the messagen is sent from
    unsigned short commandLength; //Length of the command block
    unsigned short payloadLength; //Length of the payload block
    unsigned int checksum;        //Checksum that can be used to check if the messagen was received correctly
    byte *commandPtr;             //Pointer to the start of the command block
    byte *payloadPtr;             //Pointer to the start of the payload block
} Message;

//Buffer to store the received bytes
#define MESSAGE_BUFFER_CAPACITY 1024
int CurrentMessageBufferCount = 0;
byte CurrentMessageBuffer[MESSAGE_BUFFER_CAPACITY];

Message CurrentMessage = {0};
MessageSection CurrentMessageSectionBeingRead = MS_SOURCE_INDEX;
int CurrentNumOfBytesReadInSection = 0;

//Prints a message (this would be where the user would utilize the message for its purposes)
void PrintMessage(Message msg)
{
    printf("\tsourceIndex: %d\n",     msg.sourceIndex);
    printf("\tcommandLength: %d\n",   msg.commandLength);
    printf("\tpayloadLength: %d\n",   msg.payloadLength);
    printf("\tchecksum: %d\n",        msg.checksum);
    printf("\tcommandPtr: %p\n",      msg.commandPtr);
    printf("\tpayloadPtr: %p\n",      msg.payloadPtr);
    printf("Message:\n");

    printf("\tcommand: ");
    for(int i = 0; i < msg.commandLength; i++) printf("0x%X ", msg.commandPtr[i]);
    printf("\n");

    printf("\tpayload: ");
    for(int i = 0; i < msg.payloadLength; i++) printf("0x%X ", msg.payloadPtr[i]);
    printf("\n");
}

//Reads and parses the UART byte
bool handleInterrupt(byte dataByte)
{
    CurrentMessageBuffer[CurrentMessageBufferCount] = dataByte;

    switch (CurrentMessageSectionBeingRead) {
        case MS_SOURCE_INDEX: {
            printf("\n----------Starting to parse message----------\n");
            CurrentMessage.sourceIndex = dataByte; 
            break;
        }
        case MS_COMMAND_LENGTH: CurrentMessage.commandLength |= dataByte << CurrentNumOfBytesReadInSection * BITS_IN_BYTE; break;
        case MS_PAYLOAD_LENGTH: CurrentMessage.payloadLength |= dataByte << CurrentNumOfBytesReadInSection * BITS_IN_BYTE; break;
        case MS_CHECKSUM: CurrentMessage.checksum |= dataByte << CurrentNumOfBytesReadInSection * BITS_IN_BYTE; break;
        case MS_COMMAND: {
            if (CurrentNumOfBytesReadInSection != 0) break;
            CurrentMessage.commandPtr = &CurrentMessageBuffer[CurrentMessageBufferCount];
            MessageSectionLengths[MS_COMMAND] = CurrentMessage.commandLength;
        } break;
        case MS_PAYLOAD: {
            if (CurrentNumOfBytesReadInSection != 0) break;
            CurrentMessage.payloadPtr = &CurrentMessageBuffer[CurrentMessageBufferCount];
            MessageSectionLengths[MS_PAYLOAD] = CurrentMessage.payloadLength;
        } break;
        default: break;
    }

    CurrentMessageBufferCount += 1;
    CurrentNumOfBytesReadInSection += 1;
    if (CurrentNumOfBytesReadInSection == MessageSectionLengths[CurrentMessageSectionBeingRead]){
        CurrentNumOfBytesReadInSection = 0;
        CurrentMessageSectionBeingRead += 1;

        if (CurrentMessageSectionBeingRead == MI_MESSAGE_OVER){
            printf("Message parsed correctly:\n");
            PrintMessage(CurrentMessage);

            //The message is cleared after calling PrintMessage
            //In a real application it would probably be preferrable to store the message in the heap rather than discarting it immeadiatly after
            CurrentMessage = (Message){0};
            CurrentMessageBufferCount = 0;
            CurrentMessageSectionBeingRead = MS_SOURCE_INDEX;
            return FALSE;
        }
    }
    return TRUE;
}

//Helper function that parses a hex byte
byte hexToByte(char msb, char lsb){
    byte high = (msb <= '9') ? msb - '0' : msb - 'A' + 10;
    byte low  = (lsb <= '9') ? lsb - '0' : lsb - 'A' + 10;
    return (high << 4) | low;
}

void printUsage(){
    printf("Usage Q5: [stream of byte hexcodes]\n");
    printf("Example: Q5 00 01 FF ..");
}

int main(int argc, char **args){

    if (argc < 2){
        printUsage();
        return 1;
    }

    for (int argi = 1; argi < argc; argi += 1){
        if (args[argi][0] == 0 || args[argi][1] == 0 || args[argi][2] != 0)
        {
            printUsage();
            return 1;
        }

        //Pretends each byte in the command line argument is a UART byte
        byte decodedHex = hexToByte(args[argi][0], args[argi][1]);
        handleInterrupt(decodedHex);
    }

    if (CurrentMessageSectionBeingRead != MS_SOURCE_INDEX){
        printf("Provided sequence of bytes did not produce a complete message\n");
    }

    return 0;
}