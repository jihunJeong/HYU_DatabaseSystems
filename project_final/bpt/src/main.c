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
	int table_id = open_table("test.db");
	while (scanf("%c", &instruction) != EOF) {
		switch (instruction) {
		case 'o':
			scanf("%s", input_file);
			int table_id = open_table(input_file);
			break;
		case 'i':
			//scanf("%d %ld", &id1, &input_key);
			scanf("%ld %s", &input_key, value);
			//scanf("%s", value);
			insert(1, input_key, value);
		
			/*
			for (int i = 0; i < 10000; i++) {
				insert(1, i, "a");
				printf("i %d\n", i);
			}
			*/
			break;
		case 'f':
			//scanf("%d %ld", &id1, &input_key);
			scanf("%ld", &input_key);
			f = find(1, input_key);
			
			if (f != NULL) {
				printf("Key: %ld, Value: %s\n", input_key, f);
			} else { 
				printf("Not Exists\n");
			}
			fflush(stdout);
			break;	
		
		case 'd':
			//scanf("%d %ld", &id1, &input_key);
			scanf("%ld", &input_key);
			int a =delete(1, input_key);
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
