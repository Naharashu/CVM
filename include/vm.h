#ifndef _CVM32_
#define _CVM32_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct VM
{
    int_fast32_t* memory;
    int_fast32_t reg[16];
    uint16_t pc;
} VM;

typedef enum {
    CVM_ASM_1_0_0 = 1,
    LOAD,
    MOV,
    CMP,
    INC,
    DEC,
    JNE,
    VOID,
    HALT
} opcodes;



extern VM cvm;
extern uint16_t* heap;

#define FETCH cvm.memory[cvm.pc++]
#define _GOTO_(addr) (cvm.memory[addr])

static void initVM();
static void freeVM();
static void writeHeap(uint16_t data, uint16_t addr);
static uint16_t readHeap(uint16_t addr);
void emit(uint8_t op, uint16_t addr);
void deadcode();
void run();

#endif