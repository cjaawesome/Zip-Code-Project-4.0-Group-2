#include "IndexBlock.h"


template<typename keyType, typename valueType>
void IndexBlock<keyType, valueType>::addKey(const keyType& key) {
    keys.push_back(key);
}

template<typename keyType, typename valueType>
void IndexBlock<keyType, valueType>::addChildRBN(const keyType& rbn) {
    childrenRBNs.push_back(rbn);
}

template<typename keyType, typename valueType>
size_t IndexBlock<keyType, valueType>::findChild(const keyType& key) const {
    for (size_t i = 0; i < keys.size(); ++i) {
        if (key < keys[i]) {
            return i;
        }
    }
    return keys.size();
}

template<typename keyType, typename valueType>
IndexBlock<keyType, valueType> IndexBlock<keyType, valueType>::split() {
    IndexBlock<keyType, valueType> newIndexBlock;
    size_t midIndex = keys.size() / 2;

    newIndexBlock.keys.assign(keys.begin() + midIndex, keys.end());
    newIndexBlock.childrenRBNs.assign(childrenRBNs.begin() + midIndex + 1, childrenRBNs.end());

    keys.resize(midIndex);
    childrenRBNs.resize(midIndex + 1);

    return newIndexBlock;
}