
#include <stdio.h>
#include <stdlib.h>

#include "fifo.h"

void main() 
{
	char	buffer[128];
	Fifo f1(20);
	
	int x = 0;
	for (x=1;x<30;x++) {
		f1.push(x);
		sprintf(buffer,"Num chars in buffer after %d pushes: %d\n", x, f1.count());
		printf(buffer);
	}
}
