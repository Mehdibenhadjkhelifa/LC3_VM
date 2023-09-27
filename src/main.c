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


//Add instruction layout :4-bit/3-bit/3-bit/1-bit/ (5-bit or 2-bit/3-bit)
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

//And instruction layout : 4-bit/3-bit/3-bit/1-bit/ (5-bit or 2-bit/3-bit)
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

//the conditional branch instruction layout is : 4-bit/1-bit/1-bit/1-bit/9-bit
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

//the jump instruction layout is : 4-bit/3-bit/3-bit/6-bit
//4 first bits are for opcode next 3 bits and last 6 bits are unused
// second 3-bit section contains the register (baseR) which the program counter
//will jump to unconditionnaly
//this function is also called RET when the register specified is of 0x7 value (R7 register)
void vm_jmp(uint16_t instr){
    //get baseR
    uint16_t r0=(instr >> 6) & 0x7;
    reg[R_PC] = reg[r0];
}

//the jump register instruction layout is : 4-bit/1-bit/ (11-bit or 2-bit/3-bit/6-bit)
//first 4 bits for opcode,next 1-bit if set(=1) we use the 11-bit by sign extending it and 
//adding it to the program counter as an offset else we use the 3-bit section as a register 
//and assign the program counter to the value hold by the specified register as an address
//we hold for the program counter before any operation in the 8th register(R7) as a linkage 
//to the calling routine. this instruction makes the program jump to a subroutine.
void vm_jsr(uint16_t instr){
    //save the program counter before change to recall it later
    reg[R_R7] = reg[R_PC];
    uint16_t offset_flag = (instr >> 11) & 0x1;
    if(offset_flag){
        //sign extending the offset value
        uint16_t pc_offset = sign_extend(instr & 0x7FF,11);
        reg[R_PC] += pc_offset;
    }
    else{
        // obtaining baseR register
        uint16_t r0 = (instr >> 6) & 0x7;
        reg[R_PC] = reg[r0];
    }
}

//the load instructin layout is : 4-bit/3-bit/9-bit
//4-bit for opcode , 3-bit for destination register(DR)
//9-bit for program counter offset(PCoffset9)
//this instruction load the destination register with the value read from
//the memory address of the program counter + pc offset 
void vm_load(uint16_t instr){
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF,9);
    reg[r0] = mem_read(reg[R_PC] + pc_offset);
    update_flags(r0);
}

//LDI is better than LD because it can have 16-bit full adresses rather
//than the 9-bits that are in the instruction param and this is useful for
//farther addresses from the PC
//LDI loads a value in memory to a register
//the way it is layed out : 4-bit op_code/ 3-bit DR / 9-bit for the address 
//near the pc that holds an address to the actual target value
//LDI loads the destination register with the value at the memory address pointed
//to by the memory address located in the address of program counter + pc offset
void vm_load_indirect(uint16_t instr){
    // the DR register
    uint16_t r0 = (instr >> 9) & 0x7;
    // offset of memory address from PC
    uint16_t pc_offset = sign_extend(instr & 0x1FF , 9);
    //next line can be thought of as dereferencing a double pointer
    reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
    update_flags(r0);
}

//LDR or load register instruction layout is : 4-bit/3-bit/3-bit/6-bit
//4-bit opcode , 3-bit Destination register(DR),3-bit base register(BaseR),6-bit offset
//to be sign-extended and added to the value held by the BaseR,then we load the DR value pointed to
//by the address calculated with the last mentioned operation(BaseR + offset6)
void vm_load_register(uint16_t instr){
    //DR
    uint16_t r0 = (instr >> 9) & 0x7;
    //baseR
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F,6);
    reg[r0] = mem_read(reg[r1] + offset);
    update_flags(r0);
}

// LEA known as load effective address layout is: 4-bit/3-bit/9-bit
//4-bit opcode , 3-bit Destination register , 9-bit PCoffset9
//this instruction loads the DR with program counter + pc offset(sign extended)
void vm_lea(uint16_t instr){
    //DR
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF,9);
    reg[r0] = reg[R_PC] + pc_offset;
    update_flags(r0);
}

//bitwise not instruction layout : 4-bit/3-bit/3-bit/1-bit/5-bit
//4-bit opcode , 3-bit DR,3-bit SR, 1-bit and 5-bit are unused(set to 1 by default)
//this instruction stores the bitwise complement content of SR in DR 
void vm_not(uint16_t instr){
    //DR
    uint16_t r0 = (instr >> 9) & 0x7;
    //SR
    uint16_t r1 = (instr >> 6) & 0x7;
    reg[r0] = ~r1;
    update_flags(r0);
}

//the store instruction layout is:4-bit/3-bit/9-bit
//4-bit opcode,3-bit SR,9 bit PCoffset to be sign extended
//this instruction stores the content of the source register
//in the memory address pointed to by calculating program counter + pc offset
void vm_store(uint16_t instr){
    //SR
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset= sign_extend(instr & 0x1FF,9);
    mem_write(reg[R_PC] + pc_offset , reg[r0]);
}

//the store indirect instruction layout is:4-bit/3-bit/9-bit
//4-bit opcode,3-bit SR,9 bit PCoffset to be sign extended
//this instruction stores the content of the source register
//in the memory address pointed to by the value in the 
//memory address calculated by program counter + pc offset
void vm_store_indirect(uint16_t instr){
    //SR
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF,9);
    mem_write(mem_read(reg[R_PC] + pc_offset),reg[r0]);
}

//Store register instruction layout : 4-bit/3-bit/3-bit/6-bit
//4-bit opcode,3-bit SR,3-bit baseR,6-bit offset
//this instruction stores the content of the SR in the memory
//address calculated by adding the offset (after sign extending it)
// to the content of register baseR 
void vm_store_register(uint16_t instr){
    //SR
    uint16_t r0 = (instr >> 9) & 0x7;
    //baseR
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F,6);
    mem_write(reg[r1] + offset,reg[r0]);
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
                vm_and(instr);
                break;
            case OP_NOT:
                vm_not(instr);
                break;
            case OP_BR:
                vm_br(instr);
                break;
            case OP_JMP:
                vm_jmp(instr);
                break;
            case OP_JSR:
                vm_jsr(instr);
                break;
            case OP_LD:
                vm_load(instr);
                break;
            case OP_LDI:
                vm_load_indirect(instr);
                break;
            case OP_LDR:
                vm_load_register(instr);
                break;
            case OP_LEA:
                vm_lea(instr);
                break;
            case OP_ST:
                vm_store(instr);
                break;
            case OP_STI:
                vm_store_indirect(instr);
                break;
            case OP_STR:
                vm_store_register(instr);
                break;
            case OP_TRAP:

                break;
            case OP_RES:
                abort();
            case OP_RTI:
                abort();
            default:
                abort();
                break;
        }

    }

	return 0;
}