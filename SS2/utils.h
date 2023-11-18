#ifndef __UTILS_H__
#define __UTILS_H__

typedef struct linked_list_head_struct* linked_list_head;
typedef struct linked_list_node_struct* linked_list_node;

typedef struct linked_list_head_struct {
    int number_of_nodes;
    struct linked_list_node_struct* first;
    struct linked_list_node_struct* last;
} linked_list_head_struct;

typedef struct linked_list_node_struct {
    char* path;
    struct linked_list_node_struct* next;
} linked_list_node_struct;

linked_list_head create_linked_list_head();
linked_list_node create_linked_list_node(char* path);
void insert_in_linked_list(linked_list_head linked_list, char* path);
void free_linked_list(linked_list_head linked_list);

// General utility functions
// void accessible_paths_init(void);
void send_update_paths_request(int request_type, char* paths_string);
void register_ss(void);
void free_tokens(char** tokens);
char** tokenize(const char* str, const char ch);
void send_ack(const int status_code, const int sock_fd);
void seek(char* path_to_base_dir, linked_list_head paths);
char* remove_extension(char* file_name);
void update_path(char* path, char* next_dir);
char* replace_storage_by_backup(char* path);
void create_folder(char* path);
char* create_abs_path(char* relative_path);
void send_msg_to_nfs(char* msg, int req_type);

#endif