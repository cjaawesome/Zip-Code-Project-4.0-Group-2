#ifndef BPLUSTREEHEADER_H
#define BPLUSTREEHEADER_H
#include "IndexBlock.h"
#include "LeafBlock.h"
#include "HeaderRecord.h"
#include "BlockBuffer.h"
#include <string>
#include <vector>

class BPlusTreeHeader {
public:
    BPlusTreeHeader() = default;
    ~BPlusTreeHeader() = default;

    std::vector<uint8_t> serialize() const;
    static BPlusTreeHeader deserialize(const uint8_t* data);

    void setIndexFileName(const std::string& filename);
    std::string getIndexFileName() const;

    void setHeight(uint32_t h);
    uint32_t getHeight() const;

    void setRootIndexRBN(uint32_t rbn);
    uint32_t getRootIndexRBN() const;

private:
    std::string indexFileName;
    uint32_t height;
    uint32_t rootIndexRBN;
};





#endif // BPLUSTREEHEADER_H