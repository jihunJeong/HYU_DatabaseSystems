#include "bpt.h"

// MAIN

int main () {
	
	char *input_file, *value;
	FILE *fp;
	int64_t key, input_key;
	int range2;
	char instruction;
	char license_part;

	init_db(16);
	int table_id = open_table("test.db");
	while (scanf("%c", &instruction) != EOF) {
		switch (instruction) {
		case 'i':
			scanf("%ld %s", &input_key, value);
			insert(table_id, input_key, value);
			printf("%d\n", input_key);
			break;
		case 'f':
			scanf("%ld", &input_key);
			value = find(table_id, input_key);
			if (value != NULL) {
				printf("Key: %ld, Value: %s\n", input_key, value);
			} else { 
					printf("Not Exists\n");
			}
			fflush(stdout);
			break;	
		
		case 'd':
			scanf("%ld", &input_key);
			int a =delete(table_id, input_key);
			break;
		case 'q':
			close_table(table_id);
			while (getchar() != (int)'\n');
			return EXIT_SUCCESS;
		} 
	}

	shutdown_db();
	return 0;
}
