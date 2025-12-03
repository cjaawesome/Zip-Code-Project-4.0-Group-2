// IndexBlockNode.h
#ifndef INDEXBLOCKNODE_H
#define INDEXBLOCKNODE_H

#include "Node.h"
#include <string>
#include <vector>

const size_t MAX_KEYS = 2; // Example maximum number of keys per index block

/**
 * @file IndexBlock.h
 * @author Group 2
 * @brief IndexBlock class for managing index blocks
 * @version 0.1
 * @date 2025-10-02
 */
class IndexBlockNode : private Node
{
public:
    /**
     * @brief Default constructor
     * @details Initializes index block
     */
    IndexBlockNode() = default;
    /**
     * @brief Destructor
     * @details Cleans up index block
     */
    ~IndexBlockNode() = default;
    /**
     * @brief Add key
     * @details Adds a key to the index block
     * @param key the key to be added
     */
    bool addKey(const uint32_t &key) override;
    /**
     * @brief Add child RBN
     * @details Adds a child RBN to the index block
     * @param rbn the RBN to be added
     */
    void addChildPageNumber(const uint32_t &pageNumber);
    /**
     * @brief Find child
     * @details Finds the child RBN for a given key
     * @param key the key to search for
     * @returns the index of the child RBN
     */
    size_t findChild(const uint32_t &key) const;
    /**
     * @brief Split
     * @details Splits the index block into two
     * @returns a new IndexBlock containing half the keys and children
     */
    Node* split() override;

    /**
     * @brief Is Leaf Node
     * @details Identifies this node as an index node (not a leaf)
     * @returns false (always false for IndexBlock)
     */
    bool isLeafNode() const override;

private:
    std::vector<uint32_t> childrenPageNumbers;
};


#endif // INDEXBLOCK_H