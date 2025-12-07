#include <stdio.h>
#include <stdlib.h>
//#include <conio.h>
//#include <windows.h>
#include "rs232.h"
#include "serial.h"

#include "instructionBuffer.h"
#include "functions.h"

#define bdrate 115200               /* 115200 baud */

#define WORD_BUFF_SIZE 1000

void SendCommands (char *buffer );

int main()
{

    //char mode[]= {'8','N','1',0};
    char buffer[100];

    // If we cannot open the port then give up immediately
    if ( CanRS232PortBeOpened() == -1 )
    {
        printf ("\nUnable to open the COM port (specified in serial.h) ");
        exit (0);
    }

    // Time to wake up the robot
    printf ("\nAbout to wake up the robot\n");

    // We do this by sending a new-line
    sprintf (buffer, "\n");
     // printf ("Buffer to send: %s", buffer); // For diagnostic purposes only, normally comment out
    PrintBuffer (&buffer[0]);
    Sleep(100);

    // This is a special case - we wait  until we see a dollar ($)
    WaitForDollar();

    printf ("\nThe robot is now ready to draw\n");

    //These commands get the robot into 'ready to draw mode' and need to be sent before any writing commands
    sprintf (buffer, "G1 X0 Y0 F1000\n");
    SendCommands(buffer);
    sprintf (buffer, "M3\n");
    SendCommands(buffer);
    sprintf (buffer, "S0\n");
    SendCommands(buffer);

    //========================TEXT FILE PROCESSING=============================

    // Read font data into array
    int*** fontData = readFontData("SingleStrokeFont.txt");
    
    // Initialise textHeight variable and get user input
    float textHeight;

    int isInputValid = 1;
    do {
        if (!isInputValid)
        {
            printf("Invalid input. Please try again.\n");
        }
        printf("Text height (4mm - 10mm) > ");
        isInputValid = scanf("%f", &textHeight) && textHeight >= 4.0f && textHeight <= 10.0f;

        while (getchar() != '\n'); //clears the input buffer between attempts

    }   while(!isInputValid); // Makes the user enter another value if an invalid input is detected.

    printf("you have chosen %.2fmm\n", textHeight); 


    // Initialise current word buffer
    char currentWordBuffer[WORD_BUFF_SIZE];

    // Initialise GCode instructions struct
    struct instructionBuffer GCodeBuffer = {0};

    // Open text file
    FILE* textFile = fopen("test.txt", "r");
    if (textFile == NULL)
    {
        printf("Error: Could not open text file.\n");
        exit(2);
    }

    // iterating through the text file character by character
    char c; // current character
    int index = 0; // index for currentWordBuffer

    int isEOF; //flag raised when end of file is reached

    float penX = 0.0, penY = 0; //position of pen (updated by letterToGCode() )

    //loop
    while (1)
    {
        c = fgetc(textFile); // sets c to the next character in the word

        isEOF = (c == EOF);

        if (isEOF || c == ' ' || c == '\r' || c == '\n') // checks for whitespace or EOF to decide if a full word has been read
        {
            if (index > (100 - penX) / textHeight) // checks if the word will move penX past 100mm
            {
                letterToGCode('\n', &GCodeBuffer, fontData, textHeight, &penX, &penY); // enters a newline before converting the word
            }
            for (int i = 0; i < index; i++) // iterates through the characters in the word
            {
                letterToGCode(currentWordBuffer[i], &GCodeBuffer, fontData, textHeight, &penX, &penY); // adds the GCode instructions for the character to GCodeBuffer.data
            }

            for (int i = 0; i < GCodeBuffer.numElements; i++) // iterates through GCodeBuffer.data and sends each command one by one
            {
                SendCommands(GCodeBuffer.data[i]);
            }

            resetInstructionBuffer(&GCodeBuffer); // empties GCodeBuffer so it can be filled with instructions for the next word
            memset(&currentWordBuffer, 0, sizeof(currentWordBuffer)); // empties currentWordBuffer
            index = 0;

            if (isEOF) // stops looping when the end of the file is reached
            {
                break;
            }
        }

        currentWordBuffer[index++] = c; // appends c to the string in currentWordBuffer

        if (index >= 1000) // check for buffer overflow
        {
            printf("Error: Word buffer overflow");
            exit(3);
        }
    }

    // Return the pen to origin
    SendCommands("S0 G0 X0 Y0");

    // Before we exit the program we need to close the COM port
    CloseRS232Port();
    printf("Com port now closed\n");

    return (0);
}

// Send the data to the robot - note in 'PC' mode you need to hit space twice
// as the dummy 'WaitForReply' has a getch() within the function.
void SendCommands (char *buffer )
{
    // printf ("Buffer to send: %s", buffer); // For diagnostic purposes only, normally comment out
    PrintBuffer (&buffer[0]);
    WaitForReply();
    Sleep(100); // Can omit this when using the writing robot but has minimal effect
    // getch(); // Omit this once basic testing with emulator has taken place
}
