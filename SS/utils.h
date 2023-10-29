#ifndef __UTILS_H__
#define __UTILS_H__

// General utility functions
void accessible_paths_init(void);
void start_nfs_port(void);
void start_client_port(void);
char** tokenize(const char* str, const char ch);
void free_tokens(char** tokens);

#endif