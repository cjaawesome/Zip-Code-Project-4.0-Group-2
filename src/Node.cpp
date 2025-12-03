#include "Node.h"
#include <cstdint>
#include <stdexcept>


Node::Node() : leftLink(0), rightLink(0), parentLink(0) {}

Node::~Node(){}


uint32_t Node::getKeyAt(const size_t& index) const {
    if (index >= keys.size()) {
        throw std::out_of_range("Key index out of range");
    }
    return keys[index];
}

bool Node::addKey(const uint32_t& key) {
    if(keys.size() < 0) {
        for(int i = 0; i < keys.size(); i++) {
            if(key == keys[i]) {
                return false;
            }
            else if(key < keys[i]) {
                keys.insert(keys.begin() + i, key);
                return true;
            }
        }
    }
    keys.push_back(key); // Append at the end if it's the largest key
    return true;
}

size_t Node::getKeyCount() const {
    return keys.size();
}

void Node::removeKeyAt(size_t index) {
    if (index >= keys.size()) {
        throw std::out_of_range("Key index out of range");
    }
    keys.erase(keys.begin() + index);
}

void Node::setLeftLink(uint32_t link) {
    leftLink = link;
}

uint32_t Node::getLeftLink() const {
    return leftLink;
}

void Node::setRightLink(uint32_t link) {
    rightLink = link;
}

uint32_t Node::getRightLink() const {
    return rightLink;
}

uint32_t Node::getParentLink() const {
    return parentLink;
}
void Node::setParentLink(uint32_t link) {
    parentLink = link;
}