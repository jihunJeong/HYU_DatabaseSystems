#include "bpt.h"

// MAIN

int main (int argc, char **argv) {
	
	char *input_file, value[120];
	FILE *fp;
	int64_t key;
	int range2, input_key;
	char instruction;
	char license_part;

	input_file = argv[1];

	open_db(input_file);
	usage_2();
	
	printf("> ");
	while (scanf("%c", &instruction) != EOF) {
		switch (instruction) {
		case 'i':
			scanf("%d %s", &input_key, value);
			key = input_key;
			insert(key, value);
			break;

		case 'f':
			scanf("%d", &input_key);
			record *you = find(input_key);
			if (you != NULL) {
				printf("Find key : %ld\t", you->key);
				printf("Find value : %s\n", you->value);
			} else { 
				printf("Not Found\n");
			}
			break;	
		
		case 'd':
			scanf("%d", &input_key);
			if (delete(input_key)) {
				printf("Delete Key : %ld\n", input_key);
			} else {
				printf("Not Delete\n");
			}
			break;
		}
	}

	return 0;
}
