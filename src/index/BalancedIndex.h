/******************************************************************************
*
*  Balanced Index
* 
*  Persistent key/value index based on B+ tree for search acceleration
* 
*
*  (C) Boson Database, Bolat Basheyev 2022-2024
*
******************************************************************************/
#pragma once


#include "RecordFileIO.h"

#include <cinttypes>
#include <string>
#include <memory>

namespace Boson {


    constexpr uint64_t TREE_ORDER = 5;
    constexpr uint64_t MAX_DEGREE = TREE_ORDER - 1;
    constexpr uint64_t MIN_DEGREE = TREE_ORDER / 2;
    constexpr uint32_t NOT_FOUND_KEY = -1;

    typedef enum : uint32_t { INNER = 1, LEAF = 2 } NodeType;
    typedef enum : uint32_t { KEYS = 1, CHILDREN = 2, VALUES = 2 } NodeArray;

    
    //-------------------------------------------------------------------------

    class NodeData {
    public:
        uint64_t parent;
        uint64_t leftSibling;
        uint64_t rightSibling;
        NodeType nodeType;
        uint32_t keysCount;
        union {
            uint32_t childrenCount;
            uint32_t valuesCount;
        };
        uint64_t keys[TREE_ORDER];        
        union {
            uint64_t children[TREE_ORDER];
            uint64_t values[TREE_ORDER];
        };

        NodeData();
       
        void pushBack(NodeArray mode, uint64_t value);
        void insertAt(NodeArray mode, uint32_t index, uint64_t value);
        void deleteAt(NodeArray mode, uint32_t index);
        void resize(NodeArray mode, uint32_t newSize);
    };

    //-------------------------------------------------------------------------
    
    class BalancedIndex;

    class Node {
    public:
        Node(BalancedIndex& bi);   
        Node(BalancedIndex& bi, NodeType type);
        static std::shared_ptr<Node> loadNode(BalancedIndex& bi, uint64_t offsetInFile);
        ~Node();
        void persist();
        NodeType getNodeType();
        uint32_t getKeyCount();
        bool     isOverflow();
        bool     isUnderflow();
        bool     canLendAKey();
        uint64_t getKeyAt(uint32_t index);
        void     setKeyAt(uint32_t index, uint64_t key);
        uint64_t getParent();
        void     setParent(uint64_t parentPosition);
        uint64_t getLeftSibling();
        void     setLeftSibling(uint64_t siblingPosition);
        uint64_t getRightSibling();
        void     setRightSibling(uint64_t siblingPosition);
        uint64_t dealOverflow();
        uint64_t dealUnderflow();
    protected:
        BalancedIndex& index;         // reference to index   
        uint64_t position;            // offset in file
        NodeData data;                // node data
        bool isPersisted;             // is data persisted to storage        
        virtual uint32_t search(uint64_t key) = 0;
        virtual uint64_t split() = 0;
        virtual uint64_t pushUpKey(uint64_t key, uint64_t leftChild, uint64_t rightChild) = 0;
        virtual uint64_t mergeChildren(uint64_t leftChild, uint64_t rightChild) = 0;
        virtual void  mergeWithSibling(uint64_t key, uint64_t rightSibling) = 0;
        virtual uint64_t borrowFromSibling(uint64_t key, uint64_t sibling, uint32_t borrowIndex) = 0;
        virtual void  borrowChildren(uint64_t borrow0er, uint64_t lender, uint32_t borrowIndex) = 0;
    };
          
    //-------------------------------------------------------------------------

    class InnerNode : public Node {
    public:
        InnerNode(BalancedIndex& bi);
        InnerNode(BalancedIndex& bi, uint64_t offsetInFile);
        ~InnerNode();
        uint32_t   search(uint64_t key);
        uint64_t   getChildAt(uint32_t index);
        void       setChildAt(uint32_t index, uint64_t childNode);
        void       insertAt(uint32_t index, uint64_t key, uint64_t leftChild, uint64_t rightChild);
        void       deleteAt(uint32_t index);
        uint64_t   split();
        uint64_t   pushUpKey(uint64_t key, uint64_t leftChild, uint64_t rightChild);
        void       borrowChildren(uint64_t borrower, uint64_t lender, uint32_t borrowIndex);
        uint64_t   borrowFromSibling(uint64_t key, uint64_t sibling, uint32_t borrowIndex);
        uint64_t   mergeChildren(uint64_t leftChild, uint64_t rightChild);
        void       mergeWithSibling(uint64_t key, uint64_t rightSibling);        
        NodeType   getNodeType();
    };


    //-------------------------------------------------------------------------

    class LeafNode : public Node {
    public:
        LeafNode(BalancedIndex& bi);
        LeafNode(BalancedIndex& bi, uint64_t offsetInFile);
        ~LeafNode();
        uint32_t search(uint64_t key);
        uint64_t getValueAt(uint32_t index);
        void     setValueAt(uint32_t index, const std::string& value);
        bool     insertKey(uint64_t key, const std::string& value);
        void     insertAt(uint32_t index, uint64_t key, const std::string& value);
        bool     deleteKey(uint64_t key);
        void     deleteAt(uint32_t index);
        uint64_t split();
        void     merge(uint64_t key, uint64_t siblingRight);
        uint64_t pushUpKey(uint64_t key, uint64_t leftChild, uint64_t rightChild);
        void     borrowChildren(uint64_t borrower, uint64_t lender, uint32_t borrowIndex);
        uint64_t mergeChildren(uint64_t leftChild, uint64_t rightChild);
        void     mergeWithSibling(uint64_t key, uint64_t rightSibling);
        uint64_t borrowFromSibling(uint64_t key, uint64_t sibling, uint32_t borrowIndex);
        NodeType getNodeType();
    private:
        uint32_t searchPlaceFor(uint64_t key);
    };

    //-------------------------------------------------------------------------

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
    private:

        RecordFileIO& records;
        uint64_t rootPosition;
        uint64_t cursorPosition;



        // Node manipulations


    };


}