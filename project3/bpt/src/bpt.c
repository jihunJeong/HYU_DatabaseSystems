#define Version "1.14"
#include "bpt.h"

#define	offset_FP 0
#define offset_NP 16
#define offset_RP 8
#define num_IP 248
#define num_LP 31

int MAX_FRAME;
int clk_hand = 0;
bool verbose_output = false;
buffer **buffer_pool;
FILE *of;
FILE *array_f[11];

int cut(int length) {
	if (length % 2 == 0)
		return length/2;
	else
		return length/2 + 1;
}

page init_page() {
	page o_page;
	o_page.is_leaf = 0;
	o_page.parent = 0;
	return o_page;
}

// for copy page by reference
void copy_page(page *n, page *p) {
	int i;
	*n = *p;

	for (i = 0; i < 31; i++) {
		strcpy((*n).value[i], (*p).value[i]);
	}

	for (i = 0; i < 248; i++) {
		(*n).key[i] = (*p).key[i];
		(*n).offset[i] = (*p).offset[i];
	}

	(*n).offset[i] = (*p).offset[i];
}

void decrease_pin(int table_id) {
	for (int i = 0; i < MAX_FRAME; i++) {
		if (buffer_pool[i]->table_id == table_id) {
			buffer_pool[i]->pin_count = 0;
		}
	}
}

int64_t find_last_free_page() {
	int64_t current_offset;

	fseek(of, offset_FP, SEEK_SET);
	fread(&current_offset, 8, 1, of);
	while(current_offset != 0) {
		fseek(of, current_offset, SEEK_SET);
		fread(&current_offset, 8, 1, of);
	}
	return current_offset;
}

page clock_request_page(int table_id, page new_page, int64_t offset) {
	page *retval_page = NULL;
	while (retval_page == NULL) {
		buffer *current = buffer_pool[clk_hand];
		if (current->refbit == 0 && current->pin_count == 0) {
			if (current->is_dirty == 1)
				write_page_db(table_id, current);
			retval_page = &new_page;
			current->page = new_page;
			current->is_dirty = 0;
			current->pin_count = 1;
			current->refbit = 1;
			current->page_offset = offset;
			current->table_id = table_id;
		} else if (current->refbit == 1) {		
			current->refbit = 0;
		}

		clk_hand = (clk_hand + 1) % MAX_FRAME;
	}

	return new_page;
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

int init_db(int num_buf) {
	MAX_FRAME = num_buf;
	clk_hand = 0;
	buffer_pool = (buffer**)malloc(sizeof(buffer*) * num_buf);

	for(int i = 0; i < num_buf; i++) {
		buffer_pool[i] = (buffer*)malloc(sizeof(buffer));
		buffer_pool[i]->refbit = 0;
		buffer_pool[i]->pin_count = 0;
		buffer_pool[i]->is_dirty = 0;
		buffer_pool[i]->page_offset = -1;
	}
	if (buffer_pool == NULL) {
		return 1;
	}

	return 0;
}

void initialize_db(int table_id) {
	int FP = 4096;
	int64_t cng = 0, rp = 0;
	int cnt_p = 1;
	of = array_f[table_id];

	page apage = read_page_buf(table_id, 0);
	for (int i = 0; i < 10; i++) {
		fseek(of, FP * i, SEEK_SET);
		cng += 4096;
		fwrite(&cng, 8, 1, of);
		cnt_p += 1;
		
	}
	apage.root = 0;
	apage.num_page = cnt_p;
	write_page_buf(table_id, apage, 0);
	page bpage = read_page_buf(table_id, 45056);
	bpage.parent = 0;
	write_page_buf(table_id, bpage, 45056);
}

int open_table(char *pathname) {
	int i = 0;
	of = fopen(pathname, "r+");
	if (of == NULL) {
		of = fopen(pathname, "w+");
		if(of == NULL) {
			printf("FILE OPEN ERROR\n");
			return -1;
		}
	}

	for (i = 1; i < 11; i ++) {
		if (array_f[i] == NULL) {
			array_f[i] = of;
			break;
		}
	}

	initialize_db(i);
	return i;
}

int close_table(int table_id) {
	int i = 0;;
	for(i; i < MAX_FRAME; i++) {
		if (buffer_pool[i]->table_id == table_id) {
			write_page_db(table_id, buffer_pool[i]);
		}
	}
	of = array_f[table_id];
	fclose(of);
	array_f[i] = NULL;
}

int shutdown_db() {
	for (int i = 0; i < MAX_FRAME; i++) {

		free(buffer_pool[i]);
	}

	free(buffer_pool);
}

int start_new_db(int table_id, int64_t key, record *pointer) {
	int64_t root_offset = 4096 * 11;

	page page = read_page_buf(table_id, 0);
	page.root = root_offset;
	page.num_page++;
	write_page_buf(table_id, page, 0);


	page = read_page_buf(table_id, root_offset);
	page.parent = 0;
	page.right_page = 0;
	page.is_leaf = 1;
	page.num_key = 1;
	page.key[0] = pointer->key;
	strcpy(page.value[0], pointer->value);
	write_page_buf(table_id, page, root_offset);
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

int64_t make_page(int table_id) {
	int num_page;

	page page = read_page_buf(table_id, 0);
	int64_t new_offset = 4096 * page.num_page;

	page.num_page++;
	write_page_buf(table_id, page, 0);


	page = read_page_buf(table_id, new_offset);
	page.is_leaf = 0;
	write_page_buf(table_id, page, new_offset);


	return new_offset;
}

int64_t make_leaf(int table_id) {
	int64_t offset_new_p = make_page(table_id);
	int i = 1, k = 0;

	page page = read_page_buf(table_id, offset_new_p);
	page.is_leaf = 1;
	write_page_buf(table_id, page, offset_new_p);

	
	return offset_new_p;
}

void write_page_db(int table_id, buffer *buf) {
	int i;
	of = array_f[table_id];
	char reserved[104];
	char head_reserved[4072];

	fseek(of, buf->page_offset, SEEK_SET);
	if (buf->page_offset != 0) {
		fwrite(&(buf->page.parent), 8, 1, of);
		fwrite(&(buf->page.is_leaf), 4, 1, of);
		fwrite(&(buf->page.num_key), 4, 1, of);
		fwrite(&reserved, 1, 104, of);
		if (buf->page.is_leaf) {
			fwrite(&(buf->page.right_page), 8, 1, of);
			for (i = 0; i < buf->page.num_key; i++) {
				fwrite(&(buf->page.key[i]), 8, 1, of);
				fwrite(&(buf->page.value[i]), 1, 120, of);
			}
		} else {
			for (i = 0; i < buf->page.num_key; i++) {
				fwrite(&(buf->page.offset[i]), 8, 1, of);
				fwrite(&(buf->page.key[i]), 8, 1, of);
			}
			fwrite(&(buf->page.offset[i]), 8, 1, of);
		}
	} else {
		fwrite(&(buf->page.free), 8, 1, of);
		fwrite(&(buf->page.root), 8, 1, of);
		fwrite(&(buf->page.num_page), 8, 1, of);
		fwrite(&head_reserved, 1, 4072, of);
	}
}

void write_page_buf(int table_id, page p_page, int64_t offset) {
	for (int i = 0; i < MAX_FRAME; i++) {
		if (offset == buffer_pool[i]->page_offset && table_id == buffer_pool[i]->table_id) {
			copy_page(&(buffer_pool[i]->page), &p_page);
			buffer_pool[i]->is_dirty = 1;
			buffer_pool[i]->pin_count = 0;
			break;
		}
	}
}

page read_page_db(int table_id, int64_t offset) {
	page page;
	int i;
	int64_t chk = -1;
	char head_reserved[4072];
	of = array_f[table_id];
	fseek(of, offset, SEEK_SET);
	if (fread(&chk, 8, 1, of) != 8 && chk < 0) {
		return page;
	}
	fseek(of, offset, SEEK_SET);

	if (offset != 0) {
		fread(&(page.parent), 8, 1, of);
		fread(&(page.is_leaf), 4, 1, of);
		fread(&(page.num_key), 4, 1, of);
		fseek(of, offset + 120, SEEK_SET);
		if (page.is_leaf) {
			fread(&(page.right_page), 8, 1, of);
			for (i = 0; i < page.num_key; i++) {
				fread(&(page.key[i]), 8, 1, of);
				fread(&(page.value[i]), 1, 120, of);
			}
		} else {
			for (i = 0; i < page.num_key; i++) {
				fread(&(page.offset[i]), 8, 1, of);
				fread(&(page.key[i]), 8, 1, of);
			}
			fread(&(page.offset[i]), 8, 1, of);
		}
	} else {
		fread(&(page.free), 8, 1, of);
		fread(&(page.root), 8, 1, of);
		fread(&(page.num_page), 8, 1, of);
		fread(&head_reserved, 1, 4072, of);
	}

	return page;
}

page read_page_buf(int table_id, int64_t offset) {
	int i;
	page page;
	for (i = 0; i < MAX_FRAME; i++) {
		if (offset == buffer_pool[i]->page_offset && table_id == buffer_pool[i]->table_id) {
			buffer_pool[i]->refbit = 1;
			buffer_pool[i]->pin_count = 1;
			page = buffer_pool[i]->page;
			break;
		}
	}

	if (i == MAX_FRAME) {
		page = read_page_db(table_id, offset);
		page = clock_request_page(table_id, page, offset);
	}

	return page;
}

int64_t get_left_index(int table_id, int64_t parent_offset, int64_t left_offset) {
	int64_t left_index = 0, tmp;

	page page = read_page_buf(table_id, parent_offset);
	while (left_index <= page.num_key && 
            page.offset[left_index] != left_offset)
        left_index++;

	return left_index;
}

int64_t insert_into_parent(int table_id, int64_t parent_offset, int64_t left_offset, int64_t key, int64_t
			right_offset) {
	int64_t left_index;

	if (parent_offset == 0)		
		return insert_into_new_root(table_id, left_offset, key, right_offset);

	left_index = get_left_index(table_id, parent_offset, left_offset);
	page parent_page = read_page_buf(table_id, parent_offset);
	if (parent_page.num_key < num_IP) {
		 int64_t insert_into_node1 = insert_into_node(table_id, parent_offset, left_index, key, right_offset);
		 return insert_into_node1;
	}

	int64_t insert_into_node_after_splitting1 = insert_into_node_after_splitting(table_id, parent_offset, left_index, key, right_offset);
	return insert_into_node_after_splitting1;
}

int64_t insert_into_new_root(int table_id, int64_t left_offset, int64_t key, int64_t
		right_offset) {
	int64_t new_internal_offset = make_page(table_id);
	int64_t i = 0;
	int j = 0;
	page old_page = read_page_buf(table_id, 0);
	old_page.root = new_internal_offset;
	page new_page = read_page_buf(table_id, new_internal_offset);
	new_page.parent = 0;
	new_page.is_leaf = 0;
	new_page.num_key = 1;
	new_page.offset[0] = left_offset;
	new_page.key[0] = key;
	new_page.offset[1] = right_offset;
	write_page_buf(table_id, old_page, 0);
	write_page_buf(table_id, new_page, new_internal_offset);


	page leaf_page = read_page_buf(table_id, left_offset);
	leaf_page.parent = new_internal_offset;
	write_page_buf(table_id, leaf_page, left_offset);
	page right_page = read_page_buf(table_id, right_offset);
	right_page.parent = new_internal_offset;
	write_page_buf(table_id, right_page, right_offset);


	
	return new_internal_offset;
}

int64_t insert_into_leaf(int table_id, int64_t offset, int64_t key, record *pointer) {
	int i, insertion_point = 0;
	char value[120];

	page page = read_page_buf(table_id, offset);
	while (insertion_point < page.num_key && page.key[insertion_point] < key) {
		insertion_point++;
	}

	for(i = page.num_key; i > insertion_point; i--) {
		page.key[i] = page.key[i - 1];
		strcpy(page.value[i], page.value[i - 1]);
	}

	page.key[insertion_point] = key;
	strcpy(page.value[insertion_point], pointer->value);
	page.num_key++;
	write_page_buf(table_id, page, offset);
	return offset;
}

int64_t insert_into_leaf_after_splitting(int table_id, int64_t leaf_offset, int64_t key, record *pointer) {
	int64_t new_leaf_offset;
	int insertion_index, split, i, j;
	new_leaf_offset = make_leaf(table_id);
	record **temp_record = (record**)malloc(sizeof(record *) * (num_LP + 1));
	for (i = 0; i < num_LP + 1; i++) {
		temp_record[i] = (record*)malloc(sizeof(record));
	}
	if (temp_record == NULL) {
		perror("Temporary  record array.");
		exit(EXIT_FAILURE);
	}

	insertion_index = 0;
	page old_page = read_page_buf(table_id, leaf_offset);
	while (insertion_index < num_LP && old_page.key[insertion_index] < key)
        insertion_index++;

	for (i = 0, j = 0; i < old_page.num_key; i++, j++) {
		if ( j == insertion_index) j++;
		temp_record[j]->key = old_page.key[i];
		strcpy(temp_record[j]->value, old_page.value[i]);
	}

	temp_record[insertion_index]->key = key;
	strcpy(temp_record[insertion_index]->value, pointer->value);

	old_page.num_key = 0;
	split = cut(num_LP);

	for (i = 0; i < split; i++) {
		old_page.key[i] = temp_record[i]->key;
		strcpy(old_page.value[i], temp_record[i]->value);
		old_page.num_key++;
	}

	old_page.right_page = new_leaf_offset;
	write_page_buf(table_id, old_page, leaf_offset);

	page new_page = read_page_buf(table_id, new_leaf_offset);
	new_page.num_key = 0;
	for (i = split, j = 0; i <= num_LP; i++, j++) {
		new_page.key[j] = temp_record[i]->key;
        strcpy(new_page.value[j], temp_record[i]->value);
        new_page.num_key++;
	}

	for (i = 0; i < num_LP + 1; i++) {
		free(temp_record[i]);
	}

	free(temp_record);

	int64_t new_key, parent_offset = old_page.parent;
	new_page.parent = old_page.parent;
	new_page.right_page = old_page.right_page;
	new_key = new_page.key[0];
	write_page_buf(table_id, new_page, new_leaf_offset);

	return insert_into_parent(table_id, parent_offset, leaf_offset, new_key, new_leaf_offset);
}

int64_t insert_into_node(int table_id, int64_t parent, int64_t left_index, int64_t key,
		int64_t right) {
	int i;

	page parent_page = read_page_buf(table_id, parent);
	for(i = parent_page.num_key; i > left_index; i--) {
		parent_page.key[i] = parent_page.key[i - 1];
		parent_page.offset[i + 1] = parent_page.offset[i];
	}

	parent_page.key[left_index] = key;
	parent_page.offset[left_index + 1] = right;
	parent_page.num_key++;
	write_page_buf(table_id, parent_page, parent);
	
	return 0;
}

int64_t insert_into_node_after_splitting(int table_id, int64_t old_offset, int64_t left_index, 
	int64_t key, int64_t right_offset) {

	int i, j, split, k_prime, cnt = 0;	
	int64_t new_offset;

	int64_t* temp_keys = (int64_t*)malloc(sizeof(int64_t) * (num_IP + 1));
	if (temp_keys == NULL) {
		perror("Temporary keys array for splitting nodes.");
		exit(EXIT_FAILURE);
	}

	int64_t* temp_offsets = (int64_t*)malloc(sizeof(int64_t) * (num_IP + 2));
	if (temp_offsets == NULL) {
		perror("Temporary offsets array for splitting nodes.");
		exit(EXIT_FAILURE);
	}

	page old_page = read_page_buf(table_id, old_offset);
	for (i = 0, j = 0; i < old_page.num_key + 1; i++, j++) {
		if (j == left_index + 1) j++;
		temp_offsets[j] = old_page.offset[i];
	}

	for (i = 0, j = 0; i < old_page.num_key; i++, j++) {
		if (j == left_index) j++;
		temp_keys[j] = old_page.key[i];
	}

	temp_keys[left_index] = key;
	temp_offsets[left_index + 1] = right_offset;

	split = cut(num_IP + 1);

	old_page.num_key = 0;
	for (i = 0; i < split - 1; i++) {
		old_page.offset[i] = temp_offsets[i];
		old_page.key[i] = temp_keys[i];
		old_page.num_key++;
	}

	old_page.offset[i] = temp_offsets[i];
	k_prime = temp_keys[split - 1];

	new_offset = make_page(table_id);
	page new_page = read_page_buf(table_id, new_offset);
	new_page.is_leaf = 0;
	new_page.num_key = 0;

	for (++i, j = 0; i <= num_IP; i++, j++) {
		new_page.offset[j] = temp_offsets[i];
		new_page.key[j] = temp_keys[i];
		new_page.num_key++;
	}

	new_page.offset[j] = temp_offsets[i];
	new_page.parent = old_page.parent;
	write_page_buf(table_id, old_page, old_offset);
	write_page_buf(table_id, new_page, new_offset);

	free(temp_keys);
	free(temp_offsets);

	int64_t child_offset, parent_offset = old_page.parent;
	page child_page;
	for (i = 0; i <= new_page.num_key; i++) {
		child_offset = new_page.offset[i];
		child_page = read_page_buf(table_id, child_offset);
		child_page.parent = new_offset;
		write_page_buf(table_id, child_page, child_offset);
	}

	return insert_into_parent(table_id, parent_offset, old_offset, k_prime, new_offset); 
}

int64_t insert(int table_id, int64_t key, char *value) {

	record *pointer = (record*)malloc(sizeof(record));
	char *key_find;
	int64_t leaf_offset;

	key_find = find(table_id, key);
	if (key_find != NULL) {
		return 1;
	}

	free(key_find);

	pointer = make_record(key, value);
	page page = read_page_buf(table_id, 0);

	if (page.root == 0) {
		start_new_db(table_id, key, pointer);
		decrease_pin(table_id);
		free(pointer);
		return 0;
	}

	leaf_offset = find_leaf(table_id, key);
	page = read_page_buf(table_id, leaf_offset);
	if (page.num_key < num_LP) {
		insert_into_leaf(table_id, leaf_offset, key, pointer);
		decrease_pin(table_id);
		free(pointer);
		return 0;
	}

	insert_into_leaf_after_splitting(table_id, leaf_offset, key, pointer);
	decrease_pin(table_id);
	free(pointer);
	return 0;
}

char *find(int table_id, int64_t key) {
	int i = 0;
	int64_t offset = find_leaf(table_id, key);
	char *value;
	if (offset == 0) {
		return NULL;
	}

	page page = read_page_buf(table_id, offset);
	for (i = 0; i < page.num_key; i++) {
		if (page.key[i] == key) {
			break;
		}
	}

	if (i == page.num_key) {
		return NULL;
	}

	decrease_pin(table_id);
	value = page.value[i];
	return value;
}

int64_t find_leaf(int table_id, int64_t key) {
	int i = 0, chk, first, last, mid;
	page page = read_page_buf(table_id, 0);
	int64_t offset = page.root;
	if (offset == 0) {
		return offset;
	}

	page = read_page_buf(table_id, offset);

	while (!page.is_leaf) {
		i = 0;
		chk = 0;
		first = 0;
		last = page.num_key - 1;
		mid = 0;

		while (first <= last) {
			if (chk == 0) {
				if (key < page.key[0]) {
					offset = page.offset[0];
					break;
				} else if (key >= page.key[page.num_key - 1]) {
					offset = page.offset[page.num_key];
					break;
				}
			}
			chk++;

			mid = (first + last) / 2;
			if (page.key[mid] <= key && page.key[mid + 1] > key) {
				offset = page.offset[mid + 1];
				break;
			} else  {
				if (page.key[mid] > key)
					last = mid - 1;
				else
					first = mid + 1;
			}

		}
		page = read_page_buf(table_id, offset);
	}

	return offset;
}

int get_neighbor_index(int table_id, int64_t offset) {
	page apage = read_page_buf(table_id, offset);
	page parent = read_page_buf(table_id, apage.parent);
	for (int i = 0; i <= parent.num_key; i++) {
		if (parent.offset[i] == offset) {		
			return i - 1;
		}
	}

	printf("Search fir nonexistent pointer to page in parent.\n");
	printf("Page: %ld\n", offset);
	exit(EXIT_FAILURE);
}


int64_t coalesce_pages(int table_id, int64_t root, int64_t p, int64_t neighbor, int neighbor_index, int64_t key_prime) {
	int i, j, neighbor_insertion_index, p_end;
	int64_t tmp, temp_key;
	char temp_value[120];

	if (neighbor_index == -1) {
		tmp = p;
		p = neighbor;
		neighbor = tmp;
	}

	page nei_page = read_page_buf(table_id, neighbor);
	neighbor_insertion_index = nei_page.num_key;
	page p_page = read_page_buf(table_id, p);

	if (!p_page.is_leaf) {

		nei_page.key[neighbor_insertion_index] = key_prime;
		nei_page.num_key++;

		p_end = p_page.num_key;
		for (i = neighbor_insertion_index + 1, j = 0; j < p_end; i++, j++) {
			nei_page.key[i] = p_page.key[j];
			nei_page.offset[i] = p_page.offset[j];
			nei_page.num_key++;
			p_page.num_key--;
		}

		nei_page.offset[i] = p_page.offset[j];

		for (i = 0; i < nei_page.num_key + 1; i++) {
			page child_page = read_page_buf(table_id, nei_page.offset[i]);
			child_page.parent = neighbor;
			write_page_buf(table_id, child_page, nei_page.offset[i]);
		}
	} else {
		for (i = neighbor_insertion_index, j = 0; j < p_page.num_key; i++, j++) {
			nei_page.key[i] = p_page.key[j];
			strcpy(nei_page.value[i], p_page.value[j]);
			nei_page.num_key++;
		}
		nei_page.right_page = p_page.right_page;
	}

	write_page_buf(table_id, nei_page, neighbor);

	root = delete_entry(table_id, root, p_page.parent, key_prime, p);
	/*
	fseek(of, find_last_free_page(), SEEK_SET);
	fwrite(&p, 8, 1, of);
	p_page.parent = 0;
	write_page_buf(table_id, p_page, p);
	*/

	return root;
}

int64_t redistribute_pages(int table_id, int64_t root, int64_t p, int64_t neighbor, int neighbor_index, int key_prime_index, int64_t key_prime) {
	int i;
	char value[120];

	page p_page = read_page_buf(table_id, p);
	page nei_page = read_page_buf(table_id, neighbor);
	if (neighbor_index != -1) {
		if (!p_page.is_leaf) {
			p_page.offset[p_page.num_key + 1] = p_page.offset[p_page.num_key];
			for (i = p_page.num_key; i > 0; i--) {
				p_page.offset[i] = p_page.offset[i - 1];
				p_page.key[i] = p_page.key[i - 1];
			}

			p_page.offset[0] = nei_page.offset[nei_page.num_key];
			p_page.key[0] = key_prime;
			write_page_buf(table_id, p_page, p);

			page tmp_p = read_page_buf(table_id, p_page.offset[0]);
			tmp_p.parent = p;
			write_page_buf(table_id, tmp_p, p_page.offset[0]);

			page parent_page = read_page_buf(table_id, p_page.parent);
			parent_page.key[key_prime_index] = nei_page.key[nei_page.num_key - 1];
			write_page_buf(table_id, parent_page, p_page.parent);
		} else {
			for (i = p_page.num_key; i > 0; i--) {
				p_page.key[i] = p_page.key[i - 1];
				strcpy(p_page.value[i], p_page.value[i - 1]);
			}

			p_page.key[0] = nei_page.key[nei_page.num_key - 1];
			strcpy(p_page.value[0], nei_page.value[nei_page.num_key - 1]);
			write_page_buf(table_id, p_page, p);
	
			page parent_page = read_page_buf(table_id, p_page.parent);
			parent_page.key[key_prime_index] = p_page.key[0];
			write_page_buf(table_id, parent_page, p_page.parent);
		}
	} else {
		if (p_page.is_leaf) {
			p_page.key[p_page.num_key] = nei_page.key[0];
			strcpy(p_page.value[p_page.num_key], nei_page.value[0]);
			page parent_page = read_page_buf(table_id, p_page.parent);
			parent_page.key[key_prime_index] = nei_page.key[1];
			write_page_buf(table_id, parent_page, p_page.parent);
		} else {
			p_page.key[p_page.num_key] = key_prime;
			p_page.offset[p_page.num_key + 1] = nei_page.offset[0];
			page tmp = read_page_buf(table_id, p_page.offset[p_page.num_key + 1]);
			tmp.parent = p;
			write_page_buf(table_id, tmp, p_page.offset[p_page.num_key + 1]);
			page parent_page = read_page_buf(table_id, p_page.parent);
			parent_page.key[key_prime_index] = nei_page.key[0];
			write_page_buf(table_id, parent_page, p_page.parent);
		}
		write_page_buf(table_id, p_page, p);
	
		if (nei_page.is_leaf) {
			for (i = 0; i < nei_page.num_key - 1; i++) {
				nei_page.key[i] = nei_page.key[i + 1];
				strcpy(nei_page.value[i], nei_page.value[i + 1]);
			}
		} else {
			for (i = 0; i < nei_page.num_key - 1; i++) {
				nei_page.offset[i] = nei_page.offset[i + 1];
				nei_page.key[i] = nei_page.key[i + 1];
			}
		}

		if (!p_page.is_leaf)
			nei_page.offset[i] = nei_page.offset[i + 1];
		write_page_buf(table_id, nei_page, neighbor);
	}

	p_page.num_key++;
	nei_page.num_key--;
	write_page_buf(table_id, p_page, p);
	write_page_buf(table_id, nei_page, neighbor);
	return root;
}

int64_t adjust_root(int table_id, int64_t root_offset) {
	int64_t new_root_offset = 0, FP;
	page root = read_page_buf(table_id, root_offset);
	if (root.num_key > 0) {
		return root_offset;
	}
	page new_root_page;
	if (!root.is_leaf) {
		new_root_page = read_page_buf(table_id, root.offset[0]);
		new_root_page.parent = 0;
		write_page_buf(table_id, new_root_page, root.offset[0]);
		page head = read_page_buf(table_id, 0);
		head.root = root.offset[0];
		write_page_buf(table_id, head, 0);
	} else {
		new_root_offset = 0;
		page head = read_page_buf(table_id, 0);
		head.root = 0;
		write_page_buf(table_id, head, 0);
	}

	/*
	fseek(of, find_last_free_page(), SEEK_SET);
	fwrite(&root_offset, 8, 1, of);
	root.parent = 0;
	write_page_buf(table_id, root, root_offset);
	*/
	return new_root_offset;
}

int64_t remove_entry_from_node(int table_id, int64_t key_offset, int64_t key, int64_t key_record) {
	int i = 0, leaf_, k = 0, first = 0, mid = 0, last;
	int64_t num_offset, page_key, inner_key, temp_offset;
	char value[120];
	int64_t key_temp;

	page key_page = read_page_buf(table_id, key_offset);
	last = key_page.num_key - 1;

	while (first <= last) {
		mid = (first + last) / 2;
		
		if (key_page.key[mid] == key) {
			i = mid;
			break;
		} else {
			if (key_page.key[mid] > key)
				last = mid - 1;
			else
				first = mid + 1;
		}
	}
	
  	k = i;
	if (key_page.is_leaf) {
		for (++i; i < key_page.num_key; i++) {
			key_page.key[i - 1] = key_page.key[i];
			strcpy(key_page.value[i - 1], key_page.value[i]);
		}
	} else {
		for (++k; k < key_page.num_key; k++) {
			key_page.key[k - 1] = key_page.key[k];
		}

		i = 0;
		while (key_page.offset[i] != key_record) {
			i++;
		}	
	
		for(++i; i < key_page.num_key + 1; i++) {
			key_page.offset[i - 1] = key_page.offset[i];
		}
	}

	key_page.num_key--;
	write_page_buf(table_id, key_page, key_offset);
	return key_offset;
}

int64_t delete_entry(int table_id, int64_t root_offset, int64_t key_offset, int64_t key, int64_t key_record){
	int64_t min_keys, neighbor_offset, key_prime, chk, parent_offset, temp_offset1, temp_offset2;
	int neighbor_index, key_prime_index, capacity;
	key_offset = remove_entry_from_node(table_id, key_offset, key, key_record);

	if (key_offset == root_offset)
		return adjust_root(table_id, root_offset);

	page key_page = read_page_buf(table_id, key_offset);
	min_keys = key_page.is_leaf ? cut(num_LP) : cut(num_IP + 1) - 1;
	if (key_page.num_key >= min_keys) {
		return root_offset;
	}
	
	neighbor_index = get_neighbor_index(table_id, key_offset);
	key_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
	parent_offset = key_page.parent;
	page parent = read_page_buf(table_id, parent_offset);
	key_prime = parent.key[key_prime_index];
	neighbor_offset = neighbor_index == -1 ? parent.offset[1] : parent.offset[neighbor_index];
	capacity = key_page.is_leaf ? num_LP + 1 : num_IP;
	page nei_page = read_page_buf(table_id, neighbor_offset);

	if (nei_page.num_key + key_page.num_key < capacity) {
		return coalesce_pages(table_id, root_offset, key_offset, neighbor_offset, neighbor_index, key_prime);
	} else
		return redistribute_pages(table_id, root_offset, key_offset, neighbor_offset, neighbor_index, key_prime_index, key_prime);
}

int64_t delete(int table_id, int64_t key) {
	int64_t key_leaf = find_leaf(table_id, key), root_offset, key_offset, tmp;
	char *key_record = find(table_id, key);
	int i = 0;
	page key_page = read_page_buf(table_id, key_leaf);

	for (i = 0; i < key_page.num_key; i++) {
		if (key == key_page.key[i])
			break;
	}

	key_offset = key_leaf + 128 * (i + 1);
	
	page head =read_page_buf(table_id, 0);
	root_offset = head.root;
	if (key_record != NULL && key_leaf != 0) {
		delete_entry(table_id, root_offset, key_leaf, key, key_offset);
		return 0;
	}

	return 1;
}