#include "IndexBlockNode.h"
#include <stdexcept>


bool IndexBlockNode::addKey(const uint32_t &key) {
    if(getKeyCount() >= MAX_KEYS) {
        return false; // Node is full
    }
    return Node::addKey(key);
}

void IndexBlockNode::addChildPageNumber(const uint32_t &pageNumber) {
    if(getLinkCount() > Node::getKeyCount() + 1) {
        throw std::runtime_error("IndexBlockNode cannot have more than Key Count + 1 children");
    }
    Node::addLink(pageNumber);
}

size_t IndexBlockNode::findChild(const uint32_t &key) const {
    //probably need to change this to make sure the right child is found because if new child is insered it might mess up the order
    
    if(Node::getKeyCount() == 1){
        if(key < Node::getKeyAt(0)){
            return Node::getLinkAt(0);
        }
        else{
            return Node::getLinkAt(1);
        }
    }
    else if(Node::getKeyCount() == 2){
        if(key < Node::getKeyAt(0)){
            return Node::getLinkAt(0);
        }
        else if(key >= Node::getKeyAt(0) && key < Node::getKeyAt(1)){
            return Node::getLinkAt(1);
        }
        else{
            return Node::getLinkAt(2);
        }
    }
}

Node* IndexBlockNode::split() {
    IndexBlockNode* newIndexBlock = new IndexBlockNode();
    size_t midIndex = getKeyCount() / 2;

    // Move half the keys to the new index block
    for (size_t i = midIndex + 1; i < getKeyCount(); ++i) {
        newIndexBlock->addKey(keys[i]);
    }

    // Move half the child links to the new index block
    for (size_t i = midIndex + 1; i < getLinkCount(); ++i) {
        newIndexBlock->addChildPageNumber(links[i]);
    }

    // Remove moved keys and links from the current block
    while (getKeyCount() > midIndex) {
        removeKeyAt(midIndex);
    }
    while (getLinkCount() > midIndex + 1) {
        removeLinkAt(midIndex + 1);
    }

    return newIndexBlock;
}

bool IndexBlockNode::isLeafNode() const {
    return false;
}
