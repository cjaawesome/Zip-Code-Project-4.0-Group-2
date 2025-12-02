#include "IndexBlockNode.h"


template<typename keyType, typename valueType>
void IndexBlockNode<keyType, valueType>::addKey(const keyType& key) {
    keys.push_back(key);
}

template<typename keyType, typename valueType>
void IndexBlockNode<keyType, valueType>::addChildRBN(const keyType& rbn) {
    childrenRBNs.push_back(rbn);
}

template<typename keyType, typename valueType>
size_t IndexBlockNode<keyType, valueType>::findChild(const keyType& key) const {
    for (size_t i = 0; i < keys.size(); ++i) {
        if (key < keys[i]) {
            return i;
        }
    }
    return keys.size();
}

template<typename keyType, typename valueType>
IndexBlockNode<keyType, valueType> IndexBlockNode<keyType, valueType>::split() {
    IndexBlockNode<keyType, valueType> newIndexBlock;
    size_t midIndex = keys.size() / 2;

    newIndexBlock.keys.assign(keys.begin() + midIndex, keys.end());
    newIndexBlock.childrenRBNs.assign(childrenRBNs.begin() + midIndex + 1, childrenRBNs.end());

    keys.resize(midIndex);
    childrenRBNs.resize(midIndex + 1);

    return newIndexBlock;
}