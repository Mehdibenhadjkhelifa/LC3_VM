#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include "vm.h"
#include "vmcore.h"


bool vm_init(int argc,char** argv){

    //checks the command line arguments and 
    //loads the image into memory if found
    if (argc < 2){
        //show usage string
        printf("lc3 [image-file1] ...\n");
        return false;   
    }
    for(int j = 1; j < argc; ++j){
        if(!read_image(argv[j])){
            printf("failed to load image: %s\n",argv[j]);
            return false;
        }
    }
    //this sets up the console as we like 
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    //since exactly one condition flag should be set at any given time, set the Z flag 
    reg[R_COND] = FL_ZRO;

    /* set the PC to starting position 
     0x3000 is the default */
    enum {PC_START = 0x3000};
    reg[R_PC] = PC_START;

    return true;
}

void vm_run(bool* running){
    while(*running){
        //Fethcing the instruction from PC then incrementing it
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;//left 4 bits of the instr is the opcode rest 12 are params
        //executing instruction depending on the opcode
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
                vm_trap(instr,running);
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
}
bool vm_shutdown(){
    //this restores the terminal settings back to normal
    restore_input_buffering();
    printf("VM Shutdown succesfully !\n");
    return true;
}

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

void vm_not(uint16_t instr){
    //DR
    uint16_t r0 = (instr >> 9) & 0x7;
    //SR
    uint16_t r1 = (instr >> 6) & 0x7;
    reg[r0] = ~reg[r1];
    update_flags(r0);
}
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

void vm_br(uint16_t instr){
    uint16_t pc_offset = sign_extend(instr & 0x1FF,9);
    uint16_t cond_flag = (instr >> 9) & 0x7;
    if(cond_flag & reg[R_COND])
        reg[R_PC] += pc_offset;
}

void vm_jmp(uint16_t instr){
    //get baseR
    uint16_t r0=(instr >> 6) & 0x7;
    reg[R_PC] = reg[r0];
}

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

void vm_load(uint16_t instr){
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF,9);
    reg[r0] = mem_read(reg[R_PC] + pc_offset);
    update_flags(r0);
}

void vm_load_indirect(uint16_t instr){
    // the DR register
    uint16_t r0 = (instr >> 9) & 0x7;
    // offset of memory address from PC
    uint16_t pc_offset = sign_extend(instr & 0x1FF , 9);
    //next line can be thought of as dereferencing a double pointer
    reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
    update_flags(r0);
}

void vm_load_register(uint16_t instr){
    //DR
    uint16_t r0 = (instr >> 9) & 0x7;
    //baseR
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F,6);
    reg[r0] = mem_read(reg[r1] + offset);
    update_flags(r0);
}

void vm_lea(uint16_t instr){
    //DR
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF,9);
    reg[r0] = reg[R_PC] + pc_offset;
    update_flags(r0);
}

void vm_store(uint16_t instr){
    //SR
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset= sign_extend(instr & 0x1FF,9);
    mem_write(reg[R_PC] + pc_offset , reg[r0]);
}

void vm_store_indirect(uint16_t instr){
    //SR
    uint16_t r0 = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF,9);
    mem_write(mem_read(reg[R_PC] + pc_offset),reg[r0]);
}

void vm_store_register(uint16_t instr){
    //SR
    uint16_t r0 = (instr >> 9) & 0x7;
    //baseR
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F,6);
    mem_write(reg[r1] + offset,reg[r0]);
}

void vm_trap_getc(){
    reg[R_R0] =(uint16_t)getchar();
    update_flags(R_R0);
}

void vm_trap_out(){
    putc((char)reg[R_R0],stdout);
    fflush(stdout);
}

void vm_trap_puts(){
    uint16_t* c = memory + reg[R_R0];
    while(*c)
        putc((char)*c++,stdout);//could be an error
    fflush(stdout);
    
}

void vm_trap_in(){
    printf("Enter a Character : ");
    char c = getchar();
    putc(c,stdout);
    fflush(stdout);
    reg[R_R0] = (uint16_t)c;
    update_flags(R_R0);
}

void vm_trap_putsp(){
    uint16_t* address = memory + reg[R_R0];
    while(*address){
        char c1 = *address & 0xFF;
        char c2 = *address >> 8;
        putc(c1,stdout);
        if(c2)
            putc(c2,stdout);
        else
            break;
        address++;
    }
    fflush(stdout);
}
void vm_trap_halt(){
    puts("VM Halted");
    fflush(stdout);
}

void vm_trap(uint16_t instr,bool* running){
    reg[R_R7] = reg[R_PC];
    switch(instr & 0xFF){
        case TRAP_GETC:
            vm_trap_getc();
            break;
        case TRAP_OUT:
            vm_trap_out();
            break;
        case TRAP_PUTS:
            vm_trap_puts();
            break;
        case TRAP_IN:
            vm_trap_in();
            break;
        case TRAP_PUTSP:
            vm_trap_putsp();
            break;
        case TRAP_HALT:
            vm_trap_halt();
            *running = 0;
            break;
    }
}
