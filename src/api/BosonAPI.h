#pragma once

#include "CachedFileIO.h"
#include "RecordFileIO.h"
#include "Document.h"
//#include "BalancedTree.h"

#include <string>


namespace Boson {


    class Database {
    public:
        Database();
        ~Database();

        bool open(std::string path);
        bool isOpen();
        bool close();

        bool insert(KEY key, VALUE value);
        JSON& search(size_t key);


        bool   insert(KEY key, VALUE value);
        VALUE  search(KEY key);

        bool   erase(KEY key);



    private:




    };


}