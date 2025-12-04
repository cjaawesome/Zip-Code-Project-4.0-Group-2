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
    virtual uint32_t getKeyAt(const size_t& index) const;

    /**
     * @brief Add key
     * @details Adds a key to the node
     * @param key the key to be added
     * @returns true if successful, false otherwise
     */
    virtual bool addKey(const uint32_t& link);

    /**
     * @brief Remove key at index
     * @details Removes the key at the specified index
     * @param index the position of the key to remove
     */
    virtual void removeKeyAt(size_t index);

    /**
     * @brief Add link
     * @details Adds a child link (Page Number) to the node
     * @param link the child link to be added
     */
    virtual void addLink(const uint32_t& link);

    /**
     * @brief Get link at index
     * @details Retrieves the child link at the specified index
     * @param index the position of the link to retrieve
     * @returns the child link at the specified index
     */
    virtual uint32_t getLinkAt(const size_t& index) const;

    /**
     * @brief Set link at index
     * @details Sets the child link at the specified index
     * @param index the position of the link to set
     * @param link the child link to set
     */
    virtual void setLinkAt(const size_t& index, const uint32_t& link);

    /**
     * @brief Get link for key
     * @details Retrieves the child link associated with the given key
     * @param key the key to search for
     * @returns the child link associated with the key
     */
    virtual uint32_t getLinkForKey(const uint32_t& key) const;

    /**
     * @brief Remove link at index
     * @details Removes the child link at the specified index
     * @param index the position of the link to remove
     */
    virtual void removeLinkAt(const size_t& index);

    /**
     * @brief Get number of links in node
     * @details Returns the current count of child links in this node
     * @returns the number of child links stored in this node
     */
    virtual size_t getLinkCount() const;

    /**
     * @brief Set page number
     * @details Sets the page number for this node
     * @param pageNum the page number to set
     */
    virtual void setPageNumber(uint32_t pageNum);

    /**
     * @brief Get page number
     * @details Retrieves the page number for this node
     * @returns the page number of this node
     */
    virtual uint32_t getPageNumber() const;

    /**
     * @brief Get right link
     * @details Retrieves the right sibling link for this node
     * @returns the RBN of the right sibling
     */
    virtual void setParentLink(uint32_t link);

    /**
     * @brief Get right link
     * @details Retrieves the right sibling link for this node
     * @returns the RBN of the right sibling
     */
    virtual uint32_t getParentLink() const;


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

    std::vector<uint32_t> links; // Child links (Page Numbers) for index nodes

    uint32_t pageNumber;//page number of this node

    uint32_t parentLink;//page number of parent node

    

    /**
     * @brief Clear all keys
     * @details Removes all keys from the node
     */
    virtual void clearKeys();

};



#endif // NODE_H
