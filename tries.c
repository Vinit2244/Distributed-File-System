#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A structure to represent a node in the trie
struct trie_node
{
    char *key;
    int end;
    struct trie_node *children[256];    // Total 256 characters possible that can come in any path name
};

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

int main()
{
    // Create a trie
    struct trie_node *root = create_trie_node();

    int result = insert_path(root, "./abcd/efgh/temp.txt");
    if (result == 1)
    {
        printf("Path inserted successfully.\n");
    }
    else
    {
        printf("Error in inserting path.\n");
    }

    int result1 = insert_path(root, "./abcd/efgh/temp.txt");
    if (result1 == 1)
    {
        printf("Path inserted successfully.\n");
    }
    else
    {
        printf("Error in inserting path.\n");
    }

    int present = search_path(root, "./abcd/efgh/temp.txt");
    if (present == 1)
    {
        printf("Path is in the trie\n");
    }
    else
    {
        printf("Path is not in the trie\n");
    }

    int result2 = delete_path(root, "./abcd/efgh/temp.txt");
    if (result2 == 1)
    {
        printf("Deleted successfully.\n");
    }
    else
    {
        printf("Error in deleting path, path DNE.\n");
    }
    
    int present2 = search_path(root, "./abcd/efgh/temp.txt");
    if (present2 == 1)
    {
        printf("Path is in the trie\n");
    }
    else
    {
        printf("Path is not in the trie\n");
    }

    print_paths(root);

    return 0;
}
