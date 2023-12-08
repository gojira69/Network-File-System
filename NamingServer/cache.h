#ifndef __CACHE_H
#define __CACHE_H

#define MAX_CACHE_SIZE 10
#define MAX_FILE_PATH_LENGTH 256

typedef struct cacheCell
{
    struct record *tableEntry;
} cacheCell;

typedef struct LRUCache
{
    cacheCell cacheArray[MAX_CACHE_SIZE];
    int numFiles;
    int maxFiles;
} LRUCache;

LRUCache *initCache();
bool isCacheEmpty(LRUCache *myCache);
bool isCacheFull(LRUCache *myCache);
void moveCellToStart(LRUCache *myCache, int index);
void addFile(LRUCache *myCache, struct record *newTableEntry);
struct record *searchFileInCache(LRUCache *myCache, char *filePath);
int removeFileFromCache(LRUCache *myCache, char *filePath);

#endif