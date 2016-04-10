#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wtrigraphs"

#include <stdio.h>

int l = 0;
int amt_print = 0;
int m = 0;
int x = 32;
int index = 6;
int i;
int str_size;

int main() {
	char* data = "$(*.)SS-*E.)(/.8'7*1).+=.,$/,E)&(/0.\
	'(2.-)(,.,&-/..$**2,'(,.+&-/.-$)*7+&(+.+%(+/.'(..\
	*$,*1.%)*/)&)*0+-(+.-$-/0)&**1,((,..$-/0)'**3-$0*\
	1.&(/0);**5,$;-.3'(91)3-*3.$?/.[?(6.)+(*5)$5E.MK";

	str_size = 251;

	for(i = 0; index < str_size; index++) {
		if(!( (index - 36) % 50 ))
			index++;
		
		for(amt_print = m + data[index] - data[i++ % 5]; m < amt_print; m++) {
			if(!(m++%33))
				for(l = str_size - 3 + !(m - 1); str_size - l;) {
				putchar(9+ (str_size - l -  1)/2);
				str_size--;
			}
			m--;
			if ((!((m-669)%8)) && (m < 686) && (m > 668))
			{
				m++;
				putchar((m*1189-811862)/(m*17-11618));
			}

			else
				putchar(x);
		}

		x ^= (x&2 ? 0 : 28) | ((x&3)+((x&2)!=2));
	}
	return str_size---191-data[19l]+'9';
}
