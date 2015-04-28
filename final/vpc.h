#include <stdint.h>

/*  Sizes */
#define MEM_SIZE        0x4000
#define BUFF_SIZE       0x100
#define BYTE_SIZE       0x1
#define REG_SIZE_BITS   0x20
#define REG_SIZE        0x4
#define THUMB_SIZE      0x2
#define RF_SIZE         0x10
#define MAX32           0xFFFFFFFF

/* Display 
 *  
 *  LINE_LENGTH - Length of a line when dumping memory.
 *   LINE_BREAK - Amount of registers to show with register dump.
 * 
 */
#define LINE_LENGTH     0x10
#define LINE_BREAK      0x6


/* Special Registers in Register File Offsets */
#define RF_SP   0xD
#define RF_LR   0xE
#define RF_PC   0xF
#define SP      regfile[RF_SP]
#define LR      regfile[RF_LR]
#define PC      regfile[RF_PC]

/* Instruction Registers */
#define IR0 (unsigned)ir >> 16 
#define IR1 ir & 0xFFFF


/* Bit Shifting, Byte Masks, Extensions
 *
 *  CYCLES - Amount of cycles needed to pack bits/bytes from 8-bit
 *           memory into a 32-bit register.
 *
 *   SHIFT_BYTE - 8 bits (1 byte)
 *  SHIFT_2BYTE - 16 bits (2 bytes)
 *           
 *   MSB_MASK32 - Mask for most significant bit of 32-bits
 *     SEX8TO32 - Sign extend 8 bits to 32 bits
 *     MSBTOLSB - Bits to shift from MSB to LSB
 *     SP_MASK  - As the stack pointer decrements from 0 to
 *                0xFFFFFFFF, it will be out of memory range.
 *                It must be masked to 14 bits. 
 */
#define CYCLES  (REG_SIZE / BYTE_SIZE)
#define SHIFT_3BYTE 24
#define SHIFT_2BYTE 16
#define SHIFT_BYTE  8
#define SHIFT_BIT   1
#define MSB32_MASK  0x80000000
#define MSB8_MASK   0x80
#define LSB_MASK    0x1
#define BYTE_MASK   0xFF
#define SEX8TO32    0xFFFFFF00
#define MSBTOLSB    31

/* Stack Pointer Definitons */
#define SP_MASK 0x3FFF


/* Instruction Formats  */
#define FORMAT      (unsigned)cir >> 13
#define DATA_PROC   FORMAT == 0x0
#define LOAD_STORE  FORMAT == 0x1
#define IMMEDIATE   FORMAT == 0x2 | FORMAT == 0x3
#define COND_BRANCH FORMAT == 0x4
#define PUSH_PULL   FORMAT == 0x5
#define BRANCH      FORMAT == 0x6
#define STOP        cir == 0xE000

/* Instruction Fields */
#define OPERATION   ((cir >> 8) & 0xF)
#define RN          ((cir >> 4) & 0xF)
#define RD          cir & 0xF
#define OPCODE      ((cir >> 12) & 0x3)
#define IMM_VALUE   ((cir >> 4) & 0xFF)
#define CONDITION   ((cir >> 8) & 0xF)
#define COND_ADDR   cir & 0xFF
#define LOAD_BIT    ((cir >> 11) & 0x1)
#define BYTE_BIT    ((cir >> 10) & 0x1)
#define HIGH_BIT    ((cir >> 10) & 0x1)
#define RET_BIT     ((cir >> 8) & 0x1)
#define REG_LIST    cir & 0xFF
#define LINK_BIT    ((cir >> 12) & 0x1)
#define OFFSET12    cir & 0xFFF

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

/* Immediate OpCodes */
#define MOV 0x0 == OPCODE
#define CMP 0x1 == OPCODE
#define ADD 0x2 == OPCODE
#define SUB 0x3 == OPCODE

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

/***********************************************************
 * PUSH/PULL Definitions 
 * 
 *    HI_REG - First register of the upper half.       
 * LOW_LIMIT - Highest index of the lower half of registers.
 * 
 * R0-R7  - Bit numbers for registers in the register list
 *           instruction field.
 ********************************************************/
#define HI_REG      0x8
#define LOW_LIMIT   (unsigned)0x7
#define HALF_RF     RF_SIZE/2

#define R0  0x1
#define R1  0x2
#define R2  0x4
#define R3  0x8
#define R4  0x10
#define R5  0x20
#define R6  0x40
#define R7  0x80

/* Forever loop */
#define forever for(;;)


/* Registers 
 *  
 *  cir - Unofficial hidden register for holding the current instruction. 
 *
 */
static uint32_t  regfile[RF_SIZE];
static uint32_t  mar;
static uint32_t  mbr;
static uint32_t  ir;
static uint32_t  alu;
static uint16_t  cir;


/* Flags */
static uint8_t flag_sign;
static uint8_t flag_zero;
static uint8_t flag_carry;
static uint8_t flag_stop;
static uint8_t flag_ir; 


/* Prototypes */
int start();

int go();

int dump_memory(void * memptr, unsigned int offset, unsigned int length);

int load_file(void * memory, unsigned int max);

int mem_modify(void * memptr, unsigned int offset);

int quit();

int display_registers();

int trace();

void write_file(void * memory);

int zero_registers();

void menu();

void fetch(void * memory);

uint32_t load_registers(uint32_t marValue, void * memory);

void store_registers(uint32_t marValue, uint32_t mbrValue, void * memory);

void execute(void * memory);

void instruction_cycle(void * memory);

void flags(uint32_t alu);

int iscarry(uint32_t op1, uint32_t op2, uint8_t c);
