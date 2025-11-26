#ifndef FUNCTIONS_H
#define FUNCTIONS_H

int*** readFontData(const char* fileName);

void letterToGCode(char c, struct instructionBuffer* gCodeBuffer, int***fontData, float letterHeight, float* p_penX, float* p_penY);

#endif