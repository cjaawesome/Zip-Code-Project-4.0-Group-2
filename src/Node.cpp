#include "Node.h"
#include <cstdint>
#include <stdexcept>

Node::Node() {}


bool Node::isFull() const {
    return keys.size() >= MAX_KEYS;
}


bool Node::isEmpty() const {
    return keys.empty();
}


size_t Node::getKeyCount() const {
    return keys.size();
}


uint32_t Node::getKeyAt(size_t index) const {
    if (index >= keys.size()) {
        throw std::out_of_range("Key index out of range");
    }
    return keys[index];
}


void Node::clearKeys() {
    keys.clear();
}


void Node::addKey(const uint32_t& key) {
    keys.push_back(key);
}


void Node::removeKeyAt(size_t index) {
    if (index >= keys.size()) {
        throw std::out_of_range("Key index out of range");
    }
    keys.erase(keys.begin() + index);
}
