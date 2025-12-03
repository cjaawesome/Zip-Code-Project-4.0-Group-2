// LeafBlockNode.h
#ifndef LEAFBLOCKNODE_H
#define LEAFBLOCKNODE_H

#include "Node.h"
#include <cstdint>
#include <string>
#include <vector>

const size_t MAX_KEYS = 2; // Example maximum number of keys per index block
const size_t MAX_VALUES = 2; // Example maximum number of values per leaf block

class LeafBlockNode : private Node
{
public:
    /**
     * @brief Default constructor
     * @details Initializes leaf block
     */
    LeafBlockNode();
    /**
     * @brief Destructor
     * @details Cleans up leaf block
     */
    ~LeafBlockNode() = default;
    /**
     * @brief Find
     * @details Finds the value for a given key
     * @param key the key to search for
     * @param outValue the value associated with the key (output parameter)
     * @returns true if key is found, false otherwise
     */
    bool find(const uint32_t key, uint32_t& outValue) const;
    /**
     * @brief Insert Key-Value Pair
     * @details Inserts a key-value pair into the leaf block
     * @param key the key to be inserted
     * @param value the value to be inserted
     */
    void insertKV(const uint32_t &key, const uint32_t &value);
    /**
     * @brief Set Next Leaf RBN
     * @details Sets the RBN of the next leaf block
     * @param rbn the RBN to set
     */
    void setNextLeafPageNumber(int32_t pageNumber);
    /**
     * @brief Get Next Leaf RBN
     * @details Gets the RBN of the next leaf block
     * @returns the RBN of the next leaf block
     */
    int32_t getNextLeafPageNumber() const;
    /**
     * @brief Set Previous Leaf RBN
     * @details Sets the RBN of the previous leaf block
     * @param rbn the RBN to set
     */
    void setPrevLeafPageNumber(int32_t pageNumber);
    /**
     * @brief Get Previous Leaf RBN
     * @details Gets the RBN of the previous leaf block
     * @returns the RBN of the previous leaf block
     */
    int32_t getPrevLeafPageNumber() const;

    /**
     * @brief Split
     * @details Splits the leaf block into two
     * @returns a new LeafBlock containing half the keys and values
     */
    Node* split() override;

    /**
     * @brief Is Leaf Node
     * @details Identifies this node as a leaf node
     * @returns true (always true for LeafBlock)
     */
    bool isLeafNode() const override;

private:
    std::vector<uint32_t> values;
};


#endif // LEAFBLOCKNODE_H