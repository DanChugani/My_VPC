#Virtual Computer Architecture


##Files:

###main.c
  This starts the execution of the Virtual CPU
  
###run
  auto-generated executable file
  
###vpc.h
  Decalare Registers
  Declare Variable's 
  Declare Function protypes
  
###vpc.c

Main Program
Declare Functions



##Vpc.c
###Functions:

**int dump_memory(void * memoryPtr, unsigned int offset, unsigned int length)**

  > Prompts user to enter a file_name and than writes X(hex) #of bytes to that file. will overwrite file contents, pending         overwrite confirm

**int load_file(void * memory, unsigned int max)**

  > Prompts user to enter a file_name and than loads that file into memory up to MAX_MEM bytes
  
**mem_modify(void * memoryPtr, unsigned int offset)**

  > ask user for a hex offset and hex length than begins modifying memory contents at that location until terminating character    of '.' is reached
  
**quit()**

  > Terminate running of the virtual CPU
  
**display_registers()**

  > Display all registers and flags with their current values.
  
**write_file(void * memory)**
  
 > prompts user to enter a file_name and than writes X(hex) #of bytes to that file. will overwrite file contents, pending         overwrite confirm.

**zero_registers()**
  
> Reset all registers to 0.
  
**menu()**
  
  > Menu function this displays the menu options
  
**void instruction_cycle(void * memory)**
  
  > Consists of a fetch and an excute for a particular instruction

**void fetch(void * memory)**
  
  > Fetch an instruction from memory at program counters address

**uint32_t load_registers(uint32_t marValue, void * memory)**
  
  > Load register with value in memory

**void store_registers(uint32_t marValue, uint32_t mbrValue, void * memory)**
  
  > Store current register into memory
  
  **void store_registers(uint32_t marValue, uint32_t mbrValue, void * memory)**
  
  > Store current register into memory

