#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>

int main( int argc, char **argv ) {
 srand(time(NULL));
if(argc < 2){
	fprintf( stderr, "Too few input arguments to function\n");
	exit(1);
}

//create a string, dynamically allocated length. 
char* key;
int i, length = atoi(argv[1]);
key = malloc((length + 1)*sizeof(char));
memset(key, '\0', sizeof(key));

char letter;
for(i = 0; i < length; i++){
	letter = rand()%27+65;
	if(letter == '[')
		letter = ' ';	
	printf("%c", letter);
}
printf("\n");

free(key);
return 0;
}
