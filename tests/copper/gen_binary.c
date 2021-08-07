
#include <stdlib.h>
#include <stdio.h>


int main()
{
	int  n;
	char txt[32];
	char *t;

	int b,p;
	for (n=0;n<0x10000;n++)
	{
		t=txt;
		for (b=1<<15;b;b>>=1,p++) *t++=(n&b)?'1':'0';
		*t = 0;
		printf("#define B%s\n",txt);
	}

	printf("\n");

	return 0;
}

