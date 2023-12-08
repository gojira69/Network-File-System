#include "headers.h"

bool isCacheEmpty(LRUCache *myCache)
{
    return (myCache->numFiles == 0);
}

bool isCacheFull(LRUCache *myCache)
{
    return (myCache->numFiles == myCache->maxFiles);
}

LRUCache *initCache()
{
    LRUCache *myCache = (LRUCache *)malloc(sizeof(LRUCache));
    myCache->numFiles = 0;
    myCache->maxFiles = MAX_CACHE_SIZE;
    return myCache;
}

void moveCellToStart(LRUCache *myCache, int index)
{
    cacheCell temp = myCache->cacheArray[index];

    for (int i = index; i > 0; --i)
        myCache->cacheArray[i] = myCache->cacheArray[i - 1];

    myCache->cacheArray[0] = temp;
}

void addFile(LRUCache *myCache, struct record *newTableEntry)
{
    if (isCacheFull(myCache))
    {
        myCache->numFiles--;
    }

    for (int i = myCache->numFiles; i > 0; --i)
        myCache->cacheArray[i] = myCache->cacheArray[i - 1];

    myCache->cacheArray[0].tableEntry = newTableEntry;
    myCache->numFiles++;
}

struct record *searchFileInCache(LRUCache *myCache, char *filePath)
{
    for (int i = 0; i < myCache->numFiles; ++i)
    {
        if (strcmp(myCache->cacheArray[i].tableEntry->path, filePath) == 0)
        {
            moveCellToStart(myCache, i);
            return myCache->cacheArray[0].tableEntry;
        }
    }
    return NULL;
}

int removeFileFromCache(LRUCache *myCache, char *filePath)
{
    for (int i = 0; i < myCache->numFiles; ++i)
    {
        if (strcmp(myCache->cacheArray[i].tableEntry->path, filePath) == 0)
        {
            for (int j = i; j < myCache->numFiles - 1; ++j)
            {
                myCache->cacheArray[j] = myCache->cacheArray[j + 1];
            }

            myCache->numFiles--;
            return 0;
        }
    }
    return 1;
}
