#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "vmcore.h"
#include "vm.h"

//Defining the VM hardware components globaly
//not optimal but should do it for the time being
uint16_t memory[MEMORY_MAX];
uint16_t reg[R_COUNT];


int main(int argc,char* argv[]){
    if(!vm_init(argc,argv))
        return -1;
    bool running = true;
    vm_run(&running);
    if(!vm_shutdown());
	    return -2;
    return 0;
}