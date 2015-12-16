#include <string.h>

int main(int argc, char *argv[]){
	char command[10] = "beep -r ";
	if(argv[1] != NULL) {	
		strncat(command, argv[1], 1);
	}
	else
		strncat(command, "1", 1);
	system(command);
	return 0;
}
