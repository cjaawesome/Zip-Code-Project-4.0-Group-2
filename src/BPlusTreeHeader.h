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
    BPlusTreeHeader() = default;
    ~BPlusTreeHeader() = default;

    std::vector<uint8_t> serialize() const;
    static BPlusTreeHeader deserialize(const uint8_t* data);

    void setBlockedFileName(const std::string& filename);
    std::string getBlockedFileName() const;

    void setHeight(const uint32_t h);
    uint32_t getHeight() const;

    void setRootIndexRBN(const uint32_t rbn);
    uint32_t getRootIndexRBN() const;

    void setHeaderSize(const uint32_t inHeaderSize);
    uint32_t getHeaderSize() const;


private:
    std::string blockedFileName;
    uint32_t height;
    uint32_t rootIndexRBN;
    uint32_t headerSize;
};





#endif // BPLUSTREEHEADER_H