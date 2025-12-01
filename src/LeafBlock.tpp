#include "LeafBlock.h"

template <typename keyType, typename valueType>
LeafBlock<keyType, valueType>::LeafBlock() : nextLeafRBN(0), prevLeafRBN(0) {}

template <typename keyType, typename valueType>
LeafBlock<keyType, valueType>::~LeafBlock(){

}

template <typename keyType, typename valueType>
valueType LeafBlock<keyType, valueType>::find(const keyType& key) const{
    for (size_t i = 0; i < keys.size(); ++i) {
        if (keys[i] == key) {
            return values[i];
        }
    }
    throw std::runtime_error("Key not found");
}

template <typename keyType, typename valueType>
void LeafBlock<keyType, valueType>::insertKV(const keyType &key, const valueType &value){
    if (keys.size() >= MAX_KEYS) {
        throw std::runtime_error("LeafBlock is full, cannot insert new key-value pair");
    }
    keys.push_back(key);
    values.push_back(value);
}

template <typename keyType, typename valueType>
LeafBlock<keyType, valueType> LeafBlock<keyType, valueType>::split(){
    LeafBlock<keyType, valueType> newLeaf;
    size_t midIndex = keys.size() / 2;

    newLeaf.keys.assign(keys.begin() + midIndex, keys.end());
    newLeaf.values.assign(values.begin() + midIndex, values.end());

    keys.resize(midIndex);
    values.resize(midIndex);

    return newLeaf;
}

