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
void insert_path(struct trie_node *root, char *key)
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
}

// A function to search for a string in the trie
struct trie_node *search_path(struct trie_node *root, char *key)
{
    struct trie_node *current = root;
    for (int i = 0; key[i] != '\0'; i++)
    {
        int index = (int)key[i];

        if (current->children[index] == NULL)
        {
            return NULL;
        }
        current = current->children[index];
    }

    if (current->end == 1)
    {
        return current;
    }
    else
    {
        return NULL;
    }
}

// A function to delete a string from the trie
void delete_path(struct trie_node *root, char *key)
{
    struct trie_node *current = root;
    for (int i = 0; key[i] != '\0'; i++)
    {
        int index = (int) key[i];

        if (current->children[index] == NULL)
        {
            return;
        }
        current = current->children[index];
    }
    current->key = NULL;
    current->end = 0;
}

// A function to print the trie
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

    // Insert some strings into the trie
    insert_path(root, "./abcd/efgh/temp.txt");
    insert_path(root, "./abcd/efgh/temp.txt");

    // Search for a string in the trie
    struct trie_node *node = search_path(root, "./abcd/efgh/temp.txt");
    if (node != NULL)
    {
        printf("Path is in the trie\n");
    }
    else
    {
        printf("Path is not in the trie\n");
    }

    delete_path(root, "./abcd/efgh/temp.txt");

    struct trie_node *node2 = search_path(root, "./abcd/efgh/temp.txt");
    if (node2 != NULL)
    {
        printf("Path is in the trie\n");
    }
    else
    {
        printf("Path is not in the trie\n");
    }

    struct trie_node *node3 = search_path(root, "./storage/abcd/efgh/temp.txt");
    if (node3 != NULL)
    {
        printf("Path is in the trie\n");
    }
    else
    {
        printf("Path is not in the trie\n");
    }

    delete_path(root, "./storage/abcd/efgh/temp.txt");

    struct trie_node *node4 = search_path(root, "./storage/abcd/efgh/temp.txt");
    if (node4 != NULL)
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
