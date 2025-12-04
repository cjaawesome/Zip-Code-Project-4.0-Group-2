#include "LeafBlockNode.h"
#include <stdexcept>


LeafBlockNode::LeafBlockNode() : Node() {
    Node::addLink(0); // Left sibling link
    Node::addLink(0); // Right sibling link
}


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
    if(keys.size() > MAX_KEYS) {
        split();
    }
}

void LeafBlockNode::setNextLeafPageNumber(uint32_t pageNumber) {
    Node::setLinkAt(1, pageNumber);
}

uint32_t LeafBlockNode::getNextLeafPageNumber() const {
    return Node::getLinkAt(1);
}

void LeafBlockNode::setPrevLeafPageNumber(uint32_t pageNumber) {
    Node::setLinkAt(0, pageNumber);
}

uint32_t LeafBlockNode::getPrevLeafPageNumber() const {
    return Node::getLinkAt(0);
}

Node* LeafBlockNode::split() {
    if(parentLink == 0) {
        
    }
    else {

    }
}

bool LeafBlockNode::isLeafNode() const {
    return true;
}
void LeafBlockNode::setParentLink(uint32_t link) {
    Node::setParentLink(link);
}
uint32_t LeafBlockNode::getParentLink() const {
    return Node::getParentLink();
}