#ifndef BPLUSTREEHEADERALT_H
#define BPLUSTREEHEADERALT_H
#include "NodeAlt.h"
#include "HeaderRecord.h"
#include "BlockBuffer.h"
#include <string>
#include <vector>

class BPlusTreeHeaderAlt
{
public:
    /**
     * @brief Default constructor
     * @details Initializes BPlusTreeHeader
     */
    BPlusTreeHeaderAlt() = default;
    /**
     * @brief Destructor
     * @details Cleans up BPlusTreeHeader
     */
    ~BPlusTreeHeaderAlt() = default;

    /**
     * @brief Serialize
     * @details Serializes the BPlusTreeHeader into a byte vector for storage
     * @returns a vector of bytes representing the serialized header
     */
    std::vector<uint8_t> serialize() const;
    /**
     * @brief Deserialize
     * @details Deserializes a byte array into a BPlusTreeHeader object
     * @param data the byte array containing the serialized header
     * @returns a BPlusTreeHeader object constructed from the byte array
     */
    static BPlusTreeHeaderAlt deserialize(const uint8_t* data);

    /**
     * @brief Set Blocked File Name
     * @details Sets the name of the blocked file
     * @param filename the name of the blocked file
     */
    void setBlockedFileName(const std::string& filename);
    /**
     * @brief Get Blocked File Name
     * @details Gets the name of the blocked file
     * @returns the name of the blocked file
     */
    std::string getBlockedFileName() const;
    /**
     * @brief Set Height
     * @details Sets the height of the B+ tree
     * @param h the height to set
     */
    void setHeight(const uint32_t h);
    /**
     * @brief Get Height
     * @details Gets the height of the B+ tree
     * @returns the height of the B+ tree
     */
    uint32_t getHeight() const;
    /**
     * @brief Set Root Index RBN
     * @details Sets the RBN of the root index block
     * @param rbn the RBN to set
     */
    void setRootIndexRBN(const uint32_t rbn);
    /**
     * @brief Get Root Index RBN
     * @details Gets the RBN of the root index block
     * @returns the RBN of the root index block
     */
    uint32_t getRootIndexRBN() const;
    /**
     * @brief Set Header Size
     * @details Sets the size of the header
     * @param inHeaderSize the size to set
     */
    void setHeaderSize(const uint32_t inHeaderSize);
    /**
     * @brief Get Header Size
     * @details Gets the size of the header
     * @returns the size of the header
     */
    uint32_t getHeaderSize() const;

    /**
     * @brief Set Index Start RBN
     * @details Sets the starting RBN for index blocks
     * @param rbn the RBN to set
     */
    void setIndexStartRBN(const uint32_t rbn);

    /**
     * @brief Get Index Start RBN
     * @details Gets the starting RBN for index blocks
     * @returns the starting RBN for index blocks
     */
    uint32_t getIndexStartRBN() const;

    /**
     * @brief Set Index Block Count
     * @details Sets the number of index blocks allocated
     * @param count the count to set
     */
    void setIndexBlockCount(const uint32_t count);

    /**
     * @brief Get Index Block Count
     * @details Gets the number of index blocks allocated
     * @returns the number of index blocks allocated
     */
    uint32_t getIndexBlockCount() const;

    /**
     * @brief Set Block Size
     * @details Sets the block size for the B+ tree
     * @param size the block size to set
     */
    void setBlockSize(const uint32_t size);
    /**
     * @brief Get Block Size
     * @details Gets the block size for the B+ tree
     * @returns the block size for the B+ tree
     */
    uint32_t getBlockSize() const;

private:
    std::string blockedFileName; // Name of the .sequence set file
    uint32_t headerSize; // Header size in bytes
    uint32_t height; // Height of B+ tree
    uint32_t rootIndexRBN; // RBN of root node
    uint32_t indexStartRBN; // First RBN used for index blocks
    uint32_t indexBlockCount; // Number of index blocks allocated
    uint32_t blockSize; // Block size (must match sequence set)
};





#endif // BPLUSTREEHEADERALT_H