#include <stdio.h>


int main(int argc, char **argv)
{
	FILE fp = fopen("f1.trace", "r");
	char c; int d;
	int address;
	int count
	= fscanf("%c %h,%d\n", &c, &address, &d );
	if (count < 3) printf("Brughhhhhhhh");
	

}