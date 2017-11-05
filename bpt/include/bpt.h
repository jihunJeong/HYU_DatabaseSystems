#ifndef __BPT_H__
#define __BPT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif

extern bool verbose_output;

typedef struct record {
	char value[120];
	int64_t key;
} record;

extern FILE *of;
extern int64_t cnt_p;

int cut(int length);
void increase_num_page();
int64_t get_parent_offset(int64_t offset);
void set_parent_offset(int64_t offset, int64_t parent_offset);
int get_num_key(int64_t offset);
void set_num_key(int64_t offset, int number);
int chk_is_leaf(int64_t offset);
int64_t find_last_free_page();

void usage_2(void);
void initialize_db();
int open_db(char *pathname);

int start_new_db(int64_t key, record *pointer);
record *make_record(int64_t key, char *value);
int64_t make_page();
int64_t make_leaf();

int64_t get_left_index(int64_t parent_offset, int64_t left_offset);
int64_t insert_into_parent(int64_t parent_offset, int64_t leaf_offset, int64_t key, int64_t
		new_leaf_offset);
int64_t insert_into_new_root(int64_t left_offset, int64_t key, int64_t
		right_offset);
int64_t insert_into_leaf(int64_t leaf_offset, int64_t key, record *pointer);
int64_t insert_into_leaf_after_splitting(int64_t leaf_offset, int64_t key, record *pointer);
int64_t insert_into_node(int64_t parent, int64_t left_index, int64_t key,
		int64_t right);
int64_t insert_into_node_after_splitting(int64_t old_offset, int64_t left_index,
		int64_t key, int64_t right);
int64_t insert(int64_t key, char *value);

record *find(int64_t key);
int64_t find_leaf(int64_t key);

int get_neighbor_index(int64_t offset);
int64_t adjust_root(int64_t root_offset);
int64_t coalesce_pages(int64_t root_offset, int64_t key_offset, int64_t neighbor_offset, int neighbor_index, int64_t key_prime);
int64_t redistribute_pages(int64_t root_offset, int64_t key_offset, int64_t neighbor_offset, int neighbor_index, int key_prime_index, int64_t key_prime);
int64_t remove_entry_from_node(int64_t key_offset, int64_t key, int64_t key_record);
int64_t delete_entry(int64_t root_offset, int64_t key_offset, int64_t key, int64_t key_record);
int64_t delete(int64_t key);
#endif 