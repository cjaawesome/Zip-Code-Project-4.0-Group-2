// LeafBlockNode.h
#ifndef LEAFBLOCKNODE_H
#define LEAFBLOCKNODE_H

#include "Node.h"
#include <cstdint>
#include <string>
#include <vector>

const size_t MAX_KEYS = 2; // Example maximum number of keys per index block
const size_t MAX_VALUES = 2; // Example maximum number of values per leaf block

template <typename keyType, typename valueType>
class LeafBlockNode : public Node<keyType>
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
    bool find(const keyType& key, valueType& outValue) const;
    /**
     * @brief Insert Key-Value Pair
     * @details Inserts a key-value pair into the leaf block
     * @param key the key to be inserted
     * @param value the value to be inserted
     */
    void insertKV(const keyType &key, const valueType &value);
    /**
     * @brief Set Next Leaf RBN
     * @details Sets the RBN of the next leaf block
     * @param rbn the RBN to set
     */
    void setNextLeafRBN(uint32_t rbn);
    /**
     * @brief Get Next Leaf RBN
     * @details Gets the RBN of the next leaf block
     * @returns the RBN of the next leaf block
     */
    uint32_t getNextLeafRBN() const;
    /**
     * @brief Set Previous Leaf RBN
     * @details Sets the RBN of the previous leaf block
     * @param rbn the RBN to set
     */
    void setPrevLeafRBN(uint32_t rbn);
    /**
     * @brief Get Previous Leaf RBN
     * @details Gets the RBN of the previous leaf block
     * @returns the RBN of the previous leaf block
     */
    uint32_t getPrevLeafRBN() const;

    /**
     * @brief Split
     * @details Splits the leaf block into two
     * @returns a new LeafBlock containing half the keys and values
     */
    LeafBlockNode split();

    /**
     * @brief Is Leaf Node
     * @details Identifies this node as a leaf node
     * @returns true (always true for LeafBlock)
     */
    bool isLeafNode() const override;

private:
    std::vector<keyType> keys;
    std::vector<valueType> values;
    uint32_t nextLeafRBN;
    uint32_t prevLeafRBN;
};

#include "LeafBlockNode.tpp"

#endif // LEAFBLOCKNODE_H