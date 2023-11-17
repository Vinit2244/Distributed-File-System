#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define RED_COLOR    "\033[0;31m"
#define GREEN_COLOR  "\033[0;32m"
#define BLUE_COLOR   "\033[0;34m"
#define YELLOW_COLOR "\033[0;33m"
#define CYAN_COLOR   "\033[0;36m"
#define ORANGE_COLOR "\e[38;2;255;85;0m"
#define RESET_COLOR  "\033[0m"

#define RED(str)    RED_COLOR    str RESET_COLOR
#define GREEN(str)  GREEN_COLOR  str RESET_COLOR
#define BLUE(str)   BLUE_COLOR   str RESET_COLOR
#define YELLOW(str) YELLOW_COLOR str RESET_COLOR
#define CYAN(str)   CYAN_COLOR   str RESET_COLOR
#define ORANGE(str) ORANGE_COLOR str RESET_COLOR

#define MAX_DATA_LENGTH     10000       // Maximum number of characters data being sent can have (query/file data)

#define CACHE_SIZE 2
int curr_cache_write_index;

typedef struct st_cache {
    int req_type;
    char req_data[MAX_DATA_LENGTH];
    int ss_id;
    int ss_ip;
    int ss_port;
} st_cache;

typedef st_cache* cache_array;

cache_array cache = NULL;

// Initialise the cache
void init_cache()
{
    cache = (cache_array) malloc(sizeof(st_cache) * CACHE_SIZE);
    if (cache == NULL)
    {
        fprintf(stderr, RED("malloc : Failed to initialise cache : %s\n"), strerrror(errno));
        exit(EXIT_FAILURE);
    }
    curr_cache_write_index = 0;
    return;
}

// Deletes the cache node with the specified index and leftshifts all the remaining node and hence making 
void delete_cache_index(const int idx)
{
    for (int i = idx; i < curr_cache_write_index; i++)
    {
        if (i == curr_cache_write_index - 1)
        {
            // do nothing
        }
        else
        {
            cache[i] = cache[i + 1];
        }
    }
    curr_cache_write_index--;
    return;
}

// Inserts the new information at the last index of cache (cache is like a queue in which the least recently used info is at the first spot)
void insert_in_cache(int req_type, int req_data, int ss_id, int ss_ip, int ss_port)
{
    cache[curr_cache_write_index].req_type = req_type;
    memset(cache[curr_cache_write_index].req_data, 0, MAX_DATA_LENGTH);
    strcpy(cache[curr_cache_write_index].req_data, req_data);
    cache[curr_cache_write_index].ss_id = ss_id;
    cache[curr_cache_write_index].ss_ip = ss_ip;
    cache[curr_cache_write_index].ss_port = ss_port;

    curr_cache_write_index++;
    return;
}

// Search for the request in cache and returns the cache struct if found otherwise returns NULL (If NULL is returned which means cache miss then remember to insert into the cache this new info otherwise if there is cache hit then no need to insert again) and also remember to free the memory in case of cache hit
st_cache* search_in_cache(int req_type, int req_data)
{
    for (int i = 0; i < curr_cache_write_index; i++)
    {
        if (cache[i].req_type == req_type && cache[i].req_data == req_data)
        {
            st_cache* to_return = (st_cache*) malloc(sizeof(st_cache));

            if (to_return == NULL)
            {
                fprintf(stderr, RED("malloc : %s\n"), strerror(errno));
                exit(EXIT_FAILURE);
            }

            to_return->req_type = cache[i].req_type;
            strcpy(to_return->req_data, cache[i].req_data);
            to_return->ss_id = cache[i].ss_id;
            to_return->ss_ip = cache[i].ss_ip;
            to_return->ss_port = cache[i].ss_port;

            delete_cache_index(i);
            insert_in_cache(to_return->req_type, to_return->req_data, to_return->ss_id, to_return->ss_ip, to_return->ss_port);
            return to_return;
        }
    }

    // If the cache is full then delete the least recently used info node
    if (curr_cache_write_index == CACHE_SIZE)
    {
        // Delete the first node as it is the least recently unaccessed
        delete_cache_index(0);
    }

    return NULL;
}

// Prints the contents of the cache
void print_cache()
{
    for (int i = 0; i < curr_cache_write_index; i++)
    {
        printf("%d %d %d %d %s\n", cache[i].req_type, cache[i].ss_id, cache[i].ss_ip, cache[i].ss_port, cache[i].req_data);
    }
    return;
}

int main()
{

    while(1)
    {
        printf("Enter details to add in cache : ");
        int req_type;
        char req_data[MAX_DATA_LENGTH] = {0};
        int ss_id;
        int ss_ip;
        int ss_port;
        scanf("%d|%d|%d|%d|%s", &req_type, &ss_id, &ss_ip, &ss_port, req_data);

        print_cache();
    }


}