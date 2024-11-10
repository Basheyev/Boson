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

#pragma once

#include "CachedFileIO.h"
#include "RecordFileIO.h"
#include "BalancedIndex.h"


namespace Boson {

    class BosonAPI {
    public:

        BosonAPI();
        ~BosonAPI();

        bool open(char* filename, bool readOnly = false, size_t cacheSize = DEFAULT_CACHE);
        bool close();

        uint64_t size();
        bool isExists(uint64_t key);

        uint64_t insert(std::string value);
        bool insert(uint64_t key, std::string value);
        std::shared_ptr<std::string> get(uint64_t key);
        bool erase(uint64_t key);

        std::pair<uint64_t, std::shared_ptr<std::string>> first();
        std::pair<uint64_t, std::shared_ptr<std::string>> last();
        std::pair<uint64_t, std::shared_ptr<std::string>> next();
        std::pair<uint64_t, std::shared_ptr<std::string>> previous();

        void flush();

        double getCacheHits();
        double getReadThroughput();
        double getWriteThroughput();


        void printTreeState();

    private:
        CachedFileIO* cachedFile;
        RecordFileIO* recordFile;
        BalancedIndex* balancedIndex;
        bool isReadOnly;
    };





}
