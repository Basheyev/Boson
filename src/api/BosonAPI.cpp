

#include "BosonAPI.h"


using namespace Boson;




BosonAPI::BosonAPI() {
    cachedFile = nullptr;
    recordFile = nullptr;
    balancedIndex = nullptr;
}


BosonAPI::~BosonAPI() {
    if (cachedFile != nullptr) {
        if (cachedFile->isOpen()) close();
    }
}


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


bool BosonAPI::close() {
    delete balancedIndex;
    delete recordFile;
    cachedFile->close();
    delete cachedFile;
    cachedFile = nullptr;
    recordFile = nullptr;
    balancedIndex = nullptr;
    return true;
}


uint64_t BosonAPI::size() {
    if (balancedIndex == nullptr) return 0;
    return balancedIndex->size();
}


bool BosonAPI::isExists(uint64_t key) {
    if (balancedIndex == nullptr) return false;
    std::shared_ptr<LeafNode> leaf = balancedIndex->findLeafNode(key);
    return leaf->search(key) != KEY_NOT_FOUND;
}


uint64_t BosonAPI::insert(std::string value) {
    if (balancedIndex == nullptr || isReadOnly) return 0;
    uint64_t nextKey = balancedIndex->getNextIndexCounter();
    return balancedIndex->insert(nextKey, value) ? nextKey : NOT_FOUND;
}


bool BosonAPI::insert(uint64_t key, std::string value) {
    if (balancedIndex == nullptr || isReadOnly) return false;
    return balancedIndex->insert(key, value);
}


std::shared_ptr<std::string> BosonAPI::get(uint64_t key) {
    if (balancedIndex == nullptr) return false;
    return balancedIndex->search(key);
}


bool BosonAPI::erase(uint64_t key) {
    if (balancedIndex == nullptr || isReadOnly) return false;
    return balancedIndex->erase(key);
}


std::pair<uint64_t, std::shared_ptr<std::string>> BosonAPI::first() {
    if (balancedIndex == nullptr) return std::make_pair(0, nullptr);
    return balancedIndex->first();
}


std::pair<uint64_t, std::shared_ptr<std::string>> BosonAPI::last() {
    if (balancedIndex == nullptr) return std::make_pair(0, nullptr);
    return balancedIndex->last();
}


std::pair<uint64_t, std::shared_ptr<std::string>> BosonAPI::next() {
    if (balancedIndex == nullptr) return std::make_pair(0, nullptr);
    return balancedIndex->next();
}


std::pair<uint64_t, std::shared_ptr<std::string>> BosonAPI::previous() {
    if (balancedIndex == nullptr) return std::make_pair(0, nullptr);
    return balancedIndex->previous();
}



double BosonAPI::getCacheHits() {
    if (cachedFile == nullptr) return 0;
    return cachedFile->getStats(CachedFileStats::CACHE_HITS_RATE);
}