#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instructionBuffer.h"
#include "functions.h"


int*** readFontData(const char* fileName) // COMPLETE AND TESTED
{
    FILE* file = fopen(fileName, "r"); // open the font file

    if (file == NULL)
    {
        printf("Error: Could not open font data file.\n"); //exits with code 1 if the font data file could not be opened
        exit(1);
    }

    char buff[100]; // initialise a buffer for storing lines from the font file

    // finding data structure dimensions (number of letters and maximum number of instructions)
    int instructionLength = 3;

    int maxInstructions = 0;
    int instructionCount;

    int numLetters = 0;

    while (fgets(buff, sizeof(buff), file))
    {
        if (strncmp(buff, "999", 3) == 0) // checks if the 1st 3 characters in a line are "999" (start of instructions for a new letter)
        {
            numLetters++;
            
            // checks if the instruction count for this letter is the highest so far (the first line "999 X Y" is counted as as instruction so 1 is added to instruction count)
            sscanf(buff, "%*d %*d %d", &instructionCount);
            if (instructionCount+1 > maxInstructions)
            {
                maxInstructions = instructionCount+1; //
            }
        }
    }

    // creating the 3D data structure

    int*** pointerArray = malloc(numLetters * sizeof(int**)); // pointerArray is a pointer structure such that pointerArray[letter][instruction] points to the first number in an instruction in data
    int* data = calloc(numLetters * maxInstructions * instructionLength, sizeof(int)); // a flat array storing all of the integers for each instruction

    // pointerArray[i][j][k]
    // is equivalent to
    // data[(i * maxInstructions * instructionLength) + (j * instructionLength) + k]

    // populating pointerArray with pointers to the correct locations in data
    for (int i = 0; i < numLetters; i++)
    {
        pointerArray[i] = malloc(maxInstructions * sizeof(int*));
        for (int j = 0; j < maxInstructions; j++)
        {
            pointerArray[i][j] = data + (i * maxInstructions * instructionLength) + (j * instructionLength);
        }
    }

    // populating data
    rewind(file); //start from the beginning of the font file
    
    int letter = -1;
    int instruction = 0;

    while (fgets(buff, sizeof(buff), file))
    {
        if (strncmp(buff, "999", 3) == 0) // checks if the 1st 3 characters in a line are "999" (start of instructions for a new letter)
        {
            letter++;
            instruction = 0;
        }

        int base = (letter * maxInstructions * instructionLength) + (instruction * instructionLength);

        sscanf(buff, "%d %d %d",
        &data[base + 0],
        &data[base + 1],
        &data[base + 2]
        ); // puts the integers which define each instruction into the correct position in data

        instruction++;
    }
    
    return pointerArray; // returns the pointer array which points into data
}








void letterToGCode(char c, struct instructionBuffer* gCodeBuffer, int***fontData, float letterHeight, float* p_penX, float* p_penY)
{
    int fontDataLetterSize = 18; // The default letter size in the font data file
    float lineSpacing = 5.0; // The gap between successive lines of text in mm

    float newX, newY; 

    int isPenDown; // 0-up    1-down
    int sValue; // 0-up    1000-down

    char gCodeInstruction[100];

    if (c == ' ') // if a space is detected the arm moves by the width of a character
    {
        newX = *p_penX + letterHeight;

        snprintf(gCodeInstruction, sizeof(gCodeInstruction), "S0 G0 X%.3f Y%.3f\n", newX, *p_penY);

        addToInstructionBuffer(gCodeInstruction, gCodeBuffer);

        *p_penX = newX; // modify the value of penX
    }
    else if (c == '\r')
    {
       ;
    }
    else if (c == '\n') // if a new line is detected
    {
        newX = 0; // return the pen to 0 in the x axis
        newY = *p_penY - (lineSpacing + letterHeight); // move the pen down a line

        snprintf(gCodeInstruction, sizeof(gCodeInstruction), "S0 G0 X%.3f Y%.3f\n", newX, newY);

        addToInstructionBuffer(gCodeInstruction, gCodeBuffer);

        *p_penX = newX; // modify the values of penX and penY
        *p_penY = newY;
    }
    else if ((int)c > 127) // if the character is not in regular ASCII print error message and skip
    {
        printf("Error: Could not convert character \'%c\' to GCode", c);
    }
    else
    {
        int cIndex = (int)c; // converts the char into an integer that can be used as in index in the fontData structure
        int numMovements = fontData[cIndex][0][2]; // extracts the number of movements needed to draw the character

        //printf("number of movements for -%c- : %d\n", c, numMovements);

        for (int i = 1; i <= numMovements; i++) // iterate through every instruction
        {
            newX = *p_penX + fontData[cIndex][i][0] * (letterHeight / fontDataLetterSize); // set target x and y values
            newY = *p_penY + fontData[cIndex][i][1] * (letterHeight / fontDataLetterSize);
            isPenDown = fontData[cIndex][i][2]; // set pen up or down

            sValue = isPenDown * 1000; // set "feed rate" value for GCode instrucion

            snprintf(gCodeInstruction, sizeof(gCodeInstruction), "S%d G%d X%.3f Y%.3f\n", sValue, isPenDown, newX, newY); // generate GCode instruction

            addToInstructionBuffer(gCodeInstruction, gCodeBuffer); // add instruction to GCodeBuffer
        }

        *p_penX = newX; // modify the values of penX and penY
        *p_penY = newY;
    }
}



