#include "BPlusTreeHeader.h"
#include <cstring>

BPlusTreeHeader::BPlusTreeHeader() : height(0), rootIndexRBN(0) {}

BPlusTreeHeader::~BPlusTreeHeader(){
}

void BPlusTreeHeader::setBlockedFileName(const std::string& inFileName)
{
    this->blockedFileName = inFileName;
}

void BPlusTreeHeader::setHeight(const uint32_t inHeight)
{
    this->height = inHeight;
}

void BPlusTreeHeader::setRootIndexRBN(const uint32_t inRBN)
{
    this->rootIndexRBN = inRBN;
}

std::string BPlusTreeHeader::getBlockedFileName() const
{
    return blockedFileName;
}

uint32_t BPlusTreeHeader::getHeight() const
{
    return height;
}

uint32_t BPlusTreeHeader::getRootIndexRBN() const
{
    return rootIndexRBN;
}

std::vector<uint8_t> BPlusTreeHeader::serialize() const
{
    // Data Vector
    std::vector<uint8_t> data;

    // Create Header Size and Store location since it needs to be calculated
    size_t headerSizePos = data.size();
    uint32_t tempHeaderSize = 0;
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&tempHeaderSize),
                reinterpret_cast<const uint8_t*>(&tempHeaderSize) + sizeof(tempHeaderSize));

    // Blocked Filename Length
    uint16_t filenameLength = blockedFileName.length();
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&filenameLength),
                reinterpret_cast<const uint8_t*>(&filenameLength) + sizeof(filenameLength));

    // Blocked Filename
    data.insert(data.end(), blockedFileName.begin(), blockedFileName.end());

    // Insert Height
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&height),
                reinterpret_cast<const uint8_t*>(&height) + sizeof(height));

    // Insert Root Index RBN
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&rootIndexRBN),
                reinterpret_cast<const uint8_t*>(&rootIndexRBN) + sizeof(rootIndexRBN));

     // Calculate Header Size
    uint32_t trueHeaderSize = data.size();
    memcpy(&data[headerSizePos], &trueHeaderSize, sizeof(trueHeaderSize));
}

BPlusTreeHeader BPlusTreeHeader::deserialize(const uint8_t* data)
{
    BPlusTreeHeader bHeader;
    size_t offset = 0;

    // Read Header Size
    memcpy(&bHeader.headerSize, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Read Blocked File Name Size
    uint16_t fileNameSize;
    memcpy(&fileNameSize, data + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    // Read Blocked File Name
    bHeader.blockedFileName = std::string(reinterpret_cast<const char*>(data + offset), fileNameSize);
    offset += fileNameSize;

    // Read Height
    memcpy(&bHeader.height, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Read Root Index RBN
    memcpy(&bHeader.rootIndexRBN, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    return bHeader;
}