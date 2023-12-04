#ifndef _VMCORE_H
#define _VMCORE_H
#ifdef _WIN32
/* windows only */
#include <Windows.h>
#include <conio.h>
#else
/* unix only */
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>
#endif
#include <stdint.h>
#include <stdio.h>
/* Memory map of the LC-3
  0x0000 -> 0x00FF Trap Vector Table 
  0x0100 -> 0x01FF Interrupt Vector Table
  0x0200 -> 0x2FFF Operating system and Supervisor Stack
  0x3000 -> 0xFDFF Available for user programs
  0xFE00 -> 0xFFFF Device register addresses */

//defining memory for the LC3 VM
#define MEMORY_MAX (1 << 16)

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
//defining trap codes
enum
{
    TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25   /* halt the program */
};

//MMR or Memory Mapped Registers are commonly used to interact with special hardware devices
//these are unlike normal registers they have a predifined memory location
//MMR make memory access harder,need setters and getters i.e if memory is read
//from KSBR the getter will check the keyboard and update both locations
//MR_KBSR get the status of keyboard(bit-15 is set when key is pressed)
//MR_KBDR get the key that is pressed (bits 7-0)
enum
{
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02  /* keyboard data */
};
//Declaring core Hardware components of the vm

extern uint16_t memory[MEMORY_MAX];
extern uint16_t reg[R_COUNT];
//Declaring Core functions of the VM

// Handling input buffering from terminal (platform specific)
void disable_input_buffering();
void restore_input_buffering();
uint16_t check_key();
void handle_interrupt(int signal);
//this extends the value x to a 16-bit value that is signed
uint16_t sign_extend(uint16_t x , int bit_count);
//update the condition flag register with each operation based on the DR
void update_flags(uint16_t r);
//write to a specific memory address
void mem_write(uint16_t address,uint16_t val);
//reads from a specific memory address and handles
//the read to MMRs
uint16_t mem_read(uint16_t address);
//this function swaps the 8 least significant bits with the
//8 most significant bits because in little indian(which is what modern computers target)
//the first byte is the least significant digit and big-indian(what LC-3 targets) which is it's the reverse
uint16_t swap16(uint16_t x);
//handles the copying of the binary into memory in the specified location (origin)
void read_image_file(FILE* file);
//manages the opening and closing of the binary file to be copied
int read_image(const char* image_path);
#endif
