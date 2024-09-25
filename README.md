
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
    |                    Boson API                      |      -  API Layer (Key/Value)
     ---------------------------------------------------     
                              |
     ---------------------------------------------------
    |                   B+ Tree Index                   |      -  Index Layer
     ---------------------------------------------------
                              |     
     ---------------------------------------------------
    |                Records Storage I/O                |      -  Collections & Records Storage Layer
     ---------------------------------------------------
                              |                           
     ---------------------------------------------------
    |              Cached File I/O (Pages)              |      -  Memory Cache Layer
     ---------------------------------------------------
                              |
     ---------------------------------------------------
    |                   Database File                   |      -  OS File System Layer
     --------------------------------------------------- 




## 3. Internal algorithms and performance strategies

### 3.1. Cached File I/O (Pages)

#### 3.1.1. Motivation

**CachedFileIO is designed to improve performance of file I/O 
operations specifically for JSON document store case**. We assume 
that almost all real world apps show some form of locality of 
reference. 10-15% of database size cache gives more than 95% 
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
LRU cache based on double linked list with O(1) insert/rease complexity.

On every read operation CachedFileIO looks up for page in the cache
**using hashtable with O(1) time complexity**. If there is a cache hit, 
CacheFileIo copies requested data to the user buffer, otherwise loads
page to the cache from file, and copies to the user's buffer. All 
recently loaded cache pages marked as "clean".


#### 3.1.3. Write operations (FBW)

For write operations, CachedFileIO uses Fetch-Before-Write policy.
If there is a cache hit, write operation rewrirtes cache page
and marks it as "dirty". If there is a write miss, then file 
page loaded to the cache and after changes are applied.

On file close or when there is no free pages available in the
cache, CachedFileIO frees most aged pages. When the page is freed,
if it has "dirty" mark, page persisted on the storage device.


### 3.2. Records Storage I/O

Storage file consists of a database header and two **double-linked lists 
of records** - one for data records and the second for deleted records.
When a new record is created algorithm searches available deleted records
of the appropriate size to efficiently utilize used space. If there is no
suitable deleted record of appropriate size, a new data record is created at
the end of the file.


#### 3.2.1. Records



#### 3.2.2. Motivation


### 3.3. B+ Tree Index

