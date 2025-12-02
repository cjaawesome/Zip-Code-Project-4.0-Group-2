#include "LeafBlockNode.h"
#include <stdexcept>


LeafBlockNode::LeafBlockNode() : nextLeafPageNumber(0), prevLeafPageNumber(0) {}


LeafBlockNode::~LeafBlockNode(){

}


bool LeafBlockNode::find(const uint32_t key, uint32_t& outValue) const{
    for (size_t i = 0; i < keys.size(); ++i) {
        if (keys[i] == key) {
            outValue = values[i];
            return true;
        }
    }
    return false;
}


void LeafBlockNode::insertKV(const uint32_t &key, const uint32_t &value){
    if (keys.size() >= MAX_KEYS) {
        throw std::runtime_error("LeafBlock is full, cannot insert new key-value pair");
    }
    keys.push_back(key);
    values.push_back(value);
}


Node* LeafBlockNode::split() {
    //idfk yet
}

