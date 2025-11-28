#ifndef BPLUSTREE_H
#define BPLUSTREE_H
#include "IndexBlock.h"
#include "LeafBlock.h"
#include "HeaderRecord.h"
#include "BlockBuffer.h"
#include <string>

template <typename keyType, typename valueType>
class BPlusTree {
public:
    BPlusTree() : rootIndexRBN(0) {}
    ~BPlusTree() = default;

    void insert(const keyType& key, const valueType& value);
    valueType search(const keyType& key) const;
    bool open();
    void close();
private:
    std::string indexFileName;
    HeaderRecord headerRecord;
    BlockBuffer blockBuffer;
    uint32_t rootIndexRBN;
};

#include "BPlusTree.tpp"

#endif // BPLUSTREE_H