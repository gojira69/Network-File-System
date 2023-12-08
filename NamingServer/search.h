#ifndef __SEARCH_H
#define __SEARCH_H

#define NUM_CHILDREN 256

typedef struct TrieNode
{
    struct record *tableEntry;
    struct TrieNode *children[NUM_CHILDREN]; // trie node array
} TrieNode;

TrieNode *initTrieNode();
void insertRecordToTrie(TrieNode *root, struct record *newTableEntry);
struct record *search(TrieNode *root, char *file_path);
int deleteTrieNode(TrieNode *root, char *file_path);

#endif