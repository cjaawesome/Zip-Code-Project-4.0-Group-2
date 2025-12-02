#include "Node.h"

template <typename keyType>
Node<keyType>::Node() {}

template <typename keyType>
bool Node<keyType>::isFull() const {
    return keys.size() >= MAX_KEYS;
}

template <typename keyType>
bool Node<keyType>::isEmpty() const {
    return keys.empty();
}

template <typename keyType>
size_t Node<keyType>::getKeyCount() const {
    return keys.size();
}

template <typename keyType>
keyType Node<keyType>::getKeyAt(size_t index) const {
    if (index >= keys.size()) {
        throw std::out_of_range("Key index out of range");
    }
    return keys[index];
}

template <typename keyType>
void Node<keyType>::clearKeys() {
    keys.clear();
}

template <typename keyType>
void Node<keyType>::addKey(const keyType& key) {
    keys.push_back(key);
}

template <typename keyType>
void Node<keyType>::removeKeyAt(size_t index) {
    if (index >= keys.size()) {
        throw std::out_of_range("Key index out of range");
    }
    keys.erase(keys.begin() + index);
}
