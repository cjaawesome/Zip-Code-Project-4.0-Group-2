#include "BPlusTreeHeaderAlt.h"

BPlusTreeHeaderAlt::BPlusTreeHeaderAlt() : blockedFileName(""), height(0), rootIndexRBN(0),
    headerSize(0), indexStartRBN(0), indexBlockCount(0), blockSize(0)
{
}

BPlusTreeHeaderAlt::~BPlusTreeHeaderAlt()
{
}

void BPlusTreeHeaderAlt::setBlockedFileName(const std::string& inFileName)
{
    this->blockedFileName = inFileName;
}

void BPlusTreeHeaderAlt::setHeaderSize(const uint32_t inHeaderSize)
{
    this->headerSize = inHeaderSize;
}

void BPlusTreeHeaderAlt::setHeight(const uint32_t inHeight)
{
    this->height = inHeight;
}

void BPlusTreeHeaderAlt::setRootIndexRBN(const uint32_t inRBN)
{
    this->rootIndexRBN = inRBN;
}

void BPlusTreeHeaderAlt::setIndexStartRBN(const uint32_t rbn)
{
    this->indexStartRBN = rbn;
}

void BPlusTreeHeaderAlt::setIndexBlockCount(const uint32_t count)
{
    this->indexBlockCount = count;
}

void  BPlusTreeHeaderAlt::setBlockSize(const uint32_t size)
{
    this->blockSize = size;
}

std::string BPlusTreeHeaderAlt::getBlockedFileName() const
{
    return blockedFileName;
}

uint32_t BPlusTreeHeaderAlt::getHeaderSize() const
{
    return headerSize;
}

uint32_t BPlusTreeHeaderAlt::getHeight() const
{
    return height;
}

uint32_t BPlusTreeHeaderAlt::getRootIndexRBN() const
{
    return rootIndexRBN;
}

uint32_t BPlusTreeHeaderAlt::getIndexStartRBN() const
{
    return indexStartRBN;
}

uint32_t  BPlusTreeHeaderAlt::getIndexBlockCount() const
{
    return indexBlockCount;
}

uint32_t  BPlusTreeHeaderAlt::getBlockSize() const
{
    return blockSize;
}

std::vector<uint8_t> BPlusTreeHeaderAlt::serialize()
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

    // Insert Index Start RBN
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&indexStartRBN),
                reinterpret_cast<const uint8_t*>(&indexStartRBN) + sizeof(indexStartRBN));

    // Insert Index Block Count
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&indexBlockCount),
                reinterpret_cast<const uint8_t*>(&indexBlockCount) + sizeof(indexBlockCount));

    // Block Size
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&blockSize),
                reinterpret_cast<const uint8_t*>(&blockSize) + sizeof(blockSize));

     // Calculate Header Size
    uint32_t trueHeaderSize = data.size();
    memcpy(&data[headerSizePos], &trueHeaderSize, sizeof(trueHeaderSize));

    return data;
}

BPlusTreeHeaderAlt BPlusTreeHeaderAlt::deserialize(const uint8_t* data)
{
    BPlusTreeHeaderAlt bHeader;
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

    // Read Index Start RBN
    memcpy(&bHeader.indexStartRBN, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Read Index Block Count
    memcpy(&bHeader.indexBlockCount, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Read Block Size
    memcpy(&bHeader.blockSize, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    return bHeader;
}