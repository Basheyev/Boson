
# Boson Database

## 1. Overview

**Boson is embeddable JSON document database engine written in C++.**

**Core features**:
- Standard Key/Value persistent document store (JSON).
- Fast document search by ID in B+ Tree Index.
- Support cursors for linear records traversal.
- Single file database, no temporary files.
- Simple, Clean and easy to use API.
- Self-contained & zero configuration.


## 2. Boson Database Architecture

Boson has four layers. Each of them providing required guaranties 
and level of abstraction. From low-level like cache to high level
abstraction like API working with document store entities.

     ---------------------------------------------------
    |                     Boson API                     |      -  API Layer (Key/Value)
     ---------------------------------------------------
                |                           |
     -----------------------     -----------------------
    |    Database Storage   |   |    Index (B+ Tree)    |      -  Storage and Index Layer
     -----------------------     -----------------------
                |                           |
     ---------------------------------------------------
    |              Cached File IO (LRU/FoW)             |      -  Memory Cache Layer
     ---------------------------------------------------
                              |
     ---------------------------------------------------
    |                   Database File                   |      -  File-System Layer
     --------------------------------------------------- 




## 3. Internal algorithms and performance strategies

### 3.1. Cached File IO

#### 3.1.1. Motivation

**CachedFileIO is designed to improve performance of file I/O 
operations specificly for JSON document store case**. We assume 
that almost all real world apps show some form of locality of 
reference, so 10-15% of database size cache gives more than 95% 
cache hits. Most JSON documents size are less than 1000 bytes, 
where median size is 1525 bytes. Most apps database read/write 
operations ratio is 70% / 30%. Read/write operations are faster 
when aligned to storage device sector/block size and sequential. 
Therefore LRU caching strategy will give performance benefits.


#### 3.1.2. Read operations (LRU)
 
File accessed through aligned blocks of fixed size (pages) that 
loaded into the cache that implements LRU strategy (Least Recently 
Used). If CachedFileIO has no free cache pages then cache page that 
hasn't been used for the longest time will be evicted from the cache.

On every read operation CachedFileIO looks up for page in the cache
**using hashtable with O(1) time complexity**. If there is a cache hit, 
CacheFileIo copies requested data to the user buffer, otherwise loads
page to the cache from file, and copies to the user's buffer. All 
recently loaded cache pages marked as "clean".


#### 3.1.3. Write operations (FBW)

For write operations, CachedFileIO uses Fetch-Before-Write policy.
If there is a cache hit, write operation changes cache page
and marks it as "dirty". If there is a write miss, then file 
page loaded to the cache and after changes are applied.

On file close or when there is no free pages available in the
cache, CachedFileIO frees most aged pages. When the page is freed,
if it has "dirty" mark, page persisted on the storage device.




