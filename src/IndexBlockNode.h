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
     * @brief Set middle link
     * @details Sets the middle child link for this index block
     * @param link the RBN of the middle child
     */
    void setMiddleLink(uint32_t link);
    /**
     * @brief Get middle link
     * @details Retrieves the middle child link for this index block
     * @returns the RBN of the middle child
     */
    uint32_t getMiddleLink() const;
    /**
     * @brief Set right link
     * @details Sets the right sibling link for this index block
     * @param link the RBN of the right sibling
     */
    void setRightLink(uint32_t link) override;
    /**
     * @brief Get right link
     * @details Retrieves the right sibling link for this index block
     * @returns the RBN of the right sibling
     */
    uint32_t getRightLink() const override;
    /**
     * @brief Set left link
     * @details Sets the left sibling link for this index block
     * @param link the RBN of the left sibling
     */
    void setLeftLink(uint32_t link) override;
    /**
     * @brief Get left link
     * @details Retrieves the left sibling link for this index block
     * @returns the RBN of the left sibling
     */
    uint32_t getLeftLink() const override;
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

    /**
     * @brief Get Parent Link
     * @details Retrieves the parent link for this node
     * @returns the RBN of the parent node
     */
    uint32_t getParentLink() const override;

    /**
     * @brief Set Parent Link
     * @details Sets the parent link for this node
     * @param link the RBN of the parent node
     */
    void setParentLink(uint32_t link) override;

private:
    int32_t middleLink;
};


#endif // INDEXBLOCK_BLOCKNODE_H