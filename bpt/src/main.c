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
			scanf("%ld %s", &input_key, value);
			insert(input_key, value);
			fsync(fileno(of));
			break;

		case 'f':
			//for (input_key = -50; input_key < 32777; input_key++) {
				scanf("%ld", &input_key);
				record *you = find(input_key);
				if (you != NULL) {
					printf("Key: %ld, Value: %s\n", you->key, you->value);
				} else { 
					printf("Not Exists %ld\n", input_key);
				}
			//}
			fflush(stdout);
			break;	
		
		case 'd':
			scanf("%ld", &input_key);
			delete(input_key);
			break;
		case 'q':
			while (getchar() != (int)'\n');
			return EXIT_SUCCESS;
		} 
	}
	return 0;
}
