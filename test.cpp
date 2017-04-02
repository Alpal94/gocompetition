#include<stdio.h>
#include<stdlib.h>
#include<string.h>


int main() {
	int i = 4 + '0';
	char *tmp = (char*) malloc (8 * sizeof(char));
	tmp[0] = i;	tmp[1] = '\0';
	printf("\nHELLOW ROLD%s\n\n", tmp);
}

