#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "vpc.h"


/**
 *	Description: 
 */
int start(){
    unsigned char memory[MEM_SIZE];
    unsigned char selection[BUFF_SIZE];
    unsigned char flush[BUFF_SIZE];
    unsigned int offset, length, i;
    int bytes;

    /* Zero all registers */
    zero_registers();
    
    /*  Print title and command list */
    printf("\t|*******************************|\n");
    printf("\t|      Welcome to Chandan VPC   |\n");
    printf("\t|  Created by: Chandan Chugani  |\n");
    printf("\t|*******************************|\n\n"); 
    
    menu();
    
    /*  Loop forever */
    while(1){
        // Prompt
        printf("> ");

        // Obtain a choice from the user/stdin
        fgets(selection, BUFF_SIZE, stdin);
        
        // Ensure the choice is lower-case
        for(i = 0; i < strlen(selection); i++){
            selection[i] = tolower(selection[i]);    
        }    
        
        // Switch to execture correct function 
        switch(selection[0]){
            case 'd':
                printf("Enter offset in hex:\t");
                // Test for a valid intake
                if(scanf("%x", &offset) == 0){
                    printf("Not a valid offset.\n");
                    break;
                }    
                // Test for an offset in range
                if(offset >= MEM_SIZE){
                    printf("Not a valid offset.\n");
                    // Flush input buffer
                    fgets(flush, BUFF_SIZE, stdin);
                    break;
                }
                fgets(flush,BUFF_SIZE, stdin);
                printf("Enter length in hex:\t");
                if(scanf("%x", &length) == 0){
                    printf("Not a valid length.\n");
                    break;
                }    
                // Flush input 
                fgets(flush, BUFF_SIZE, stdin);
                
                dump_memory(memory, offset, length);
                break;
            case 'g':
                
                 while(!flag_stop){
                    instruction_cycle(memory);
                 }
                 
                break;
            case 'l':
                bytes = load_file(memory, MEM_SIZE);
                if(bytes >= 0){
                    printf("0x%x(%d) bytes have been loaded into memory from file.\n", (unsigned int)bytes, (unsigned int)bytes);        
                }    
                break;
            case 'm':
                printf("Enter offset in hex:\t");
                // Ensure proper value is taken in
                if(scanf("%x", &offset) == 0){
                    printf("Not a valid offset.\n");
                    break;
                }    
                // Flush input
                fgets(flush, BUFF_SIZE, stdin);
                // Test for an offset in range
                if(offset >= MEM_SIZE){
                    printf("Not a valid offset.\n");
                    break;
                }    
                       
                mem_modify(memory, offset);
                break;
            case 'q':
                printf("Goodbye.\n");
                return quit();
            case 'r':
                display_registers();
                break;
            case 't':
                instruction_cycle(memory);  
                display_registers();
                break;
            case 'w':
                write_file(memory);
                break;
            case 'z':
                zero_registers();
                printf("Registers have been reset.\n");
                break;
            // If the user selects '?' then the case will fall into 'h'
            case '?':
            case 'h':
                menu();
                break;
            default:
                printf("%c is not an option.  Enter H/h/? for help.\n", selection[0]);
        }
    }    
}

int go(){
    return 0;
}


/*
 * dumpMemory prompts user to enter a filename and
 * than writes X(hex) #of bytes to that file.
 * will overwrite file contents, pending overwrite confirm. 
 * Lab Date: January 30th, 2015
 * Revision Date: February 6th,2015
 */
int dump_memory(void * memptr, unsigned int offset, unsigned int length){
    unsigned int i;
    unsigned char line[LINE_LENGTH];
    unsigned int lineLength = LINE_LENGTH;
    unsigned int count = 0;

    while(count < length){
    
        if(offset == MEM_SIZE){
            break;
        } 

        /* Print the offset/block number */
        printf("%04X\t", offset);
        /* Create the line */
        for(i = 0; i < LINE_LENGTH; i++, offset++, count++){
            /* Ensure that the pointer does not go out of bounds */
            if(offset == MEM_SIZE){
                lineLength = i;
                break;
            }
            /* Ensure that the line length does not exceed desired length */
            if(count == length){
                lineLength = i;
                break;
            }      
            line[i] = *((char*)memptr + offset);
            printf("%02X ", line[i]);
        }
        /* Move to a newline and continue to ASCII representation of the 
         * line dumped.
         */   
        printf("\n\t");
        for(i = 0; i < lineLength; i++){
            if(isprint(line[i])){
                printf(" %c ", line[i]);
            }else{
                printf(" . ");
            }    
        }
        /* Newline for next block */
        printf("\n");
    }

    return 0;
}


/*
 * LoadFile prompts user to enter a filename and
 * than loads that file into memory up to MAX_MEM bytes
 * Lab #: 3
 * Lab Date: January 30th, 2015
 * Revision Date: February 6th,2015
 */
int load_file(void * memory, unsigned int max){
    FILE* file;
    int nbytes;
    unsigned char filename[BUFF_SIZE];
    unsigned char buff[BUFF_SIZE];
    unsigned char error[BUFF_SIZE];
    unsigned char flush[BUFF_SIZE];
    unsigned long fsize;


    /* Prompt for filename */
    printf("\nEnter a filename: ");
    
    /* Obtain filename */
    fgets(filename, BUFF_SIZE, stdin);
    
    /* Nullify the last byte */
    filename[strlen(filename)-1] = '\0';

    /* Open the file */
    if((file = fopen(filename, "rb")) == NULL){
        sprintf(error, "load: fopen: %s", filename);
        perror(error);
        return -1;
    }

    printf("Loading %s to memory.\n", filename);
    
    /* Discover file size */
    if(fseek(file, 0, SEEK_END) == -1){
        perror("load: fseek");
        fclose(file);
        return -1;
    }
    
    /* Record file size as file position inidcator of ftell */
    if((fsize = ftell(file)) == -1){
        perror("load: ftell");
        fclose(file);
        return -1;
    }
    
    /* Set file position to beginning of file */
    rewind(file);

    /* Truncate file if it cannot fit in memory */
    if(max < fsize){
        fsize = max;
        printf("Data from file has been truncated to fit into memory.\n");
    }

    /* Read the file into memory */
    nbytes = (int)fread(memory, BYTE_SIZE, (size_t)fsize, file);
    
    if(ferror(file)){
        perror("load: fread");
        fclose(file);
        return -1;
    }

    fclose(file);
    
    return (int)nbytes;
}


/*
 * modMemory prompts user to enter a filename and
 * than writes X(hex) #of bytes to that file.
 * will overwrite file contents, pending overwrite confirm. 
 * Lab Date: January 30th, 2015
 * Revision Date: February 6th,2015
 */
int mem_modify(void * memptr, unsigned int offset){
    unsigned char input[BUFF_SIZE];
    unsigned char flush[BUFF_SIZE];
    unsigned int byte;
    unsigned int i;
    
    printf("*All byte values accepted in hex.*\nEnter '.' to stop.\n\n");

    forever{
        // Print current offset and value
        printf("%04X : %02X > ", offset, *((unsigned char*)memptr + offset));
        // Get input
        fgets(input, BUFF_SIZE, stdin);
        // Nullify newline element
        input[2] ='\0';

        // Check if input is quit
        if(input[0] == '.'){
            break;    
        }
        // Check if input is null
        if(strcmp(input, "") == 0){
            // Ignore invalid input and continue
            continue;
        }
        // Check if input is enter
        if(strcmp(input, "\n") == 0){
            // Ignore invalid input continue
            continue;
        }    

        // Ensure the string is lowercase
        for(i = 0; i < strlen(input); i++){
            input[i] = tolower(input[i]);
        }

        // Capture input as a hex-byte
        if(sscanf(input, "%x", &byte) == 0){
            continue;
        }

        // Assign offset with new byte value
        *((unsigned char*)memptr + offset) = byte;
        // Increment offset to next byte
        ++offset;
        
        if(offset == MEM_SIZE){
            printf("End of memory.\n");
            break;
        }    
    }
    return 0;
}    

int quit(){
    return 0;    
}


/**
 * Register dump:   
 *      Display all registers and flags with their current values.
 */
int display_registers(){
    unsigned int i;

    /* Print regsiter file */
    for(i = 0; i < RF_SIZE; i ++){
        if(i % LINE_BREAK == 0){
            printf("\n");
        }
        if(i == RF_SP){
            printf(" SP:%08X ", SP);
        }else if(i == RF_LR){
            printf(" LR:%08X ",  LR);
        }else if(i == RF_PC){
            printf(" PC:%08X ",  PC);
        }else{    
            printf("r%02d:%08X ", i,  regfile[i]);
        }    
    }    
    
    /* Print flags */
    printf("\t SZC:%d%d%d", flag_sign, flag_zero, flag_carry);

    /* Print non-visible registers */
    printf("\n   MAR:%08X   MBR:%08X   IR0:%04X   IR1:%04X   Stop:%0d   IR Flag:%01d\n",  mar,  mbr, IR0, IR1, flag_stop, flag_ir);

    return 0;
}


/*
 * WriteFile prompts user to enter a filename and
 * than writes X(hex) #of bytes to that file.
 * will overwrite file contents, pending overwrite confirm. 
 * Lab #: 3
 * Lab Date: January 30th, 2015
 * Revision Date: February 6th,2015
 */
void write_file(void * memory){
    FILE* file;
    int nbytes;
    int wbytes;
    unsigned char filename[BUFF_SIZE];
    unsigned char error[BUFF_SIZE];
    unsigned char flush[BUFF_SIZE];

    // Retrieve filename
    printf("\nEnter a filename: ");
    fgets(filename, BUFF_SIZE, stdin);
    
    // Nullify last byte
    filename[strlen(filename) -1] = '\0';

    // Retrieve number of bytes to write
    printf("\nEnter the amount of bytes to write: ");
    if(scanf("%d", &nbytes) == 0){
        printf("Invalid input.\n");
        return;
    }    
    // Flush input stream
    fgets(flush, BUFF_SIZE, stdin);
    
    /** Check if number of bytes specified is greater than memory, 
     *  less than 0, or 0.  
     */
    if(nbytes > MEM_SIZE){
        printf("File not written.  Cannot write more bytes than in memory.\n");
        return;
    
    }else if(nbytes <= 0){
        printf("0 bytes written.\n");
        return;
    }

    // Open the file
    if((file = fopen(filename, "wb")) == NULL){
        sprintf(error, "write: %s", filename);
        perror(error);
        return;
    }

    // Write memory to file
    if((wbytes = fwrite(memory, BYTE_SIZE, nbytes, file)) < 0){
        perror("dpu: write: ");
    }else{
        printf("%d bytes have been written to %s.\n", wbytes, filename);
    }
    
    // Close the file
    fclose(file);

    return;
}


/**
 *  Reset: Reset all registers to 0.
 */
int zero_registers(){
    unsigned int i;
    
    // Reset visible registers
    for(i = 0; i < RF_SIZE; i++){
        regfile[i] = 0;
    }
    // Reset flags
    flag_sign = 0;
    flag_zero = 0;
    flag_carry = 0;
    flag_stop = 0;
    flag_ir = 0;
    // Non-visible registers
    mar = 0;
    mbr = 0;
    ir = 0;
    // Unofficial current instruction register
    cir = 0;
    
    return 0;
}


/*
 * Menu function this displays the menu options
 * Lab #: 3
 * Lab Date: January 30th, 2015
 * Revision Date: February 6th,2015
 */
void menu(){
    printf("\td\tdump memory\n"
            "\tg\tgo - run the entire program\n"
            "\tl\tload a file into memory\n"
            "\tm\tmemory modify\n"
            "\tq\tquit\n"
            "\tr\tdisplay registers\n"
            "\tt\ttrace - execute one instruction\n"
            "\tw\twrite file\n"
            "\tz\treset all registers to zero\n"
            "\t?, h\tdisplay list of commands\n");
}


/********************************************************************
 * Instruction Cycle:   
 ***********************************************************************/
void instruction_cycle(void * memory){
    /* Determine which IR to use via IR Active flag */
    if(flag_ir == 0){
        flag_ir = 1;
        /* Fetch new set of instructions */
        fetch(memory);
        /* Current instruction is now IR0 */
        cir = IR0;
        execute(memory);
    }else{
        flag_ir = 0;
        cir = IR1;
        execute(memory);
    }     
}


/********************************************************
 * Fetch:  
 **************************************************************/         
void fetch(void * memory){
    /* MAR <- PC */
   // mar = PC;
    
    ir = load_registers(PC, memory);
    
    /* PC + 1 instruction */
    PC += REG_SIZE;
}

/***************************************************************
 * Load Register: .
 ******************************************************************/
uint32_t load_registers(uint32_t marValue, void * memory){
    unsigned int i;

    mar = marValue;

    /* MBR <- memory[MAR] */        /* PC <- + 1 instruction */
    for(i = 0; i < CYCLES; i++, mar++){
        mbr = mbr << SHIFT_BYTE;
        /* Add memory at mar to mbr */
        mbr += *((unsigned char*)memory + mar);
    }     

    /* Register <- MBR */
    return mbr;    
}

/***************************************************************
 * Store Register:                
 ******************************************************************/
void store_registers(uint32_t marValue, uint32_t mbrValue, void * memory){
    
    mar = marValue;
    mbr = mbrValue;

    *((unsigned char*)memory + mar++) = (unsigned char)(mbr >> SHIFT_3BYTE & BYTE_MASK);
    *((unsigned char*)memory + mar++) = (unsigned char)(mbr >> SHIFT_2BYTE & BYTE_MASK);
    *((unsigned char*)memory + mar++) = (unsigned char)(mbr >> SHIFT_BYTE & BYTE_MASK);
    *((unsigned char*)memory + mar) = (unsigned char)mbr & BYTE_MASK;
}

/***************************************************************
 * Execute: 
 ******************************************************************/
void execute(void * memory){
    int i;

    /* Recognize instruction type */
    
    /* 
     * Data Processing 
     */
    if(DATA_PROC){
        /* Acknowledge Operation field */
        if(DATA_AND){
            alu = regfile[RD] & regfile[RN];
            flags(alu);
            regfile[RD] = alu;
        }else if(DATA_EOR){
            alu = regfile[RD] ^ regfile[RN];
            flags(alu);
            regfile[RD] = alu;
        }else if(DATA_SUB){
            alu = regfile[RD] + ~regfile[RN] + 1;
            flags(alu);
            flag_carry = iscarry(regfile[RD], ~regfile[RN], 1);
            regfile[RD] = alu;
        }else if(DATA_SXB){
            alu = regfile[RN];
            if((alu & MSB8_MASK) == 1){
                alu += SEX8TO32;
            }
            flags(alu);
            regfile[RD] = alu;
        }else if(DATA_ADD){
            alu = regfile[RD] + regfile[RN];
            flags(alu);
            flag_carry = iscarry(regfile[RD], ~regfile[RN], 0);
            regfile[RD] = alu;
        }else if(DATA_ADC){
            alu = regfile[RD] + regfile[RN] + flag_carry; 
            flags(alu);
            flag_carry = iscarry(regfile[RD], regfile[RN], flag_carry);
            regfile[RD] = alu;
        }else if(DATA_LSR){
            for(i = 0; i < regfile[RN]; i++){
                flag_carry = regfile[RN] & LSB_MASK;
                alu = regfile[RD] >> 1;
            }
            flags(alu);
            regfile[RD] = alu;
        }else if(DATA_LSL){
            for(i = 0; i < regfile[RN]; i++){
                flag_carry = regfile[RN] & LSB_MASK;
                alu = regfile[RD] << 1;
            }
            flags(alu);
            regfile[RD] = alu;
        }else if(DATA_TST){
            alu = regfile[RD] & regfile[RN];
            flags(alu);
        }else if(DATA_TEQ){
            alu = regfile[RD] ^ regfile[RN];
            flags(alu);
        }else if(DATA_CMP){
            alu = regfile[RD] + ~regfile[RN] + 1;
            flags(alu);
            flag_carry = iscarry(regfile[RD], ~regfile[RN], 1);
        }else if(DATA_ROR){
            for(i = 0; i < regfile[RN]; i++){
                flag_carry = regfile[RD] & LSB_MASK;
                alu = regfile[RD] >> 1;
                /* Set the MSB of the alu to the value shifted left */
                if(flag_carry){
                    alu |= MSB32_MASK;
                }
            }
            flags(alu);
            regfile[RD] = alu;
        }else if(DATA_ORR){
            alu = regfile[RD] | regfile[RN];
            flags(alu);
            regfile[RD] = alu;
        }else if(DATA_MOV){
            regfile[RD] = regfile[RN];
            flags(regfile[RD]);
        }else if(DATA_BIC){
            alu = regfile[RD] & ~regfile[RN];
            flags(alu);
            regfile[RD] = alu;
        }else if(DATA_MVN){
            alu = ~regfile[RN];
            flags(alu);
            regfile[RD] = alu;
        }
    }
    /* 
     * Load/Store 
     */
    else if(LOAD_STORE){
        /* MAR <- regfile[RN] */
        
        if(LOAD_BIT){
            /*Load Byte*/
            if(BYTE_BIT){
                regfile[RD] = load_registers(regfile[RN], memory);
                regfile[RD] = regfile[RD] & BYTE_MASK;
            }
            /*Load Double Word*/
            else{
                regfile[RD] = load_registers(regfile[RN], memory);
            }
        }else{
            mbr = regfile[RD];
            /* Store one byte of the register into memory */
            if(BYTE_BIT){
                mar = regfile[RN];
                mbr = regfile[RD];
                *((unsigned char*)memory + mar) = (unsigned char)mbr & BYTE_MASK;
            }
            /*Store double word*/
            else{
                store_registers(regfile[RN], regfile[RD], memory);
            }
        } 
    /* 
     * Immediate Operations 
     */
    }else if(IMMEDIATE){
        /* Move immediate value into regfile at RD */
        if(MOV){
            regfile[RD] = IMM_VALUE;    
            flags(regfile[RD]);
        }else if(CMP){
            alu = regfile[RD] + ~IMM_VALUE + 1;
            flags(alu);
            flag_carry = iscarry(regfile[RD], ~IMM_VALUE, 0);
        }else if(ADD){
            alu = regfile[RD] + IMM_VALUE;
            flags(alu);
            flag_carry = iscarry(regfile[RD], IMM_VALUE, 0);
            regfile[RD] = alu;
        }else if(SUB){
            alu = regfile[RD] + ~IMM_VALUE + 1;
            flags(alu);
            flag_carry = iscarry(regfile[RD], ~IMM_VALUE, 1);
            regfile[RD] = alu;
        }    
    /* 
     * Conditonal Branch 
     */
    }else if(COND_BRANCH){
        /* Check condition codes and flags */
        if(chk_branch()){
            /* Add relative address as a signed 8-bit */
            alu = PC + (int8_t)COND_ADDR;
	    if(flag_ir != 0){
                flag_ir = 0;
                alu = alu + ~THUMB_SIZE + 1;
            }
            PC = alu;
        }        
    /* 
     * PUSH / PULL
     */
    }else if(PUSH_PULL){
        /* PULL */
        if(LOAD_BIT){
            /* High Registers */
            if(HIGH_BIT){
                /* Registers 8 - 15 */
                for(i = HI_REG; i < RF_SIZE; i++){
                    /* Registers must be represented by what bit number
                    * they occupy.  HIGH reg's subtract half the list size.*/
                    if(chk_register( i - HALF_RF )){
                        /*If the current index is set on the register list: */
                        /* Set MAR to be the stack pointer */
                        regfile[i] = load_registers(SP & SP_MASK, memory);
                        /* Post increment */
                        alu = SP + REG_SIZE;
                        SP = alu;
                    }
                }
            }
            /* Low Registers */
            else{
                /* Registers 0 - 7 */
                for(i = 0; i <= LOW_LIMIT; i++){
                    if(chk_register(i)){
                        regfile[i] = load_registers(SP & SP_MASK, memory);
                        alu = SP + REG_SIZE;
                        SP = alu;
                    }
                }
            }

            /* Check if PC is to be pulled for return. */
            if(RET_BIT){
                /* If the IR flag is 1, change it to 0 so the next thumb 
                 * instruction is not executed. 
                 */
                PC = load_registers(SP & SP_MASK, memory);
                if(flag_ir !=0){
                    flag_ir = 0;
                }
                alu = SP + REG_SIZE;
                SP = alu;
            }

        }
        /* PUSH */
        else{
            if(RET_BIT){
                /* Pre-decrement 
                 * Considering there is only 
                 * */
                alu = SP + ~REG_SIZE + 1;
                SP = (SP_MASK & alu) >> (unsigned) SHIFT_2BYTE;
                /* Store the Link Register/return address for jump-returns */
                store_registers(SP & SP_MASK, LR, memory);
            }
            if(HIGH_BIT){
                for(i = (RF_SIZE - 1); i >= HI_REG; i--){
                    if(chk_register( i - HALF_RF )){
                        alu = SP + ~REG_SIZE + 1;
                        SP = alu;
                        store_registers(SP & SP_MASK, regfile[i], memory);
                    }
                }
            }else{
                for(i = LOW_LIMIT; i >= 0; --i){
                    if(chk_register(i)){
                        alu = SP + ~REG_SIZE + 1;    
                        SP = alu;
                        //SP_DEC;
                        store_registers(SP & SP_MASK, regfile[i], memory);
                    }
                }
            }
        }
    }
    /* 
     * Unonditional Branch 
     */
    else if(BRANCH){
        if(LINK_BIT){
            LR = PC;
        }    
        PC = OFFSET12;
        /* Make sure the IR flag is not still HI after the PC has changed.
         * If it is, IR1 will execute before a fetch is made to reach the 
         * instruction being branched to.
         */
        flag_ir = 0;
    /* 
     * Stop 
     */
    }else if(STOP){
        flag_stop = 1;
    }    

}    

/*************************************************************
 *  chk_branch()
 *************************************************************/
int chk_branch(){
    if(EQ){
        if(flag_zero){
            return 1;
        }    
    }else if(NE){
        if(flag_zero == 0){
            return 1;
        }
    }else if(CS){
        if(flag_carry){
            return 1;
        }
    }else if(CC){
        if(!flag_carry){
            return 1;
        }
    }else if(MI){
        if(flag_sign){
            return 1;      
        }    
    }else if(PL){
        if(!flag_sign){
            return 1;
        }
    }else if(HI){
        if(flag_carry && flag_zero == 0){
            return 1;   
        }
    }else if(LS){
        if(flag_carry == 0 || flag_zero){
            return 1;
        }
    }else if(AL){
        return 1;
    }
    
    return 0;
}


/*************************************************************************
 * chk_register()
 ********************************************************************/
int chk_register(int index){
    switch(index){
        case 0:
            return REG_LIST & R0;
        case 1:
            return REG_LIST & R1;
        case 2:
            return REG_LIST & R2;
        case 3:
            return REG_LIST & R3;
        case 4:
            return REG_LIST & R4;
        case 5:
            return REG_LIST & R5;
        case 6:
            return REG_LIST & R6;
        case 7:
            return REG_LIST & R7;
    }
}

/****************************************************************
 * flags() 
 *************************************************************/          
void flags(uint32_t alu){        
    if(alu == 0){
        flag_zero = 1;
    }else{
        flag_zero = 0;
    }    
    
    flag_sign = (alu & MSB32_MASK) >> MSBTOLSB;

}


/**********************************************************
 *   iscarry()
 ************************************************************/
int iscarry(uint32_t op1,uint32_t op2, uint8_t c){
    if ((op2== MAX32)&&(c==1)) 
        return(1); // special case where op2 is at MAX32
    return((op1 > (MAX32 - op2 - c))?1:0);
}









