#include "bpt.h"

// MAIN

int main () {
	
	char input_file[32], value[64], *f;
	FILE *fp;
	int64_t key, input_key;
	int range2, id1, id2;
	char instruction;
	char license_part;

	init_db(16);
	
	while (scanf("%c", &instruction) != EOF) {
		switch (instruction) {
		case 'o':
			scanf("%s", input_file);
			int table_id = open_table(input_file);
			printf("%d\n", table_id);
			break;
		case 'i':
			scanf("%d %ld", &id1, &input_key);
			scanf("%s", value);
			insert(id1, input_key, value);
			break;
		case 'f':
			scanf("%d %ld", &id1, &input_key);
			f = find(id1, input_key);
			
			if (f != NULL) {
				printf("Key: %ld, Value: %s\n", input_key, f);
			} else { 
				printf("Not Exists\n");
			}
			fflush(stdout);
			break;	
		
		case 'd':
			scanf("%d %ld", &id1, &input_key);
			int a =delete(id1, input_key);
			break;
		case 'j':
			scanf("%d %d", &id1, &id2);
			scanf("%s", input_file);
			join_table(id1, id2, input_file);
			break;
		case 'q':
			while (getchar() != (int)'\n');
			return EXIT_SUCCESS;
		case 'c':
			scanf("%d", &id1);
			close_table(id1);
		} 
	}
	shutdown_db();
	return 0;
}
