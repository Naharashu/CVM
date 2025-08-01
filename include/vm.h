#ifndef _CVM32_
#define _CVM32_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct VM
{
    int32_t* memory;
    uint_fast8_t reg[16];
    uint16_t pc;
} VM;

typedef enum {
    CVM_ASM_1_0_0 = 1,
    LOAD,
    MOV,
    HALT
} opcodes;

uint16_t* heap;

#define FETCH cvm.memory[cvm.pc++]
#define _GOTO_(addr) (cvm.memory[addr])

static void initVM();
static void freeVM();
static void writeHeap(uint16_t data, uint16_t addr);
static uint16_t readHeap(uint16_t addr);
void run();

#endif