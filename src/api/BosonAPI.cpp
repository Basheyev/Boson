/******************************************************************************
*
*  Boson Database API
*
*  Features:
*  - NoSQL database engine.
*  - Single database file.
*  - Standard Key/Value store.
*  - Document store (JSON).
*  - Support cursors for linear records traversal.
*  - Support for on-disk as well in-memory databases.
*  - Support Terabyte sized databases.
*
*  (C) Boson Database, Bolat Basheyev 2022-2024
*
******************************************************************************/

#include "BosonAPI.h"


using namespace Boson;


/*
*  @brief Boson API constructructor
*/
BosonAPI::BosonAPI() {
    cachedFile = nullptr;
    recordFile = nullptr;
    balancedIndex = nullptr;
}


/*
*  @brief Boson API destructructor
*/
BosonAPI::~BosonAPI() {
    if (cachedFile != nullptr) {
        if (cachedFile->isOpen()) close();
    }
}


/*
*  @brief Opens database file and allocate required resources
*  @param filename - path to file (C-style string)
*  @param readOnly - true to open with read only rights, false to write permission (default)
*  @return true if database file successfuly opened, false if not
*/
bool BosonAPI::open(char* filename, bool readOnly) {
    isReadOnly = readOnly;
    cachedFile = new CachedFileIO();
    if (!cachedFile->open(filename, DEFAULT_CACHE, readOnly)) {
        delete cachedFile;
        return false;
    }
    recordFile = new RecordFileIO(*cachedFile);
    balancedIndex = new BalancedIndex(*recordFile);    
    return true;
}


/*
*  @brief Close database file and release resources
*  @return true if file was closed, false if it wasn't open
*/
bool BosonAPI::close() {
    delete balancedIndex;
    delete recordFile;
    bool wasOpen = cachedFile->close();
    delete cachedFile;
    cachedFile = nullptr;
    recordFile = nullptr;
    balancedIndex = nullptr;
    return wasOpen;
}


/*
*  @brief Return total amount of key/value pairs
*  @return total amount of key/value pairs
*/
uint64_t BosonAPI::size() {
    if (balancedIndex == nullptr) return 0;
    return balancedIndex->size();
}



/*
*  @brief Checks if key/value pair exists
*  @return true if exists, false otherwise
*/
bool BosonAPI::isExists(uint64_t key) {
    if (balancedIndex == nullptr) return false;
    std::shared_ptr<LeafNode> leaf = balancedIndex->findLeafNode(key);
    return leaf->search(key) != KEY_NOT_FOUND;
}



/*
*  @brief Inserts new string entry into database and return its ID
*  @return ID of new entry created or NOT_FOUND if not created (database read only)
*/
uint64_t BosonAPI::insert(std::string value) {
    if (balancedIndex == nullptr || isReadOnly) return NOT_FOUND;
    uint64_t nextKey = balancedIndex->getNextIndexCounter();
    return balancedIndex->insert(nextKey, value) ? nextKey : NOT_FOUND;
}


/*
*  @brief Inserts new key/string pair into database
*  @param key ID of new entry
*  @param value string of new entry
*  @return true if succeded, false if failed (ID duplicate, file is not open or read only)
*/
bool BosonAPI::insert(uint64_t key, std::string value) {
    if (balancedIndex == nullptr || isReadOnly) return false;
    return balancedIndex->insert(key, value);
}


/*
*  @brief Return value by specified key
*  @param key of required value
*  @return value string if key found or nullptr if not found
*/
std::shared_ptr<std::string> BosonAPI::get(uint64_t key) {
    if (balancedIndex == nullptr) return nullptr;
    return balancedIndex->search(key);
}


/*
*  @brief Delete key/value pair from database
*  @param key ID of entry to delete
*  @return true if succeded, false if key not found or database is read only
*/
bool BosonAPI::erase(uint64_t key) {
    if (balancedIndex == nullptr || isReadOnly) return false;
    return balancedIndex->erase(key);
}


/*
*  @brief Go to the database first entry and return key/value pair
*  @return key/value pair
*/
std::pair<uint64_t, std::shared_ptr<std::string>> BosonAPI::first() {
    if (balancedIndex == nullptr) return std::make_pair(0, nullptr);
    return balancedIndex->first();
}



/*
*  @brief Go to the database last entry and return key/value pair
*  @return key/value pair
*/
std::pair<uint64_t, std::shared_ptr<std::string>> BosonAPI::last() {
    if (balancedIndex == nullptr) return std::make_pair(0, nullptr);
    return balancedIndex->last();
}



/*
*  @brief Fetch next entry in ascending order and return key/value pair
*  @return next key/value pair
*/
std::pair<uint64_t, std::shared_ptr<std::string>> BosonAPI::next() {
    if (balancedIndex == nullptr) return std::make_pair(0, nullptr);
    return balancedIndex->next();
}



/*
*  @brief Fetch previous entry in descending order and return key/value pair
*  @return previous key/value pair
*/
std::pair<uint64_t, std::shared_ptr<std::string>> BosonAPI::previous() {
    if (balancedIndex == nullptr) return std::make_pair(0, nullptr);
    return balancedIndex->previous();
}


/*
*  @brief Return percent of cache hits on read/write operations
*  @return percent of cache hits on read/write operations
*/
double BosonAPI::getCacheHits() {
    if (cachedFile == nullptr) return 0;
    return cachedFile->getStats(CachedFileStats::CACHE_HITS_RATE);
}