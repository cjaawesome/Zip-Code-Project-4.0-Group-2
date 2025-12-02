#include "LeafBlockNode.h"

template <typename keyType, typename valueType>
LeafBlockNode<keyType, valueType>::LeafBlockNode() : nextLeafRBN(0), prevLeafRBN(0) {}

template <typename keyType, typename valueType>
LeafBlockNode<keyType, valueType>::~LeafBlockNode(){

}

template <typename keyType, typename valueType>
bool LeafBlockNode<keyType, valueType>::find(const keyType& key, valueType& outValue) const{
    for (size_t i = 0; i < keys.size(); ++i) {
        if (keys[i] == key) {
            outValue = values[i];
            return true;
        }
    }
    return false;
}

template <typename keyType, typename valueType>
void LeafBlockNode<keyType, valueType>::insertKV(const keyType &key, const valueType &value){
    if (keys.size() >= MAX_KEYS) {
        throw std::runtime_error("LeafBlock is full, cannot insert new key-value pair");
    }
    keys.push_back(key);
    values.push_back(value);
}

template <typename keyType, typename valueType>
LeafBlockNode<keyType, valueType> LeafBlockNode<keyType, valueType>::split(){
    LeafBlockNode<keyType, valueType> newLeaf;
    size_t midIndex = keys.size() / 2;

    newLeaf.keys.assign(keys.begin() + midIndex, keys.end());
    newLeaf.values.assign(values.begin() + midIndex, values.end());

    newLeaf.nextLeafRBN = this->nextLeafRBN; // New leaf's next RBN is current leaf's next RBN
    this->nextLeafRBN = newLeaf.values.begin() + midIndex - 1; // Update next leaf RBN to point to new leaf
    newLeaf.prevLeafRBN = values.begin(); // New leaf's previous RBN is current leaf's RBN

    
    keys.resize(midIndex);
    values.resize(midIndex);

    

    return newLeaf;
}

