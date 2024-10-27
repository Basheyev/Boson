
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
    |                    Boson API                      |      -  API Layer
     ---------------------------------------------------     
                              |
     ---------------------------------------------------
    |                   B+ Tree Index                   |      -  Index Layer (Key/Value)
     ---------------------------------------------------
                              |     
     ---------------------------------------------------
    |                Records Storage I/O                |      -  Records Storage Layer
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

#### 3.2.1. Motivation

**RecordFileIO is designed for seamless storage of binary records of
arbitrary size (maximum record size limited to 4GB), accessing records as
a linked list and reusing space from deleted records**. Features:
- Create/read/update/delete records of arbitrary size
- Navigate records: first, last, next, previous, absolute position
- Reuse space from deleted records (linked list of deleted records)
- Data consistency check (Adler-32 checksum algoritm)

#### 3.2.2. Records

The storage file consists of a database header and two doubly-linked lists
of records - one for data records and the second for deleted records.
When a new record is created, the algorithm searches for available deleted records
of the appropriate size to efficiently utilize previously used space. If there is no
suitable deleted record of the appropriate size, a new data record is allocated
at the end of the file. Deleted records added to the deleted records list to reuse.
RecordFileIO uses CachedFileIO to cache frequently accessed data and improve I/O performance.




### 3.3. B+ Tree Index

#### 3.3.1. Motivation

**The B+ Tree algorithm is optimized for scenarios involving a large volume of data that 
does not fit into RAM and is persisted in storage. The algorithm maintains a tree index 
to keep key/value pairs balanced and sorted, ensuring that operations like insert, delete, 
and search have O(log n) complexity, even with very large datasets**. The linked list structure 
of the leaf nodes makes range queries and sequential scans extremely fast, which is essential
for database indexing and applications requiring sorted data access.

#### 3.3.2. Structure and operations of a B+ Tree

     Root node                            [  80  ]
                                       /           \
     Inner node             [ 20 | 40 ]             [ 90 | 110 ]
                          /      |      \             /  |   \
     Leaf node     [10|15] -> [25|30] -> [45|50] -> ... ... ...
                    |   |      |   |      |   |
     Value         [D1,D2]    [D3,D4]    [D5,D6]    ... ... ...
      


B+ Tree is composed of Internal nodes and Leaf nodes. Internal nodes only store keys to guide the search, 
while the actual data is kept in the leaf nodes:
- **Internal Nodes**: These nodes contain keys and pointers to child nodes but do not store 
  actual data. Each internal node can hold between ⌈M/2⌉ to M-1 keys and M children. Every key in internal
  node separates the child nodes: the left child node contains keys less than parent node's key 
  and the right child node contains keys greater than or equal to the parent node's key.
- **Leaf Nodes**: These nodes store pointers to data and are linked together in a doubly linked 
  list for efficient range queries. Each leaf node can also hold between ⌈M/2⌉to M-1 keys and M-1 values. 
- **A B+ Tree is balanced**, meaning all leaf nodes are at the same depth to guarantee same deterministic
  search time.

To keep B+ tree balanced and sorted these operations performed:

1. **Search Operation**: To search for a specific key in a B+ Tree, start at the root node. Compare 
   the target key with the keys in the current node to determine the appropriate child node. 
   If the target key is smaller, traverse to the left child; if larger or equal, proceed to the 
   right child. This process is repeated down the tree until reaching the appropriate leaf node, 
   where the search is finalized. Since B+ Trees are balanced, search operations have a complexity 
   of O(log n).

2. **Insertion Operation**: Inserting a key begins by locating the correct leaf node using the search 
   procedure. The key is then inserted into the leaf node in its correct sorted position. If the node 
   exceeds its maximum capacity (M-1 keys), a split occurs. The node divides into two, with half the 
   keys moving to a new sibling node. The median key is promoted to the parent node. If the parent 
   node also overflows, it may trigger further splits that can propagate up to the root, potentially 
   increasing the tree’s height.

3. **Deletion Operation**: Deleting a key starts by locating the target key in the relevant leaf node. 
   The key is removed, but if this causes the node to have fewer keys than the minimum (⌈M/2⌉), 
   an adjustment is needed. The tree can borrow a key from a sibling node if it has more than the 
   minimum number of keys. If borrowing isn't possible, the node merges with a sibling, and a key 
   from the parent node is removed or adjusted accordingly. This merge process can propagate up 
   the tree if necessary, keeping the B+ Tree balanced.

4. **Range Queries**: B+ Trees excel at range queries due to the linked list structure of the leaf nodes. 
   To perform a range query, start by locating the first key in the desired range using the 
   search procedure. Once the starting key is found, traverse through the linked list of leaf 
   nodes until the end of the range is reached. This linked structure makes scanning a sequence 
   of values efficient and straightforward.

5. **Update Operation**: Updating a key's value involves locating the key in the relevant leaf node 
   through a search. Once found, the associated value is modified. If the value needs to be repositioned 
   due to record capacity constraints, new record can be allocated in storage without changing th key.

Each operation relies on the balanced nature of the B+ Tree to maintain efficiency, resulting in 
logarithmic complexity for searches, insertions, and deletions, even with large datasets.
