#ifndef BPLUSTREE_H
#define BPLUSTREE_H
#include "IndexBlock.h"
#include "LeafBlock.h"
#include "HeaderRecord.h"
#include "BlockBuffer.h"
#include <string>

template <typename keyType, typename valueType>
class BPlusTree {
public:
    /**
     * @brief Default constructor
     * @details Initializes BPlusTree
     */
    BPlusTree();
    /**
     * @brief Destructor
     * @details Cleans up BPlusTree
     */
    ~BPlusTree() = default;
    /**
     * @brief Insert
     * @details Inserts a key-value pair into the BPlusTree
     * @param key the key to be inserted
     * @param value the value to be inserted
     */
    void insert(const keyType& key, const valueType& value);
    /**
     * @brief Search
     * @details Searches for a value by key in the BPlusTree
     * @param key the key to search for
     * @returns the value associated with the key
     */
    valueType search(const keyType& key) const;
    /**
     * @brief Open
     * @details Opens the BPlusTree index file
     * @returns true if the file was opened successfully, false otherwise
     */
    bool open();
    /**
     * @brief Close
     * @details Closes the BPlusTree index file
     */
    void close();
private:
    std::string indexFileName;
    HeaderRecord headerRecord;
    BlockBuffer blockBuffer;
    uint32_t rootIndexRBN;
};

#include "BPlusTree.tpp"

#endif // BPLUSTREE_H