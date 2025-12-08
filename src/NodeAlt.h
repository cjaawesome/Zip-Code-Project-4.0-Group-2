#ifndef NODE_ALT_H
#define NODE_ALT_H

#include "stdint.h"
#include <cstring> 
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
/**
 * @class NodeAlt
 * @brief Node class used for building a B+ tree. Used for both index and leaf nodes.
 * @details Provides all necessary utilities for the building and modifying of a B+ tree.
 */
class NodeAlt
{
public:
    /**
     * @brief Paramterized Constructor
     * @details Initializes a node based with it's status as leaf or index and it's block size.
     * @param isLeaf Is the node a leaf or index.
     * @param blockSize What is the standard block size used by nodes.
     */
    NodeAlt(bool isLeaf, size_t blockSize);
    
    /**
     * @brief Default Destructor
     */
    ~NodeAlt();

    /**
     * @brief Returns whether the node is a leaf node or not.
     * @return Returns 1 if the node is a leaf node. 0 if the node is an index node.
     */
    uint8_t isLeafNode() const;

    /**
     * @brief Sets a key at a given index within a node.
     * @param index The index the key should be set at.
     * @param key The key to set at the given index.
     * @return True if the key was successfully set. False if otherwise.
     */
    bool setKeyAt(size_t index, uint32_t key);

    /**
     * @brief Inserts a key at a given index, shifting subsequent keys to the right.
     * @param index The position at which to insert the key.
     * @param key The key value to insert.
     * @return True if the key was successfully inserted. False otherwise (e.g., node is full or index is invalid).
     */
    bool insertKeyAt(size_t index, uint32_t key);

    /**
     * @brief Removes a key at the specified index, shifting subsequent keys to the left.
     * @param index The index of the key to remove.
     * @return True if the key was successfully removed. False otherwise (e.g., index out of bounds).
     */
    bool removeKeyAt(size_t index);

    /**
     * @brief Inserts a child RBN (Relative Block Number) at the specified index.
     * @details Only applicable for index nodes. Shifts subsequent child RBNs to the right.
     * @param index The position at which to insert the child RBN.
     * @param rbn The RBN value to insert.
     * @return True if successfully inserted. False otherwise.
     */
    bool insertChildRBN(size_t index, uint32_t rbn);

    /**
     * @brief Removes a child RBN at the specified index.
     * @details Only applicable for index nodes. Shifts subsequent child RBNs to the left.
     * @param index The index of the child RBN to remove.
     * @return True if successfully removed. False otherwise.
     */
    bool removeChildRBN(size_t index);

    /**
     * @brief Checks if the node is full (has reached maximum key capacity).
     * @return True if the node is full. False otherwise.
     */
    bool isFull() const;

    /**
     * @brief Checks if the node is underfull (has fewer than the minimum required keys).
     * @return True if the node is underfull. False otherwise.
     */
    bool isUnderfull() const;

    /**
     * @brief Sets a value at a given index within a leaf node.
     * @details Only applicable for leaf nodes where keys map to values.
     * @param index The index at which to set the value.
     * @param value The value to set.
     * @return True if the value was successfully set. False otherwise.
     */
    bool setValueAt(size_t index, uint32_t value);

    /**
     * @brief Inserts a value at the specified index, shifting subsequent values to the right.
     * @details Only applicable for leaf nodes.
     * @param index The position at which to insert the value.
     * @param value The value to insert.
     * @return True if successfully inserted. False otherwise.
     */
    bool insertValueAt(size_t index, uint32_t value);

    /**
     * @brief Removes a value at the specified index, shifting subsequent values to the left.
     * @details Only applicable for leaf nodes.
     * @param index The index of the value to remove.
     * @return True if successfully removed. False otherwise.
     */
    bool removeValueAt(size_t index);

    /**
     * @brief Unpacks serialized byte data into the node's internal structure.
     * @param data Vector of bytes containing the serialized node data.
     * @return True if unpacking was successful. False if data is malformed or invalid.
     */
    bool unpack(const std::vector<uint8_t>& data);

    /**
     * @brief Checks if the node is in an error state.
     * @return True if an error has occurred. False otherwise.
     */
    bool getErrorState() const;

    /**
     * @brief Gets the current number of keys stored in the node.
     * @return The count of keys currently in the node.
     */
    size_t getKeyCount() const;

    /**
     * @brief Gets the block size used by this node.
     * @return The block size in bytes.
     */
    size_t getBlockSize() const;

    /**
     * @brief Gets the maximum number of keys this node can hold.
     * @return The maximum key capacity.
     */
    size_t getMaxKeys() const;

    /**
     * @brief Gets the current number of child pointers in the node.
     * @details Only applicable for index nodes. Typically equals key count + 1.
     * @return The count of child RBNs.
     */
    size_t getChildCount() const;

    /**
     * @brief Finds the appropriate child index for a given key.
     * @details Determines which child pointer to follow when searching for a key in index nodes.
     * @param key The key to search for.
     * @return The index of the child to traverse.
     */
    size_t findChildIndex(uint32_t key) const;

    /**
     * @brief Calculates the maximum number of keys that can fit in a node given block size.
     * @param blockSize The size of the block in bytes.
     * @param isLeaf Whether the node is a leaf (affects overhead due to values vs child pointers).
     * @return The maximum number of keys that can be stored.
     */
    static size_t calculateMaxKeys(size_t blockSize, bool isLeaf);

    /**
     * @brief Gets the key at the specified index.
     * @param index The index of the key to retrieve.
     * @return The key value at the given index.
     */
    uint32_t getKeyAt(size_t index) const;

    /**
     * @brief Gets the value at the specified index.
     * @details Only applicable for leaf nodes.
     * @param index The index of the value to retrieve.
     * @return The value at the given index.
     */
    uint32_t getValueAt(size_t index) const;

    /**
     * @brief Gets the RBN of the previous leaf node in the linked list.
     * @details Only applicable for leaf nodes. Used for range queries and sequential access.
     * @return The RBN of the previous leaf, or a sentinel value if none exists.
     */
    uint32_t getPrevLeafRBN() const;

    /**
     * @brief Gets the RBN of the next leaf node in the linked list.
     * @details Only applicable for leaf nodes. Used for range queries and sequential access.
     * @return The RBN of the next leaf, or a sentinel value if none exists.
     */
    uint32_t getNextLeafRBN() const;

    /**
     * @brief Gets the child RBN at the specified index.
     * @details Only applicable for index nodes.
     * @param index The index of the child pointer.
     * @return The RBN of the child node.
     */
    uint32_t getChildRBN(size_t index) const;

    /**
     * @brief Gets the RBN of the parent node.
     * @return The parent node's RBN, or a sentinel value if this is the root.
     */
    uint32_t getParentRBN() const;

    /**
     * @brief Sets the RBN of the previous leaf node.
     * @details Only applicable for leaf nodes.
     * @param rbn The RBN to set as the previous leaf.
     */
    void setPrevLeafRBN(uint32_t rbn);

    /**
     * @brief Sets the RBN of the next leaf node.
     * @details Only applicable for leaf nodes.
     * @param rbn The RBN to set as the next leaf.
     */
    void setNextLeafRBN(uint32_t rbn);

    /**
     * @brief Sets the parent node's RBN.
     * @param rbn The RBN of the parent node.
     */
    void setParentRBN(uint32_t rbn);

    /**
     * @brief Sets a child RBN at the specified index.
     * @details Only applicable for index nodes.
     * @param index The index at which to set the child RBN.
     * @param rbn The RBN value to set.
     */
    void setChildRBN(size_t index, uint32_t rbn);

    /**
     * @brief Sets the block size for the node.
     * @details May trigger recalculation of maximum key capacity.
     * @param inSize The new block size in bytes.
     */
    void setBlockSize(size_t inSize);

    /**
     * @brief Sets whether the node is a leaf or index node.
     * @param leaf 1 for leaf node, 0 for index node.
     */
    void setIsLeaf(uint8_t leaf);

    /**
     * @brief Serializes the node into a byte vector for storage.
     * @param data Output vector to store the serialized node data.
     */
    void pack(std::vector<uint8_t>& data) const;

    /**
     * @brief Clears all data from the node, resetting it to an empty state.
     */
    void clear();

    /**
     * @brief Sets the max key var of the node after node.
     * @details Used after node creation since it needs valid data first.
     * @param max The calculated max size.
     */
    void setMaxKeys(size_t max);

    /**
     * @brief Gets the current error message if the node is in an error state.
     * @return A string describing the error, or an empty string if no error exists.
     */
    std::string getErrorMessage() const;

private:
    // Common Node Data
    uint8_t isLeaf;                    // Flag indicating if this is a leaf (1) or index (0) node
    size_t blockSize;                  // Size of the block in bytes
    std::vector<uint32_t> keys;        // Sorted array of keys in the node
    uint32_t parentRBN;                // RBN of the parent node
    size_t maxKeys;                    // Maximum number of keys this node can hold

    // Leaf Node Data
    std::vector<uint32_t> values;      // Values corresponding to keys (leaf nodes only)
    uint32_t prevLeafRBN;              // RBN of previous leaf in linked list (leaf nodes only)
    uint32_t nextLeafRBN;              // RBN of next leaf in linked list (leaf nodes only)

    // Index Node Data
    std::vector<uint32_t> childRBNs;   // Array of child node RBNs (index nodes only, size = keys + 1)

    // Error Handling
    bool hasError;                     // Flag indicating if an error has occurred
    std::string errorMessage;          // Description of the most recent error
    
    /**
     * @brief Sets the node into an error state with a descriptive message.
     * @param message The error message to store.
     */
    void setError(const std::string& message);
};
#endif // NODE_ALT_H