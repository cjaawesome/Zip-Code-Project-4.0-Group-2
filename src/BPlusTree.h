#ifndef BPLUSTREE_H
#define BPLUSTREE_H
#include "IndexBlock.h"
#include "LeafBlock.h"
#include "HeaderRecord.h"
#include "BlockBuffer.h"
#include <string>
#include <cstdint>

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
     * @details Inserts a key-value pair into the BPlusTree file
     * @param key the key to be inserted
     * @param value the value to be inserted
     * @returns true if insertion was successful
     */
    bool insert(const keyType& key, const valueType& value);
    /**
     * @brief Search
     * @details Searches for a value by key in the BPlusTree file
     * @param key the key to search for
     * @param outValue the value associated with the key (output parameter)
     * @returns true if search was successful, false if key not found
     */
    bool search(const keyType& key, valueType& outValue) const;
    /**
     * @brief Open
     * @details Opens or creates the BPlusTree index file. Loads the header record into RAM.
     * @param filename the name of the index file to open/create
     * @returns true if the file was opened successfully, false otherwise
     */
    bool open(const std::string& filename);
    /**
     * @brief Close
     * @details Closes the BPlusTree index file and writes header record back to disk
     */
    void close();
    /**
     * @brief Is Open
     * @details Checks if the B+tree file is currently open
     * @returns true if file is open, false otherwise
     */
    bool isOpen() const;

private:
    // === Persistent on Disk, Minimal in RAM ===
    std::string indexFileName;              // File path (RAM only during open)
    HeaderRecord headerRecord;              // Always in RAM while file is open
    BlockBuffer blockBuffer;                // Manages file I/O and block caching
    
    // === Cached for efficiency (optional optimization) ===
    IndexBlock<keyType, uint32_t>* rootIndexBlock;  // Root index block cached in RAM
    uint32_t rootIndexRBN;                  // RBN of root index block
    
    // === File State ===
    bool isFileOpen;                        // Track if B+tree file is currently open
    
    // === Private Helper Methods ===
    /**
     * @brief Load Index Block from File
     * @details Loads an index block from disk at the specified RBN
     * @param rbn the relative block number to load
     * @returns pointer to loaded IndexBlock (caller responsible for deletion)
     */
    IndexBlock<keyType, uint32_t>* loadIndexBlockAtRBN(uint32_t rbn) const;
    
    /**
     * @brief Load Leaf Block from File
     * @details Loads a leaf block from disk at the specified RBN
     * @param rbn the relative block number to load
     * @returns pointer to loaded LeafBlock (caller responsible for deletion)
     */
    LeafBlock<keyType, valueType>* loadLeafBlockAtRBN(uint32_t rbn) const;
    
    /**
     * @brief Write Index Block to File
     * @details Writes an index block to disk at the specified RBN
     * @param rbn the relative block number to write to
     * @param block the index block to write
     * @returns true if write was successful
     */
    bool writeIndexBlockAtRBN(uint32_t rbn, const IndexBlock<keyType, uint32_t>& block);
    
    /**
     * @brief Write Leaf Block to File
     * @details Writes a leaf block to disk at the specified RBN
     * @param rbn the relative block number to write to
     * @param block the leaf block to write
     * @returns true if write was successful
     */
    bool writeLeafBlockAtRBN(uint32_t rbn, const LeafBlock<keyType, valueType>& block);
    
    /**
     * @brief Search Index Path
     * @details Traverses the index blocks to find the appropriate leaf block RBN
     * @param key the key to search for
     * @returns the RBN of the leaf block containing the key
     */
    uint32_t searchIndexPath(const keyType& key) const;
    
    /**
     * @brief Split Index Block
     * @details Handles splitting of full index blocks during insertion
     * @param rbn the RBN of the index block to split
     * @returns RBN of the newly created index block
     */
    uint32_t splitIndexBlock(uint32_t rbn);
    
    /**
     * @brief Split Leaf Block
     * @details Handles splitting of full leaf blocks during insertion
     * @param rbn the RBN of the leaf block to split
     * @returns RBN of the newly created leaf block
     */
    uint32_t splitLeafBlock(uint32_t rbn);
};

#include "BPlusTree.tpp"

#endif // BPLUSTREE_H