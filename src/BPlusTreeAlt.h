#ifndef BPLUS_TREE_ALT
#define BPLUS_TREE_ALT

#include "HeaderBuffer.h"
#include "BlockBuffer.h"
#include "BlockIndexFile.h"
#include "BPlusTreeHeaderBufferAlt.h"
#include "NodeAlt.h"
#include "PageBufferAlt.h"
#include <string>
#include <cstdint>
#include <iostream>
#include <vector>

class BPlusTreeAlt
{
public:
    BPlusTreeAlt();
    ~BPlusTreeAlt();

    bool open(const std::string& indexFileName, const std::string& sequenceSetFilename);
    bool isFileOpen() const;
    bool buildFromSequenceSet();
    bool search(uint32_t key, uint32_t& outValue);
    bool insert(uint32_t key, uint32_t blockRBN);
    bool remove(uint32_t key);
    bool hasError() const;
    bool getIsStale() const;

    std::string getLastError() const;

    uint32_t findLeafRBN(uint32_t key);

    void printTree();
    void close();

    void convertIndexToBPlusTree(const std::vector<BlockIndexFile::IndexEntry>& indexEntries, const std::string& bPlusTreeFileName, uint32_t blockSize);

private:
    bool isOpen; // Is the B+ tree file open (redundant with PageBufferAlt?)
    bool errorState; // Error state flag
    bool isStale; // Indicates if the B+ tree is stale

    std::string errorMessage;

    BPlusTreeHeaderAlt treeHeader; // Stored for necessary index realted metadata
    HeaderRecord sequenceHeader; // Stored for necessary sequence set related metadata

    PageBufferAlt indexPageBuffer; // Buffer for index file pages
    BlockBuffer sequenceSetBuffer; // Buffer for sequence set blocks

    uint32_t sequenceHeaderSize; // Cahced header size for convenience
    uint32_t blockSize; // Cahced block size for convenience

    struct IndexEntry
    {
        uint32_t key;
        uint32_t blockRBN;
    };

    NodeAlt* loadNode(uint32_t rbn);
    
    uint32_t searchRecursive(uint32_t nodeRBN, uint32_t key) const;
    uint32_t splitNode(uint32_t nodeRBN, uint32_t& promotedKey);
    uint32_t allocateTreeBlock();

    bool writeNode(uint32_t rbn, const NodeAlt& node);
    bool buildTreeFromEntries(const std::vector<IndexEntry>& entries);
    bool insertRecursive(uint32_t nodeRBN, uint32_t key, uint32_t value, 
                                    uint32_t& newChildRBN, uint32_t& newPromotedKey);
    bool removeRecursive(uint32_t nodeRBN, uint32_t key, bool& underflow);

    bool borrowFromSibling(uint32_t nodeRBN, uint32_t parentRBN, size_t indexInParent, bool isLeaf);
    bool mergeWithSibling(uint32_t nodeRBN, uint32_t parentRBN, size_t indexInParent, bool isLeaf);

    void setError(const std::string& message);
    void freeIndexBlock(uint32_t rbn);
    void printNode(uint32_t rbn, int depth);
    void insertIntoLeaf(NodeAlt* node, uint32_t key, uint32_t value);
    void insertIntoIndex(NodeAlt* node, uint32_t key, uint32_t childRBN);

    std::vector<uint32_t> buildLeafLevel(const std::vector<IndexEntry>& entries);
    std::vector<uint32_t> buildIndexLevel(const std::vector<uint32_t>& childRBNs);

};

#endif

