// IndexBlock.h
#ifndef LEAFBLOCK_H
#define LeafBLOCK_H

#include <string>
#include <vector>

const size_t MAX_KEYS = 2; // Example maximum number of keys per index block
const size_t MAX_VALUES = 2; // Example maximum number of values per leaf block

template <typename keyType, typename valueType>
class LeafBlock
{
public:
    /**
     * @brief Default constructor
     * @details Initializes leaf block
     */
    LeafBlock() : nextLeafRBN(0) {}
    /**
     * @brief Destructor
     * @details Cleans up leaf block
     */
    ~LeafBlock() = default;
    /**
     * @brief Find
     * @details Finds the value for a given key
     * @param key the key to search for
     * @returns the value associated with the key
     */
    valueType find(const keyType& key) const;
    /**
     * @brief Insert Key-Value Pair
     * @details Inserts a key-value pair into the leaf block
     * @param key the key to be inserted
     * @param value the value to be inserted
     */
    void insertKV(const keyType &key, const valueType &value);
    /**
     * @brief Split
     * @details Splits the leaf block into two
     * @returns a new LeafBlock containing half the keys and values
     */
    LeafBlock split();
    


private:
    std::Vector<keyType> keys;
    std::Vector<valueType> values;
    uint32_t nextLeafRBN;
    uint32_t prevLeafRBN;
};

#include "LeafBlock.tpp"

#endif // INDEXBLOCK_H