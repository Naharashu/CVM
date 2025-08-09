#include "../include/vm.h"
#include "../include/sdk.h"

// ! Declarations

VM cvm;
stack16 call_stack;
uint16_t* heap = NULL;
uint8_t opt = 0;
bool ZF = false;
bool EF = false;

// * VM managment


static void initVM() {
    cvm.memory = calloc(UINT16_MAX+1, sizeof(int_fast32_t));
    call_stack.stack = calloc(256, sizeof(uint16_t));
    call_stack.sp = 0;
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
    free(call_stack.stack);
}

// * Stack managment

void push_call(uint8_t value) {
    call_stack.stack[call_stack.sp] = value;
    call_stack.sp++;
}

uint16_t pop_call() {
    call_stack.sp--;
    return call_stack.stack[call_stack.sp];
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
// TODO:  upgrade stack(09.08.25)
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

        if(i+2 < UINT16_MAX+1 && mem[i] == VOID && mem[i+1] == VOID && mem[i+2] == VOID) {
            emit(0, i+1);
            emit(0, i+2);
        }

        if(mem[i] == HALT && opt == 1) {
            for(int j = i+1; j < UINT16_MAX+1; j++) {
                if(mem[j] != 0) mem[j] = 0;
            }
        }
    }
    #undef mem
}

void emit(uint8_t op, uint16_t addr) {
    cvm.memory[addr] = op;
}


void run_CALL(int limit) {
    while(cvm.pc < limit)  {
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
            case IADD: {
                uint8_t r = FETCH;
                int32_t imm = FETCH;
                opt == 0 ? writeHeap(cvm.reg[r], 1) : 0; // ? If opt= 0 we caching last value of register in heap
                cvm.reg[r] += imm;
                goto next;
            }
            case ISUB: {
                uint8_t r = FETCH;
                int32_t imm = FETCH;
                opt == 0 ? writeHeap(cvm.reg[r], 2) : 0;
                cvm.reg[r] -= imm;
                goto next;
            }
            case IMUL: {
                uint8_t r = FETCH;
                int32_t imm = FETCH;
                opt == 0 ? writeHeap(cvm.reg[r], 3) : 0;
                cvm.reg[r] *= imm;
                goto next;
            }
            case IDIV: {
                uint8_t r = FETCH;
                int32_t imm = FETCH;
                opt == 0 ? writeHeap(cvm.reg[r], 4) : 0;
                cvm.reg[r] /= imm;
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
                    _GOTO_(addr);
                }
                goto next;
            }
            case JE: {
                uint16_t addr = FETCH;
                if (EF) {
                    _GOTO_(addr);
                }
                goto next;
            }
            case JNZ: {
                uint16_t addr = FETCH;
                if (!ZF) {
                    _GOTO_(addr);
                }
                goto next;
            }
            case JZ: {
                uint16_t addr = FETCH;
                if (ZF) {
                    _GOTO_(addr);
                }
                goto next;
            }
            case CALL: {
                uint_fast16_t addr1 = FETCH;
                uint_fast16_t addr2 = FETCH;
                push_call(cvm.pc);
                cvm.pc = addr1;
                run_CALL(addr2);
                cvm.pc = pop_call();
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
            case IADD: {
                uint8_t r = FETCH;
                int32_t imm = FETCH;
                opt == 0 ? writeHeap(cvm.reg[r], 1) : 0; // ? If opt= 0 we caching last value of register in heap
                cvm.reg[r] += imm;
                goto next;
            }
            case ISUB: {
                uint8_t r = FETCH;
                int32_t imm = FETCH;
                opt == 0 ? writeHeap(cvm.reg[r], 2) : 0;
                cvm.reg[r] -= imm;
                goto next;
            }
            case IMUL: {
                uint8_t r = FETCH;
                int32_t imm = FETCH;
                opt == 0 ? writeHeap(cvm.reg[r], 3) : 0;
                cvm.reg[r] *= imm;
                goto next;
            }
            case IDIV: {
                uint8_t r = FETCH;
                int32_t imm = FETCH;
                opt == 0 ? writeHeap(cvm.reg[r], 4) : 0;
                cvm.reg[r] /= imm;
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
                    _GOTO_(addr);
                }
                goto next;
            }
            case JE: {
                uint16_t addr = FETCH;
                if (EF) {
                    _GOTO_(addr);
                }
                goto next;
            }
            case JNZ: {
                uint16_t addr = FETCH;
                if (!ZF) {
                    _GOTO_(addr);
                }
                goto next;
            }
            case JZ: {
                uint16_t addr = FETCH;
                if (ZF) {
                    _GOTO_(addr);
                }
                goto next;
            }
            case JMP: {
                uint16_t addr = FETCH;
                _GOTO_(addr);
                goto next;
            }
            case CALL: {
                uint_fast16_t addr1 = FETCH;
                uint_fast16_t addr2 = FETCH;
                push_call(cvm.pc);
                cvm.pc = addr1;
                run_CALL(addr2);
                cvm.pc = pop_call();
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
    if(argc > 1 && strcmp(argv[1], "-O1") == 0) {
        opt = 1;
    }
    initVM();
    int_fast32_t memor[] = {
        INC, 0,
        CALL, 1, 2,
        CALL, 1, 2,
        CALL, 3, 8,
        HALT
    };
    memcpy(&cvm.memory[1], memor, sizeof(memor));
    deadcode();
    bytecode("out", (uint_fast8_t[]){0});
    run();
    printf("%d\n", cvm.reg[0]);
    freeVM();
    return 0;
}
