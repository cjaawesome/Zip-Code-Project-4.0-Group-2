#include "IndexBlockNode.h"
#include <stdexcept>

IndexBlockNode::IndexBlockNode() : Node(), middleLink(0) {}

IndexBlockNode::~IndexBlockNode(){}

bool IndexBlockNode::addKey(const uint32_t &key) {
    if(keys.size() >= MAX_KEYS) {
        return false; // Index block is full
    }
    return Node::addKey(key);
}

void IndexBlockNode::setMiddleLink(uint32_t link) {
    middleLink = link;
}

uint32_t IndexBlockNode::getMiddleLink() const {
    return middleLink;
}

void IndexBlockNode::setRightLink(uint32_t link) {
    Node::setRightLink(link);
}

uint32_t IndexBlockNode::getRightLink() const {
    return Node::getRightLink();
}

void IndexBlockNode::setLeftLink(uint32_t link) {
    Node::setLeftLink(link);
}

uint32_t IndexBlockNode::getLeftLink() const {
    return Node::getLeftLink();
}

size_t IndexBlockNode::findChild(const uint32_t &key) const {
    for (size_t i = 0; i < keys.size(); ++i) {
        if (key < keys[i]) {
            return i;
        }
    }
    return keys.size(); // Return last child if key is greater than all keys
}

Node* IndexBlockNode::split() {
    //that splitty split shit
}

bool IndexBlockNode::isLeafNode() const {
    return false;
}