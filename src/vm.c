#include "../include/vm.h"
#include "../include/sdk.h"

// ! Declarations

VM cvm;
uint16_t* heap = NULL;
bool ZF = false;
bool EF = false;

// * VM managment


static void initVM() {
    cvm.memory = calloc(UINT16_MAX+1, sizeof(int_fast32_t));
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

void deadcode() {
    #define mem cvm.memory
    for(int i = 0; i < UINT16_MAX+1; i++) {
        if(i+5 < UINT16_MAX+1 && mem[i] == LOAD && mem[i+3] == LOAD && mem[i+1] == mem[i+4] &&  mem[i+2] == mem[i+5]) {
            emit(0, i+3);
            emit(0, i+4);
            emit(0, i+5);
        }
        if(i+3 < UINT16_MAX+1 && mem[i] == INC && mem[i+2] == DEC && mem[i+1] == mem[i+3]) {
            emit(VOID, i);
            emit(VOID, i+1);
            emit(VOID, i+2);
            emit(VOID, i+3);
        }
        /*
        if(mem[i] == VOID) {
            mem[i] = 0;
        }
        */
    }
    #undef mem
}

void emit(uint8_t op, uint16_t addr) {
    cvm.memory[addr] = op;
}



void run() {
    for (;;) {
        int op = FETCH;
        switch (op) {
            case LOAD: {
                uint8_t r = FETCH;
                int32_t val = FETCH;
                cvm.reg[r] = val;
                goto next;
            }
            case MOV: {
                uint8_t r = FETCH;
                uint8_t r1 = FETCH;
                cvm.reg[r] = cvm.reg[r1];
                goto next;
            }
            case CMP: {
                uint8_t r = FETCH;
                uint8_t r1 = FETCH;
                EF = cvm.reg[r] == cvm.reg[r1];
                ZF = cvm.reg[r] == 0;
                goto next;
            }
            case INC: {
                uint8_t r = FETCH;
                cvm.reg[r]++;
                goto next;
            }
            case DEC: {
                uint8_t r = FETCH;
                cvm.reg[r]--;
                goto next;
            }
            case JNE: {
                uint16_t addr = FETCH;
                if (!EF) {
                    cvm.pc = addr;
                }
                goto next;
            }
            case VOID:
                goto next;

            case HALT:
                return;

            default:
                printf("Unknown opcode: %d at %d\n", op, cvm.pc - 1);
                return;
        }
    next:
        continue;
    }
}


int main(int argc, char *argv[]) {
    initVM();
    int_fast32_t memor[] = {
        LOAD, 1, 100000000,
        CMP, 0, 1,
        INC, 0, 
        JNE, 4,
        HALT
    };
    memcpy(&cvm.memory[1], memor, sizeof(memor));
    //deadcode();
    bytecode("out", 0);
    run();
    printf("%d\n", cvm.reg[0]);
    freeVM();
    return 0;
}