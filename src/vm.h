#include <stdbool.h>

//Declaring VM Functions
bool vm_init(int argc,char** argv);
void vm_run(bool* running);
bool vm_shutdown();
//Declaring VM instructions functions
//Add instruction layout :4-bit/3-bit/3-bit/1-bit/ (5-bit or 2-bit/3-bit)
// 4-bit op_code/ 3-bit DR (destination register)/
// 3-bit register countaing first value/ 1-bit immediate mode flag
// if immediate mode is 1 the rest 5-bits are to be treated as a direct value(with sign extending)
// if immediate mode is 0  next 2-bits are unused and rest 3-bits are for the second register tha holds the second value
void vm_add(uint16_t instr);
//bitwise not instruction layout : 4-bit/3-bit/3-bit/1-bit/5-bit
//4-bit opcode , 3-bit DR,3-bit SR, 1-bit and 5-bit are unused(set to 1 by default)
//this instruction stores the bitwise complement content of SR in DR 
void vm_not(uint16_t instr);
//And instruction layout : 4-bit/3-bit/3-bit/1-bit/ (5-bit or 2-bit/3-bit)
//same as the Add instruction but the result is calculated by bitwise anding the two params
void vm_and(uint16_t instr);
//the conditional branch instruction layout is : 4-bit/1-bit/1-bit/1-bit/9-bit
// first 4-bits are for the opcode and next 3-bits are n/z/p to be tested
// for each one set we test it's counterpart from the conditional register
//if n or z or p is set and it's counterpart then we offset the program counter
//by the value of the rest sign-extended 9-bits (PCoffset9)
void vm_br(uint16_t instr);
//the jump instruction layout is : 4-bit/3-bit/3-bit/6-bit
//4 first bits are for opcode next 3 bits and last 6 bits are unused
//second 3-bit section contains the register (baseR) which the program counter
//will jump to unconditionnaly
//this function is also called RET when the register specified is of 0x7 value (R7 register)
void vm_jmp(uint16_t instr);
//the jump register instruction layout is : 4-bit/1-bit/ (11-bit or 2-bit/3-bit/6-bit)
//first 4 bits for opcode,next 1-bit if set(=1) we use the 11-bit by sign extending it and 
//adding it to the program counter as an offset else we use the 3-bit section as a register 
//and assign the program counter to the value hold by the specified register as an address
//we hold for the program counter before any operation in the 8th register(R7) as a linkage 
//to the calling routine. this instruction makes the program jump to a subroutine.
void vm_jsr(uint16_t instr);
//the load instructin layout is : 4-bit/3-bit/9-bit
//4-bit for opcode , 3-bit for destination register(DR)
//9-bit for program counter offset(PCoffset9)
//this instruction load the destination register with the value read from
//the memory address of the program counter + pc offset 
void vm_load(uint16_t instr);
//LDI is better than LD because it can have 16-bit full adresses rather
//than the 9-bits that are in the instruction param and this is useful for
//farther addresses from the PC
//LDI loads a value in memory to a register
//the way it is layed out : 4-bit op_code/ 3-bit DR / 9-bit for the address 
//near the pc that holds an address to the actual target value
//LDI loads the destination register with the value at the memory address pointed
//to by the memory address located in the address of program counter + pc offset
void vm_load_indirect(uint16_t instr);
//LDR or load register instruction layout is : 4-bit/3-bit/3-bit/6-bit
//4-bit opcode , 3-bit Destination register(DR),3-bit base register(BaseR),6-bit offset
//to be sign-extended and added to the value held by the BaseR,then we load the DR value pointed to
//by the address calculated with the last mentioned operation(BaseR + offset6)
void vm_load_register(uint16_t instr);
// LEA known as load effective address layout is: 4-bit/3-bit/9-bit
//4-bit opcode , 3-bit Destination register , 9-bit PCoffset9
//this instruction loads the DR with program counter + pc offset(sign extended)
void vm_lea(uint16_t instr);
//the store instruction layout is:4-bit/3-bit/9-bit
//4-bit opcode,3-bit SR,9 bit PCoffset to be sign extended
//this instruction stores the content of the source register
//in the memory address pointed to by calculating program counter + pc offset
void vm_store(uint16_t instr);
//the store indirect instruction layout is:4-bit/3-bit/9-bit
//4-bit opcode,3-bit SR,9 bit PCoffset to be sign extended
//this instruction stores the content of the source register
//in the memory address pointed to by the value in the 
//memory address calculated by program counter + pc offset
void vm_store_indirect(uint16_t instr);
//Store register instruction layout : 4-bit/3-bit/3-bit/6-bit
//4-bit opcode,3-bit SR,3-bit baseR,6-bit offset
//this instruction stores the content of the SR in the memory
//address calculated by adding the offset (after sign extending it)
// to the content of register baseR 
void vm_store_register(uint16_t instr);
//Trap instruction layout :4-bit/4-bit/8-bit
//4-bit opcode , 4-bit unused(set to 0000 by default)
//8-bit to indicate which trap to activate
//sets the R7 register as a callback point and enters the 
//trap Vector table depending on the trap code
void vm_trap(uint16_t instr,bool* running);
//Declaring VM traps

//reads a single character from the console
//and store it in R0 with 8 most significant bits of R0 are cleared
void vm_trap_getc();
//outputs the character represented by the first 8-bits(least significant bits)
//of the register R0
void vm_trap_out();
//puts display a whole null terminated string into the console
//where the string is store in the address hold by register R0
void vm_trap_puts();
//prompts the user by displaying a console message to enter a char
//then reads a character , displays it and stores it in the 8 least
//significant bits of the register R0
void vm_trap_in();
//outputs a string to the console by iterating over memory starting
//from the address hold by the register R0 with each memory address
//holding two characters 8-bit(second char)/8-bit(first char)
//if string has odd letters second char with have 0x00 value
void vm_trap_putsp();
//Halts the program i.e stops the program from running completly
void vm_trap_halt();