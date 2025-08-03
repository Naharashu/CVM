#include "../include/sdk.h"

void bytecode(char* param, uint_fast8_t params[]) {
    if(strcmp(param, "out")==0) {
        for (size_t i = 0; i < UINT16_MAX; i++)
        {
            printf("(%d)\n", cvm.memory[i]);
            if((cvm.memory[i] == HALT && cvm.memory[i+1] == 0) || (cvm.memory[i] == 0 && cvm.memory[i+1] == 0 && cvm.memory[i+2] == 0)) break;
        }
    }
    if(strcmp(param, "in")==0) {
        cvm.memory[params[0]] = params[1];
    }
}