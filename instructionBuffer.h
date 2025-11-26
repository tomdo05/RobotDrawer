#define INST_BUFF_SIZE 1000

#ifndef INSTRUCTION_BUFFER_H
#define INSTRUCTION_BUFFER_H

struct instructionBuffer
{
    char data[INST_BUFF_SIZE][INST_BUFF_SIZE]; // an array of strings
    int numElements; // number of elements currently in array
};

void addToInstructionBuffer(char s[], struct instructionBuffer* instBuff);
void resetInstructionBuffer(struct instructionBuffer* instBuff);

#endif