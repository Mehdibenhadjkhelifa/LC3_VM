# LC3_VM

**LC3** : or 'little computer 3' features a RISC (Reduced instruction set computer) 
designed to simplify the individual instructions given to the computer to accomplish tasks.

**PROJECT_STATUS** : *Completed*.  May add multithreading to support handled multiple LC3-VMs at once.

## Setup

#### 1. Download the repository

-navigate to where you want to clone the repository using the command line

-clone the repository by enter the following command in your terminal : `git clone https://github.com/Mehdibenhadjkhelifa/LC3_VM.git`

#### 2. Building the project 

**FOR LINUX**:

-simply use the shell script which can : 
 
 1- setup and build the project by typing in the command line : `./CMakeSetup.sh setup "config"`
 
 where "config" is either Debug or Release depending on your desired configuration

 this will generate build files and generate binaries for your choosen configuration

 2- debug the built binaries using gdb by using this command : `./CMakeSetup.sh debug` 
 
 3- remove the project and binary simply with the following command : `./CMakeSetup.sh remove`

 4-run the binary (Need to build Release) with the command :`./CMakeSetup.sh run "test"`

 with test being either rogue or 2048

 **FOR WINDOWS**:

 1-make a build directory and navigate to it : `mkdir build && cd build`
  
 2-execute cmake in the build directory:`cmake -DCMAKE_BUILD_TYPE=config ..`

 replace config with either Debug or Release depending on your desired configuration

 3-build the binaries : `cmake --build .`
 
 you should now have a bin-config folder which contains your binaries

 ##### NOTE : the working directory should be the root of the project to properly follow the above guide for both windows and linux
                                      
