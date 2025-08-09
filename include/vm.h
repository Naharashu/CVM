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

typedef struct stack16 {
    uint16_t* stack;
    uint8_t sp;
} stack16;

typedef enum __attribute__((packed)){
    CVM_ASM_1_0_0 = 1,
    LOAD,
    MOV,
    IADD,
    ISUB,
    IMUL,
    IDIV,
    CMP,
    INC,
    DEC,
    JNE,
    JE,
    JNZ,
    JZ,
    JMP,
    CALL,
    VOID,
    HALT
} opcodes;



extern VM cvm;
extern stack16 call_stack;
extern uint16_t* heap;

#define FETCH cvm.memory[cvm.pc++]
#define _GOTO_(addr) (cvm.pc=addr)

static void initVM();
static void freeVM();
void push_call(uint8_t value);
uint16_t pop_call();
static void writeHeap(uint16_t data, uint16_t addr);
static uint16_t readHeap(uint16_t addr);
void emit(uint8_t op, uint16_t addr);
void deadcode();
void run();

#endif