#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
//defining memory for the LC3 VM
#define MEMORY_MAX (1 << 16)
uint16_t memory[MEMORY_MAX];


//defining CPU registers for the LC3 VM
enum
{ 
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, /* program counter */
    R_COND,
    R_COUNT
};
uint16_t reg[R_COUNT];

//condition flags for the R_COND register
enum
{
    FL_POS = 1 << 0, /* P */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};

//defining opcodes for the LC3 VM
//4 left bits are instructions,12 bits for task parametres
enum
{
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
}; 


//to be moved to a different file
//this extends the value x to a 16-bit value that is signed
uint16_t sign_extend(uint16_t x , int bit_count){
    if((x >> (bit_count - 1)) & 1)
        x|=(0xFFFF << bit_count);
    return x;
}

//update the condition flag register with each operation based on the DR
void update_flags(uint16_t r){
    if(reg[r] == 0)
        reg[R_COND] = FL_ZRO;
    else if(reg[r] >> 15)
        reg[R_COND] = FL_NEG;
    else
        reg[R_COND] = FL_POS;
}


//Add instruction layout :4-bit/3-bit/3-bit/1-bit/5-bit or 2-bit/3-bit
// 4-bit op_code/ 3-bit DR (destination register)/
// 3-bit register countaing first value/ 1-bit immediate mode flag
// if immediate mode is 1 the rest 5-bits are to be treated as a direct value(with sign extending)
// if immediate mode is 0  next 2-bits are unused and rest 3-bits are for the second register tha holds the second value
void vm_add(uint16_t instr){
    //register for the DR
    uint16_t r0 = (instr >> 9) & 0x7;
    //first operand (param)
    uint16_t r1 = (instr >> 6) & 0x7;
    //check for the immediate flag
    uint16_t imm_flag = (instr >> 5) & 0x1;
    if(imm_flag){
        uint16_t imm5 = sign_extend(instr & 0x1F,5);
        reg[r0] = reg[r1] + imm5;
    }
    else{
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] + reg[r2];
    }
    update_flags(r0);
}

//And instruction layout : 4-bit/3-bit/3-bit/1-bit/5-bit or 2-bit/3-bit
//same as the Add instruction but the result is calculated by bitwise anding the two params
void vm_and(uint16_t instr){
    //get DR
    uint16_t r0 = (instr >> 9) & 0x7;
    //get R1
    uint16_t r1 = (instr >> 6) & 0x7;
    //get immediate flag
    uint16_t imm_flag =(instr >> 5) & 0x1;
    if(imm_flag){
        uint16_t imm5 = sign_extend(instr & 0x1F,5);
        reg[r0] = reg[r1] & imm5;
    }
    else{
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] & reg[r2];
    }
    update_flags(r0);
}
//the conditional branch layout is : 4-bit/1-bit/1-bit/1-bit/9-bit
// first 4-bits are for the opcode and next 3-bits are n/z/p to be tested
// for each one set we test it's counterpart from the conditional register
//if n or z or p is set and it's counterpart then we offset the program counter
//by the value of the rest sign-extended 9-bits (PCoffset9)
void vm_br(uint16_t instr){
    uint16_t pc_offset = sign_extend(instr & 0x1FF,9);
    uint16_t cond_flag = (instr >> 9) & 0x7;
    if(cond_flag & reg[R_COND])
        reg[R_PC] += pc_offset;
}
//LDI is better than LD because it can have 16-bit full adresses rather
//than the 9-bits that are in the instruction param and this is useful for
//farther addresses from the PC
//LDI loads a value in memory to a register
//the way it is layed out : 4-bit op_code/ 3-bit DR / 9-bit for the address 
//near the pc that holds an address to the actual target value
void vm_load_indirect(uint16_t instr){
    // the DR register
    uint16_t r0 = (instr >> 9) & 0x7;
    // offset of memory address from PC
    uint16_t pc_offset = sign_extend(instr & 0x1FF , 9);
    //next line can be thought of as dereferencing a double pointer
    reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
    update_flags(r0);
}





int main(int argc,char* argv[]){
    if (argc < 2){
        //show usage string
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }
    for(int j = 1; j < argc; ++j){
        if(!read_image(argv[j])){
            printf("failed to load image: %s\n",argv[j]);
            exit(1);
        }
    }
    //since exactly one condition flag should be set at any given time, set the Z flag 
    reg[R_COND] = FL_ZRO;
    /* set the PC to starting position 
     0x3000 is the default */
    enum {PC_START = 0x3000};
    reg[R_PC] = PC_START;
    char running = 1;
    while(running){
        //Fethcing the instruction from PC then incrementing it
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;//left 4 bits of the instr is the opcode rest 12 are params

        switch(op){
            case OP_ADD:
                vm_add(instr);
                break;
            case OP_AND:

                break;
            case OP_NOT:

                break;
            case OP_BR:

                break;
            case OP_JMP:

                break;
            case OP_JSR:

                break;
            case OP_LD:

                break;
            case OP_LDI:
                vm_load_indirect(instr);
                break;
             case OP_LDR:

                break;
            case OP_LEA:

                break;
            case OP_ST:

                break;
            case OP_STI:

                break;
            case OP_STR:

                break;
            case OP_TRAP:

                break;
            case OP_RES:

            case OP_RTI:

            default:

                break;
        }

    }

	return 0;
}