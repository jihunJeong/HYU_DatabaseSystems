#include "bpt.h"

// MAIN

int main () {
	
	char *input_file, value[120];
	FILE *fp;
	int64_t key, input_key;
	int range2;
	char instruction;
	char license_part;


	open_db("test.db");
	while (scanf("%c", &instruction) != EOF) {
		switch (instruction) {
		case 'i':
			//for (int i = 0; i < 1000000; i++) {

				//input_key = i;
				//strcpy(value, "a");
				
			scanf("%ld %s", &input_key, value);
			insert(input_key, value);
			//printf("%d\n", i);
			//fsync(fileno(of));
			//}
			break;

		case 'f':
			//for (input_key = -50; input_key < 32777; input_key++) {
				scanf("%ld", &input_key);
				record *you = find(input_key);
				if (you != NULL) {
					printf("Key: %ld, Value: %s\n", you->key, you->value);
				} else { 
					printf("Not Exists\n");
				}
			//}
			fflush(stdout);
			break;	
		
		case 'd':
			//for (int i = 0; i < 1000000; i++){
				//input_key = i;
			scanf("%ld", &input_key);
			delete(input_key);
					//printf("Succes %d\n", input_key);
				//} else
					//printf("not delete %d\n", input_key);
			//}
			break;
		case 'q':
			while (getchar() != (int)'\n');
			return EXIT_SUCCESS;
		} 
	}
	return 0;
}
