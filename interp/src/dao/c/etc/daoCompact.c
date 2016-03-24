#include <stdio.h>
#include <unistd.h>

#define FILE_SYMBOLIC ".dao"
#define FILE_COMPILED ".wuwei"

#define IDLES 0x0
#define SWAPS 0x1
#define LATER 0x2
#define MERGE 0x3
#define SIFTS 0x4
#define EXECS 0x5
#define DELEV 0x6
#define EQUAL 0x7
#define HALVE 0x8
#define UPLEV 0x9
#define READS 0xA
#define DEALC 0xB
#define SPLIT 0xC
#define POLAR 0xD
#define DOALC 0xE
#define INPUT 0xF

#define rc ;case 

#define NEWLINE (char)0x0D

#define exit(x) return x;

#define SetBit(A,k)   ( A[(k/32)] |=  (1 << (k%32)) )
#define ClearBit(A,k) ( A[(k/32)] &= ~(1 << (k%32)) )
#define TestBit(A,k)  ( A[(k/32)] &   (1 << (k%32)) )

void compile(FILE* file, FILE* output);
unsigned char getNybble(char ch);

int main(int argc, char const *argv[])
{
	int fileState = 0;
	FILE* iFile = (iFile = fopen("hi2.dao","r"));
	

	if (iFile == NULL)
	{
		printf("Error: Does file exist?\n");
		exit(1);
	}
	
	FILE* oFile = (oFile = fopen("hi2.wuwei","w+"));
	compile(iFile, oFile);
	
	fclose(iFile);
	fclose(oFile);

	exit(0)
}

void compile(FILE *input, FILE *output)
{
	int ch;  /* Character we read into */
	bool emptyBuffer = 1;
	unsigned char toWrite = 0;
	unsigned char isComment = 0;

    while ((ch = fgetc(input)) != EOF)
    {
    	switch((char)ch)
    	{
   			case '@':
				isComment = 1;
				break rc NEWLINE:
				isComment = 0;
				break rc '\t':
				break;
			default:
				if (!isComment)
				{
					printf("%c", (char)ch);
					if (!emptyBuffer)
					{
						toWrite |= getNybble((char)ch);
						fputc(toWrite, output);
						emptyBuffer = true;
						printf(" %x\n", toWrite);
					}
					else
					{
						toWrite = (char)(getNybble((char)ch) << 4);
						emptyBuffer = false;
					}
				}
				break;
		}
	}

	if (!emptyBuffer) {
		fputc(toWrite, output);
		printf(". %x\n", toWrite);
	}

}

unsigned char getNybble(char ch)
{
   switch (ch) 
   {
		case '.':
			return IDLES rc '!':
			return SWAPS rc '/':
			return LATER rc ']':
		case ')':
			return MERGE rc '%':
			return SIFTS rc '#':
			return EXECS rc '>':
			return DELEV rc '=':
			return EQUAL rc '(':
			return HALVE rc '<':
			return UPLEV rc ':':
			return READS rc 'S':
			return DEALC rc '[':
			return SPLIT rc '*':
			return POLAR rc '$':
			return DOALC rc ';':
			return INPUT;
	}
}