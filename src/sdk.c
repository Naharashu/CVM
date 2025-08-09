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

/*
void include(FILE* f) {
    if (!f) {
        fprintf(stderr, "Error opening file\n");
        return;
    }
    int_fast32_t value;
    while (fscanf(f, "%d", &value) == 1) {
        cvm.memory[cvm.pc++] = value;
    }
    fclose(f);
}
*/
void include(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error opening file\n");
        return;
    }
    int_fast32_t value;
    while (fscanf(f, "%d", &value) == 1) {
        cvm.memory[cvm.pc++] = value;
    }
    fclose(f);
}