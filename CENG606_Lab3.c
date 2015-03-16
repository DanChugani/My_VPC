#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_MEM 0x4000
#define BUFFER_IN 0xFF
#define MENU 10
#define HEX_INPUT 4
#define HEX 0x10

void menu();
int LoadFile(void *memory, unsigned int max);
void WriteFile(void *memory);
void dumpMemory(void *memory, unsigned int offset, unsigned int length);
void modMemory(void *memory, unsigned int offset);

int main()
{
	static char memory[MAX_MEM];
	char selection; 
	int filesize;
	int size; /* result of load_file(); */
	unsigned int offset, length; /* for user input */


	
	while (selection !='q' || 'Q'){
	fflush(stdin);
	printf("\n\tEnter Selection h or ? to display options: ");
	fflush(stdin);
	fgets(&selection,MENU, stdin);
	

	
	switch(selection)
	{
	 case 'd':
	 case 'D':
		fprintf(stdout, "Enter an offset in hex: ");
		fscanf(stdin, "%X", &offset);
		fprintf(stdout, "Enter a length in hex: ");
		fscanf(stdin, "%X", &length);
		offset = (offset >= MAX_MEM)? 0 : offset;
		length = (length > (MAX_MEM - offset)) ? (MAX_MEM - offset) : length;
		dumpMemory((void *)memory, offset, length);
		break;
	 case 'g':
	 case 'G':
		printf(" go - run entire program\n");
		break;
	 case 'l':
	 case 'L':
		filesize = LoadFile(memory, MAX_MEM);
		printf("\nfile_size is %d\n",filesize);
		if (filesize>0)
		{
		 printf("\nnumber of bytes read %d",filesize);
		}
		else
		{
		 printf("\nerror-- reading file");
		}
		break;
	 case 'm':
	 case 'M':
		fprintf(stdout, "offset> ");
		fscanf(stdin, "%X", &offset);
		modmem((void *) memory, offset);
		break;
		
		break;
	 case 'q':
	 case 'Q':
		printf(" QUIT!\n");		
		return 0;
		break;
	 case 'r':
	 case 'R':
		printf(" display registers\n");
		break;
	 case 't':
	 case 'T':
		printf(" trace - execute one instruction\n");
		break;
	 case 'w':
	 case 'W':
		printf(" write file\n");
		WriteFile(memory);
		break;
	 case 'z':
	 case 'Z':
		printf(" reset all registers\n");
		break;
	 
	 case 'h':
	 case 'H':
	 case '?':

		printf("\n\t You've selected Help/Options please see below: \n");			
		menu();
		break;
	default:
		printf("\n Selection Entered:  %c",selection);
		printf("\nInvalid type please select right value\n");
		break;
	}
    }
}
  // Functions Declarations	
	/*
	 * menu function this displays the menu options
	 *
	*/
	void menu()
	{
 	 printf("\n\tPlease select an option below: \n");
 	 printf("\t d\tdump memory\n");
 	 printf("\t g\tgo - run the entire program\n");
 	 printf("\t l\tload a file into memory\n");
 	 printf("\t m\tmemory modify\n");
 	 printf("\t q\tquit\n");
 	 printf("\t r\tdisplay registers\n");
 	 printf("\t t\ttrace - execute one instruction\n");
 	 printf("\t w\twrite file\n");
 	 printf("\t z\treset all registers to zero\n");
 	 printf("\t ?,h\tdisplay list of commands\n");	 	
	}

	int LoadFile(void *memory, unsigned int max)
	{
	int size = 0;
	char file_name[BUFFER_IN];
	int result;
	
	fprintf(stdout, "Enter name of file to read: ");
	fgets(file_name, BUFFER_IN, stdin);	
	file_name[strlen(file_name) - 1] = '\0';

	FILE *f = fopen(file_name, "rb");

		if (f == NULL) 
		{ 
			return -1; // -1 means file opening fail 
		} 

	if(fseek(f, 0, SEEK_END) == 0)
	{
		size = ftell(f);
	}
	else
	{
	return -2;
	}
	if(size > max)
	{
	printf("the file has been truncated...my condolences\n");
	size = max;	
	}
	rewind(f);
	return size;

	}

	void WriteFile(void *memory)
	{

	int size;
	unsigned int res; /* result of fwrite */
	char file_name[BUFFER_IN];
	FILE *f;

	/* Get the user to input a new file name. */
	printf("name of file: ");
	fgets(file_name, BUFFER_IN, stdin);
	file_name[strlen(file_name) - 1] = '\0';
	f= fopen((const char*) file_name, "w+");

	if(f == NULL) {

	printf("\nerror opening file to write.");
	exit(1);
	}
	

	/* Ask for how many bytes to write. */
	printf( "Number of bytes in hex to write to file(max value of 0xFFFF): ");
	 fscanf(stdin, "%X", &size);
	if(size > MAX_MEM){
	size = MAX_MEM;
	}
	/* actually write the memory to       a file */
	if((res = fwrite(memory, 1, size, f)) > 0){
	printf("\nwrote %d bytes to file successfully.\n", res);
	}
	else{
	printf("\nerror writing file.\n");
	}
	fclose(f);

	}

	void dumpMemory(void *memory, unsigned int offset, unsigned int length)
	{
	unsigned int i, j;
	unsigned int row_length = 0x10;
	
	length--; /* for some reason it needs this? */

	for(i = offset; i < (offset + length); i += row_length) {
	/* the row off set number */
	fprintf(stdout, "%4X\t", i);
	
	/* top row of the display, just the hex value in memory */
	for(j = i; j < (i + row_length); j++) {
	fprintf(stdout, "%2X ", *((unsigned char *) memory + j));
	if(j == (offset + length))
	break;
	}
	fprintf(stdout, "\n\t");
	/* the contents of the memory */
	for(j = i; j < (i + row_length); j++) {
	if(isprint((int) *((char *) memory + j)))
	fprintf(stdout, " %c ", *((char *) memory + j));
	else
	fprintf(stdout, " . ");
	if(j == (offset + length))
	break;
	}
	fprintf(stdout, "\n");
	/* stop the routine if its reached the limit. */
	if(j == length)
	break;
	}
	return;
	}

	void modMemory(void *memory, unsigned int offset)
	{
	char input[HEX_INPUT];
	unsigned long buffer;
	
	fprintf(stdout, "modify the memory. enter '.' to end.\n");
	/* Failsafe the memory location to manipulate. */
	if(offset >= MAX_MEM)
	offset = 0;

	while(getchar() != '\n');
	while(1) {
	fprintf(stdout, "%4X> ", offset);
	if(fgets(input, HEX_INPUT, stdin) == NULL) {
	perror("fgets");
	return;
	}
	else
	input[(strlen(input) - 1)] = '\0'; /* remove the \n */
	if(strcmp(input, ".") == 0)
	return;
	/* stroul() = string to unsigned long */
	buffer = strtoul(input, NULL, HEX);
	*((char *) memory + offset) = (char) buffer;
	if(++offset == MAX_MEM)
	return;
	}
	return;
	}
