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
template <typename keyType, typename valueType>
class IndexBlockNode : public Node<keyType>
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
    void addKey(const keyType &key);
    /**
     * @brief Add child RBN
     * @details Adds a child RBN to the index block
     * @param rbn the RBN to be added
     */
    void addChildRBN(const keyType &rbn);
    /**
     * @brief Find child
     * @details Finds the child RBN for a given key
     * @param key the key to search for
     * @returns the index of the child RBN
     */
    size_t findChild(const keyType &key) const;
    /**
     * @brief Split
     * @details Splits the index block into two
     * @returns a new IndexBlock containing half the keys and children
     */
    IndexBlockNode split();

    /**
     * @brief Is Leaf Node
     * @details Identifies this node as an index node (not a leaf)
     * @returns false (always false for IndexBlock)
     */
    bool isLeafNode() const override;

private:
    std::vector<valueType> childrenRBNs;
};

#include "IndexBlockNode.tpp"

#endif // INDEXBLOCK_H