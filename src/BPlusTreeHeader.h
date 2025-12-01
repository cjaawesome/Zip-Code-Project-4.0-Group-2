#ifndef BPLUSTREEHEADER_H
#define BPLUSTREEHEADER_H
#include "IndexBlock.h"
#include "LeafBlock.h"
#include "HeaderRecord.h"
#include "BlockBuffer.h"
#include <string>
#include <vector>

class BPlusTreeHeader 
{
public:
    /**
     * @brief Default constructor
     * @details Initializes BPlusTreeHeader
     */
    BPlusTreeHeader() = default;
    /**
     * @brief Destructor
     * @details Cleans up BPlusTreeHeader
     */
    ~BPlusTreeHeader() = default;

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
    static BPlusTreeHeader deserialize(const uint8_t* data);

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


private:
    std::string blockedFileName;
    uint32_t height;
    uint32_t rootIndexRBN;
    uint32_t headerSize;
};





#endif // BPLUSTREEHEADER_H