
#Boson Database Specification

##1. Overview

Boson is embeddable JSON document database engine.

Core features:
- Standard Key/Value persistent document store (JSON).
- Fast document search by ID in B+ Tree Index.
- Support cursors for linear records traversal.
- Single file database, no temporary files.
- Simple, Clean and easy to use API.
- Self-contained & zero configuration.

Considerations:
- Median JSON document size is 1525 bytes.
- Almost all real world apps show some form of locality of reference.
- Most apps database read/write operations ratio is 70% / 30%.
- 10-15% of database size cache gives more than 95% cache hits.
- Storage device sector/block aligned read/writes are faster.
- Sequential read/write operations are faster.



## 2. Boson Database Architecture


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




## 3. Internal algorithms and performance strategies

### 3.1. Database file caching (CachedFileIO Class)

#### 3.1.1. Motivation

CachedFileIO is designed to improve pe rformance of file I/O operations.
Almost all real world applications show some form of locality of reference, 
and 70%/30% average read/write ratio. Memory LRU caching strategy could 
give performance benefits.



#### 3.1.2. Read operations (LRU)

 
File accessed through aligned blocks of fixed size (pages) that loaded 
into the cache that implements LRU strategy (Least Recently Used). The 
element that hasn't been used for the longest time will be evicted from 
the cache.

On every read operation CachedFileIO class looks up for page in the cache
using hashtable O(1). If there is a cache hit copies requested data to the 
user buffer, otherwise load page to the cache from file, and copies to the 
user's buffer. All recently loaded cache pages marked as "clean".


#### 3.1.3. Write operations (FBW)

For write operations, CachedFileIO uses Fetch-On-Write policy.
If there is a cache hit, write operation changes cache page
and marks it as "dirty". If there is a cache miss, then file 
page loaded to the cache and after changes are applied.

On file close or when there is no free pages available in the
cache, CachedFileIO frees most aged pages. When the page is freed,
if it has "dirty" mark, page persisted on the storage device.


#### 3.1.4. IO performance and cache page O(1) lookup 

Performance is always trade-off, so this class designed to speed up 
frequent IO for JSON documents with average JSON size is 1525 bytes.
Sequential read and writes supposed to be much 3-5 times faster for
HDD and SSD. Cache page lookup uses hashmap with O(1) time complexity.


