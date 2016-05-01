/*COMPACT_DAOYU_INTERPRETER-KAYNATO_2016*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Y if
#define r return
#define B (S*8)
#define R(c) Y(c)r

#define L(l) Y(PR_LEV>=l)r;
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

unsigned long S = sizeof(unsigned long);

typedef struct PATH
{
	struct PATH*	owner;						/* OWNER	  PROGRAM */
	struct PATH*	child;						/* CHILD	  PROGRAM */
	unsigned long*	prg_data;					/*		   DATA	 	  */
	unsigned long	prg_allocbits;				/* OPEN	   DATA   BITS*/
	unsigned long	prg_index;					/* INSTRUCTION POINTER*/
	unsigned long	prg_level;					/* OPERATING   LEVEL  */
	unsigned long	sel_length;					/* LENGTH OF SELECTION*/
	unsigned long	sel_index;					/* INDEX  OF SELECTION*/
	unsigned long	prg_floor;					/* FLOOR  OF PATH     */
	unsigned long	prg_start;					/* START  OF RUNNING  */
} Pathstrx;

typedef Pathstrx* Path;

void swaps(Path), later(Path), merge(Path), sifts(Path), delev(Path), equal(Path), halve(Path);
void uplev(Path), reads(Path), dealc(Path), split(Path), polar(Path), doalc(Path), input(Path), execs(Path, Path);

char			getInput();
char			algn(Path);
void			skip();
void			W(Path, unsigned long, unsigned long, unsigned long);
unsigned long 	G(Path, unsigned long, unsigned long);
unsigned long 	mask(unsigned long);

int doloop = 1;

void(*functions[16])(Path) = \
	{NULL, swaps, later, merge, sifts, NULL , delev, equal, \
	halve, uplev, reads, dealc, split, polar, doalc, input};

const struct PATH NEW_PATH = { NULL, NULL, NULL, 1, 0, 0, 1, 0, 0, 0 };

Path P_RUNNING = NULL,
	 P_WRITTEN = NULL;

const char* symbols = ".!/)%#>=(<:S[*$;";

char* I;
unsigned long x=0;
unsigned long y;
unsigned char z=0;

/* Run argv[0] as code. Input separated by '!' and once empty reads the null character. */
int main(int argc, char * argv[])
{
	Pathstrx newpath=NEW_PATH;
	Path path=&newpath;
	I=argv[1];
	R(argc<2)0;

	while(I[x]&&I[x]-64)x++;
	R(!x++)0;
	y=x/2;
	while(y/2){y/=2;z++;}
	y<<=z;
	x=x/2-y?y*2:y;

	P_ALC=x*8;
	Y(x%S)x=S;

	Y(!(path->prg_data=calloc(x,1))){
		printf("CANNOT ALLOCATE %d BYTES: ",x);
		perror("");
		abort();
	}

	x=0;
	while(I[x]&&I[x]-64)
	{
		y=I[x];
		W(path,4*x++,4,y-33?y-47?y-93?y-41?y-37?y-35?y-62?y-61?y-40?y-60?y-58?y-83?y-91?y-42?y-36?y-59?0:15:14:13:12:11:10:9:8:7:6:5:4:3:3:2:1);
	}

	while(*I&&*I-64)I++;
	I=*I-64?0:I+1;

	execs(P_RUNNING=path,0);
	free(P_DATA);
	r 0;
}

char getInput(){r I&&*I?*I++:(I=0);}

void swaps(Path path){x=0;L(1)R(P_LEN<2);Y(P_LEN<=B){y=P_LEN/2;W(path,P_IND,P_LEN,G(path,P_IND,y)|(G(path,P_IND+y,y)<<y));r;}while(x<P_LEN/B/2){
		P_DATA[x+P_IND/B]=P_DATA[x+P_IND/B+P_LEN/B/2];P_DATA[x+P_IND/B+P_LEN/B/2]=P_DATA[x+P_IND/B];x++;}}

void later(Path path){Y(algn(path)||PR_LEV>3){P_IND+=P_LEN;r;}merge(path);}

void merge(Path path){L(7)Y(P_LEN<P_ALC){Y(!algn(path))P_IND-=P_LEN;P_LEN*=2;r;}R(!P_OWNER);P_WRITTEN=P_OWNER;P_WRITTEN->sel_index=P_WRITTEN->sel_length=1;}

void sifts(Path path){x=P_IND;y=0;L(5);while(x<P_ALC){while(!G(path,x+y,4))y+=4;Y(y)W(path,x,4,(x+y<P_ALC)?G(path,x+y,4):0);x+=4;y+=4;}}

void execs(Path path, Path caller)
{
	y=0;L(8)P_RUNNING=path;Y (!P_CHILD){Y(!(P_CHILD=calloc(1,sizeof(Pathstrx)))){printf("MEMORY_OVERFLOW\n");r;}
	memcpy(P_CHILD,&NEW_PATH,sizeof(Pathstrx));path->child->owner=path;path->child->prg_floor=path->prg_floor+1;path->child->prg_data=calloc(1,S);}

	P_WRITTEN=P_CHILD;
	P_PIND=P_IND/4;
	PR_START=P_PIND;

	while(doloop&&P_PIND<P_ALC/4&&path&&P_WRITTEN){
		y=P_RUNNING->prg_index;
		z=(P_RUNNING->prg_data[y*4/32]>>(32-((y*4)%32)-4))&mask(4);
		Y(z==5)execs(P_WRITTEN,path);else Y(z)functions[z](P_WRITTEN);P_PIND++;}
	Y(!caller){free(P_CHILD);P_CHILD=NULL;r;}
	Y(!doloop){free(P_CHILD);P_CHILD=NULL;doloop=1;}
	P_RUNNING=caller;P_WRITTEN=caller->child;}

void delev(Path path){Y(PR_LEV)PR_LEV--;}

void equal(Path path){L(5)Y(G(path,P_IND,1)^G(path,P_IND+P_LEN-1,1))skip();}

void halve(Path path){L(7)Y(P_LEN>1){P_LEN/=2;r;}R(!P_CHILD);P_WRITTEN=P_CHILD;P_WRITTEN->sel_length=P_WRITTEN->prg_allocbits;}

void uplev(Path path){L(9)PR_LEV++;P_RUNNING->prg_index=PR_START-1;}

void reads(Path path){y=P_IND;L(6)while(y<P_IND+P_LEN){putchar(G(path,y,8));y+=8;}}

void dealc(Path path){L(2)Y(P_ALC==1){Y(!P_RUNNING->owner)W(P_RUNNING->owner,P_RUNNING->owner->prg_index*4,4,G(path,0,1));free(P_DATA);doloop=P_DATA=0;r;}P_ALC/=2;
	realloc(P_DATA,P_ALC<9?1:P_ALC/8);Y(P_LEN>1)halve(path);Y(P_IND+P_LEN>P_ALC)P_IND-=P_ALC;}

void split(Path path){Y(PR_LEV<1){x=P_LEN;Y(x==1){R(!P_CHILD);P_WRITTEN=P_CHILD;P_WRITTEN->sel_length=P_WRITTEN->prg_allocbits;split(P_WRITTEN);halve(P_WRITTEN);r;}
	Y(x>B){y=P_IND/B;z=y-1+x/B;while(y<z){P_DATA[y++]=0xFFFFFFFF;P_DATA[z--]=0;}}else{W(path,P_IND,x/2,mask(x));W(path,P_IND+(x/2),x/2,~mask(x));}}halve(path);}

void polar(Path path){L(3)Y(!(G(path,P_IND,1)&&!G(path,P_IND+P_LEN-1,1)))skip();}

void doalc(Path path){unsigned long*i=0;L(1)z=0;P_ALC*=2;z=(P_ALC>B?P_ALC:B)/8/S;Y(!(i=calloc(z,S))){printf("CANNOT ALLOCATE %d bytes: ",z*S);perror("");abort();}
	memcpy(i,P_DATA,z>1?z*S/2:S);P_DATA=i;merge(path);}

void input(Path path){z=P_IND;L(6);Y(P_LEN<8){W(path,P_IND,P_LEN,getInput());r;}while(z<(P_IND+P_LEN)){W(path,z,8,getInput());z+=8;}}

char algn(Path path){r!(P_IND%(P_LEN*2));}

unsigned long mask(unsigned long i){R(i<B)((int)1<<i)-1;r 0xFFFFFFFF;}

unsigned long G(Path path, unsigned long i, unsigned long j){r(P_DATA[i/B]>>(B-j-i%B))&mask(j);}

void W(Path path, unsigned long i, unsigned long j, unsigned long k){R(j>B);z=B-j-i%B;P_DATA[i/B]&=~(mask(j)<<z);P_DATA[i/B]|=(k&mask(j))<<z;}

void skip(){R(!P_RUNNING);P_RUNNING->prg_index++;}
