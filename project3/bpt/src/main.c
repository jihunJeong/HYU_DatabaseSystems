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
			//for (int i = 0; i < 1000000; i++) {

				//input_key = i;
				//strcpy(value, "a");
				//value = "a";			
				scanf("%ld %s", &input_key, value);
				insert(table_id, input_key, value);
				//printf("%d\n", input_key);
			//}
			break;

		case 'f':
			//for (input_key = 0; input_key < 1000000; input_key++) {
				scanf("%ld", &input_key);
				value = find(table_id, input_key);
				if (value != NULL) {
					printf("Key: %ld, Value: %s\n", input_key, value);
				} else { 
					printf("Not Exists\n");
				}
			//}
			fflush(stdout);
			break;	
		
		case 'd':
			//for (int i = 262144; i < 524288; i++){
				//input_key = i;
				scanf("%ld", &input_key);
				int a =delete(table_id, input_key);
				//if (a) {
				//	printf("Succes %d\n", input_key);
				//} else {
				//	printf("not delete %d\n", input_key);
					//return 0;
				//}
			//}
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
