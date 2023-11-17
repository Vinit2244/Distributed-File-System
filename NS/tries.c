#include "headers.h"

// A structure to represent a node in the trie


// A function to create a new trie node
struct trie_node *create_trie_node()
{
    struct trie_node *node = (struct trie_node *) malloc(sizeof(struct trie_node));
    node->key = NULL;
    node->end = 0;
    for (int i = 0; i < 256; i++)
    {
        node->children[i] = NULL;
    }
    return node;
}



// A function to insert a string into the trie
// Returns 1 if successfull else 0
int insert_path(struct trie_node *root, char *key)
{
    struct trie_node *current = root;
    for (int i = 0; key[i] != '\0'; i++)
    {
        int index = (int)key[i];

        if (current->children[index] == NULL)
        {
            current->children[index] = create_trie_node();
        }
        current = current->children[index];
    }
    current->key = key;
    current->end = 1;
    return 1;
}

// A function to search for a string in the trie
// Returns 1 if path is found else 0
int search_path(struct trie_node *root, char *key)
{
    struct trie_node *current = root;
    for (int i = 0; key[i] != '\0'; i++)
    {
        int index = (int)key[i];

        if (current->children[index] == NULL)
        {
            return 0;
        }
        current = current->children[index];
    }

    if (current->end == 1)
    {
        return 1;
    }
    return 0;
}

// A function to delete a string from the trie
// Returns 1 if successfull else 0
int delete_path(struct trie_node *root, char *key)
{
    struct trie_node *current = root;
    for (int i = 0; key[i] != '\0'; i++)
    {
        int index = (int) key[i];

        if (current->children[index] == NULL)
        {
            return 0;
        }
        current = current->children[index];
    }
    current->key = NULL;
    current->end = 0;
    return 1;
}

linked_list_head create_linked_list_head() {
    linked_list_head linked_list = (linked_list_head) malloc(sizeof(linked_list_head_struct));
    linked_list->number_of_nodes = 0;
    linked_list->first = NULL;
    linked_list->last = NULL;
    return linked_list;
}

linked_list_node create_linked_list_node(char* path) {
    linked_list_node N = (linked_list_node) malloc(sizeof(linked_list_node_struct));
    N->next = NULL;
    N->path = (char*) calloc(MAX_PATH_LEN, sizeof(char));
    strcpy(N->path, path);
    return N;
}

void insert_in_linked_list(linked_list_head linked_list, char* path) {
    linked_list_node N = create_linked_list_node(path);
    if (linked_list->number_of_nodes == 0) {
        linked_list->first = N;
        linked_list->last = N;
        linked_list->number_of_nodes++;
    } else if (linked_list->number_of_nodes == 1) {
        linked_list->last = N;
        linked_list->first->next = N;
        linked_list->number_of_nodes++;
    } else {
        linked_list->last->next = N;
        linked_list->last = N;
        linked_list->number_of_nodes++;
    }
}

void free_linked_list(linked_list_head linked_list) {
    linked_list_node trav = linked_list->first;
    while (trav != NULL) {
        free(trav->path);
        linked_list_node temp = trav->next;
        free(trav);
        trav = temp;
    }
    free(linked_list);
}

// Prints all the paths present in the trie
void print_paths(struct trie_node *root)
{
    if (root == NULL)
    {
        return;
    }

    if (root->end == 1)
    {
        printf("%s\n", root->key);
    }

    for (int i = 0; i < 256; i++)
    {
        if (root->children[i] != NULL)
        {
            print_paths(root->children[i]);
        }
    }
}

void add_paths(linked_list_head ll, struct trie_node *root)
{
    if (root == NULL)
    {
        return;
    }

    if (root->end == 1)
    {
        insert_in_linked_list(ll, root->key);
    }

    for (int i = 0; i < 256; i++)
    {
        if (root->children[i] != NULL)
        {
            add_paths(ll, root->children[i]);
        }
    }
}


linked_list_head return_paths(struct trie_node *root)
{
    linked_list_head ll = create_linked_list_head();
    add_paths(ll,root);
    return ll;
}



// int main()
// {
//     // Create a trie
//     struct trie_node *root = create_trie_node();

//     int result = insert_path(root, "./abcd/efgh/temp.txt");
//     if (result == 1)
//     {
//         printf("Path inserted successfully.\n");
//     }
//     else
//     {
//         printf("Error in inserting path.\n");
//     }

//     int result1 = insert_path(root, "./abcd/efgh/temp.txt");
//     if (result1 == 1)
//     {
//         printf("Path inserted successfully.\n");
//     }
//     else
//     {
//         printf("Error in inserting path.\n");
//     }

//     int present = search_path(root, "./abcd/efgh/temp.txt");
//     if (present == 1)
//     {
//         printf("Path is in the trie\n");
//     }
//     else
//     {
//         printf("Path is not in the trie\n");
//     }

//     int result2 = delete_path(root, "./abcd/efgh/temp.txt");
//     if (result2 == 1)
//     {
//         printf("Deleted successfully.\n");
//     }
//     else
//     {
//         printf("Error in deleting path, path DNE.\n");
//     }
    
//     int present2 = search_path(root, "./abcd/efgh/temp.txt");
//     if (present2 == 1)
//     {
//         printf("Path is in the trie\n");
//     }
//     else
//     {
//         printf("Path is not in the trie\n");
//     }

//     print_paths(root);

//     return 0;
// }
