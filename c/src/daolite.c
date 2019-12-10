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
* Kaynato - 2016
* See splash() for details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BITS_IN_BYTE	8
#define BITS_IN_CELL 	(sizeof(unsigned long) * 8)
#define INPUT_DELIMITER '@'

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

static void interpret(char*);

static void swaps(Path);
static void later(Path);
static void merge(Path);
static void sifts(Path);
static void delev(Path);
static void equal(Path);
static void halve(Path);
static void uplev(Path);
static void reads(Path);
static void dealc(Path);
static void split(Path);
static void polar(Path);
static void doalc(Path);
static void input(Path);
static void execs(Path, Path);

char 			getInput();
char 			algn(Path);
char 			getChar(unsigned char);
char*			bin(unsigned long);
char*			str_dup(char *s);
char*			l_to_str(unsigned long, unsigned char, unsigned char);
static void		skip();
static void 	write_by_bit_index(Path, unsigned long, unsigned long, unsigned long);
unsigned char 	getNybble(char);
unsigned long 	read_by_bit_index(Path, unsigned long, unsigned long);
unsigned long 	mask(int);
unsigned long	ptwo_round(unsigned long);

static unsigned char command = 0;
static int doloop = 1;

typedef void(*PathFunc)(Path);

static PathFunc functions[16] = \
	{NULL, swaps, later, merge, \
	sifts, NULL , delev, equal, \
	halve, uplev, reads, dealc, \
	split, polar, doalc, input};

const struct PATH NEW_PATH = { NULL, NULL, NULL, 1, 0, 0, 1, 0, 0, 0 };

static Path P_RUNNING = NULL,
			P_WRITTEN = NULL;

static const char* symbols = ".!/)%#>=(<:S[*$;";

static char* inputptr = NULL;

/* Run argv[0] as code. Input separated by '!' and once empty reads the null character. */
int main(int argc, char * argv[])
{
	char* i = argv[1];

	/* No argument(s)? */
	if (argc < 2)
		return 0;

	/* Seek input until it either points to delimiter or NUL */
	while (*i && *i != INPUT_DELIMITER) i++;

	/* If it is the input delimiter then put the inputptr there*/
	if (*i == INPUT_DELIMITER)
		inputptr = ++i;

	interpret(argv[1]);
	return 0;
}

unsigned char getNybble(char ch)
{
	switch (ch)
	{
		case '.': return 0x0;
		case '!': return 0x1;
		case '/': return 0x2;
		case ']':
		case ')': return 0x3;
		
		case '%': return 0x4;
		case '#': return 0x5;
		case '>': return 0x6;
		case '=': return 0x7;
		
		case '(': return 0x8;
		case '<': return 0x9;
		case ':': return 0xA;
		case 'S': return 0xB;
		
		case '[': return 0xC;
		case '*': return 0xD;
		case '$': return 0xE;
		case ';': return 0xF;
		default: return 0x0;
	}
}

void interpret(char* input)
{
	unsigned long length = 0;										/* How many bytes in input 							*/

	/* Initialize path */
	struct PATH newpath = NEW_PATH;
	Path dao = &newpath;

	/* Seek end of program input */
	while (input[length] && input[length] != INPUT_DELIMITER) length++;

	/* Terminate empty program */
	if (length == 0) return;

	/* Get necessary byte number from nybbles */
	length = ptwo_round((length+1) / 2);

	/* Set bit length of path */
	(dao->prg_allocbits) = length * 8;

	/* Prevent zero-sized allocation */
	if (length % sizeof(unsigned long) != 0)
		length = sizeof(unsigned long);		

	/* Allocate bytes for data array */
	if (((dao->prg_data) = calloc(length, 1)) == NULL)
	{
		printf("Error allocating %d bytes: ", length);
		perror("");
		abort();
	}

	/* Copy over data */
	for (length = 0; input[length] && input[length] != INPUT_DELIMITER; length++)
	{
		int hex = getNybble(input[length]);
		write_by_bit_index(dao, 4*length, 4, hex);
	}

	P_RUNNING = dao;												/* For the sake of levlim							*/
	
	/***************************************************** EXECUTE ******************************************************/
	execs(dao, NULL);
	free((dao->prg_data));
	(dao -> prg_data) = NULL;
	/********************************************************************************************************************/

}

unsigned long ptwo_round(unsigned long x)
{
	unsigned long rounded = x;										/* Initialize bytes_alloc with the file_size value. */
	unsigned long	shift = 0;										/* Shift for first one of file size for rounding    */

	while ((rounded >> 1) != 0)										/* Determine leftmost '1' bit of file size.		 	*/
	{																/*													*/
		rounded >>= 1;												/* Shift right until the next shift zeroes it.		*/
		shift++;													/* Keep track of shifts.							*/
	}																/*													*/
	rounded <<= shift; 												/* Unshift. 										*/
	if (x != rounded)												/* If not a power of two, round up.					*/
		rounded <<= 1;

	return rounded;
}

char getInput()
{
	/* if null return zero */
	if (!inputptr)
		return 0;

	/* If not zero then return-advance */
	if (*inputptr)
		return *inputptr++;
	
	inputptr = NULL;
	return 0;
}


char *str_dup (char *s) {
    char *d = malloc (strlen (s) + 1);   /*Allocate memory			*/
    if (d != NULL) strcpy (d,s);         /*Copy string if okay		*/
    return d;                            /*Return new memory		*/
}

char* bin(unsigned long val) { return l_to_str(val, 32, 2); }

char getChar(unsigned char ch)
{
	if (ch > 0xF) return '?';
	return symbols[ch];
}

char* l_to_str(unsigned long val, unsigned char len, unsigned char radix)
{
	static char buf[32] = { '0' };
	int i = 33;
	for (; val && i; --i, val /= radix)
		buf[i] = "0123456789ABCDEFGHIJKLMNOPQRSTUV"[val % radix];
	for (; i; i--)
		buf[i] = '0';
	return &buf[2 + (32 - len)];
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
 */

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
	if (PR_LEV >= 1) return;
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
	if (algn(path) || (PR_LEV >= 4))
		P_IND += P_LEN;
	else
		merge(path);
}

static void merge(Path path)
{
	if (PR_LEV >= 7) return;
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
	unsigned long write = P_IND;
	unsigned long read = 0;
	if (PR_LEV >= 5) return;
	while(write<P_ALC)
	{
		if (write+read<P_ALC)
			while(!read_by_bit_index(path, write+read, 4))
				read += 4;
		if(read)
			write_by_bit_index(path, write, 4, (write+read<P_ALC)?read_by_bit_index(path, write+read, 4):0);
		write += 4;
		read += 4;
	}
}

static void execs(Path path, Path caller)
{
	unsigned long tempNum1 = 0;																/* Expedite calculation								*/
	if (PR_LEV >= 8) return;																				/* Level operation checking							*/
	P_RUNNING = path;																		/* Set running 										*/

	if (P_CHILD == NULL)																	/* If there is no child 							*/
	{
		if ((P_CHILD = (calloc(1, sizeof(struct PATH)))) == NULL)							/* Allocate memory space 							*/
		{																					/* Cover error case							 		*/
			printf("FATAL ERROR: Unable to allocate memory.");
			return;
		}
		memcpy(P_CHILD, &NEW_PATH, sizeof(struct PATH));									/* Copy over initialization data			 		*/
		path->child->owner = path;															/* Set owner of this new Path 						*/
		path->child->prg_floor = (path->prg_floor) + 1;										/* Set floor of this new Path 						*/
		path->child->prg_data = calloc(1, sizeof(unsigned long));							/* Set data  of this new Path 						*/
	}

	P_WRITTEN = P_CHILD;																	/* Set this as written on 							*/
	P_PIND = (P_IND / 4);																	/* Set program pointer. Rounds down.x				*/
	PR_START = P_PIND;																		/* Track start position 							*/

	for (; doloop && P_PIND < (P_ALC / 4) && path != NULL && P_WRITTEN != NULL ; P_PIND++)	/* Execution Loop 									*/
	{
		tempNum1 = (P_RUNNING->prg_index);
		command = ((P_RUNNING->prg_data)[(tempNum1 * 4) / BITS_IN_CELL] >> (BITS_IN_CELL - ((tempNum1 * 4) % BITS_IN_CELL) - 4)) & mask(4);	/* Calculate command			*/

		if (command == 5)
			execs(P_WRITTEN, path);
		else if (command != 0)
			functions[command](P_WRITTEN);
	}
	if (caller == NULL)
	{
		free(P_CHILD);
		P_CHILD = NULL;
		return;
	}
	if (!doloop)
	{
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
	if (PR_LEV >= 5) return;
	if (read_by_bit_index(path, P_IND, 1) ^ read_by_bit_index(path, P_IND + P_LEN - 1, 1))
		skip();
}

static void halve(Path path)
{
	if (PR_LEV >= 7) return;
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
	if (PR_LEV >= 9) return;
	PR_LEV++;
	(P_RUNNING->prg_index) = PR_START - 1;
}

static void reads(Path path)
{
	long pos = P_IND;
	if (PR_LEV >= 6) return;
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
	if (PR_LEV >= 2) return;
	if (P_ALC == 1)
	{
		int report = read_by_bit_index(path, 0, 1);
		if ((P_RUNNING->owner) != NULL)
		{
			unsigned long ownind = ((P_RUNNING->owner)->prg_index);
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
	if (PR_LEV >= 3) return;
	if (!(read_by_bit_index(path, P_IND, 1) && !read_by_bit_index(path, P_IND + P_LEN - 1, 1)))
		skip();
}

static void doalc(Path path)
{
	unsigned long new_cell_count = 0;
	unsigned long* new_data_pointer = NULL;
	if (PR_LEV >= 1) return;
		P_ALC <<= 1;

	if (P_ALC <= BITS_IN_CELL)
		new_cell_count = BITS_IN_CELL / BITS_IN_BYTE;
	else
		new_cell_count = P_ALC / BITS_IN_BYTE;

	new_cell_count /= sizeof(unsigned long);

	if ((new_data_pointer = calloc(new_cell_count, sizeof(unsigned long))) == NULL)
	{
		printf("Error allocating %d bytes: ", new_cell_count * sizeof(unsigned long));
		perror("");
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
	if (PR_LEV >= 6) return;
	if (P_LEN < 8)
	{
		write_by_bit_index(path, P_IND, P_LEN, getInput());
		return;
	}
	for (; i < (P_IND + P_LEN); i += 8)
		write_by_bit_index(path, i, 8, getInput());
}

/***
 *    oooooooooooo ooooooooooooo   .oooooo.   
 *    `888'     `8 8'   888   `8  d8P'  `Y8b  
 *     888              888      888          
 *     888oooo8         888      888          
 *     888    "         888      888          
 *     888       o      888      `88b    ooo  
 *    o888ooooood8     o888o      `Y8bood8P'  
 */

char algn(Path path)
{
	return P_IND % (P_LEN << 1) == 0;
}

unsigned long mask(int length)
{
	if (length < BITS_IN_CELL)	return ((int)1 << length) - 1;
	else			 			return 0xFFFFFFFF;
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

static void skip()
{
	if (P_RUNNING == NULL) return;
	(P_RUNNING->prg_index)++;
}
