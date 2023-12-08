#include "headers.h"

TrieNode *initTrieNode()
{
    TrieNode *newNode = (TrieNode *)malloc(sizeof(TrieNode));
    if (newNode)
    {
        for (int i = 0; i < NUM_CHILDREN; i++)
        {
            newNode->children[i] = NULL;
        }
        newNode->tableEntry = NULL;
    }
    return newNode;
}

void insertRecordToTrie(TrieNode *root, struct record *newTableEntry)
{
    TrieNode *temp = root;
    for (int i = 0; newTableEntry->path[i] != '\0'; i++)
    {
        int index = (int)newTableEntry->path[i];
        if (temp->children[index] == NULL)
        {
            temp->children[index] = initTrieNode(); // for each character in the file path, create another trienode....You know how tries work :)
        }
        temp = temp->children[index];
    }
    temp->tableEntry = newTableEntry;
}

struct record *search(TrieNode *root, char *file_path)
{
    TrieNode *temp = root;
    for (int i = 0; file_path[i] != '\0'; i++)
    {
        int index = (int)file_path[i];
        if (temp->children[index] == NULL)
        {
            return NULL;
        }
        temp = temp->children[index];
    }
    if (temp->tableEntry)
    {
        return temp->tableEntry;
    }
    else
    {
        return NULL;
    }
}

int deleteTrieNode(TrieNode *root, char *file_path)
{
    TrieNode *temp = root;
    TrieNode *prev = NULL;
    int i;
    for (i = 0; file_path[i] != '\0'; i++)
    {
        int index = (int)file_path[i];
        if (temp->children[index] == NULL)
        {
            return 0;
        }
        prev = temp;
        temp = temp->children[index];
    }

    free(prev->children[(int)file_path[i - 1]]);
    prev->children[(int)file_path[i - 1]] = NULL;
    return 1;
}