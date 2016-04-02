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

#define FILE_SYMBOLIC ".dao"
#define FILE_COMPILED ".wuwei"
#define MAX_BIT_SIZE (unsigned long)(1 << 17)

typedef struct PATH* Path;
typedef FILE* File;
typedef char* String;
/*
#define static void static void
typedef unsigned long unsigned long;
typedef unsigned char unsigned char;
typedef unsigned int unsigned int;
*/
static void compile(File, File);

static void swaps(Path); static void later(Path); static void merge(Path);
static void sifts(Path); static void execs(Path, Path); static void delev(Path); static void equal(Path);
static void halve(Path); static void uplev(Path); static void reads(Path); static void dealc(Path);
static void split(Path); static void polar(Path); static void doalc(Path); static void input(Path);

char getChar(unsigned char);
unsigned long mask(int);
unsigned char getNybble(char);
unsigned long rbbi(Path, unsigned long, unsigned long);
static void wbbi(Path, unsigned long, unsigned long, unsigned long);

char*	bin(unsigned long);
char*	itoa(unsigned long, unsigned char, unsigned char);
char 	algn(Path);
static void		bin_print(Path);
static void		skip();
void flip_UL(unsigned long*);

struct PATH
{
	Path 			owner;						/* OWNER      PROGRAM */
	Path 			child;						/* CHILD      PROGRAM */
	unsigned long*	prg_data;					/*		   DATA	      */
	unsigned long	prg_allocbits;				/* OPEN	   DATA   BITS*/
	unsigned long	prg_index;					/* INSTRUCTION POINTER*/
	unsigned char	prg_level;					/* OPERATING   LEVEL  */
	unsigned long	sel_length;					/* LENGTH OF SELECTION*/
	unsigned long	sel_index;					/* INDEX  OF SELECTION*/
	unsigned int    prg_floor;					/* FLOOR  OF PATH     */
	unsigned long   prg_start;					/* START  OF RUNNING  */
};

const struct PATH NEW_PATH = { NULL, NULL, NULL, 1, 0, 0, 1, 0, 0, 0 };

#define wheel(k, n, x) if (++k == n) {k = 0; x}
#define roc(o,c) case o:c=1;break;
#define verp(x) vrx{printf(x);}
#define vrx if (VERBOSE)

static char VERBOSE = 0,
COMP_ONLY = 0,
FORCE = 0,
HIDE_DATA = 0,
SKIP_OVERFLOW = 0;
static const String symbols = ".!/)%#>=(<:S[*$;";
static Path P_RUNNING = NULL,
P_WRITTEN = NULL;

static void splash();

int main(int argc, char * argv[])
{
	String inputFileName = NULL;
	File ifle = NULL;
	unsigned long bytes_read = 0;

	if (argc < 2)
	{
		splash();
		return(0);
	}

	inputFileName = argv[1];

	ifle = fopen(inputFileName, "rb");

	/***************************                           PARSE ARGS                            ****************************/
	while (argc-- > 2)															/* Using argc as index, get ind 2+ flags	*/
	{
		if (argv[argc][0] == '-')
		{
			switch (argv[argc][1])
			{
				roc('f', FORCE)
					roc('v', VERBOSE)
					roc('c', COMP_ONLY)
					roc('h', HIDE_DATA)
					roc('s', SKIP_OVERFLOW)
			default:
				printf("Unknown option -%c.\n\n", argv[argc][1]);
				break;
			}
		}
	}
	/***************************                           END PARSE ARGS                        ****************************/

	if (ifle == NULL)
	{
		printf("Could not find \"%s\" - is it in this directory?\n", inputFileName);
		fclose(ifle);
		return 1;
	}

	if (~strcmp(FILE_SYMBOLIC, &inputFileName[strlen(inputFileName) - 4]))
	{
		File ofle = NULL;
		inputFileName[strlen(inputFileName) - 4] = 0;
		inputFileName = strncat(inputFileName, FILE_COMPILED, sizeof(FILE_COMPILED));
		ofle = fopen(inputFileName, "wb+");
		vrx printf("\n%s%s\n", "Compiling to ", inputFileName);
		compile(ifle, ofle);
		verp("Finished compiling.");
		fclose(ifle);
		fclose(ofle);
		if (COMP_ONLY)
			return 0;
	}

	ifle = fopen(inputFileName, "rb");

	vrx
	{
		printf("\n\n\t=====================\n");
	printf("\t|Beginning Execution|\n");
	printf("\t=====================\n\n");
	}

		if (FORCE || ~strcmp(FILE_COMPILED, &inputFileName[strlen(inputFileName) - 6]))
		{
			/*************************                           RUN THE CODE                           *************************/
			unsigned long 	print_index = 0;								/* Index for printing                               */
			unsigned long	bytes_alloc = 0;								/* Bytes allocated to data                          */
			unsigned long	file_size = 0;									/* Byte size of file 								*/
			unsigned long	shift = 0;										/* Shift for first one of file size for rounding    */

			struct PATH newpath = NEW_PATH;									/* Make a new PATH with the initialization values.	*/
			Path dao = &newpath;											/* Make a pointer to the newly initialized PATH.	*/

			fseek(ifle, 0L, SEEK_END);										/* Find size of input file in bytes.				*/
			file_size = ftell(ifle);										/*													*/
			fseek(ifle, 0L, SEEK_SET);										/* Rewind file.									 	*/

			/*************************ROUND DATA ARRAY SIZE TO LOWEST POWER OF TWO LARGER THAN FILE SIZE*************************/
			bytes_alloc = file_size;										/* Initialize bytes_alloc with the file_size value. */

			while ((bytes_alloc >> 1) != 0)									/* Determine leftmost '1' bit of file size.		 	*/
			{																/*													*/
				bytes_alloc >>= 1;											/* Shift right until the next shift zeroes it.		*/
				shift++;													/* Keep track of shifts.							*/
			}																/*													*/

			bytes_alloc <<= shift; 											/* Unshift. Bytes_alloc becomes file size without   */
																			/*              bits less than the leftmost 1 bit.	*/

			if (file_size != bytes_alloc)									/* If not a power of two, round up.					*/
				bytes_alloc <<= 1;

			(dao->prg_allocbits) = bytes_alloc * 8;						/* Set literal allocation bit size.				 	*/
			/********************************************************************************************************************/

			vrx printf("%s%s.\nLoading data:\n", "Running ", inputFileName);

			if (bytes_alloc % sizeof(unsigned long) != 0)					/* Only occurs if it's less than one UL, one cell   */
				bytes_alloc = sizeof(unsigned long);						/* Set the minimum									*/

			if (((dao->prg_data) = calloc(bytes_alloc, 1)) == NULL)		/* Allocate data array to bytes needed.				*/
			{
				printf("Error allocating %d bytes: ", bytes_alloc);
				perror("");
				abort();
			}
			vrx printf("Allocated %d bytes for %d byte file.\n", bytes_alloc, file_size);

			bytes_read = fread((dao->prg_data), 1, file_size, ifle);

			vrx printf("Read %d bytes.\n\n", bytes_read);					/* Read file data into data array.		*/

			while (print_index++ < (bytes_alloc / sizeof(unsigned long)))	/* Traverse the array, and...						*/
			{
				flip_UL((dao->prg_data) + print_index - 1);				/* Flip the byte order to the correct one 			*/
				vrx
				{
					printf("%s   ", itoa((dao->prg_data)[print_index - 1], 8, 16));		/* If verbose, print out array contents	*/
				if (print_index % 8 == 0) printf("\n");					/* New line every seven unsigned longs.				*/
				}
			}

			vrx printf("(%d bytes)\n\n", (dao->prg_allocbits) / 8);		/* If verbose, output number of bytes.				*/
			P_RUNNING = dao;												/* For the sake of levlim							*/

																			/***************************************************** EXECUTE ******************************************************/
			execs(dao, NULL);
			vrx printf("Freeing %d bytes of data.\n", bytes_alloc);
			free((dao->prg_data));
			verp("Data freed.")
				/********************************************************************************************************************/
		}
	return 0;
}

static void splash()
{
	printf("\t\t#################################\n");
	printf("\t\t##############     ##############\n");
	printf("\t\t#########     #####     #########\n");
	printf("\t\t######   ###############   ######\n");
	printf("\t\t####   ###################   ####\n");
	printf("\t\t###  #######################  ###\n");
	printf("\t\t##  #####   ########====#####  ##\n");
	printf("\t\t#  #####     #####==    ===###  #\n");
	printf("\t\t# =#####     ####=         ==## #\n");
	printf("\t\t#+ =#####   ####=    +++     =# #\n");
	printf("\t\t#+  ==#########=    +++++     =+#\n");
	printf("\t\t#++   ===####==     +++++     ++#\n");
	printf("\t\t##++     ====        +++     ++##\n");
	printf("\t\t###++                       ++###\n");
	printf("\t\t####+++                   +++####\n");
	printf("\t\t######+++               +++######\n");
	printf("\t\t#########+++++     +++++#########\n");
	printf("\t\t##############+++++##############\n");
	printf("\t\t#################################\n");
	printf("\t\t###===========================###\n");
	printf("\t\t##|      D      A      O      |##\n");
	printf("\t\t###===========================###\n");
	printf("\t\t#################################\n");
	printf("\r\n[Welcome to C-DAOYU-UTILITY 1.0.0.0]\n");
	printf("\tPlease remember to enter a filename as a parameter, for example:\n\n");
	printf("\t\"> dao hello_world.dao\"\n");
	printf("\t\tto compile and execute.\n\n");
	printf("Options:\n");
	printf("\t-c : Compile without running\n");
	printf("\t-v : Enable Verbose Execution (For Debugging)\n");
	printf("\t-w : Get Input before closing (For Debugging)\n");
	printf("\t-f : Force Execution of Any File as COMPILED DAOYU (DANGEROUS)\n");
	printf("\t-s : When attempting to allocate more memory than is supported, skip the command instead of aborting. (NOT RECOMMENDED)\n");
	printf("\t-h : Do not print the data of the written file when using Verbose Execution (For excessively large programs)\n");
	printf("\n");
}

static void compile(File input, File output)
{
	unsigned char emptyBuffer = 1, toWrite = 0, isComment = 0, k = 0;
	int ch;
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
			vrx putchar((char)ch);
			if (!emptyBuffer)
			{
				toWrite |= getNybble((char)ch);
				fputc(toWrite, output);
				vrx printf(" %s ", itoa(toWrite, 2, 16));
				wheel(k, 8, vrx putchar('\n');)
			}
			else
				toWrite = (char)(getNybble((char)ch) << 4);
			emptyBuffer = !emptyBuffer;
			break;
		}

	}
	if (!emptyBuffer) {
		fputc(toWrite, output);
		vrx printf(". %x\n", toWrite);
	}
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

char* bin(unsigned long val) { return itoa(val, 32, 2); }

char getChar(unsigned char ch)
{
	if (ch > 0xF) return '?';
	return symbols[ch];
}

char* itoa(unsigned long val, unsigned char len, unsigned char radix)
{
	static char buf[32] = { '0' };
	int i = 33;
	for (; val && i; --i, val /= radix)
		buf[i] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[val % radix];
	for (; i; i--)
		buf[i] = '0';
	return &buf[2 + (32 - len)];
}

#define BITS_IN_BYTE	8
#define BITS_IN_CELL 	(sizeof(unsigned long) * 8)
#define BYTE_MASK		0xff

void flip_UL(unsigned long* target)									/* Generalize this sometime, ok? */
{
	unsigned long num = *target;
	*target = 0;
	*target |= ((num >> (BITS_IN_CELL - 1 * BITS_IN_BYTE)) & (BYTE_MASK << 0 * BITS_IN_BYTE));
	*target |= ((num >> (BITS_IN_CELL - 3 * BITS_IN_BYTE)) & (BYTE_MASK << 1 * BITS_IN_BYTE));
	*target |= ((num << (BITS_IN_CELL - 3 * BITS_IN_BYTE)) & (BYTE_MASK << 2 * BITS_IN_BYTE));
	*target |= ((num << (BITS_IN_CELL - 1 * BITS_IN_BYTE)) & (BYTE_MASK << 3 * BITS_IN_BYTE));
}

#define levlim(l)		if (PR_LEV >= l) {verp("LEV_SKIP");return;}
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
	levlim(2)
		vrx printf("Swapped length %d.", P_LEN);
	if (P_LEN == 1)	return;
	if (P_LEN <= BITS_IN_CELL)
	{
		unsigned long hlen = P_LEN / 2;
		wbbi(path, P_IND, P_LEN, rbbi(path, P_IND, hlen) | (rbbi(path, P_IND + hlen, hlen) << hlen));
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
			if (!rbbi(path, l, 4))
			{
				int r = l;
				for (; !rbbi(path, r, 4) && ((r + 4) < P_ALC); r += 4);			/* OPTIMIZE THIS ASAP */
				wbbi(path, l, 4, rbbi(path, r, 4));
				wbbi(path, r, 4, 0);
			}
			l += 4;
		}
}

static unsigned char command = 0;
static int doloop = 1;

static void(*functions[16])(Path) = \
{NULL, swaps, later, merge, \
sifts, NULL, delev, equal, \
halve, uplev, reads, dealc, \
split, polar, doalc, input};

void execs(Path path, Path caller)
{
	/***************************************************************EXECUTION LOOP***************************************************************/
	unsigned long tempNum1 = 0;																/* Expedite calculation								*/
	levlim(8)																				/* Level operation checking							*/
		P_RUNNING = path;																		/* Set running 										*/

	if ((P_CHILD = (calloc(1, sizeof(struct PATH)))) == NULL)								/* Allocate memory space 							*/
	{																						/* Cover error case							 		*/
		printf("FATAL ERROR: Unable to allocate memory.");
		return;
	}

	vrx printf("Allocated %d bytes.\n\n", sizeof(*P_CHILD));


	memcpy(P_CHILD, &NEW_PATH, sizeof(struct PATH));										/* Copy over initialization data			 		*/
	(*(*path).child).owner = path;															/* Set owner of this new Path 						*/
	(*(*path).child).prg_floor = (path->prg_floor) + 1;									/* Set floor of this new Path 						*/
	(*(*path).child).prg_data = calloc(1, sizeof(unsigned long));							/* Set data  of this new Path 						*/

	P_WRITTEN = P_CHILD;																	/* Set this as written on 							*/
	P_PIND = (P_IND / 4);																	/* Set program pointer 								*/
	PR_START = P_PIND;																		/* Track start position 							*/

	for (; doloop && P_PIND < (P_ALC / 4); P_PIND++)										/* Execution Loop 									*/
	{
		tempNum1 = (P_RUNNING->prg_index);
		command = ((P_RUNNING->prg_data)[(tempNum1 * 4) / 32] >> (32 - ((tempNum1 * 4) % 32) - 4)) & mask(4);	/* Calculate command				*/
		vrx
		{
			printf("%s R%d W%d L%d %c ", itoa(P_PIND, 5, 16), (P_RUNNING->prg_floor), (P_WRITTEN->prg_floor), PR_LEV, getChar(command));
		if (!HIDE_DATA) bin_print(P_WRITTEN);
		printf(" : ");
		}
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
		verp("\n");
	}
	if (caller == NULL)
	{
		verp("Top-level program terminated.\n")
			free(P_CHILD);
		return;
	}
	if (!doloop)
	{
		vrx printf("Freed %d bytes.\n\n", sizeof(*P_CHILD));
		free(P_CHILD);
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
		if (rbbi(path, P_IND, 1) ^ rbbi(path, P_IND + P_LEN - 1, 1))
			skip();
		else
			verp("EQUAL");
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
			String out = bin(rbbi(path, pos, P_LEN));
			printf("%s", &out[strlen(out) - P_LEN]);
			return;
		}
	for (; pos < (P_IND + P_LEN); pos += 8)
		putchar(rbbi(path, pos, 8));
}

static void dealc(Path path)
{
	levlim(2)
		if (P_ALC == 1)
		{
			int report = rbbi(path, 0, 1);
			if ((P_RUNNING->owner) != NULL)
			{
				unsigned long ownind = ((P_RUNNING->owner)->prg_index);
				vrx printf("Terminating program from position %x with value %x", ownind, report);
				wbbi(P_RUNNING->owner, (ownind) * 4, 4, report);
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
			wbbi(path, P_IND, len >> 1, mask(len));
			wbbi(path, P_IND + (len >> 1), len >> 1, ~mask(len));
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
		if (!(rbbi(path, P_IND, 1) && !rbbi(path, P_IND + P_LEN - 1, 1)))
			skip();
		else
			verp("POLAR");
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
			wbbi(path, P_IND, P_LEN, getchar());
			return;
		}
	for (; i < (P_IND + P_LEN); i += 8)
		wbbi(path, i, 8, getchar());
}

char algn(Path path)
{
	return P_IND % (P_LEN << 1) == 0;
}

unsigned long mask(int length)
{
	if (length < BITS_IN_CELL)	return ((int)1 << length) - 1;
	else			 	return 0xFFFFFFFF;
}

unsigned long rbbi(Path path, unsigned long i, unsigned long len)
{
	return (P_DATA[i / BITS_IN_CELL] >> (BITS_IN_CELL - (i % BITS_IN_CELL) - len)) & mask(len);
}

static void wbbi(Path path, unsigned long i, unsigned long len, unsigned long write)
{
	int shift = BITS_IN_CELL - (i % BITS_IN_CELL) - len;
	if (len > BITS_IN_CELL) abort();
	P_DATA[i / BITS_IN_CELL] &= ~(mask(len) << shift);
	P_DATA[i / BITS_IN_CELL] |= ((write & mask(len)) << shift);
}

static void bin_print(Path path)
{
	long i = 0;
	String out;
	if (P_ALC <= BITS_IN_CELL)
	{
		out = bin(rbbi(path, 0, P_ALC));
		printf("%s", &out[strlen(out) - P_ALC]);
		return;
	}
	for (; i < (P_ALC / BITS_IN_CELL); i++)
	{
		out = itoa(P_DATA[i], 8, 16);
		printf("%s", out);
	}
}

static void skip()
{
	if (P_RUNNING == NULL) return;
	verp("SKIP");
	(P_RUNNING->prg_index)++;
}