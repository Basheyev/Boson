/******************************************************************************
*
*  Balanced Index
* 
*  Persistent key/value index based on B+ tree for search acceleration
* 
*
*  (C) Boson Database, Bolat Basheyev 2022-2023
*
******************************************************************************/
#pragma once


#include "RecordFileIO.h"

#include <cinttypes>
#include <string>


namespace Boson {


    constexpr uint64_t TREE_ORDER = 5;
    constexpr uint64_t MAX_DEGREE = TREE_ORDER - 1;
    constexpr uint64_t MIN_DEGREE = TREE_ORDER / 2;

    typedef enum : uint32_t { INNER = 1, LEAF = 2 } NodeType;


    typedef struct {
        uint64_t parent;
        uint64_t leftSibling;
        uint64_t rightSibling;
        uint32_t nodeType;
        uint32_t keysCount;
        uint64_t keys[TREE_ORDER];
        union {
            uint64_t children[TREE_ORDER];
            uint64_t values[TREE_ORDER];
        };
    } NodeData;



    class Node {
    friend class BalancedIndex;
    public:

        Node(BalancedIndex& bi, uint64_t offsetInFile);
        ~Node();

        void persist();

        uint32_t getKeyCount();
        bool     isOverflow();
        bool     isUnderflow();
        bool     canLendAKey();
        uint64_t getKeyAt(uint32_t index);
        void     setKeyAt(uint32_t index, uint64_t key);
        uint64_t getParent();
        void     setParent(uint64_t parentPosition);
        uint64_t getLeftSibling();
        void     setLeftSibling(uint64_t silbingPosition);
        uint64_t getRightSibling();
        void     setRightSibling(uint64_t siblingPosition);
        uint64_t dealOverflow();
        uint64_t dealUnderflow();

    protected:
        BalancedIndex& index;         // reference to index   
        uint64_t position;            // offset in file
        NodeData data;                // node data
        bool isPersisted;             // is data persisted to storage

        virtual NodeType getNodeType() = 0;
        virtual uint32_t search(uint64_t key) = 0;
        virtual uint64_t split() = 0;
        virtual uint64_t pushUpKey(uint64_t key, uint64_t leftChild, uint64_t rightChild) = 0;
        virtual uint64_t mergeChildren(uint64_t leftChild, uint64_t rightChild) = 0;
        virtual void  mergeWithSibling(uint64_t key, uint64_t rightSibling) = 0;
        virtual uint64_t borrowFromSibling(uint64_t key, uint64_t sibling, uint32_t borrowIndex) = 0;
        virtual void  borrowChildren(uint64_t borrow0er, uint64_t lender, uint32_t borrowIndex) = 0;
    };
          


    class BalancedIndex {
    friend class Node;
    public:
        
        BalancedIndex(RecordFileIO& rf);
        ~BalancedIndex();                
        
        bool insert(uint64_t key, const std::string& value);
        bool update(uint64_t key, const std::string& value);
        bool search(uint64_t key, std::string& value);
        bool erase(uint64_t key);
        
        uint64_t getEntriesCount();
        bool first();
        bool last();
        bool next();
        bool previous();
        bool getValue(std::string& value);
        
    protected:

        RecordFileIO& getRecordsFile();
        uint64_t allocateSpace(uint32_t size);
        bool releaseSpace(uint64_t position);
        


    private:

        RecordFileIO& records;
        uint64_t rootPosition;
        uint64_t cursorPosition;



        // Node manipulations


    };


}