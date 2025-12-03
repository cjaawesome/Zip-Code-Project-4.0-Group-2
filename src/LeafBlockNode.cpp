#include "LeafBlockNode.h"
#include <stdexcept>


LeafBlockNode::LeafBlockNode() {}


LeafBlockNode::~LeafBlockNode(){}


bool LeafBlockNode::find(const uint32_t key, uint32_t& outValue) const {
    for (size_t i = 0; i < keys.size(); ++i) {
        if (keys[i] == key) {
            outValue = values[i];
            return true;
        }
    }
    return false;
}

void LeafBlockNode::insertKV(const uint32_t &key, const uint32_t &value) {
    for (size_t i = 0; i < keys.size(); ++i) {
        if (key == keys[i]) {
            values[i] = value; // Update existing key
            return;
        } else if (key < keys[i]) {
            keys.insert(keys.begin() + i, key);
            values.insert(values.begin() + i, value);
            return;
        }
    }
    keys.push_back(key);   // Append at the end if it's the largest key
    values.push_back(value);
}

void LeafBlockNode::setNextLeafPageNumber(int32_t pageNumber) {
    Node::setRightLink(pageNumber);
}

int32_t LeafBlockNode::getNextLeafPageNumber() const {
    return Node::getRightLink();
}

void LeafBlockNode::setPrevLeafPageNumber(int32_t pageNumber) {
    Node::setLeftLink(pageNumber);
}

int32_t LeafBlockNode::getPrevLeafPageNumber() const {
    return Node::getLeftLink();
}

Node* LeafBlockNode::split() {
    //that splitty split shit
}

bool LeafBlockNode::isLeafNode() const {
    return true;
}