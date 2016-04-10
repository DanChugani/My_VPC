#include <stdint.h>

/*  Size Definitions */
#define MAX_MEM       	0x4000
#define BUFFER       	0x100
#define MAX_BYTE       	0x1
#define MAX_REG_BITS   	0x20
#define MAX_REG        	0x4
#define THUMB      	0x2
#define REGISTER_FILE   0x10
#define MAX32           0xFFFFFFFF

/* Display Formatting
 *  
 *  MAX_LINE - Length of a line when dumping memory.
 *  LINE_BREAK - Amount of registers to show with register dump.
 * 
 */
#define MAX_LINE     	0x10
#define LINE_BREAK      0x6


/* Special Registers in Register File Offsets */
#define REGISTER_FILE_SP   	0xD
#define REGISTER_FILE_LR   	0xE
#define REGISTER_FILE_PC   	0xF
#define SP      		regfile[REGISTER_FILE_SP]
#define LR      		regfile[REGISTER_FILE_LR]
#define PC      		regfile[REGISTER_FILE_PC]

/* Instruction Registers */
#define IR0 	(unsigned)ir >> 16 
#define IR1 	ir & 0xFFFF


/* Bit Definitions
 *
 *  	MAX_CYCLES - Amount of cycles needed to pack bits/bytes from 8-bit
 *           	     memory into a 32-bit register.
 *   	SHIFT_BIT -  1 bit (1 bit)
 *   	SHIFT_BYTE - 8 bits (1 byte)
 *  	SHIFT_2BYTE- 16 bits (2 bytes)
 *   	SHIFT_3BYTE- 24 bits (3 bytes)
 *   	MSB_MASK32-  Mask the most significant bit of 32-bits
 *   	MSB_MASK8-   Mask the most significant bit of 8-bits
 *     	SIGN_EXT_32- Sign extend 8 bits to 32 bits
 *     	SHIFT_MSB_TO_LSB - Shift bits from MSB to LSB
 *     	MASK_SP  -   Mask the stack pointer so as it decrements to
 *                    0xFFFFFFFF, it will not fall out of memory range.
 *       
 */
#define MAX_CYCLES  (MAX_REG / MAX_BYTE)
#define SHIFT_BIT   1
#define SHIFT_BYTE  8
#define SHIFT_2BYTE 16
#define SHIFT_3BYTE 24
#define MSB_MASK32  0x80000000
#define MSB_MASK8   0x80
#define MASK_LSB    0x1
#define MASK_BYTE   0xFF
#define SIGN_EXT_32    0xFFFFFF00
#define SHIFT_MSB_TO_LSB    31
#define MASK_SP 0x3FFF

/* Instruction Definitions */
#define FORMAT      (unsigned)cir >> 13
#define PROCESS_DATA   FORMAT == 0x0
#define LOAD_STORE  FORMAT == 0x1
#define IMMEDIATE   FORMAT == 0x2 | FORMAT == 0x3
#define COND_BRANCH FORMAT == 0x4
#define PUSH_PULL   FORMAT == 0x5
#define BRANCH      FORMAT == 0x6
#define STOP        cir == 0xE000

/* Data Processing OpCodes */
#define DATA_AND 0x0 == OPERATION
#define DATA_EOR 0x1 == OPERATION
#define DATA_SUB 0x2 == OPERATION
#define DATA_SXB 0x3 == OPERATION
#define DATA_ADD 0x4 == OPERATION
#define DATA_ADC 0x5 == OPERATION
#define DATA_LSR 0x6 == OPERATION
#define DATA_LSL 0x7 == OPERATION
#define DATA_TST 0x8 == OPERATION
#define DATA_TEQ 0x9 == OPERATION 
#define DATA_CMP 0xA == OPERATION 
#define DATA_ROR 0xB == OPERATION
#define DATA_ORR 0xC == OPERATION 
#define DATA_MOV 0xD == OPERATION
#define DATA_BIC 0xE == OPERATION
#define DATA_MVN 0xF == OPERATION

/* Instruction Fields */
#define OPERATION   ((cir >> 8) & 0xF)
#define RN          ((cir >> 4) & 0xF)
#define RD          cir & 0xF
#define OPCODE      ((cir >> 12) & 0x3)
#define IMMEDIATE_VALUE   ((cir >> 4) & 0xFF)
#define CONDITION   ((cir >> 8) & 0xF)
#define CONDITION_ADDRESS   cir & 0xFF
#define LOAD_BIT    ((cir >> 11) & 0x1)
#define BIT_FIELD    ((cir >> 10) & 0x1)
#define BIT_HIGH    ((cir >> 10) & 0x1)
#define RETURN_BIT     ((cir >> 8) & 0x1)
#define REG_LIST    cir & 0xFF
#define LINK_BIT    ((cir >> 12) & 0x1)
#define OFFSET12    cir & 0xFFF

/* Branch Condition Codes */
#define EQ 0x0 == CONDITION
#define NE 0x1 == CONDITION
#define CS 0x2 == CONDITION
#define CC 0x3 == CONDITION
#define MI 0x4 == CONDITION 
#define PL 0x5 == CONDITION
#define HI 0x8 == CONDITION
#define LS 0x9 == CONDITION
#define AL 0xE == CONDITION

/* Immediate OpCodes */
#define MOV 0x0 == OPCODE
#define CMP 0x1 == OPCODE
#define ADD 0x2 == OPCODE
#define SUB 0x3 == OPCODE

/*
 * PUSH/PULL Definitions
 * 
 *    	REG_UPPR - First register of the upper half.       
 *	REG_LIMIT - Highest index of the lower half of registers.
 * 	R0-R7  - Bit numbers for registers in the register list
 *        
 */
#define REG_UPPR      0x8
#define REG_LIMIT   (unsigned)0x7
#define REG_HALF     REGISTER_FILE/2

#define R0  0x1
#define R1  0x2
#define R2  0x4
#define R3  0x8
#define R4  0x10
#define R5  0x20
#define R6  0x40
#define R7  0x80

/* Registers 
 *  
 *  mar - Memory Address Register: stores address from where data will be fetched.
 *  mbr - Memory Buffer Register: Stores data being transferred to and from memory.
 *  ir -  Instruction Register: Stores instruction to be executed.
 *  alu - Arithmetic Logic Unit: Handles all the Arithmetic operations for the cpu.
 *  cir - Current Instruction Register: Hidden register used to hold the current instruction. 
 *
 */
static uint32_t  regfile[REGISTER_FILE];
static uint32_t  mar;
static uint32_t  mbr;
static uint32_t  ir;
static uint32_t  alu;
static uint16_t  cir;


/* Flags */
static uint8_t sign_flag;
static uint8_t zero_flag;
static uint8_t carry_flag;
static uint8_t stop_flag;
static uint8_t ir_flag; 


/* Function Prototypes
 * 
 * These are a list of all functions being used in the program	
 *
 */
/* begin the execution of the V-CPU */
int begin(); 

/* go- run an entire code*/
int go(); 

/* dump memory as specified by the offset and length */
int dump_memory(void * memoryPtr, unsigned int offset, unsigned int length);

/* load a file into memory */
int load_file(void * memory, unsigned int max);

/* modify a memory location begining at specified offset */
int mem_modify(void * memoryPtr, unsigned int offset); 

/* Quit- but why would you? */
int quit(); 

/* display all the registers, nicely formatted of course */
int display_registers();

/* trace one instruction at a time, retaining nice formatting */
int trace(); 

/* write specified amount of bytes to a file*/
void write_file(void * memory); 

/* zero all registers, careful there is no turning back after this*/
int zero_registers(); 

/* Little menu guide to make navigation easier */
void menu(); 

/* fetch instructions to execute */
void fetch(void * memory); 

/* load all the registers with memory address value */
uint32_t load_registers(uint32_t marValue, void * memory);

/* store the value of the registers */
void store_registers(uint32_t marValue, uint32_t mbrValue, void * memory);

/* execute an instruction */
void execute(void * memory);

/* instruction cycle */
void instruction_cycle(void * memory);

/* function that handles all the cpu's flags */
void flags(uint32_t alu); 

/* not to be confused with "is scary" but this check for the carry flag being set  */
int iscarry(uint32_t op1, uint32_t op2, uint8_t c); 
