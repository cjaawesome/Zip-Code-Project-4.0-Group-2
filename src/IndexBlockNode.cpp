#include "IndexBlockNode.h"



void IndexBlockNode::addKey(const uint32_t& key) {
    keys.push_back(key);
}


void IndexBlockNode::addChildPageNumber(const uint32_t& rbn) {
    //refactor later
}


size_t IndexBlockNode::findChild(const uint32_t& key) const {
    for (size_t i = 0; i < keys.size(); ++i) {
        if (key < keys[i]) {
            return i;
        }
    }
    return keys.size();
}


Node* IndexBlockNode::split() {
    //idfk yet
}