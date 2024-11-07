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

#include <algorithm>
#include <unordered_map>
#include <cinttypes>
#include <string>
#include <memory>
#include <sstream>
#include <iostream>
#include <ios>

#include "RecordFileIO.h"

namespace Boson {


    constexpr uint64_t TREE_ORDER = 5;
    constexpr uint64_t MAX_DEGREE = TREE_ORDER - 1;
    constexpr uint64_t MIN_DEGREE = TREE_ORDER / 2;
    constexpr uint32_t KEY_NOT_FOUND = -1;

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
    class LeafNode;
    class InnerNode;

    class Node {
        friend class BalancedIndex;
        friend class LeafNode;
        friend class InnerNode;
    public:
        Node(BalancedIndex& bi, NodeType type);        
        ~Node();
        uint64_t getPosition();
        uint64_t persist();
        NodeType getNodeType();
        uint32_t getKeyCount();
        bool     isRootNode();
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
    
        Node(BalancedIndex& bi);
        static std::shared_ptr<Node> loadNode(BalancedIndex& bi, uint64_t offsetInFile);
        static void deleteNode(BalancedIndex& bi, uint64_t offsetInFile);

        virtual uint32_t search(uint64_t key) = 0;
        virtual uint64_t split() = 0;
        virtual uint64_t pushUpKey(uint64_t key, uint64_t leftChild, uint64_t rightChild) = 0;
        virtual uint64_t mergeChildren(uint64_t leftChild, uint64_t rightChild) = 0;
        virtual void     mergeWithSibling(uint64_t key, uint64_t rightSibling) = 0;
        virtual uint64_t borrowFromSibling(uint64_t key, uint64_t sibling, uint32_t borrowIndex) = 0;
        virtual void     borrowChildren(uint64_t borrow0er, uint64_t lender, uint32_t borrowIndex) = 0;
        virtual std::shared_ptr<std::string> toString() = 0;
    };

    //-------------------------------------------------------------------------

    class InnerNode : public Node {
        friend class BalancedIndex;
        friend class Node;
        friend class LeafNode;        
    public:
        InnerNode(BalancedIndex& bi);
        InnerNode(BalancedIndex& bi, uint64_t offsetInFile, NodeData& loadedData);
        ~InnerNode();
        uint32_t   search(uint64_t key);
        uint64_t   getChildAt(uint32_t index);
        void       setChildAt(uint32_t index, uint64_t childNode);
        void       insertAt(uint32_t index, uint64_t key, uint64_t leftChild, uint64_t rightChild);
        void       deleteAt(uint32_t index);        
        NodeType   getNodeType();
        std::shared_ptr<std::string> toString();
    protected:
        uint64_t   split();
        uint64_t   pushUpKey(uint64_t key, uint64_t leftChild, uint64_t rightChild);
        void       borrowChildren(uint64_t borrower, uint64_t lender, uint32_t borrowIndex);
        uint64_t   borrowFromSibling(uint64_t key, uint64_t sibling, uint32_t borrowIndex);
        uint64_t   mergeChildren(uint64_t leftChild, uint64_t rightChild);
        void       mergeWithSibling(uint64_t key, uint64_t rightSibling);
    };


    //-------------------------------------------------------------------------

    class LeafNode : public Node {
        friend class BalancedIndex;
        friend class Node;
        friend class InnerNode;
    public:
        LeafNode(BalancedIndex& bi);
        LeafNode(BalancedIndex& bi, uint64_t offsetInFile, NodeData& loadedData);
        ~LeafNode();
        uint32_t search(uint64_t key);
        std::shared_ptr<std::string> getValueAt(uint32_t index);
        void     setValueAt(uint32_t index, const std::string& value);
        bool     insertKey(uint64_t key, const std::string& value);
        bool     insertKey(uint64_t key, uint64_t valuePosition);
        void     insertAt(uint32_t index, uint64_t key, const std::string& value);
        void     insertAt(uint32_t index, uint64_t key, uint64_t valuePosition);
        bool     deleteKey(uint64_t key);
        void     deleteAt(uint32_t index);        
        NodeType getNodeType();
        std::shared_ptr<std::string> toString();
    protected:
        uint64_t split();
        uint64_t pushUpKey(uint64_t key, uint64_t leftChild, uint64_t rightChild);
        void     borrowChildren(uint64_t borrower, uint64_t lender, uint32_t borrowIndex);
        uint64_t mergeChildren(uint64_t leftChild, uint64_t rightChild);
        void     mergeWithSibling(uint64_t key, uint64_t rightSibling);
        uint64_t borrowFromSibling(uint64_t key, uint64_t sibling, uint32_t borrowIndex);
        uint32_t searchPlaceFor(uint64_t key);
    };

    //-------------------------------------------------------------------------


    class IndexHeader {
    public:
        uint64_t treeOrder;       // Tree order
        uint64_t rootPosition;    // Root node position in the storage file
        uint64_t recordsCount;    // Total records count
        uint64_t indexCounter;    // Index key counter
    };


    class BalancedIndex {
        friend class Node;
        friend class LeafNode;
        friend class InnerNode;
        friend class BosonAPI;
    public:
        BalancedIndex(RecordFileIO& rf);
        ~BalancedIndex();       

        uint64_t size();

        bool insert(uint64_t key, const std::string& value);
        bool update(uint64_t key, const std::string& value);
        std::shared_ptr<std::string> search(uint64_t key);
        bool erase(uint64_t key);

        std::pair<uint64_t, std::shared_ptr<std::string>> first();
        std::pair<uint64_t, std::shared_ptr<std::string>> last();
        std::pair<uint64_t, std::shared_ptr<std::string>> next();
        std::pair<uint64_t, std::shared_ptr<std::string>> previous();

        void printTree();        

    protected:

        uint64_t getNextIndexCounter();
        RecordFileIO& getRecordsFile();
        std::shared_ptr<LeafNode> findLeafNode(uint64_t key);                
        void updateRoot(uint64_t newRootPosition);
        void persistIndexHeader();
        void printTreeLevel(std::shared_ptr<Node> node, int level);

    private:
        RecordFileIO& recordsFile;
        IndexHeader indexHeader;
        std::shared_ptr<Node> root;

        std::shared_ptr<LeafNode> cursorNode;
        uint32_t cursorIndex;
        bool isTreeChanged;
    };


}