#ifndef NODE_ALT_H
#define NODE_ALT_H

#include "stdint.h"
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

class NodeAlt
{
public:
    NodeAlt(bool isLeaf, size_t blockSize);
    ~NodeAlt();

    uint8_t isLeafNode() const;

    bool setKeyAt(size_t index, uint32_t key);

    bool insertKeyAt(size_t index, uint32_t key);

    bool removeKeyAt(size_t index);

    bool insertChildRBN(size_t index, uint32_t rbn);

    bool removeChildRBN(size_t index);

    bool isFull() const;

    bool isUnderfull() const;

    bool setValueAt(size_t index, uint32_t value);

    bool insertValueAt(size_t index, uint32_t value);

    bool removeValueAt(size_t index);

    bool unpack(const std::vector<uint8_t>& data);

    bool getErrorState() const;

    size_t getKeyCount() const;

    size_t getBlockSize() const;

    size_t getMaxKeys() const;

    size_t getChildCount() const;

    size_t findChildIndex(uint32_t key) const;

    static size_t calculateMaxKeys(size_t blockSize, bool isLeaf);

    uint32_t getKeyAt(size_t index) const;

    uint32_t getValueAt(size_t index) const;

    uint32_t getPrevLeafRBN() const;

    uint32_t getNextLeafRBN() const;

    uint32_t getChildRBN(size_t index) const;

    uint32_t getParentRBN() const;

    void setPrevLeafRBN(uint32_t rbn);

    void setNextLeafRBN(uint32_t rbn);

    void setParentRBN(uint32_t rbn);

    void setChildRBN(size_t index, uint32_t rbn);

    void setBlockSize(size_t inSize);

    void setIsLeaf(uint8_t leaf);

    void pack(std::vector<uint8_t>& data) const;

    void clear();

    std::string getErrorMessage() const;

private:
    // Common Node Data
    uint8_t isLeaf;
    size_t blockSize;
    std::vector<uint32_t> keys;
    uint32_t parentRBN;
    size_t maxKeys;

    // Leaf Node Data
    std::vector<uint32_t> values;
    uint32_t prevLeafRBN;
    uint32_t nextLeafRBN;

    // Index Node Data
    std::vector<uint32_t> childRBNs;

    // Error Handling
    bool hasError;
    std::string errorMessage;
    void setError(const std::string& message);
};
#endif // NODE_ALT_H