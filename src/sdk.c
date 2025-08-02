#include "../include/sdk.h"

void bytecode(char* param, uint_fast8_t* params) {
    if(strcmp(param, "out")==0) {
        for (size_t i = 0; i < UINT16_MAX+1; i++)
        {
            cvm.memory[i] != 0 ? printf("(%d)\n", cvm.memory[i]) : 0;
        }
    }
}