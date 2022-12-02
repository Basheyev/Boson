
Boson Database

Core features:
- Emdeddable NoSQL document database engine.
- Standard Key/Value persistent document store (JSON).
- Fast document search by ID in b+ tree index.
- Support cursors for linear records traversal.
- Self-contained, without dependencies
- Single file database.


Considerations:
- IO performance grow when page size is aligned to a storage device read 
  block size (4Kb page/sector for HDD, 4Kb pages and 256-512Kb block for SSD)
- Average JSON size value 1525 bytes (varying between 512-2048 bytes)
-


# 1. BOSON DATABASE ARCHITECTURE





     ---------------------------------------------------
    |                 Boson Key-Value API               | 
     ---------------------------------------------------
                |                           |
     -----------------------     -----------------------
    |    Database Storage   |   |    Index (B+ Tree)    |
     -----------------------     -----------------------
                |                           |
     ---------------------------------------------------
    |              Cached File IO (LRU/FBW)             | 
     ---------------------------------------------------
                              |
     ---------------------------------------------------
    |                   Database File                   |
     --------------------------------------------------- 




===============================================================================

# 2. INTERNAL ALGORITHMS AND PERFORMANCE STRATEGIES

===============================================================================

## 2.1. Database file caching (CachedFileIO Class)

### 2.1.1. Motivation

CachedFileIO is designed to improve pe rformance of file I/O operations.
Almost all real world applications show some form of locality of reference, 
and 70%/30% average read/write ratio. 



Memory LRU caching strategy could give performance benefits.



### 2.1.2. Read operations (LRU)

 
File accessed through aligned blocks of fixed size (pages) that loaded 
into the cache that implements LRU strategy (Least Recently Used). The 
element that hasn't been used for the longest time will be evicted from 
the cache.

On every read operation CachedFileIO class looks up for page in the cache
using hashtable O(1). If there is a cache hit copies requested data to the 
user buffer, otherwise load page to the cache from file, and copies to the 
user's buffer. All recently loaded cache pages marked as "clean".


2.1.3. Write operations (FBW)

For write operations, CachedFileIO uses Fetch-On-Write policy.
If there is a cache hit, write operation changes cache page
and marks it as "dirty". If there is a cache miss, then file 
page loaded to the cache and after changes are applied.

On file close or when there is no free pages available in the
cache, CachedFileIO frees most aged pages. When the page is freed,
if it has "dirty" mark, page persisted on the storage device.


2.1.4. IO performance and cache page O(1) lookup 

Performance is always trade-off, so this class designed to speed up 
frequent IO for JSON documents with average JSON size is 1525 bytes.
Sequential read and writes supposed to be much 3-5 times faster for
HDD and SSD. Cache page lookup uses hashmap with O(1) time complexity.


