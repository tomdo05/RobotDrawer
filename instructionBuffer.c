#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "instructionBuffer.h"

void addToInstructionBuffer(char s[], struct instructionBuffer* instBuff)
{
    snprintf(instBuff->data[instBuff->numElements], sizeof(instBuff->data[0]), "%s", s); // adds the string into its spot in the buffer with a null terminator "\0"
    instBuff->numElements++;
    if (instBuff->numElements > INST_BUFF_SIZE)
    {
        printf("Error: Instruction buffer overflow\n");
        exit(4);
    }
}

void resetInstructionBuffer(struct instructionBuffer* instBuff)
{
    memset(instBuff->data, 0, sizeof(instBuff->data));
    instBuff->numElements = 0;
}