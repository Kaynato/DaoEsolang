/*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
* DaoLanguage / Daoyu Compiler and Interpreter.
* Zicheng Gao - 2016
* See splash() for details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FILE_SYMBOLIC ".dao"
#define FILE_COMPILED ".wuwei"
#define DEFAULT_INTERPRET_CELL_LENGTH 32
#define BITS_IN_BYTE	8
#define BITS_IN_CELL 	(sizeof(unsigned long) * 8)
#define BYTE_MASK		0xff

typedef struct PATH
{
	struct PATH*	owner;						/* OWNER      PROGRAM */
	struct PATH*	child;						/* CHILD      PROGRAM */
	unsigned long*	prg_data;					/*		   DATA	      */
	unsigned long	prg_allocbits;				/* OPEN	   DATA   BITS*/
	unsigned long	prg_index;					/* INSTRUCTION POINTER*/
	unsigned char	prg_level;					/* OPERATING   LEVEL  */
	unsigned long	sel_length;					/* LENGTH OF SELECTION*/
	unsigned long	sel_index;					/* INDEX  OF SELECTION*/
	unsigned int    prg_floor;					/* FLOOR  OF PATH     */
	unsigned long   prg_start;					/* START  OF RUNNING  */
} Pathstrx;

typedef Pathstrx* Path;

static void prompt();
static void compile(FILE*, FILE*, char*);
static void interpret(char*);

static void swaps(Path), later(Path), merge(Path), sifts(Path), delev(Path), equal(Path), halve(Path);
static void uplev(Path), reads(Path), dealc(Path), split(Path), polar(Path), doalc(Path), input(Path), execs(Path, Path);

void 			flip_UL(unsigned long*);
void			freeparsedargs(char **argv);
char 			algn(Path);
char 			getChar(unsigned char);
char*			bin(unsigned long);
char*			str_dup(char *s);
char*			set_option(char*, char);
char*			l_to_str(unsigned long, unsigned char, unsigned char, unsigned char);
char**			parsedargs(char *arguments, int *argc);
static void		skip();
static void 	flags();
static void 	splash();
static void		bin_print(Path);
static void		diagnose(Path, unsigned char);
static void 	write_by_bit_index(Path, unsigned long, unsigned long, unsigned long);
unsigned char 	getNybble(char);
unsigned long 	read_by_bit_index(Path, unsigned long, unsigned long);
unsigned long 	mask(int);

static unsigned char command = 0;
static int doloop = 1;

typedef void(*PathFunc)(Path);

static PathFunc functions[16] = \
	{NULL, swaps, later, merge, \
	sifts, NULL , delev, equal, \
	halve, uplev, reads, dealc, \
	split, polar, doalc, input};

const struct PATH NEW_PATH = { NULL, NULL, NULL, 1, 0, 0, 1, 0, 0, 0 };

#define is_option(str) (str[0] == '-' && str[1] != 0 && str[2] == 0)
#define verbprint(x) verbosely{printf(x);}
#define verbosely if (VERBOSE)

static char VERBOSE = 0,
			COMP_ONLY = 0,
			FORCE = 0,
			HIDE_DATA = 0,
			PRINT_CODE = 0,
			SKIP_OVERFLOW = 0,
			PRINT_EVERYTHING = 0;
static Path P_RUNNING = NULL,
			P_WRITTEN = NULL;
static const char* symbols = ".!/)%#>=(<:S[*$;";

/***
 *    ooo        ooooo       .o.       ooooo ooooo      ooo 
 *    `88.       .888'      .888.      `888' `888b.     `8' 
 *     888b     d'888      .8"888.      888   8 `88b.    8  
 *     8 Y88. .P  888     .8' `888.     888   8   `88b.  8  
 *     8  `888'   888    .88ooo8888.    888   8     `88b.8  
 *     8    Y     888   .8'     `888.   888   8       `888  
 *    o8o        o888o o88o     o8888o o888o o8o        `8  
 *                                                          
 *                                                          
 *                                                          
 */

int main(int argc, char * argv[])
{
	char* fileName = NULL;
	FILE* inputFile = NULL;

	if (argc < 2)
	{
		splash();
		prompt();
		return 0;
	}
	else
		fileName = argv[1];

	while (argc-- > 2)
		if (is_option(argv[argc])) set_option(argv[argc], 1);

	if ((inputFile = fopen(fileName, "rb")) == NULL)
	{
		printf("Could not find \"%s\" - is it in this directory?\n", fileName);
		fclose(inputFile);
		return 1;
	}

	if (~strcmp(FILE_SYMBOLIC, &fileName[strlen(fileName) - 4]))
	{
		FILE* outputFile = NULL;
		fileName[strlen(fileName) - 4] = 0;
		fileName = strncat(fileName, FILE_COMPILED, sizeof(FILE_COMPILED));
		outputFile = fopen(fileName, "wb+");
		compile(inputFile, outputFile, fileName);
	}

	if (COMP_ONLY)
		return 0;

	if (FORCE || ~strcmp(FILE_COMPILED, &fileName[strlen(fileName) - 6]))
		interpret(fileName);

	return 0;
}

/***
 *      .oooooo.     .oooooo.   ooo        ooooo ooooooooo.   ooooo ooooo        oooooooooooo 
 *     d8P'  `Y8b   d8P'  `Y8b  `88.       .888' `888   `Y88. `888' `888'        `888'     `8 
 *    888          888      888  888b     d'888   888   .d88'  888   888          888         
 *    888          888      888  8 Y88. .P  888   888ooo88P'   888   888          888oooo8    
 *    888          888      888  8  `888'   888   888          888   888          888    "    
 *    `88b    ooo  `88b    d88'  8    Y     888   888          888   888       o  888       o 
 *     `Y8bood8P'   `Y8bood8P'  o8o        o888o o888o        o888o o888ooooood8 o888ooooood8 
 *                                                                                            
 *                                                                                            
 *                                                                                            
 */

static void compile(FILE* input, FILE* output, char* inputFileName)
{
	unsigned char emptyBuffer = 1, toWrite = 0, isComment = 0, k = 0;
	int ch;
	verbosely printf("\n%s%s\n", "Compiling to ", inputFileName);
	while ((ch = fgetc(input)) != EOF)
	{
		switch ((char)ch)
		{
		case '\t':			case ' ':			break;
		case '@': 			isComment = 1; 		break;
		case (char)0x0D:
		case (char)0x0A: 	isComment = 0; 		break;
		default:
			if (isComment)
				break;
			verbosely putchar((char)ch);
			if (!emptyBuffer)
			{
				toWrite |= getNybble((char)ch);
				fputc(toWrite, output);
				verbosely printf(" %s ", l_to_str(toWrite, 2, 16, 1));
				if (++k == 8)
				{
					k = 0; 
					verbosely putchar('\n');
				}
			}
			else
				toWrite = (char)(getNybble((char)ch) << 4);
			emptyBuffer = !emptyBuffer;
			break;
		}
	}

	if (!emptyBuffer) {
		fputc(toWrite, output);
		verbosely printf(". %x\n", toWrite);
	}

	verbprint("Finished compiling.\n");
	fclose(input);
	fclose(output);
}

#define rc(r,c) case c: return r;

unsigned char getNybble(char ch)
{
	switch (ch)
	{
		rc(0x0, '.')	rc(0x1, '!')	rc(0x2, '/')	rc(0x3, ']': case ')')
		rc(0x4, '%')	rc(0x5, '#')	rc(0x6, '>')	rc(0x7, '=')
		rc(0x8, '(')	rc(0x9, '<')	rc(0xA, ':')	rc(0xB, 'S')
		rc(0xC, '[')	rc(0xD, '*')	rc(0xE, '$')	rc(0xF, ';')
		default: return 0x0;
	}
}

/***
 *    ooooo ooooo      ooo ooooooooooooo oooooooooooo ooooooooo.   ooooooooo.   ooooooooo.   oooooooooooo ooooooooooooo 
 *    `888' `888b.     `8' 8'   888   `8 `888'     `8 `888   `Y88. `888   `Y88. `888   `Y88. `888'     `8 8'   888   `8 
 *     888   8 `88b.    8       888       888          888   .d88'  888   .d88'  888   .d88'  888              888      
 *     888   8   `88b.  8       888       888oooo8     888ooo88P'   888ooo88P'   888ooo88P'   888oooo8         888      
 *     888   8     `88b.8       888       888    "     888`88b.     888          888`88b.     888    "         888      
 *     888   8       `888       888       888       o  888  `88b.   888          888  `88b.   888       o      888      
 *    o888o o8o        `8      o888o     o888ooooood8 o888o  o888o o888o        o888o  o888o o888ooooood8     o888o     
 *                                                                                                                      
 *                                                                                                                      
 *                                                                                                                      
 */

static void interpret(char* inputFileName)
{
	FILE* inputFile = fopen(inputFileName, "rb");
	unsigned long bytes_read = 0;	

	/*************************                           RUN THE CODE                           *************************/
	unsigned long 	print_index = 0;								/* Index for printing                               */
	unsigned long	bytes_alloc = 0;								/* Bytes allocated to data                          */
	unsigned long	file_size = 0;									/* Byte size of file 								*/
	unsigned long	shift = 0;										/* Shift for first one of file size for rounding    */

	struct PATH newpath = NEW_PATH;									/* Make a new PATH with the initialization values.	*/
	Path dao = &newpath;											/* Make a pointer to the newly initialized PATH.	*/

	verbosely
	{
		printf("\n\n\t=====================\n"  );
		printf(    "\t|Beginning Execution|\n"  );
		printf(    "\t=====================\n\n");
	}

	fseek(inputFile, 0L, SEEK_END);									/* Find size of input file in bytes.				*/
	file_size = ftell(inputFile);									/*													*/
	fseek(inputFile, 0L, SEEK_SET);									/* Rewind file.									 	*/
	/*************************ROUND DATA ARRAY SIZE TO LOWEST POWER OF TWO LARGER THAN FILE SIZE*************************/
	bytes_alloc = file_size;										/* Initialize bytes_alloc with the file_size value. */

	while ((bytes_alloc >> 1) != 0)									/* Determine leftmost '1' bit of file size.		 	*/
	{																/*													*/
		bytes_alloc >>= 1;											/* Shift right until the next shift zeroes it.		*/
		shift++;													/* Keep track of shifts.							*/
	}																/*													*/
	bytes_alloc <<= shift; 											/* Unshift. 										*/
	if (file_size != bytes_alloc)									/* If not a power of two, round up.					*/
		bytes_alloc <<= 1;
	(dao->prg_allocbits) = bytes_alloc * 8;							/* Set literal allocation bit size.				 	*/
	/********************************************************************************************************************/

	verbosely printf("%s%s.\nLoading data:\n", "Running ", inputFileName);

	if (bytes_alloc % sizeof(unsigned long) != 0)					/* Only occurs if it's less than one UL, one cell   */
		bytes_alloc = sizeof(unsigned long);						/* Set the minimum									*/

	if (((dao->prg_data) = calloc(bytes_alloc, 1)) == NULL)			/* Allocate data array to bytes needed.				*/
	{
		printf("Error allocating %d bytes: ", bytes_alloc);
		perror("");
		abort();
	}
	verbosely printf("Allocated %d bytes for %d byte file.\n", bytes_alloc, file_size);

	bytes_read = fread((dao->prg_data), 1, file_size, inputFile);

	verbosely printf("Read %d bytes.\n\n", bytes_read);				/* Read file data into data array.					*/

	while (print_index++ < (bytes_alloc / sizeof(unsigned long)))	/* Traverse the array, and...						*/
	{
		flip_UL((dao->prg_data) + print_index - 1);					/* Flip the byte order to the correct one 			*/
		verbosely
		{
			printf("%s   ", l_to_str((dao->prg_data)[print_index - 1], 8, 16, 0));/* If verbose, print out array contents*/
		if (print_index % 8 == 0) printf("\n");						/* New line every seven unsigned longs.				*/
		}
	}

	verbosely printf("(%d bytes)\n\n", (dao->prg_allocbits) / 8);			/* If verbose, output number of bytes.				*/
	P_RUNNING = dao;												/* For the sake of levlim							*/
	
	/***************************************************** EXECUTE ******************************************************/
	execs(dao, NULL);
	verbosely printf("Freeing %d bytes of data.\n", bytes_alloc);
	free((dao->prg_data));
	(dao -> prg_data) = NULL;
	verbprint("Data freed.\n")
	/********************************************************************************************************************/

}

/***
 *    ooooooooo.   ooooooooo.     .oooooo.   ooo        ooooo ooooooooo.   ooooooooooooo 
 *    `888   `Y88. `888   `Y88.  d8P'  `Y8b  `88.       .888' `888   `Y88. 8'   888   `8 
 *     888   .d88'  888   .d88' 888      888  888b     d'888   888   .d88'      888      
 *     888ooo88P'   888ooo88P'  888      888  8 Y88. .P  888   888ooo88P'       888      
 *     888          888`88b.    888      888  8  `888'   888   888              888      
 *     888          888  `88b.  `88b    d88'  8    Y     888   888              888      
 *    o888o        o888o  o888o  `Y8bood8P'  o8o        o888o o888o            o888o     
 *                                                                                       
 *                                                                                       
 *                                                                                       
 */

#define arg_is(i, a) (!strcmp(parsed[i], a))

static unsigned char hasExtension(char*);
static int  		 parsePosInt(char*, unsigned int);
static void 		 rad_print(Path, unsigned int);
static void			 flag(char**, int);

static void prompt()
{
	int ac;
	unsigned char prompting = 1;
	char* input = calloc(2048, sizeof(char));
	char** parsed = NULL;


	while(prompting)
	{
		fputs("DAOYU :: ", stdout);
		fgets(input, 2048, stdin);
		parsed = parsedargs(input,&ac);
		
		/* Input is non-empty*/
		if (ac > 0)
		{
			/*************************************************** QUIT OPTION CASE ***************************************************/
			if (arg_is(0, "quit") || arg_is(0, "q") || arg_is(0, "exit") || arg_is(0, "escape"))
			{
				printf("Quitting. . .\n");
				prompting = 0;
			}
			/*************************************************** HELP OPTION CASE ***************************************************/
			else if (arg_is(0, "help") || arg_is(0, "?"))
			{
				if (ac == 1)
				{
					printf("\tUse: help [command] to see detailed information.\n");
					printf("\t\thelp : get help\n");
					printf("\t\tquit : quit\n");
					printf("\t\tflag : set flags\n");
					printf("\t\trun <file> : run file\n");
					printf("\t\tcompile <file> : compile file\n");
					printf("\t\tnew : enter code mode for immediate interpretation\n");
				}
				else
				{
					if (arg_is(1, "help"))
						printf("Use: help [command] to see detailed information.\n");
					if (arg_is(1, "quit"))
						printf("\tExits the program.\n");
					if (arg_is(1, "flag"))
					{
						printf("\tSet flags.\n");
						printf("\t\tUse: flag <flag> <ON/OFF> <flag2> <ON/OFF> ...\n");
						printf("\t\tFlags available:\n");
						flags();
					}
					if (arg_is(1, "run"))
					{
						printf("\tRun a Daoyu file.\n");
						printf("\t\tUse: run <filename>\n");
						printf("\t\tIf filename is not specified to be .dao or .wuwei, this will run filename.wuwei if it exists.\n");
						printf("\t\tIf -f is ON, this will run filename regardless.\n");
						printf("\t\tOtherwise it will compile and run filename.dao.\n");
						printf("\t\tIf an extension is specified, this will only attempt to compile (if needed) and run that file.\n");
					}
					if (arg_is(1, "compile"))
					{
						printf("\tCompile a Daoyu file.\n");
						printf("\t\tUse: compile <filename>\n");
						printf("\t\tIf filename has no extension, this will compile filename.dao if it exists.\n");
						printf("\t\tIf -f is ON, this will attempt to compile filename as a .dao file regardless.\n");
					}
					if (arg_is(1, "new"))
					{
						printf("\tEnter code mode.\n");
						printf("\t\tStarts immediate interpretation on a newly generated path.\n");
					}
				}
			}
			/*************************************************** FLAG OPTION CASE ***************************************************/
			else if (arg_is(0, "flag") || arg_is(0, "flags") || arg_is(0, "f"))
				flag(parsed, ac);
			/***************************************************  RUN OPTION CASE ***************************************************/
			else if (arg_is(0, "run") || arg_is(0, "r"))
			{
				if (ac > 1)
				{
					FILE* inputFile = NULL;
					char* fileName = parsed[1];
					char* exeName = str_dup(fileName);
					char* daoName = str_dup(fileName);

					if (!hasExtension(fileName))
					{
						exeName = strncat(exeName, FILE_COMPILED, sizeof(FILE_COMPILED));
						if ((inputFile = fopen(exeName, "rb")) != NULL)
						{
							fclose(inputFile);
							interpret(exeName);
						}
						else
						{
							daoName = strncat(daoName, FILE_SYMBOLIC, sizeof(FILE_SYMBOLIC));
							if ((inputFile = fopen(daoName, "rb")) != NULL)
							{
								FILE* outputFile = fopen(exeName, "wb+");
								compile(inputFile, outputFile, exeName);
								interpret(fileName);
							}
							else
							{
								printf("Could not find \"%s\" - is it in this directory?\n", parsed[1]);
								fclose(inputFile);
							}
						}
					}
					else if ((inputFile = fopen(fileName, "rb")) != NULL)
					{
						if (~strcmp(FILE_SYMBOLIC, &fileName[strlen(fileName) - 4]))
						{
							FILE* outputFile = NULL;
							fileName[strlen(fileName) - 4] = 0;
							fileName = strncat(fileName, FILE_COMPILED, sizeof(FILE_COMPILED));
							outputFile = fopen(fileName, "wb+");
							compile(inputFile, outputFile, fileName);
						}
						if (FORCE || ~strcmp(FILE_COMPILED, &fileName[strlen(fileName) - 6]))
							interpret(fileName);
					}
					else
					{
						printf("Could not find \"%s\" - is it in this directory?\n", fileName);
						fclose(inputFile);
					}
				}
				else
					printf("Please input a filename to run.\n");
			}
			/************************************************** COMPILE OPTION CASE *************************************************/
			else if (arg_is(0, "compile") || arg_is(0, "c"))
			{
				if (ac > 1)
				{
					FILE* inputFile = NULL;
					char* fileName = parsed[1];
					
					/* If it has no extension and it is not being forced, look the .dao file */
					if (!FORCE && !hasExtension(fileName))
						fileName = strncat(fileName, FILE_SYMBOLIC, sizeof(FILE_SYMBOLIC));
					
					/* If such a file exists, AND (is of correct extension OR you are forcing) */
					if ((inputFile = fopen(fileName, "rb")) != NULL && (FORCE || ~strcmp(FILE_SYMBOLIC, &fileName[strlen(fileName) - 4])))
					{
						FILE* outputFile = NULL;
						if (!FORCE)												/* If not forcing, truncate .dao to add .wuwei	*/
							fileName[strlen(fileName) - 4] = 0;
						fileName = strncat(fileName, FILE_COMPILED, sizeof(FILE_COMPILED));	/* Add .wuwei for output file 		*/
						outputFile = fopen(fileName, "wb+");								/* Open output file 				*/
						compile(inputFile, outputFile, fileName);
					}
					else
					{
						printf("Could not find \"%s\" - is it in this directory?\n", fileName);
						fclose(inputFile);
					}
				}
				else
					printf("Please input a filename to compile.\n");
			}
			/**************************************************** NEW OPTION CASE ***************************************************/
			else if (arg_is(0, "new") || arg_is(0, "`") || arg_is(0, ">") || arg_is(0, ">>") || arg_is(0, "n"))
			{
				unsigned char activeinterpret = 1;								/* Loop control 									*/
				struct PATH newpath = NEW_PATH;									/* Initialize path with initialization values.		*/
				Path TLP = &newpath;											/* Use the correct format (a pointer)				*/

				printf("\tCODE MODE INITIALIZED\n");
				printf("\tWarning: This mode is not recommended. It is safer to write .dao files.\n");
				printf("\tPrograms may crash for various reasons if initialized through code mode.\n");
				printf("\tIt is also impractical to retrieve stored programs through this mode.\n");

				P_RUNNING = TLP;												/* Set running 										*/

				if (((TLP -> child) = (calloc(1, sizeof(struct PATH)))) == NULL)/* Allocate memory space 							*/
				{																/* Cover error case							 		*/
					printf("FATAL ERROR: Unable to allocate memory.");
					return;
				}

				verbosely printf("Allocated %d bytes.\n\n", sizeof(*(TLP -> child)));

				memcpy((TLP -> child), &NEW_PATH, sizeof(struct PATH));			/* Copy over initialization data			 		*/
				((TLP -> child) -> owner) = TLP;								/* Set owner of this new Path 						*/
				((TLP -> child) -> prg_floor) = (TLP -> prg_floor) + 1;			/* Set floor of this new Path 						*/
				((TLP -> child) -> prg_data) = calloc(1, sizeof(unsigned long));/* Set data  of this new Path 						*/
				P_WRITTEN = (TLP -> child);										/* Set this as written on 							*/

				(TLP -> prg_allocbits) = BITS_IN_CELL;
				if (((TLP->prg_data) = calloc(DEFAULT_INTERPRET_CELL_LENGTH, sizeof(unsigned long))) == NULL)	/* Allocate data space 	*/
				{
					printf("Error allocating %d bytes", DEFAULT_INTERPRET_CELL_LENGTH * sizeof(unsigned long));
					perror("");
					return;
				}

				freeparsedargs(parsed);

				while (activeinterpret) {
					fputs("dao > ", stdout);
					fgets(input, 2048, stdin);
					parsed = parsedargs(input,&ac);

					/* Inputs to care about: 
						:: new
							enter code mode, makes a path
							non-tilded things will be taken as code
							> ~kill / ~quit
								abort execution
							> ~what / print / show [x]
								prints out path in base-x
								default is binary (2) if small enough, hex if bigger. use bin_print.
							> ~flag(s)
								same deal
							> @heaven / above / up / higher / high
								path above current path
							> @earth / under / down / lower / low
								path below current path
					*/

					/* Any args inputted */
					if (ac > 0)
					{
						if (arg_is(0, "help") || arg_is(0, "?"))
						{
							/* help */
							printf("\tUse: help [command] for more information.\n");
							printf("\t\tExample: help ~end\n\n");
							printf("\t\t~end : terminate program and exit immediate interpretation\n");
							printf("\t\t~print <~path> <x> : print out a representation of the path in base x numerals.\n");
							printf("\t\t\tDefault path is current.\n");
							printf("\t\t\tDefault base is automatically determined.\n");
							printf("\t\t@above : keyword for the above path. Can be chained. Causes error if nonexistent.\n");
							printf("\t\t@below : keyword for the below path. Can be chained. Causes error if nonexistent.\n\n");
							printf("\t\tAny other input is taken as code to be immediately loaded into and executed from the top-level program.\n");
						}
						if (parsed[0][0] == '~')
						{
							if (arg_is(0, "~end") || arg_is(0, "~quit") || arg_is(0, "~q") || arg_is(0, "~kill") || arg_is(0, "~exit"))
								activeinterpret = 0;
							else if (arg_is(0, "~flag") || arg_is(0, "~flags") || arg_is(0, "~f"))
								flag(parsed, ac);
							else if (arg_is(0, "~print") || arg_is(0, "~what") || arg_is(0, "~show"))
							{
								/* Check if there is a keyword combination and then check for an indication of base. */
								int i = 1;

								/* 0: autobase. Other: base. Negative: error */
								int base = 0;

								/* 0: Keyword det. 1: Base det. -1: Nonexistent path. */
								char portion = 0;

								/* Default. */
								Path pathToPrint = TLP;

								/* Advance pointer across args to determine keyword */
								while (i < ac && portion == 0)
									/* Is keyword */
									if (arg_is(i, "@above") || arg_is(i, "@below"))
									{
										if (arg_is(i, "@above"))
											pathToPrint = (pathToPrint -> owner);
										else
											pathToPrint = (pathToPrint -> child);
										if (pathToPrint == NULL)
											portion = -1;
										i++;
									}
									/* Is not keyword: Kicks out of loop via portion */
									else
										portion = 1;
								/* If the path exists */
								if (portion != -1)
								{
									/* More args after keywords */
									if (portion == 1)
									{
										/* Parse int of parsed[i]. Is it even a number? */
										base = parsePosInt(parsed[i], 36);
										/* Incorrect format */
										if (base == -1)
										{
											printf("%s is not a valid positive integer. Reverting to default.\n", parsed[i]);
											base = 0;
										}
									}
									/* For the default base, set it according to the allocbits */
									if (base == 0)
										base = ((TLP -> prg_allocbits) > 32) ? 16 : 2;
									/* Then with base set, we can go and print accordingly */
									rad_print(pathToPrint, (unsigned int)base);
									putchar('\n');
								}
								else
									printf("PRINT ERROR: Such a path does not exist.\n");
							}
						}
						else
						{
							/* Code for immediate interpretation */
							int i = 0;
							int j = 0;
							int lim = 0;
							/* For each arg (why would you space them though come on */
							for (; i < ac; i++)
							{
								/* For each character */
								for (j = 0, lim = strlen(parsed[i]); j < lim; j++)
								{
									command = getNybble(parsed[i][j]);
									/* Insert parsed symbol into the Top level program */
									write_by_bit_index(TLP, (TLP -> prg_index), 4, command);
									/* Increase size if necessary */
									if ((TLP -> prg_index) > (TLP -> prg_allocbits))
										doalc(TLP);

									/* DEALC condition through this is a problem. */

									if (command == 5)
										execs(P_WRITTEN, P_RUNNING);
									else if (command != 0)
										functions[command](P_WRITTEN);

									if (doloop)
									{
										verbosely diagnose(P_RUNNING, command);
										verbprint("\n")
									}
									else
									{
										verbosely printf("Freed %d bytes.\n\n", sizeof(*P_WRITTEN));
										free(P_WRITTEN);
									}
								}
							}
						}
					}
					freeparsedargs(parsed);
				}
				/* Deallocate the paths involved to avoid a memory leak!! */
				free((TLP -> child));
			}
			/************************************************** INVALID OPTION CASE *************************************************/
			else printf("%s is not a recognized or valid option.\n", parsed[0]);
		}
	}
	freeparsedargs(parsed);
}

static void	flag(char** parsed, int ac)
{
	int i = 1;
	/* We want the format: -flag ON/OFF */
	for (; i < ac; i++)
	{
		if (is_option(parsed[i]))							/* This one sets a flag - so we should advance i right here and now.*/
		{
			if (++i < ac) 																		/* Ensure it's still in bounds  */
			{
				if (arg_is(i, "ON") || arg_is(i, "OFF")) 										/* Next input is correct form   */
				{
					set_option(parsed[i-1], strcmp(parsed[i], "OFF")); 							/* Set option correctly. 		*/
					printf("Set option -%c to %d.\n", parsed[i-1][1], strcmp(parsed[i], "OFF"));
				}
				else																  			/* Incorrect form 				*/
					printf("Syntax error: Expected \"OFF\" or \"ON\" after flag assignment -%c.\n", parsed[i-1][1]);
			}
			else
				printf("Syntax error: Expected \"OFF\" or \"ON\" after flag assignment -%c.\n", parsed[i-1][1]);
		}
		else
			printf("%s is an invalid option.\n", parsed[i]);
	}
	if (i == 1)
		printf("Flag assignment expected. Use -<flag> ON/OFF.\n");
}

static unsigned char hasExtension(char* input)
{
	unsigned int i = 0;
	unsigned int length = strlen(input);
	for (; i < length; i++)
		if (input[i] == '.')
			return 1;
	return 0;
}

static int parsePosInt(char* input, unsigned int max)
{
	unsigned int i = 0;
	unsigned int out = 0;
	unsigned int len = strlen(input);
	char val = 0;
	for (; i < len; i++)
	{
		out *= 10;
		val = input[i] - '0';
		if (val >= 0 && val < 10)
			out += val;
		else
			return -1;
		if (out > max)
			return max;
	}
	return out;
}

static int setargs(char *args, char **argv)
{
	int count = 0;
	while (isspace(*args)) ++args;
	while (*args)
	{
		if (argv)
			argv[count] = args;
		while (*args && !isspace(*args))
			++args;
		if (argv && *args)
			*args++ = '\0';
		while (isspace(*args))
			++args;
		count++;
	}
	   return count;
	}

char **parsedargs(char *args, int *argc)
{
	char **argv = NULL;
	int	argn = 0;

	if (args && *args
		&& (args = str_dup(args))
		&& (argn = setargs(args,NULL))
		&& (argv = malloc((argn+1) * sizeof(char *)))) 
	{
		*argv++ = args;
		argn = setargs(args,argv);
	}
	
	if (args && !argv)
		free(args);

	*argc = argn;
	return argv;
}

void freeparsedargs(char **argv)
{
	if (argv)
	{
		free(argv[-1]);
		free(argv-1);
	} 
}

/***
 *    oooooooooooo ooooooooooooo   .oooooo.   
 *    `888'     `8 8'   888   `8  d8P'  `Y8b  
 *     888              888      888          
 *     888oooo8         888      888          
 *     888    "         888      888          
 *     888       o      888      `88b    ooo  
 *    o888ooooood8     o888o      `Y8bood8P'  
 *                                            
 *                                            
 *                                            
 */

#define roc(o,v,c) case o:c=v; return &c;

char* set_option(char* str, char value)
{
	if (is_option(str))
		switch (str[1])
		{
		roc('f', value, FORCE)
		roc('v', value, VERBOSE)
		roc('c', value, COMP_ONLY)
		roc('h', value, HIDE_DATA)
		roc('d', value, PRINT_CODE)
		roc('s', value, SKIP_OVERFLOW)
		roc('p', value, PRINT_EVERYTHING)
		default:
			printf("Unknown option -%c.\n\n", str[1]);
		}
	return NULL;
}

static void flags()
{
	printf("\t-c : Compile without running\n");
	printf("\t-d : Print code instead of numeric values.\n");
	printf("\t-v : Enable Verbose Execution (For Debugging)\n");
	printf("\t-w : Get Input before closing (For Debugging)\n");
	printf("\t-f : Force Execution of Any FILE* as COMPILED DAOYU (DANGEROUS)\n");
	printf("\t-p : Print all data in every 32 tetrad line, even if all zeroes.\n");
	printf("\t-s : When attempting to allocate more memory than is supported, skip the command instead of aborting. (NOT RECOMMENDED)\n");
	printf("\t-h : Do not print the data of the written file when using Verbose Execution (For excessively large programs)\n\n");
}

static void splash()
{	
	printf("\t\t                                 \n");
	printf("\t\t              =====              \n");
	printf("\t\t         =====#####=====         \n");
	printf("\t\t      ===###############===      \n");
	printf("\t\t    ===###################===    \n");
	printf("\t\t   ==#######################==   \n");
	printf("\t\t  ==#####   ########====#####==  \n");
	printf("\t\t ==#####     #####==    ===###== \n");
	printf("\t\t ==#####     ####=         ==##= \n");
	printf("\t\t = =#####   ####=    ###     =#= \n");
	printf("\t\t =  ==#########=    #####     == \n");
	printf("\t\t ==   ===####==     #####     == \n");
	printf("\t\t  ==     ====        ###     ==  \n");
	printf("\t\t   ==                       ==   \n");
	printf("\t\t    ===                   ===    \n");
	printf("\t\t      ===               ===      \n");
	printf("\t\t         =====     =====         \n");
	printf("\t\t              =====              \n");
	printf("\t\t                                 \n");
	printf("\t\t   ===========================   \n");
	printf("\t\t         D      A      O         \n");
	printf("\t\t   ===========================   \n");
	printf("\t\t                                 \n");
	printf("\r\n[Welcome to C-DAOYU-UTILITY 1.5.0.0]\n");
	printf("\tEnter a filename as a parameter, for example:\n\n");
	printf("\t\"> dao hello_world.dao\"\n");
	printf("\t\tto compile and execute.\n\n");
	printf("Options:\n");
	flags();
	putchar('\n');
}

char *str_dup (char *s) {
    char *d = malloc (strlen (s) + 1);   /*Allocate memory			*/
    if (d != NULL) strcpy (d,s);         /*Copy string if okay		*/
    return d;                            /*Return new memory		*/
}

char* bin(unsigned long val) { return l_to_str(val, 32, 2, 1); }

char getChar(unsigned char ch)
{
	if (ch > 0xF) return '?';
	return symbols[ch];
}

char* l_to_str(unsigned long val, unsigned char len, unsigned char radix, unsigned char override_num_only)
{
	static char buf[32] = { '0' };
	int i = 33;
	for (; val && i; --i, val /= radix)
		buf[i] = ((PRINT_CODE && !override_num_only) ? ".!/)%#>=(<:S[*$;????????????????" : "0123456789ABCDEFGHIJKLMNOPQRSTUV")[val % radix];
	for (; i; i--)
		buf[i] = (PRINT_CODE && !override_num_only) ? '.' : '0';
	return &buf[2 + (32 - len)];
}

void flip_UL(unsigned long* target)									/* Generalize this sometime, ok? */
{
	unsigned long num = *target;
	*target = 0;
	*target |= ((num >> (BITS_IN_CELL - 1 * BITS_IN_BYTE)) & (BYTE_MASK << 0 * BITS_IN_BYTE));
	*target |= ((num >> (BITS_IN_CELL - 3 * BITS_IN_BYTE)) & (BYTE_MASK << 1 * BITS_IN_BYTE));
	*target |= ((num << (BITS_IN_CELL - 3 * BITS_IN_BYTE)) & (BYTE_MASK << 2 * BITS_IN_BYTE));
	*target |= ((num << (BITS_IN_CELL - 1 * BITS_IN_BYTE)) & (BYTE_MASK << 3 * BITS_IN_BYTE));
}

/***
 *     .oooooo..o oooooo   oooo ooo        ooooo oooooooooo.    .oooooo.   ooooo         .oooooo..o 
 *    d8P'    `Y8  `888.   .8'  `88.       .888' `888'   `Y8b  d8P'  `Y8b  `888'        d8P'    `Y8 
 *    Y88bo.        `888. .8'    888b     d'888   888     888 888      888  888         Y88bo.      
 *     `"Y8888o.     `888.8'     8 Y88. .P  888   888oooo888' 888      888  888          `"Y8888o.  
 *         `"Y88b     `888'      8  `888'   888   888    `88b 888      888  888              `"Y88b 
 *    oo     .d8P      888       8    Y     888   888    .88P `88b    d88'  888       o oo     .d8P 
 *    8""88888P'      o888o     o8o        o888o o888bood8P'   `Y8bood8P'  o888ooooood8 8""88888P'  
 *                                                                                                  
 *                                                                                                  
 *                                                                                                  
 */

#define levlim(l)		if (PR_LEV >= l) {verbprint("LEV_SKIP");return;}
#define P_LEN 			(path -> sel_length)
#define P_IND 			(path -> sel_index)
#define P_ALC 			(path -> prg_allocbits)
#define P_LEV 			(path -> prg_level)
#define P_PIND			(path -> prg_index)
#define P_DATA 			(path -> prg_data)
#define P_OWNER			(path -> owner)
#define P_CHILD			(path -> child)
#define PR_START  		(P_RUNNING -> prg_start)
#define PR_LEV 			(P_RUNNING -> prg_level)

static void swaps(Path path)
{
	unsigned int i = 0;
	unsigned long report = 0;
	levlim(1)
	verbosely printf("Swapped length %d.", P_LEN);
	if (P_LEN == 1)	return;
	if (P_LEN <= BITS_IN_CELL)
	{
		unsigned long half_len = P_LEN / 2;
		write_by_bit_index(path, P_IND, P_LEN, read_by_bit_index(path, P_IND, half_len) | (read_by_bit_index(path, P_IND + half_len, half_len) << half_len));
		return;
	}
	while (i < ((P_LEN / BITS_IN_CELL) / 2))
	{
		report = P_DATA[(P_IND / BITS_IN_CELL) + i];
		P_DATA[(P_IND / BITS_IN_CELL) + i] = P_DATA[(P_IND / BITS_IN_CELL) + ((P_LEN / BITS_IN_CELL) / 2) + i];
		P_DATA[(P_IND / BITS_IN_CELL) + ((P_LEN / BITS_IN_CELL) / 2) + i++] = report;
	}
}

static void later(Path path)
{
	if (algn(path) || (PR_LEV >= 4))	P_IND += P_LEN;
	else								merge(path);
}

static void merge(Path path)
{
	levlim(7)
	if (P_LEN < P_ALC)
	{
		if (!algn(path))
			P_IND -= P_LEN;
		P_LEN <<= 1;
		return;
	}
	if (P_OWNER == NULL)
		return;
	P_WRITTEN = P_OWNER;
	(P_WRITTEN->sel_length) = 1;
	(P_WRITTEN->sel_index) = 1;
}

static void sifts(Path path)
{
	int l = P_IND;
	levlim(5)
	while (l + 4 < P_ALC)
	{
		if (!read_by_bit_index(path, l, 4))
		{
			int r = l;
			for (; !read_by_bit_index(path, r, 4) && ((r + 4) < P_ALC); r += 4);			/* OPTIMIZE THIS ASAP */
			write_by_bit_index(path, l, 4, read_by_bit_index(path, r, 4));
			write_by_bit_index(path, r, 4, 0);
		}
		l += 4;
	}
}

static void execs(Path path, Path caller)
{
	/***************************************************************EXECUTION LOOP***************************************************************/
	unsigned long tempNum1 = 0;																/* Expedite calculation								*/
	levlim(8)																				/* Level operation checking							*/
	P_RUNNING = path;																		/* Set running 										*/

	if (P_CHILD == NULL)																	/* If there is no child 							*/
	{
		if ((P_CHILD = (calloc(1, sizeof(struct PATH)))) == NULL)							/* Allocate memory space 							*/
		{																					/* Cover error case							 		*/
			printf("FATAL ERROR: Unable to allocate memory.");
			return;
		}
		verbosely printf("Allocated %d bytes.\n\n", sizeof(*P_CHILD));
		memcpy(P_CHILD, &NEW_PATH, sizeof(struct PATH));									/* Copy over initialization data			 		*/
		(*(*path).child).owner = path;														/* Set owner of this new Path 						*/
		(*(*path).child).prg_floor = (path->prg_floor) + 1;									/* Set floor of this new Path 						*/
		(*(*path).child).prg_data = calloc(1, sizeof(unsigned long));						/* Set data  of this new Path 						*/
	}
	else
		verbosely putchar('\n');
	P_WRITTEN = P_CHILD;																	/* Set this as written on 							*/
	P_PIND = (P_IND / 4);																	/* Set program pointer. Rounds down.x				*/
	PR_START = P_PIND;																		/* Track start position 							*/

	for (; doloop && P_PIND < (P_ALC / 4) && path != NULL && P_WRITTEN != NULL ; P_PIND++)	/* Execution Loop 									*/
	{
		tempNum1 = (P_RUNNING->prg_index);
		command = ((P_RUNNING->prg_data)[(tempNum1 * 4) / 32] >> (32 - ((tempNum1 * 4) % 32) - 4)) & mask(4);	/* Calculate command			*/
		verbosely diagnose(path, command);
		
		if (command == 5)
			execs(P_WRITTEN, path);
		else if (command != 0)
			functions[command](P_WRITTEN);

		/*
		switch(command)
		{
		case 0x1: swaps(P_WRITTEN); break;
		case 0x2: later(P_WRITTEN); break;
		case 0x3: merge(P_WRITTEN); break;
		case 0x4: sifts(P_WRITTEN); break;
		case 0x5: execs(P_WRITTEN, path); break;
		case 0x6: delev(P_WRITTEN); break;
		case 0x7: equal(P_WRITTEN); break;
		case 0x8: halve(P_WRITTEN); break;
		case 0x9: uplev(P_WRITTEN); break;
		case 0xA: reads(P_WRITTEN); break;
		case 0xB: dealc(P_WRITTEN); break;
		case 0xC: split(P_WRITTEN); break;
		case 0xD: polar(P_WRITTEN); break;
		case 0xE: doalc(P_WRITTEN); break;
		case 0xF: input(P_WRITTEN); break;
		}
		*/
		verbprint("\n");
	}
	if (caller == NULL)
	{
		verbprint("Top-level program terminated.\n")
		free(P_CHILD);
		P_CHILD = NULL;
		return;
	}
	if (!doloop)
	{
		verbosely printf("Freed %d bytes.\n\n", sizeof(*P_CHILD));
		free(P_CHILD);
		P_CHILD = NULL;
		doloop = 1;
	}
	P_RUNNING = caller;
	P_WRITTEN = caller->child;
	return;
}

static void delev(Path path)
{
	if (PR_LEV > 0) PR_LEV--;
}

static void equal(Path path)
{
	levlim(5)
	if (read_by_bit_index(path, P_IND, 1) ^ read_by_bit_index(path, P_IND + P_LEN - 1, 1))
		skip();
	else
		verbprint("EQUAL");
}

static void halve(Path path)
{
	levlim(7)
	if (P_LEN > 1)
	{
		P_LEN /= 2;
		return;
	}
	if (P_CHILD == NULL)
		return;
	P_WRITTEN = P_CHILD;
	(P_WRITTEN->sel_length) = (P_WRITTEN->prg_allocbits);
}

static void uplev(Path path)
{
	levlim(9)
	PR_LEV++;
	(P_RUNNING->prg_index) = PR_START - 1;
}

static void reads(Path path)
{
	long pos = P_IND;
	levlim(6)
	if (P_LEN < 8)
	{
		char* out = bin(read_by_bit_index(path, pos, P_LEN));
		printf("%s", &out[strlen(out) - P_LEN]);
		return;
	}
	for (; pos < (P_IND + P_LEN); pos += 8)
		putchar(read_by_bit_index(path, pos, 8));
}

static void dealc(Path path)
{
	levlim(2)
	if (P_ALC == 1)
	{
		int report = read_by_bit_index(path, 0, 1);
		if ((P_RUNNING->owner) != NULL)
		{
			unsigned long ownind = ((P_RUNNING->owner)->prg_index);
			verbosely printf("Terminating program from position %x with value %x", ownind, report);
			write_by_bit_index(P_RUNNING->owner, (ownind) * 4, 4, report);
		}
		free(P_DATA);
		P_DATA = NULL;
		doloop = 0;
		return;
	}
	P_ALC >>= 1;
	if (P_ALC <= 8)
		realloc(P_DATA, 1);
	else
		realloc(P_DATA, P_ALC / 8);
	if (P_LEN > 1)
		halve(path);
	if ((P_IND + P_LEN) > P_ALC)
		P_IND -= P_ALC;
}

static void split(Path path)
{
	if (PR_LEV < 1)
	{
		unsigned int len = P_LEN;
		if (len == 1)
		{
			if (P_CHILD == NULL)
				return;
			P_WRITTEN = P_CHILD;
			(P_WRITTEN->sel_length) = (P_WRITTEN->prg_allocbits);
			split(P_WRITTEN);
			halve(P_WRITTEN);
			return;
		}
		if (len <= BITS_IN_CELL)
		{
			write_by_bit_index(path, P_IND, len >> 1, mask(len));
			write_by_bit_index(path, P_IND + (len >> 1), len >> 1, ~mask(len));
		}
		else
		{
			unsigned int leftIndex = (P_IND / BITS_IN_CELL);
			unsigned int rightIndex = leftIndex + (len / BITS_IN_CELL) - 1;
			while (leftIndex < rightIndex)
			{
				P_DATA[leftIndex++] = 0xFFFFFFFF;
				P_DATA[rightIndex--] = 0;
			}
		}
	}
	halve(path);
}

static void polar(Path path)
{
	levlim(3)
	if (!(read_by_bit_index(path, P_IND, 1) && !read_by_bit_index(path, P_IND + P_LEN - 1, 1)))
		skip();
	else
		verbprint("POLAR");
}

static void doalc(Path path)
{
	unsigned long new_cell_count = 0;
	unsigned long* new_data_pointer = NULL;
	levlim(1)
		P_ALC <<= 1;

	if (P_ALC <= BITS_IN_CELL)
		new_cell_count = BITS_IN_CELL / BITS_IN_BYTE;
	else
		new_cell_count = P_ALC / BITS_IN_BYTE;

	new_cell_count /= sizeof(unsigned long);

	printf("");

	if ((new_data_pointer = calloc(new_cell_count, sizeof(unsigned long))) == NULL)
	{
		printf("Error allocating %d bytes: ", new_cell_count * sizeof(unsigned long));
		perror("");
		if (SKIP_OVERFLOW)
			return;
		abort();
	}

	if (new_cell_count > 1)
		memcpy(new_data_pointer, P_DATA, new_cell_count * sizeof(unsigned long) / 2);
	else
		memcpy(new_data_pointer, P_DATA, sizeof(unsigned long));

	P_DATA = new_data_pointer;

	merge(path);
}

static void input(Path path)
{
	int i = P_IND;
	levlim(6)
	if (P_LEN < 8)
	{
		write_by_bit_index(path, P_IND, P_LEN, getchar());
		return;
	}
	for (; i < (P_IND + P_LEN); i += 8)
		write_by_bit_index(path, i, 8, getchar());
}

/***
 *    oooooooooooo ooooooooooooo   .oooooo.   
 *    `888'     `8 8'   888   `8  d8P'  `Y8b  
 *     888              888      888          
 *     888oooo8         888      888          
 *     888    "         888      888          
 *     888       o      888      `88b    ooo  
 *    o888ooooood8     o888o      `Y8bood8P'  
 *                                            
 *                                            
 *                                            
 */

char algn(Path path)
{
	return P_IND % (P_LEN << 1) == 0;
}

unsigned long mask(int length)
{
	if (length < BITS_IN_CELL)	return ((int)1 << length) - 1;
	else			 	return 0xFFFFFFFF;
}

unsigned long read_by_bit_index(Path path, unsigned long i, unsigned long len)
{
	return (P_DATA[i / BITS_IN_CELL] >> (BITS_IN_CELL - (i % BITS_IN_CELL) - len)) & mask(len);
}

static void write_by_bit_index(Path path, unsigned long i, unsigned long len, unsigned long write)
{
	int shift = BITS_IN_CELL - (i % BITS_IN_CELL) - len;
	if (len > BITS_IN_CELL) abort();
	P_DATA[i / BITS_IN_CELL] &= ~(mask(len) << shift);
	P_DATA[i / BITS_IN_CELL] |= ((write & mask(len)) << shift);
}

static void bin_print(Path path)
{
	unsigned long c_ind = 0;
	unsigned long c_num = P_ALC / BITS_IN_CELL;
	unsigned long empty_lines = 0;
	unsigned char j = 0;
	char* out;
	/* One or less cells */
	if (c_num <= 1)
	{
		out = bin(read_by_bit_index(path, 0, P_ALC));
		printf("%s", &out[strlen(out) - P_ALC]);
		return;
	}
	/* One or less lines */
	if (c_num <= 4 || PRINT_EVERYTHING)
	{
		/* For each cell */
		for (; c_ind < c_num; c_ind++)
		{
			/* Print the contents */
			printf("%s", l_to_str(P_DATA[c_ind], 8, 16, 0));

			/* If not the last index of a line */
			if ((c_ind + 1) % 4 != 0)
				putchar(' ');
			/* If the last index of a not-final line */
			else if (c_ind != (c_num - 1))
				printf("\n                 ");
		}
		return;
	}
	/* More than one line */
	/* Scan by lines      */
	for (; (c_ind) < c_num; c_ind += 4)
	{
		
		/* If line is not all zeroes      */
		/* That is: If any are not zeroes */
		if (P_DATA[c_ind] || P_DATA[c_ind+1] || P_DATA[c_ind+2] || P_DATA[c_ind+3])
		{

			/* See if we have backlogged empty lines */
			if (empty_lines != 0)
			{
				if (empty_lines > 1)
				{
					out = str_dup(l_to_str(32 * empty_lines, 6, 10, 1));
					for (j = 0; j < strlen(out) && out[j] == '0'; j++)
							out[j] = ' ';
					printf(".         %s n x 0            .", out);
					printf("\n                        ");
					empty_lines = 0;
				}
				/* Just one empty line */
				else
				{
					printf("........ ........ ........ ........");
					printf("\n                        ");
					empty_lines = 0;
				}
			}

			/* Print this line */
			
			printf("%s ", l_to_str(P_DATA[c_ind  ], 8, 16, 0));
			printf("%s ", l_to_str(P_DATA[c_ind+1], 8, 16, 0));
			printf("%s ", l_to_str(P_DATA[c_ind+2], 8, 16, 0));
			printf("%s" , l_to_str(P_DATA[c_ind+3], 8, 16, 0));

			/* Newline and indent if not last line */
			if ((c_ind + 4) < c_num)
				printf("\n                        ");
		}
		/* Line is all zeroes */
		else
			empty_lines++;

		/* If we have empty-backlog on the last line */
		if ((c_ind + 4) >= c_num)
		{
			if (empty_lines > 1)
			{
				out = str_dup(l_to_str(32 * empty_lines, 6, 10, 1));
				for (j = 0; j < strlen(out) && out[j] == '0'; j++)
						out[j] = ' ';
				printf(".         %s n x 0            .", out);
				empty_lines = 0;
			}
			/* Just one empty line */
			else if (empty_lines == 1)
			{
				printf("........ ........ ........ ........");
				empty_lines = 0;
			}
		}
	}
}

static void rad_print(Path path, unsigned int radix)
{
	unsigned long i = 0;
	unsigned char len = 0;
	char* out;
	for (; radix >> len != 0; len++);
	len = 32 / len;
	if (P_ALC <= BITS_IN_CELL)
	{
		out = l_to_str(read_by_bit_index(path, 0, P_ALC), 32, 2, 1);
		printf("%s", &out[strlen(out) - P_ALC]);
	}
	while (i < (P_ALC / BITS_IN_CELL))
	{
		out = l_to_str(P_DATA[i], len, radix, 1);
		printf("%s", out);
		if (i++ < (P_ALC / BITS_IN_CELL))
			putchar(' ');
	}
}

static void skip()
{
	if (P_RUNNING == NULL) return;
	verbprint("SKIP");
	(P_RUNNING->prg_index)++;
}

static void diagnose(Path path, unsigned char command)
{
	printf("%s " , l_to_str(P_PIND, 5, 16, 1));
	printf("R%d ", (P_RUNNING->prg_floor));
	printf("W%d ", (P_WRITTEN->prg_floor));
	printf("L%d ", PR_LEV);
	printf("*%s ", l_to_str(P_LEN, 5, 10, 1));
	printf("%c " , getChar(command));
	if (!HIDE_DATA)
		bin_print(P_WRITTEN);
	printf(" : ");
}