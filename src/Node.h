#ifndef NODE_H
#define NODE_H

#include <vector>
#include <cstdint>

/**
 * @file Node.h
 * @author Group 2
 * @brief Base Node class for B+ tree nodes
 * @version 0.1
 * @date 2025-12-02
 */



const size_t MAX_KEYS = 2; // Example maximum number of keys per index block
const size_t MAX_VALUES = 2; // Example maximum number of values per leaf block

/**
 * @class Node
 * @brief Abstract base class for B+ tree nodes (both index and leaf blocks)
 * @details Defines common interface and behavior for all B+ tree nodes.
 *          Both IndexBlock and LeafBlock inherit from this class to share
 *          common functionality and enforce a consistent interface.
 */
class Node
{
public:
    /**
     * @brief Default constructor
     * @details Initializes a node with default values
     */
    Node();

    /**
     * @brief Virtual destructor
     * @details Ensures proper cleanup of derived classes
     */
    virtual ~Node() = default;

    /**
     * @brief Check if node is full
     * @details Determines if the node has reached maximum capacity
     * @returns true if node has MAX_KEYS keys, false otherwise
     */
    virtual bool isFull() const;

    /**
     * @brief Check if node is empty
     * @details Determines if the node has no keys
     * @returns true if node has no keys, false otherwise
     */
    virtual bool isEmpty() const;

    /**
     * @brief Get number of keys in node
     * @details Returns the current count of keys in this node
     * @returns the number of keys stored in this node
     */
    virtual size_t getKeyCount() const;

    /**
     * @brief Get key at index
     * @details Retrieves the key at the specified index
     * @param index the position of the key to retrieve
     * @returns the key at the specified index
     */
    virtual uint32_t getKeyAt(size_t index) const;

    /**
     * @brief Check if node is a leaf node
     * @details Determines the node type
     * @returns true if this is a leaf node, false if index node
     */
    virtual bool isLeafNode() const = 0;

    /**
     * @brief Split node
     * @details Virtual method for splitting the node (implemented by derived classes)
     * @returns a new Node containing half the keys
     */
    virtual Node* split() = 0;

protected:
    std::vector<uint32_t> keys;  // Keys stored in this node

    /**
     * @brief Clear all keys
     * @details Removes all keys from the node
     */
    virtual void clearKeys();

    /**
     * @brief Add key to node
     * @details Adds a key to the node's key vector
     * @param key the key to add
     */
    virtual void addKey(const uint32_t& key);

    /**
     * @brief Remove key at index
     * @details Removes the key at the specified index
     * @param index the position of the key to remove
     */
    virtual void removeKeyAt(size_t index);
};



#endif // NODE_H
