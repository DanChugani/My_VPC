#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "vpc.h"


/**
 *	Description: 
 */
int begin(){
    
    unsigned char memory[MAX_MEM];
    unsigned int offset, length, i;
    int bytes;    
    unsigned char selection[BUFFER];
    unsigned char flush[BUFFER];


    /* Zero all registers */
    zero_registers();
    
    /*  Print title and command list */
    printf("\t|*******************************|\n");
    printf("\t|      Welcome to Chandan VPC   |\n");
    printf("\t|  Created by: Chandan Chugani  |\n");
    printf("\t|*******************************|\n\n"); 
    
    menu();
    
    /*  Loop until the end of time, or user quits */
    while(1){
        /* default option */
        printf("('h' or '?' for help)> ");

        /* Obtain a choice from the user*/
        fgets(selection, BUFFER, stdin);
        
        /* change the selection to lower-case */
        for(i = 0; i < strlen(selection); i++){
            selection[i] = tolower(selection[i]);    
        }    
        
        /* execute the correct function */ 
        switch(selection[0]){
		
            case 'd': /* dump_memory */
                printf("Enter offset in hex:\t");
                /* Test for a valid value */
                if(scanf("%x", &offset) == 0){
                    printf("Not a valid offset.\n");
                    break;
                }    
                /* Test that the specified offset is in range */
                if(offset >= MAX_MEM){
                    printf("Not a valid offset.\n");
                    /* Flush the input buffer */
                    fgets(flush, BUFFER, stdin);
                    break;
                }
                fgets(flush,BUFFER, stdin);
		/* Get a valid length, test if valid */
                printf("Enter length in hex:\t");
                if(scanf("%x", &length) == 0){
                    printf("Not a valid length.\n");
                    break;
                }    
                /* Flush the input buffer */ 
                fgets(flush, BUFFER, stdin);
                /* goto dump memory function */
                dump_memory(memory, offset, length);
                break;

            case 'g': /* go function */
                
                 while(!stop_flag){
		    /* keep running through until the stop flag is reached */
                    instruction_cycle(memory);
                 }
                 
                break;
		
            case 'l': /* load file function */
                bytes = load_file(memory, MAX_MEM);
                if(bytes >= 0){
                    printf("0x%x(%d) bytes have been loaded into memory from file.\n", (unsigned int)bytes, (unsigned int)bytes);        
                }     
                break;

            case 'm': /* modify memory function */
                printf("Enter offset in hex:\t");
                /* Test for a valid value */
                if(scanf("%x", &offset) == 0){
                    printf("Not a valid offset.\n");
                    break;
                }    
                /* Flush input */
                fgets(flush, BUFFER, stdin);
                //* Test that the offset is in range */
                if(offset >= MAX_MEM){
                    printf("Not a valid offset.\n");
                    break;
                }    
                       
                mem_modify(memory, offset);
                break;

            case 'q': /* Quit, but why please stay :( */
                printf("Goodbye.\n");
                return quit();

            case 'r': /* display registers function */
                display_registers();
                break;

            case 't': /* trace function */
                instruction_cycle(memory);  
                display_registers();
                break;

            case 'w': /* write file function */
                write_file(memory);
                break;

            case 'z': /* zero registers function */
                zero_registers();
                printf("Registers have been reset.\n");
                break;
            /* help this will display the menu to help user navigate */
            case '?':
            case 'h':
                menu();
                break;
	    
            default: /* This Virtual CPU is not a god, so it can't do anything its not trained to do */
		/* Invalid option- default response */
                printf("%c is not an option.  Enter H/h/? for help.\n", selection[0]);
        }
    }    
}

/*
 * go: run an entire program set as specified 
 * Revision Date: April 27th,2015
 */

int go(){
    return 0;
}


/*
 * dumpMemory: prompts user to enter a file_name and
 * than writes X(hex) #of bytes to that file.
 * will overwrite file contents, pending overwrite confirm. 
 * Lab Date: January 30th, 2015
 * Revision Date: April 27th,2015
 */
int dump_memory(void * memoryPtr, unsigned int offset, unsigned int length){
    unsigned int i;
    unsigned char line[MAX_LINE];
    unsigned int lineLength = MAX_LINE;
    unsigned int count = 0;

    while(count < length){
    
        if(offset == MAX_MEM){
            break;
        } 

        /* Print the offset value */
        printf("%04X\t", offset);
        /* Create the line */
        for(i = 0; i < MAX_LINE; i++, offset++, count++){
            /* Safeguard to ensure the pointer does not go out of memory */
            if(offset == MAX_MEM){
                lineLength = i;
                break;
            }
            /* Ensure that count does not exceed linelength */
            if(count == length){
                lineLength = i;
                break;
            }      
            line[i] = *((char*)memoryPtr + offset);
            printf("%02X ", line[i]);
        }
        /* Move to a newline and continue to ASCII to display dumped memory */   
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
 * LoadFile: prompts user to enter a file_name and
 * than loads that file into memory up to MAX_MEM bytes
 * Lab Date: January 30th, 2015
 * Revision Date: April 27th,2015
 */
int load_file(void * memory, unsigned int max){
    
    FILE* file;
    int fileBytes;
    unsigned char file_name[BUFFER];
    unsigned char mbuffer[BUFFER];
    unsigned char error[BUFFER];
    unsigned char flush[BUFFER];
    unsigned long fileSize;

    /* Get a file_name from user */
    printf("\nEnter a file_name: ");
    fgets(file_name, BUFFER, stdin);
    
    /* Remove the pesky last byte */
    file_name[strlen(file_name)-1] = '\0';

    /* Open the file in question */
    if((file = fopen(file_name, "rb")) == NULL){
        sprintf(error, "load: fopen: %s", file_name);
        perror(error);
        return -1;
    }

    printf("Loading %s to memory.\n", file_name);
    
    /* Determine the file size */
    if(fseek(file, 0, SEEK_END) == -1){
        perror("error: fseek");
        fclose(file);
        return -1;
    }
    
    /* Record file size as file position */
    if((fileSize = ftell(file)) == -1){
        perror("error: ftell");
        fclose(file);
        return -1;
    }
    
    /* reset to the begining of the file */
    rewind(file);

    /* Truncate file if it exceeds memory */
    if(max < fileSize){
        fileSize = max;
        printf("Data from file has been truncated to fit into memory.\n");
    }

    /* Read the file into memory */
    fileBytes = (int)fread(memory, MAX_BYTE, (size_t)fileSize, file);
    
    if(ferror(file)){
        perror("error: fread");
        fclose(file);
        return -1;
    }

    fclose(file);
    
    return (int)fileBytes;
}


/*
 * mem_modify: ask user for a hex offset and hex length
 * than begins modifying memory contents at that location until
 * terminating character of '.' is reached
 * Revision Date: April 27th,2015
 */
int mem_modify(void * memoryPtr, unsigned int offset){
    unsigned char input[BUFFER];
    unsigned char flush[BUFFER];
    unsigned int byte;
    unsigned int i;
    
    printf("*Enter a value in hex.*\nEnter '.' to stop modifying.\n\n");

    while(1){
        /* Print current offset and value */
        printf("%04X : %02X > ", offset, *((unsigned char*)memoryPtr + offset));
        // Get input */
        fgets(input, BUFFER, stdin);
	/* make sure there is no newline */
        input[2] ='\0';

        /* Check for terminating character */
        if(input[0] == '.'){
            break;    
        }
        /* Check for a non-value input */
        if(strcmp(input, "") == 0){
            continue;
        }
        /* Check if the user just pressed the "enter" key */
        if(strcmp(input, "\n") == 0){
            continue;
        }    

        /* change the selection to lower-case */
        for(i = 0; i < strlen(input); i++){
            input[i] = tolower(input[i]);
        }

        /* make sure value entered is a hex value */
        if(sscanf(input, "%x", &byte) == 0){
            continue;
        }

        /* Assign offset with new byte value */
        *((unsigned char*)memoryPtr + offset) = byte;
        ++offset;
        
	 /* Display if the user reaches the end of memory */
        if(offset == MAX_MEM){
            printf("End of memory.\n");
            break;
        }    
    }
    return 0;
}    

/*
 * quit: exit the program and continue living life
 * Revision Date: April 27th,2015
 */

int quit(){
    return 0;    
}


/*
 * display_registers: Display all registers and flags with their current values.
 * Revision Date: April 27th,2015
 */
int display_registers(){
    unsigned int i;

    /* Print contents of the regsiter file */
    for(i = 0; i < REGISTER_FILE; i ++){
        if(i % LINE_BREAK == 0){
            printf("\n");
        }
        if(i == REGISTER_FILE_SP){
            printf(" SP:%08X ", SP);
        }else if(i == REGISTER_FILE_LR){
            printf(" LR:%08X ",  LR);
        }else if(i == REGISTER_FILE_PC){
            printf(" PC:%08X ",  PC);
        }else{    
            printf("r%02d:%08X ", i,  regfile[i]);
        }    
    }    
    
    /* Print out all the flags */
    printf("\t SZC:%d%d%d", sign_flag, zero_flag, carry_flag);

    /* Print out the 'invisible' registers */
    printf("\n   MAR:%08X   MBR:%08X   IR0:%04X   IR1:%04X   Stop:%0d   IR Flag:%01d\n",  mar,  mbr, IR0, IR1, stop_flag, ir_flag);

    return 0;
}


/*
 * write_file: prompts user to enter a file_name and
 * than writes X(hex) #of bytes to that file.
 * will overwrite file contents, pending overwrite confirm. 
 * Revision Date: April 27th,2015
 */
void write_file(void * memory){
    
    FILE* file;
    int fileBytes;
    int writeBytes;
    unsigned char file_name[BUFFER];
    unsigned char error[BUFFER];
    unsigned char flush[BUFFER];

    /* Get a file_name from user */
    printf("\nEnter a file_name: ");
    fgets(file_name, BUFFER, stdin);
    
    file_name[strlen(file_name) -1] = '\0';

    /* Get number of bytes to write  */
    printf("\nEnter the amount of bytes to write: ");
    if(scanf("%d", &fileBytes) == 0){
        printf("Invalid input.\n");
        return;
    }    
    /* Flush input stream */
    fgets(flush, BUFFER, stdin);
    
    /* Check if number of bytes is greater than memory. */
    if(fileBytes > MAX_MEM){
        printf("File not written.  Cannot write more bytes than in memory.\n");
        return;
    
    }else if(fileBytes <= 0){
        printf("0 bytes written.\n");
        return;
    }

    /* Open the file */
    if((file = fopen(file_name, "wb")) == NULL){
        sprintf(error, "write: %s", file_name);
        perror(error);
        return;
    }

    /* Write memory to file */
    if((writeBytes = fwrite(memory, MAX_BYTE, fileBytes, file)) < 0){
        perror("dpu: write: ");
    }else{
        printf("%d bytes have been written to %s.\n", writeBytes, file_name);
    }
    
    /* Close the file */
    fclose(file);

    return;
}


/**
 *  zero_registers: Reset all registers to 0.
 *  Revision Date: April 27th,2015
 */
int zero_registers(){
    unsigned int i;
    
    /* Zero all visible registers*/
    for(i = 0; i < REGISTER_FILE; i++){
        regfile[i] = 0;
    }
    // set all flags to zero
    sign_flag = 0;
    zero_flag = 0;
    carry_flag = 0;
    stop_flag = 0;
    ir_flag = 0;
   /* Non-visible registers*/
    mar = 0;
    mbr = 0;
    ir = 0;
    cir = 0;
    
    return 0;
}


/*
 * Menu function this displays the menu options
 * Lab #: 3
 * Lab Date: January 30th, 2015
 * Revision Date: April 27th,2015
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


/*
 * Instruction Cycle: Consists of a fetch and an excute for that instruction
 * Revision Date: April 27th,2015
 */
void instruction_cycle(void * memory){
    /* Determine which Instruction register to use */
    if(ir_flag == 0){
        ir_flag = 1;
        /* Fetch new set of instructions */
        fetch(memory);
        /* Set current instruction to IR0 */
        cir = IR0;
        execute(memory);
    }else{
        ir_flag = 0;
        cir = IR1;
        execute(memory);
    }     
}


/*
 * Fetch: Fetch an instruction from memory at program counters address
 * Revision Date: April 27th,2015 
 */         
void fetch(void * memory){
    
    ir = load_registers(PC, memory);
    PC += MAX_REG;
}

/* Load Register: Load register with value in memory
 * Revision Date: April 27th,2015
 */
    uint32_t load_registers(uint32_t marValue, void * memory){
    unsigned int i;

    mar = marValue;

    for(i = 0; i < MAX_CYCLES; i++, mar++){
        mbr = mbr << SHIFT_BYTE;
        mbr += *((unsigned char*)memory + mar);
    }     
    return mbr;    
}

/* Store Register: Store current register into memory
 * Revision Date: April 27th,2015
 */
void store_registers(uint32_t marValue, uint32_t mbrValue, void * memory){
    
    mar = marValue;
    mbr = mbrValue;

    *((unsigned char*)memory + mar++) = (unsigned char)(mbr >> SHIFT_3BYTE & MASK_BYTE);
    *((unsigned char*)memory + mar++) = (unsigned char)(mbr >> SHIFT_2BYTE & MASK_BYTE);
    *((unsigned char*)memory + mar++) = (unsigned char)(mbr >> SHIFT_BYTE & MASK_BYTE);
    *((unsigned char*)memory + mar) = (unsigned char)mbr & MASK_BYTE;
}

/*
 * Execute: Determine instruction type, determine the required fields 
 * than execute the that instruction.
 * Revision Date: April 27th,2015
 */
void execute(void * memory){
    int i;
    
    /* Process Data */

    if(PROCESS_DATA){

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
            carry_flag = iscarry(regfile[RD], ~regfile[RN], 1);
            regfile[RD] = alu;
        }else if(DATA_SXB){
            alu = regfile[RN];
            if((alu & MSB_MASK8) == 1){
                alu += SIGN_EXT_32;
            }
            flags(alu);
            regfile[RD] = alu;
        }else if(DATA_ADD){
            alu = regfile[RD] + regfile[RN];
            flags(alu);
            carry_flag = iscarry(regfile[RD], ~regfile[RN], 0);
            regfile[RD] = alu;
        }else if(DATA_ADC){
            alu = regfile[RD] + regfile[RN] + carry_flag; 
            flags(alu);
            carry_flag = iscarry(regfile[RD], regfile[RN], carry_flag);
            regfile[RD] = alu;
        }else if(DATA_LSR){
            for(i = 0; i < regfile[RN]; i++){
                carry_flag = regfile[RN] & MASK_LSB;
                alu = regfile[RD] >> 1;
            }
            flags(alu);
            regfile[RD] = alu;
        }else if(DATA_LSL){
            for(i = 0; i < regfile[RN]; i++){
                carry_flag = regfile[RN] & MASK_LSB;
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
            carry_flag = iscarry(regfile[RD], ~regfile[RN], 1);
        }else if(DATA_ROR){
            for(i = 0; i < regfile[RN]; i++){
                carry_flag = regfile[RD] & MASK_LSB;
                alu = regfile[RD] >> 1;
                if(carry_flag){
                    alu |= MSB_MASK32;
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
    /* Load/Store */
    else if(LOAD_STORE){
        
        if(LOAD_BIT){
            /*Load a Byte*/
            if(BIT_FIELD){
                regfile[RD] = load_registers(regfile[RN], memory);
                regfile[RD] = regfile[RD] & MASK_BYTE;
            }
            /*Load a Double Word*/
            else{
                regfile[RD] = load_registers(regfile[RN], memory);
            }
        }else{
            mbr = regfile[RD];
            /* Store one byte of the register into cpu memory */
            if(BIT_FIELD){
                mar = regfile[RN];
                mbr = regfile[RD];
                *((unsigned char*)memory + mar) = (unsigned char)mbr & MASK_BYTE;
            }
            /* Store the double word */
            else{
                store_registers(regfile[RN], regfile[RD], memory);
            }
        } 
 
     /* Immediate Operations */
    }else if(IMMEDIATE){
        /* Move immediate value into regfile at register RD */
        if(MOV){
            regfile[RD] = IMMEDIATE_VALUE;    
            flags(regfile[RD]);
        }else if(CMP){
            alu = regfile[RD] + ~IMMEDIATE_VALUE + 1;
            flags(alu);
            carry_flag = iscarry(regfile[RD], ~IMMEDIATE_VALUE, 0);
        }else if(ADD){
            alu = regfile[RD] + IMMEDIATE_VALUE;
            flags(alu);
            carry_flag = iscarry(regfile[RD], IMMEDIATE_VALUE, 0);
            regfile[RD] = alu;
        }else if(SUB){
            alu = regfile[RD] + ~IMMEDIATE_VALUE + 1;
            flags(alu);
            carry_flag = iscarry(regfile[RD], ~IMMEDIATE_VALUE, 1);
            regfile[RD] = alu;
        }    
    
     /* Conditonal Branch */
    }else if(COND_BRANCH){
        /* Check condition codes and flags */
        if(chk_branch()){
            /* Add relative address making sure its a signed 8-bit value */
            alu = PC + (int8_t)CONDITION_ADDRESS;
	    if(ir_flag != 0){
                ir_flag = 0;
                alu = alu + ~THUMB + 1;
            }
            PC = alu;
        }        
    /* PUSH/PULL Instructions */
    }else if(PUSH_PULL){
        /* PULL */
        if(LOAD_BIT){
            /* Upper Registers */
            if(BIT_HIGH){
                /* Registers # 8 - 15 */
                for(i = REG_UPPR; i < REGISTER_FILE; i++){
                    if(chk_register( i - REG_HALF )){
                        regfile[i] = load_registers(SP & MASK_SP, memory);
                        alu = SP + MAX_REG;
                        SP = alu;
                    }
                }
            }
            /* Lower Registers */
            else{
                /* Registers # 0 - 7 */
                for(i = 0; i <= REG_LIMIT; i++){
                    if(chk_register(i)){
                        regfile[i] = load_registers(SP & MASK_SP, memory);
                        alu = SP + MAX_REG;
                        SP = alu;
                    }
                }
            }
	    /* Check for return call */
            if(RETURN_BIT){
                /* If the IR flag is set, change it to 0 */
                PC = load_registers(SP & MASK_SP, memory);
                if(ir_flag !=0){
                    ir_flag = 0;
                }
                alu = SP + MAX_REG;
                SP = alu;
            }

        }
        /* PUSH */
        else{
            if(RETURN_BIT){
                alu = SP + ~MAX_REG + 1;
                SP = (MASK_SP & alu) >> (unsigned) SHIFT_2BYTE;
                /* Store the Link Register/return address */
                store_registers(SP & MASK_SP, LR, memory);
            }
            if(BIT_HIGH){
                for(i = (REGISTER_FILE - 1); i >= REG_UPPR; i--){
                    if(chk_register( i - REG_HALF )){
                        alu = SP + ~MAX_REG + 1;
                        SP = alu;
                        store_registers(SP & MASK_SP, regfile[i], memory);
                    }
                }
            }else{
                for(i = REG_LIMIT; i >= 0; --i){
                    if(chk_register(i)){
                        alu = SP + ~MAX_REG + 1;    
                        SP = alu;
                        store_registers(SP & MASK_SP, regfile[i], memory);
                    }
                }
            }
        }
    }
     
    /* Unonditional Branch */
    else if(BRANCH){
        if(LINK_BIT){
            LR = PC;
        }    
        PC = OFFSET12;
        /* Make sure the IR flag is not still HI after the PC has changed. */
        ir_flag = 0;

    }else if(STOP){
        stop_flag = 1;
    }    

}    

/*
 *  chk_branch: Checks to see if a branch needs to be made
 *  this is done by checking condtion codes and flags.
 * Revision Date: April 27th,2015
 */
int chk_branch(){
    if(EQ){
        if(zero_flag){
            return 1;
        }    
    }else if(NE){
        if(zero_flag == 0){
            return 1;
        }
    }else if(CS){
        if(carry_flag){
            return 1;
        }
    }else if(CC){
        if(!carry_flag){
            return 1;
        }
    }else if(MI){
        if(sign_flag){
            return 1;      
        }    
    }else if(PL){
        if(!sign_flag){
            return 1;
        }
    }else if(HI){
        if(carry_flag && zero_flag == 0){
            return 1;   
        }
    }else if(LS){
        if(carry_flag == 0 || zero_flag){
            return 1;
        }
    }else if(AL){
        return 1;
    }
    
    return 0;
}


/*
 * chk_register: Checks register field for the current PUSH/PULL
 * instruction.
 * Revision Date: April 27th,2015
 */
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

/*
 * flags: This will check the alu and if it is not equal to zero
 * the zero_flag will be set to 1, if the alu has all zeros the zero_flag
 * is set to one.
 * Revision Date: April 27th,2015
 */          
void flags(uint32_t alu){        
    if(alu == 0){
        zero_flag = 1;
    }else{
        zero_flag = 0;
    }    
    
    sign_flag = (alu & MSB_MASK32) >> SHIFT_MSB_TO_LSB;

}

/*
 *   iscarry: determine if a carry is generated by the addition of op1+op2+C
 *   Revision Date: April 27th,2015
 */
int iscarry(uint32_t op1,uint32_t op2, uint8_t c){
    if ((op2== MAX32)&&(c==1)) 
        return(1);
    return((op1 > (MAX32 - op2 - c))?1:0);
}
