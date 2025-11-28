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
