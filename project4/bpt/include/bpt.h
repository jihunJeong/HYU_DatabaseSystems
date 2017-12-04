#ifndef __BPT_H__
#define __BPT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#ifdef WINDOWS

#define false 0
#define true 1

#endif

extern bool verbose_output;
extern int MAX_FRAME;
extern int clk_hand;

typedef struct record {
	char value[120];
	int64_t key;
} record;

typedef struct page {
	 int64_t free;
	 int64_t root;
	 int64_t num_page;
	 int64_t parent;
	 int is_leaf;
	 int num_key;
	 int64_t right_page;
	 char value[31][120];
	 int64_t key[248];
	 int64_t offset[249];
} page;

typedef struct buffer {
	page page;
	int table_id;
	int64_t page_offset;
	int is_dirty;
	int pin_count;
	int refbit;
} buffer;

extern buffer **buffer_pool;
extern FILE *array_f[11];

int cut(int length);
page init_page();
void copy_page();
void decrease_pin(int table_id);
int64_t find_last_free_page(int table_id);
page clock_request_page(int table_id, page n_page, int64_t offset);

void usage_2(void);
int init_db(int num_buf);
void initialize_db(int table_id);
int open_table(char *pathname);
int close_table(int table_id);
int shutdown_db();

int start_new_db(int table_id, int64_t key, record *pointer);
record *make_record(int64_t key, char *value);
int64_t make_page(int table_id);
int64_t make_leaf(int table_id);
void write_page_db(int table_id, buffer *buf);
void write_page_buf(int table_id, page page, int64_t offset);
page read_page_db(int table_id, int64_t offset);
page read_page_buf(int table_id, int64_t offset);

int64_t get_left_index(int table_id, int64_t parent_offset, int64_t left_offset);
int64_t insert_into_parent(int table_id, int64_t parent_offset, int64_t leaf_offset, int64_t key, int64_t
		new_leaf_offset);
int64_t insert_into_new_root(int table_id, int64_t left_offset, int64_t key, int64_t
		right_offset);
int64_t insert_into_leaf(int table_id, int64_t offset, int64_t key, record *pointer);
int64_t insert_into_leaf_after_splitting(int table_id, int64_t leaf_offset, int64_t key, record *pointer);
int64_t insert_into_node(int table_id, int64_t parent, int64_t left_index, int64_t key,
		int64_t right);
int64_t insert_into_node_after_splitting(int table_id, int64_t old_offset, int64_t left_index,
		int64_t key, int64_t right);
int64_t insert(int table_id, int64_t key, char *value);

char *find(int table_id, int64_t key);
int64_t find_leaf(int table_id, int64_t key);

int get_neighbor_index(int table_id, int64_t offset);
int64_t adjust_root(int table_id, int64_t root_offset);
int64_t coalesce_pages(int table_id, int64_t root_offset, int64_t key_offset, int64_t neighbor_offset, int neighbor_index, int64_t key_prime);
int64_t redistribute_pages(int table_id, int64_t root_offset, int64_t key_offset, int64_t neighbor_offset, int neighbor_index, int key_prime_index, int64_t key_prime);
int64_t remove_entry_from_node(int table_id, int64_t key_offset, int64_t key, int64_t key_record);
int64_t delete_entry(int table_id, int64_t root_offset, int64_t key_offset, int64_t key, int64_t key_record);
int64_t delete(int table_id, int64_t key);

int64_t read_first_leaf_page(int table_id);
void write_txt_page(int table_id);
int join_table(int table_id_1, int table_id_2, char *pathname);

#endif 