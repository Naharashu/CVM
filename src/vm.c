#include "../include/vm.h"

// * VM managment

VM cvm;

static void initVM() {
    cvm.memory = calloc(UINT16_MAX+1, sizeof(int32_t));
    cvm.pc = 1;
    for (size_t i = 0; i < 16; i++)
    {
        cvm.reg[i] = 0;
    }
    cvm.memory[0] = CVM_ASM_1_0_0;
    heap = calloc(2048, sizeof(uint16_t));
}

static void freeVM() {
    free(cvm.memory);
    free(heap);
}



// * heap managment

static void writeHeap(uint16_t data, uint16_t addr) {
    heap[addr] = data;
}

static uint16_t readHeap(uint16_t addr) {
    uint16_t data = heap[addr];
    return data;
}

// * Bytecode mamagment
// TODO: add more bytecodes + optimizations(01.08.25)
// ! may be slow

void emit(uint8_t op, uint16_t addr) {
    cvm.memory[addr] = op;
}

void run() {
    register int16_t i;
    for(;;) {
        i = FETCH;
        switch(i) {
            case LOAD: {
                uint8_t r = FETCH;
                int16_t val = FETCH;
                cvm.reg[r] = val;
                break;
            }
            case MOV: {
                uint_fast8_t r = FETCH;
                uint_fast8_t r1 = FETCH;
                cvm.reg[r] = cvm.reg[r1];
                break;
            }
            case HALT: return;
            default: return;
        }
    }
}

int main(int argc, char *argv[]) {
    initVM();
    int32_t mem[] = {
        LOAD, 1, 200,
        MOV, 0, 1,
        HALT
    };
    memcpy(&cvm.memory[1], mem, sizeof(mem));
    run();
    printf("%d\n", cvm.reg[0]);
    freeVM();
    return 0;
}