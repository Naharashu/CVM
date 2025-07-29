#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void error(char msg[64]);

/*  CVM 1.0.0.0
    LICENSE: MIT
    AUTHOR: Naharashu
*/

// VM

#ifndef _CVM32_
#define _CVM32_
#define FETCH cvm.memory[cvm.pc++]
#endif

typedef enum {
    NOP = 1,
    LOAD,
    ADD,
    SUB,
    MUL,
    DIV,
    AND,
    NOT,
    OR,
    XOR,
    ADDI,
    SUBI,
    MULI,
    DIVI,
    JMP,
    JNZ,
    JZ,
    JBT,
    JST,
    JNE,
    JE,
    JBE,
    JSE,
    CMP,
    INC,
    STOREINT,
    GET,
    HALT,
    OP_SIGN
} opcode;

typedef struct VM {
    int32_t reg[16];
    uint32_t pc;
    int32_t* memory;
    uint8_t stack[256];
    uint8_t opt;
} VM;

typedef struct __attribute__((packed)) {
    char name[16];
    int16_t L1[64];
    int16_t L2[128];
    int32_t L3[256];
} CACHE;

bool ZF = false;
bool SF = false;
bool CF = false;
bool OF = false;
bool UF = false;
bool IDF = false;

// CVM_Objects

typedef enum {
    _U8_,
    _U16_,
    _U32_,
    _INT8_,
    _INT16_,
    _INT32_,
    _FLOAT_,
    _DOUBLE_,
    _BOOL_,
    _NULL_,
    _UNDEFINED_,
    _CHAR_,
    _STRING_,
    _Obj_
} CVM_Obj_Type;




typedef struct CVM_Obj {
    CVM_Obj_Type type;
    union {
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        int8_t i8;
        int16_t i16;
        int32_t i32;
        float f;
        bool b;
        void* obj;
        char ch;
        char* string;
    } value;
    size_t size;
    bool isNone;
} CVM_Obj;

typedef struct __attribute__((packed)) {
    uint8_t type;
    union {
        uint32_t u32;
        int32_t i32;
        float f;
        void* obj;
        char* string;
    } value;
} CVM_Obj_Packed;

void compileObj(CVM_Obj* obj) {
    obj->isNone = false;
    switch(obj->type) {
        case _U8_: obj->size = sizeof(uint8_t); break;
        case _U16_: obj->size = sizeof(uint16_t); break;
        case _U32_: obj->size = sizeof(uint32_t); break;
        case _INT8_: obj->size = sizeof(int8_t); break;
        case _INT16_: obj->size = sizeof(int16_t); break;
        case _INT32_: obj->size = sizeof(int32_t); break;
        case _FLOAT_: obj->size = sizeof(float); break;
        case _BOOL_: obj->size = sizeof(bool); break;
        case _CHAR_: obj->size = sizeof(char); break;
        case _STRING_: obj->size = sizeof(char*); break;
        case _NULL_: {
            obj->size = 0; 
            obj->isNone = true;
            break;
        }    
        case _UNDEFINED_: obj->size = -1; break;
        case _Obj_: obj->size = sizeof(obj->value.obj); break;
    }
}

CVM_Obj_Packed packObj(CVM_Obj* obj) {
    CVM_Obj_Packed a;
    a.type = obj->type;
    switch(a.type) {
        case _U8_:
        case _U16_:
        case _U32_: a.value.u32 = obj->value.u32; break;
        case _INT8_:
        case _INT16_:
        case _INT32_: a.value.i32 = obj->value.i32; break;
        case _FLOAT_: a.value.f = obj->value.f; break;
        case _STRING_: {
            obj->value.string = strcat(obj->value.ch, obj->value.string);
            a.value.string = obj->value.string;
            free(obj->value.string);
            break;
        }
    }
    return a;
}

CVM_Obj unpackObj(CVM_Obj_Packed* obj) {
    CVM_Obj a;
    a.type = obj->type;
    switch(a.type) {
        case _U8_:
        case _U16_:
        case _U32_: a.value.u32 = obj->value.u32; break;
        case _INT8_:
        case _INT16_:
        case _INT32_: a.value.i32 = obj->value.i32; break;
        case _FLOAT_: a.value.f = obj->value.f; break;
        case _STRING_: {
            a.value.string = obj->value.string;
            break;
        }
    }
    compileObj(&a);
    return a;
}

bool isEmptyObj(CVM_Obj obj) {
    if(obj.isNone == true) return true;
    if(obj.type == _NULL_) {
        obj.isNone = true;
        return true;
    }
    return false;
}


// CVM

VM cvm;
CACHE vmc;

// System tools and cache

static void storeL1(uint8_t addr, int16_t val) {
    vmc.L1[addr] = val;
    return;
}

static int16_t readL1(uint8_t addr) {
    return vmc.L1[addr];
}

static void writeL3(uint8_t addr, int32_t val) {
    vmc.L3[addr] = val;
    return;
}

static void readL3(uint8_t addr, uint8_t reg) {
    cvm.reg[reg] = vmc.L3[addr];
    return;
}

// CVM-Bytecode 


int init(uint8_t security) {
    // VM
    for(int i = 0; i < 16; i++) cvm.reg[i] = 0;
    cvm.memory = (uint32_t*)calloc(1 << 18, sizeof(uint32_t));
    cvm.memory[0] = OP_SIGN;
    cvm.pc = 0;
    if(security == 1) cvm.memory[262143] = HALT;

    // CACHE
    strcpy(vmc.name, "CVM-SCS");
    if(security == 1) (strcmp(vmc.name, "CVM-SCS") == 0) ? 0 : error("Used non-cvm system of cache");
    for(int i = 0; i < 64; i++) vmc.L1[i] = 0;
    for(int i = 0; i < 128; i++) vmc.L2[i] = 0;
    for(int i = 0; i < 256; i++) vmc.L3[i] = 0;

    return 0;
}


static void inline_constant() {
    #define mem cvm.memory
    if (cvm.opt != 1) return;

    for (int j = 0; j < 262140; j++) {
        if (mem[j] == LOAD && mem[j+3] == LOAD &&
        (mem[j+6] == ADD || mem[j+6] == SUB || mem[j+6] == MUL || mem[j+6] == DIV)) {
        uint8_t r1 = mem[j+1];
            uint8_t r2 = mem[j+4];
            int32_t val1 = mem[j+2];
            int32_t val2 = mem[j+5];
            int32_t val3 = 0;
            uint8_t op = mem[j+6];
            if (mem[j+7] == r1 && mem[j+8] == r2) {
                switch(op) {
                case ADD:
                    val3 = val1 + val2;
                    break;
                case SUB:
                    val3 = val1 - val2;
                    break;
                case MUL:
                    val3 = val1 * val2;
                    break;
                case DIV:
                    val3 = val1 / val2;
                    break;
                }
                mem[j] = LOAD;
                mem[j+1] = r1;
                mem[j+2] = val3;

                mem[j+3] = NOP;
                mem[j+4] = NOP;
                mem[j+5] = NOP;
                mem[j+6] = NOP;
                mem[j+7] = NOP;
                mem[j+8] = NOP;
            }
        }
    }

    #undef mem
}



/*

static void inc_loop() {
    if(cvm.opt != 1) return;
    #define mem cvm.memory
    for (int j = 0; j < 262140; j++) {
        if (mem[j] == INC && mem[j+3] == JMP) {
            uint8_t r1 = mem[j+1];
            uint8_t addr = mem[j+4];
            if (mem[j+7] == r1 && mem[j+8] == r2) {
                switch(op) {
                case ADD:
                    val3 = val1 + val2;
                    break;
                case SUB:
                    val3 = val1 - val2;
                    break;
                case MUL:
                    val3 = val1 * val2;
                    break;
                case DIV:
                    val3 = val1 / val2;
                    break;
                }
                mem[j] = ADDI;
                mem[j+1] = r1;
                mem[j+2] = 100;

                mem[j+3] = 0;
                mem[j+4] = 0;
            }
        }
    }
    #undef mem
}
*/

int run() {
    if(cvm.memory[0] != OP_SIGN) {
        printf("Used bytecode without CVM32 signature!\n");
        exit(1);
    }
    cvm.pc++;
    register uint32_t i;
    for(;;) {
        i = FETCH;
        switch(i) {
        case LOAD: {
            uint8_t r = FETCH;
            int32_t val = FETCH;
            cvm.reg[r] = val;
            break;
        }
        case STOREINT: {
            uint8_t addr = FETCH;
            writeL3(addr, FETCH);
            break;
        }
        case GET: {
            uint8_t r = FETCH;
            readL3(FETCH, r);
            break;
        }
        case ADD: {
            uint8_t r = FETCH;
            uint8_t r_ = FETCH;
            cvm.reg[r] += cvm.reg[r_];
            break;
        }
        case ADDI: {
            uint8_t r = FETCH;
            cvm.reg[r] += cvm.reg[FETCH];
            break;
        }
        case SUB: {
            uint8_t r = FETCH;
            uint8_t r_ = FETCH;
            cvm.reg[r] -= cvm.reg[r_];
            break;
        }
        case SUBI: {
            uint8_t r = FETCH;
            uint8_t r_ = FETCH;
            cvm.reg[r] -= cvm.reg[r_];
            break;
        }
        case MUL: {
            uint8_t r = FETCH;
            uint8_t r_ = FETCH;
            cvm.reg[r] *= cvm.reg[r_];
            break;
        }
        case MULI: {
            uint8_t r = FETCH;
            uint8_t r_ = FETCH;
            cvm.reg[r] *= cvm.reg[r_];
            break;
        }
        case DIV: {
            uint8_t r = FETCH;
            uint8_t r_ = FETCH;
            cvm.reg[r] /= cvm.reg[r_];
            break;
        }
        case DIVI: {
            uint8_t r = FETCH;
            uint8_t r_ = FETCH;
            cvm.reg[r] /= cvm.reg[r_];
            break;
        }
        case AND: {
            uint8_t r = FETCH;
            uint8_t r_ = FETCH;
            cvm.reg[r] &= cvm.reg[r_];
            break;
        }
        case NOT: {
            uint8_t r = FETCH;
            uint8_t r_ = FETCH;
            cvm.reg[r] = ~cvm.reg[r_];
            break;
        }
        case OR: {
            uint8_t r = FETCH;
            uint8_t r_ = FETCH;
            cvm.reg[r] |= cvm.reg[r_];
            break;
        }
        case XOR: {
            uint8_t r = FETCH;
            uint8_t r_ = FETCH;
            cvm.reg[r] ^= cvm.reg[r_];
            break;
        }
        case JMP: {
            uint32_t addr = FETCH;
            cvm.pc = addr;
            break;
        }
        case JZ: {
            uint32_t r = FETCH;
            uint32_t addr = FETCH;
            if(!(ZF)) continue;
            cvm.pc = addr;
            break;
        }
        case JNZ: {
            uint32_t r = FETCH;
            uint32_t addr = FETCH;
            if(ZF) continue;
            cvm.pc = addr;
            break;
        }
        case JE: {
            uint32_t addr = FETCH;
            if(IDF) cvm.pc = addr;
            break;
        }
        case JNE: {
            uint32_t addr = FETCH;
            if(!(IDF)) cvm.pc = addr;
            break;
        }
        case JST: {
            uint32_t r = FETCH;
            uint32_t r_ = FETCH;
            uint32_t addr = FETCH;
            if(cvm.reg[r] < cvm.reg[r_]) continue;
            cvm.pc = addr;
            break;
        }
        case JBT: {
            uint32_t r = FETCH;
            uint32_t r_ = FETCH;
            uint32_t addr = FETCH;
            if(cvm.reg[r] > cvm.reg[r_]) continue;
            cvm.pc = addr;
            break;
        }
        case JSE: {
            uint32_t r = FETCH;
            uint32_t r_ = FETCH;
            uint32_t addr = FETCH;
            if(cvm.reg[r] <= cvm.reg[r_]) continue;
            cvm.pc = addr;
            break;
        }
        case JBE: {
            uint32_t r = FETCH;
            uint32_t r_ = FETCH;
            uint32_t addr = FETCH;
            if(cvm.reg[r] >= cvm.reg[r_]) continue;
            cvm.pc = addr;
            break;
        }
        case CMP: {
            uint32_t r = FETCH;
            uint32_t _r = FETCH;     
            int32_t v = cvm.reg[r];
            int32_t v_ = cvm.reg[_r];
            int32_t res = v - v_;
            ZF = (res == 0);
            SF = (res < 0);
            UF = (res > 0);
            CF = ((uint32_t)v < (uint32_t)v_);
            OF = ((v > 0 && v_ < 0 && res < 0) || (v < 0 && v_ > 0 && res > 0));
            IDF = (v == v_);
            break;
        }
        case INC: {
            cvm.reg[FETCH]++;
            break;
        }
        case HALT:
            return 0;
        case NOP:
            break;
        default:
            return 1;
        }
    }
}

// Debug and tools for core

void bytecode(char* op, uint32_t addr, int32_t val) {
    if(strcmp(op, "out") == 0) {
        for(int32_t j = 0; j < 262144; j++) {
            if(cvm.memory[j] != 0) printf("%d\n", cvm.memory[j]);
        }
    }
    if(strcmp(op, "in") == 0) {
        cvm.memory[addr] = val;
    }
    if(strcmp(op, "r") == 0) {
        printf("%d\n", cvm.memory[addr]);
    }
}

void registers(char* op, uint8_t n, int32_t val) {
    if(n > 16) error("Register index out of range(16)");
    if(strcmp(op, "out") == 0) {
        for(uint8_t j = 0; j < 16; j++) {
            printf("%d\n", cvm.reg[j]);
        }
    }
    if(strcmp(op, "in") == 0) {
        cvm.reg[n] = val;
    }
    if(strcmp(op, "r") == 0) {
        printf("%d\n", cvm.reg[n]);
    }
}

void sign(int32_t* mem) {
    mem[0] = OP_SIGN;
}

void error(char msg[64]) {
    printf(msg);
    printf("\n");
    exit(1);
}

void errorno(uint8_t c, uint32_t d) {
    switch(c) {
    case 0:
        printf("External system error\n");
        break;
    case 1:
        printf("Incorrect bytecode No: %d\n", d);
        break;
    case 2:
        printf("Unknown internal error\n");
        break;
    case 3:
        printf("Program is to big, limit size of memory: %d MB\n", 1);
        break;
    }
    exit(1);
}

void consume(uint8_t op, uint8_t op2, char msg[128]) {
    if(op != op2) {
        printf(msg);
        printf("\n");
        exit(1);
    }
    return;
}




// Main




int main()
{
    init(0);
    cvm.opt = 0;

    int32_t program[] __attribute__((aligned(32)))= {
        LOAD, 0, 1,
        LOAD, 1, 100000000,
        CMP, 0, 1,
        INC, 0,
        JNE, 7,
        HALT
    };

    memcpy(&cvm.memory[1], program, sizeof(program));
    //inline_constant();
    run();
    bytecode("out", 0,0);
    printf("(%d)\n", cvm.reg[0]);

    free(cvm.memory);
    return 0;
}