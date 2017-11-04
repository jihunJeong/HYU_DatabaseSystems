#define Version "1.14"
#include "bpt.h"

#define offset_NP 16
#define offset_RP 8
#define num_IP 4
#define num_LP 4

bool verbose_output = false;
FILE *of;
int64_t cnt_p;

int cut(int length) {
	if (length % 2 == 0)
		return length/2;
	else
		return length/2 + 1;
}

void increase_num_page() {
	int num_page;

	fseek(of, offset_NP, SEEK_SET);
	fread(&num_page, 4, 1, of);
	num_page++;
	fseek(of, offset_NP, SEEK_SET);
	fwrite(&num_page, 4, 1, of);
}

void decrease_num_page() {
	int num_page;

	fseek(of, offset_NP, SEEK_SET);
	fread(&num_page, 4, 1, of);
	num_page--;
	fseek(of, offset_NP, SEEK_SET);
	fwrite(&num_page, 4, 1, of);
}

int get_num_key(int64_t offset) {
	int num_keys;
	fseek(of, offset + 12, SEEK_SET);
	fread(&num_keys, 4, 1, of);

	return num_keys;
}

void set_num_key(int64_t offset, int number) {
	fseek(of, offset + 12, SEEK_SET);
	fwrite(&number, 4, 1, of);
}

void usage_2 (void) {
	printf("Enter any of the following commands after the prompt > :\n"
	"\ti <k> -- Insert <k> (an integer) as both key and value).\n"
	"\ti <k> -- Insert <k> (an integer) as both key and value).\n" 
	"\tf <k> -- Find the value under key <k>.\n" 
 	"\tp <k> -- Print the path from the root to key k and its associated " 
		"value.\n" 
	"\tr <k1> <k2> -- Print the keys and values found in the range " 
		"[<k1>, <k2>\n" 
	"\td <k> -- Delete key <k> and its associated value.\n" 
	"\tx -- Destroy the whole tree.  Start again with an empty tree of the " 
		"same order.\n" 
	"\tt -- Print the B+ tree.\n" 
	"\tl -- Print the keys of the leaves (bottom row of the tree).\n" 
	"\tv -- Toggle output of pointer addresses (\"verbose\") in tree and " "leaves.\n" 
	"\tq -- Quit. (Or use Ctl-D.)\n"
	"\t? -- Print this help message.\n");
}

void initialize_db() {
	int offset_FP = 4096;
	int64_t cng = 0, rp = 0;

	int cnt_p = 1;

	for (int i = 0; i < 10; i++) {
		fseek(of, offset_FP * i, SEEK_SET);
		cng += 4096;
		fwrite(&cng, 8, 1, of);
		cnt_p += 1;
		sync();
	}
	
	fseek(of, offset_RP, SEEK_SET);
	fwrite(&rp, 8, 1, of);
	fwrite(&cnt_p, 8, 1, of);
}

int open_db (char *pathname) {
	of = fopen(pathname, "r+");
	if (of == NULL) {
		of = fopen(pathname, "w+");
		if(of == NULL) {
			printf("FILE OPEN ERROR\n");
			return 1;
		} else {
			initialize_db();
		}
	}
	sync();
	return 0;
}

int start_new_db(int64_t key, record *pointer) {
	int64_t root_offset = 4096 * 11, parent = 0, none = 0;
	int is_leaf = 1, cnt_key = 1;

	increase_num_page();

	fseek(of, offset_RP, SEEK_SET);
	fwrite(&root_offset, 8, 1, of);
	fseek(of, root_offset, SEEK_SET);
	fwrite(&parent, 8, 1, of);
	fwrite(&is_leaf, 4, 1, of);
	fwrite(&cnt_key, 4, 1, of);
	fseek(of, root_offset + 120, SEEK_SET);
	fwrite(&none, 8, 1, of);
	fwrite(&(pointer->key), 8, 1, of);
	fwrite(&(pointer->value), 1, 120, of);
	
	return 0;
}

record *make_record(int64_t key, char *value) {
	record *new_record = (record *)malloc(sizeof(record));
	
	if (new_record == NULL) {
		perror("Record creation.");
		exit(EXIT_FAILURE);
	} else {
		strcpy(new_record->value, value);
		new_record->key = key;
	}

	return new_record;
}

int64_t make_page() {
	int num_page;

	fseek(of, offset_NP, SEEK_SET);
	fread(&num_page, 4, 1, of);

	int64_t new_offset = 4096 * num_page;
	increase_num_page();
	
	return new_offset;
}

int64_t make_leaf() {
	int64_t offset_new_p = make_page();
	int i = 1, k = 0;

	fseek(of, offset_new_p + 8, SEEK_SET);
	fwrite(&i, 4, 1, of);
	fwrite(&k, 4, 1, of);
	
	return offset_new_p;
}

int64_t get_left_index(int64_t parent_offset, int64_t left_offset) {
	int64_t current_offset, current_key, leaf_key;
	int cnt_key = get_num_key(parent_offset), i = 0;

	fseek(of, left_offset + 128, SEEK_SET);
	fread(&leaf_key, 8, 1, of);

	current_offset = parent_offset + 128;
	fseek(of, current_offset, SEEK_SET);
	while (i < cnt_key) {
		fread(&current_key, 8, 1, of);
		current_offset += 16;
		if (current_key == leaf_key) {
			break;
		} else if (current_key > leaf_key) {
			return -1;
		}
		i++;
		fseek(of, current_offset, SEEK_SET);
	}
		
	current_offset = ((current_offset) % 4096 - 128) / 16 - 1;
	return current_offset;
}

int64_t insert_into_parent(int64_t parent_offset, int64_t left_offset, int64_t key, int64_t
		right_offset) {
	int64_t left_index_offset;

	if (parent_offset == 0)
		return insert_into_new_root(left_offset, key, right_offset);

	left_index_offset = get_left_index(parent_offset, left_offset);
	int num_parent_key = get_num_key(parent_offset);
	if (num_parent_key < num_IP)
		 return insert_into_node(parent_offset, left_index_offset, key,
				right_offset);

	return insert_into_node_after_splitting(parent_offset, left_index_offset,
			key, right_offset);
}

int64_t insert_into_new_root(int64_t left_offset, int64_t key, int64_t
		right_offset) {
	int64_t new_internal_page_offset = make_page();
	int64_t i = 0;
	int j = 0;
	fseek(of, offset_RP, SEEK_SET);
	fwrite(&new_internal_page_offset, 8, 1, of);
	fseek(of, new_internal_page_offset, SEEK_SET);
	fwrite(&i, 8, 1, of);
	fwrite(&j, 4, 1, of);
	j++;
	fwrite(&j, 4, 1, of);
	fseek(of, new_internal_page_offset + 120, SEEK_SET);
	fwrite(&left_offset, 8, 1, of);
	fwrite(&key, 8, 1, of);
	fwrite(&right_offset, 8, 1, of);
	fseek(of, left_offset, SEEK_SET);
	fwrite(&new_internal_page_offset, 8, 1, of);
	fseek(of, right_offset, SEEK_SET);
	fwrite(&new_internal_page_offset, 8, 1, of);
	return new_internal_page_offset;
}

int64_t insert_into_leaf(int64_t leaf_offset, int64_t key, record *pointer) {
	int i, cnt_leaf_key = get_num_key(leaf_offset), insertion_point = 0;
	int current_offset = leaf_offset;
	int64_t leaf_key;
	char value[120];

	while (insertion_point < cnt_leaf_key) {
		current_offset += 128;
		fseek(of, current_offset, SEEK_SET);
		fread(&leaf_key, 8, 1, of);

		if (key < leaf_key) {
			break;
		}
		
		insertion_point++;
	} 
	for(i = cnt_leaf_key; i > insertion_point; i--) {
		current_offset = leaf_offset + (128 * i);
		fseek(of, current_offset, SEEK_SET);
		fread(&leaf_key, 8, 1, of);
		fread(&value, 1, 120, of);
		fwrite(&leaf_key, 8, 1, of);
		fwrite(&value, 1, 120, of);
	}
	
	current_offset = leaf_offset + (128 * (insertion_point + 1));
	fseek(of, current_offset , SEEK_SET);
	fwrite(&(pointer->key), 8, 1, of);
	fwrite(&(pointer->value), 1, 120, of);
	fseek(of, current_offset, SEEK_SET);
	fread(&leaf_key, 8, 1, of);
	fread(&value, 1, 120, of);
	cnt_leaf_key++;
	set_num_key(leaf_offset, cnt_leaf_key);
	return 0;
}


int64_t insert_into_leaf_after_splitting(int64_t leaf_offset, int64_t key, record *pointer) {
	int64_t new_leaf_offset;
	int insertion_index, split, i, j;
	new_leaf_offset = make_leaf();
	record **temp_record = (record**)malloc(sizeof(record *) * (num_LP + 1));
	for (i = 0; i < num_LP + 1; i++) {
		temp_record[i] = (record*)malloc(sizeof(record));
	}
	if (temp_record == NULL) {
		perror("Temporary  record array.");
		exit(EXIT_FAILURE);
	}
	
	int64_t leaf_key, current_offset;
	insertion_index = 0;
	current_offset = leaf_offset;
	while (insertion_index < num_LP) {
		current_offset += 128;
		fseek(of, current_offset, SEEK_SET);
		fread(&leaf_key, 8, 1, of);

		if (key < leaf_key) {
			break;
		}
		
		insertion_index++;
	} 
	int num_keys = get_num_key(leaf_offset);
	int64_t temp_key;
	char temp_value[120];
	
	fseek(of, leaf_offset + 128, SEEK_SET);
	for (i = 0, j = 0; i < num_keys; i++, j++) {
		if ( j == insertion_index) j++;
		fread(&temp_key, 8, 1, of);
		temp_record[j]->key = temp_key;
		fread(&(temp_record[j]->value), 1, 120, of);
	}
	
	temp_record[insertion_index]->key = key;
	strcpy(temp_record[insertion_index]->value, pointer->value);
	
	num_keys = 0;
	split = cut(num_LP);
	
	fseek(of, leaf_offset + 128, SEEK_SET);
	for (i = 0; i < split; i++) {
		fwrite(&(temp_record[i]->key), 8, 1, of);
		fwrite(&(temp_record[i]->value), 1, 120, of);
		num_keys++;
	}

	set_num_key(leaf_offset, num_keys);

	num_keys = 0;
	fseek(of, new_leaf_offset + 128, SEEK_SET);
	for (i = split; i <= num_LP; i++) {
		fwrite(&(temp_record[i]->key), 8, 1, of);
		fwrite(&(temp_record[i]->value), 1, 120, of);
		num_keys++;
	}

	set_num_key(new_leaf_offset, num_keys);
	free(temp_record);

	int64_t next_offset, parent_offset, new_key;
	fseek(of, leaf_offset, SEEK_SET);
	fread(&parent_offset, 8, 1, of);
	fseek(of, leaf_offset + 120, SEEK_SET);
	fread(&next_offset, 8, 1, of);
	fseek(of, new_leaf_offset, SEEK_SET);
	fwrite(&parent_offset, 8, 1, of);
	fseek(of, leaf_offset + 120, SEEK_SET);
	fwrite(&new_leaf_offset, 8, 1, of);
	fseek(of, new_leaf_offset + 120, SEEK_SET);
	fwrite(&next_offset, 8, 1, of);
	fread(&new_key, 8, 1, of);
	return insert_into_parent(parent_offset, leaf_offset, new_key, new_leaf_offset);
}

int64_t insert_into_node(int64_t parent, int64_t left_index, int64_t key,
		int64_t right) {
	int i, num_parent_key = get_num_key(parent);
	int64_t current_offset, internal_key, internal_offset;
	for(i = num_parent_key - 1; i > left_index; i--) {
		current_offset = parent + 128 + (16 * i);
		fseek(of, current_offset, SEEK_SET);
		fread(&internal_key, 8, 1, of);
		fread(&internal_offset, 8, 1, of);
		fwrite(&internal_key, 8, 1, of);\
		fwrite(&internal_offset, 8, 1, of);
	}
	
	fseek(of, parent + 128 + (16 * (left_index + 1)), SEEK_SET);
	fwrite(&key, 8, 1, of);
	fwrite(&right, 8, 1, of);
	i = get_num_key(parent) + 1;
	set_num_key(parent, i);
	return 0;
}

int64_t insert_into_node_after_splitting(int64_t old_offset, int64_t
		left_index, int64_t key,
		int64_t right_offset) {
	int i, j, split, k_prime, num_old = get_num_key(old_offset), cnt = 0;	
	int64_t new_offset;
	int64_t* temp_keys = (int64_t*)malloc(sizeof(int64_t) * (num_IP + 1));
	if (temp_keys == NULL) {
		perror("Temporary keys array for splitting nodes.");
		exit(EXIT_FAILURE);
	}
	int64_t* temp_offsets = (int64_t*)malloc(sizeof(int64_t) * (num_IP + 1));
	if (temp_offsets == NULL) {
		perror("Temporary offsets array for splitting nodes.");
		exit(EXIT_FAILURE);
	}
	
	fseek(of, old_offset + 120, SEEK_SET);
	for (i = 0, j = 0; i < num_old + 1; i++, j++) {
		if (j == left_index + 2) j++;
		fread(&temp_offsets[j], 8, 1, of);
		fseek(of, 8, SEEK_CUR);
	}

	fseek(of, old_offset + 128, SEEK_SET);
	for (i = 0, j = 0; i < num_old; i++, j++) {
		if (j == left_index + 1) j++;
		fread(&temp_keys[j], 8, 1, of);
		fseek(of, 8, SEEK_CUR);
	}

	temp_keys[left_index + 1] = key;
	temp_offsets[left_index + 2] = right_offset;
	
	split = cut(num_IP + 1);
	new_offset = make_page();
	fseek(of, old_offset + 120, SEEK_SET);
	for (i = 0; i < split - 1; i++) {
		fwrite(&temp_offsets[i], 8, 1, of);
		fwrite(&temp_keys[i], 8, 1, of);
		cnt++;
	}
	fwrite(&temp_offsets[i], 8, 1, of);
	set_num_key(old_offset, cnt);
	k_prime = temp_keys[split - 1];
	
	cnt = 0;
	fseek(of, new_offset + 8, SEEK_SET);
	fwrite(&cnt, 4, 1, of);
	fseek(of, new_offset + 120, SEEK_SET);
	for (++i, j = 0; i <= num_IP; i++, j++) {
		fwrite(&temp_offsets[i], 8, 1, of);
		fwrite(&temp_keys[i], 8, 1, of);
		cnt++;
	}

	fwrite(&temp_offsets[i], 8, 1, of);
	set_num_key(new_offset, cnt);
	free(temp_keys);
	free(temp_offsets);

	int64_t child_offset, parent_offset;
	fseek(of, old_offset, SEEK_SET);
	fread(&parent_offset, 8, 1, of);
	fseek(of, new_offset, SEEK_SET);
	fwrite(&parent_offset, 8, 1, of);
	
	for (i = 0; i <= get_num_key(new_offset); i++) {
		fseek(of, new_offset + 120 + (16 * i), SEEK_SET);
		fread(&child_offset, 8, 1, of);
		fseek(of, child_offset, SEEK_SET);
		fwrite(&new_offset, 8, 1, of);
	}
	return insert_into_parent(parent_offset, old_offset, k_prime, new_offset); 
}

int64_t insert(int64_t key, char *value) {
	
	record *pointer = (record*)malloc(sizeof(record));
	record *key_find = (record*)malloc(sizeof(record));
	int64_t root_offset, leaf_offset, key_offset;
	
	pointer = make_record(key, value);
	fseek(of, offset_RP, SEEK_SET);
	fread(&root_offset, 8, 1, of);	

	if (root_offset == 0)
		return start_new_db(key, pointer);
	 
	key_find = find(key);
	if (key_find != NULL) {
		printf("Duplicated key\n");
		return 0;
	}

	key_offset = find_leaf(key) - 128;
	leaf_offset =  key_offset - (key_offset % 4096);
	int cnt_leaf_key = get_num_key(leaf_offset);
	if (cnt_leaf_key < num_LP) {
		return insert_into_leaf(leaf_offset, key, pointer);
	}
	
	return insert_into_leaf_after_splitting(leaf_offset, key, pointer);
}

record *find(int64_t key) {
	int i = 0;
	int64_t chk_key;
	int64_t chk_find = find_leaf(key);
	fseek(of, chk_find - 128, SEEK_SET);
	fread(&chk_key, 8, 1, of);

	record *pointer = (record*)malloc(sizeof(record));
	
	if (chk_find == 0 || chk_key != key) {
		return NULL;
	} else {
		pointer->key = key;
		fread(&(pointer->value), 1, 120, of);
		return pointer;
	}
}

int64_t find_leaf(int64_t key) {
	int i = 0, leaf_, in_offset, set, k;
	int64_t next_offset, internal_key, num_p, chk, new_offset, current_offset;
	
	fseek(of, offset_RP, SEEK_SET);
	fread(&next_offset, 8, 1, of);
	new_offset = next_offset;
	if (next_offset == 0) {
		printf("Empty File.\n");
		return next_offset;
	}

	fseek(of, next_offset + 8, SEEK_SET);
	fread(&leaf_, 4, 1, of);
	i = 0;
	current_offset = next_offset + 128;
	fseek(of, current_offset, SEEK_SET);
	while (leaf_ == 0) {
		current_offset +=8;
		fread(&internal_key, 8, 1, of);
		while (i < get_num_key(next_offset)) {
			i++;
			if (internal_key <= key) {
				if (i == get_num_key(next_offset)) {
					fseek(of, current_offset, SEEK_SET);
					fread(&new_offset, 8, 1, of);
					break;
				}

				current_offset +=8;
			} else {
				current_offset -= 16;
				fseek(of, current_offset, SEEK_SET);
				fread(&new_offset, 8, 1, of);
				break;
			}
			fseek(of, current_offset, SEEK_SET);
			fread(&internal_key, 8, 1, of);
			current_offset += 8;
		}

		fseek(of, new_offset + 8, SEEK_SET);
		fread(&leaf_, 4, 1, of);

		if (leaf_ == 0) {
			current_offset = new_offset + 128;
			next_offset = new_offset;
			fseek(of, current_offset, SEEK_SET);
			i = 0;
		}
	}

	next_offset = new_offset;
	int cnt_leaf = get_num_key(next_offset);
	int64_t leaf_key, leaf_offset;

	leaf_offset = next_offset + 128;
	
	for (i = 0; i < cnt_leaf; i++) {
		fseek(of, leaf_offset, SEEK_SET);
		fread(&leaf_key, 8, 1, of);
		
		if (key < leaf_key) {
			return leaf_offset;
		} else {
			leaf_offset += 128;
		}	
	}
	
	return leaf_offset;
}

int64_t delete_entry(int64_t key_leaf_offset, int64_t key, record *key_record){
	int64_t min_keys, neighbor_offset, key_prime;
	int neighbor_index, key_prime_index, capacity;

	//key_leaf_offset = remove_entry_from_node(key_leaf_offset, key, key_record);
}

int64_t delete(int64_t key) {
	int64_t key_offset = find_leaf(key) - 128;
	record *key_record = find(key);

	int64_t key_leaf_offset =  key_offset - (key_offset % 4096);
	
	if (key_record != NULL && key_leaf_offset != 0) {
		delete_entry(key_leaf_offset, key, key_record);
		free(key_record);
		return 1;
	}
	
	return 0;
}
