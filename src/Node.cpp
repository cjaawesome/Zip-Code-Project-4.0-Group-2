#include "Node.h"
#include <cstdint>
#include <stdexcept>


Node::Node() : parentLink(0) {}

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

void Node::addLink(const uint32_t& link) {
    links.push_back(link);
}

uint32_t Node::getLinkAt(const size_t& index) const {
    if (index >= links.size()) {
        throw std::out_of_range("Link index out of range");
    }
    return links[index];
}

void Node::setLinkAt(const size_t& index, const uint32_t& link) {
    if (index >= links.size()) {
        throw std::out_of_range("Link index out of range");
    }
    links[index] = link;
}

uint32_t Node::getLinkForKey(const uint32_t& key) const {
    for (size_t i = 0; i < keys.size(); ++i) {
        if (key < keys[i]) {
            return links[i];
        }
    }
    return links.back(); // Return last link if key is greater than all keys
}

void Node::removeLinkAt(const size_t& index) {
    if (index >= links.size()) {
        throw std::out_of_range("Link index out of range");
    }
    links.erase(links.begin() + index);
}

size_t Node::getLinkCount() const {
    return links.size();
}

void Node::setPageNumber(uint32_t pageNum) {
    pageNumber = pageNum;
}

uint32_t Node::getPageNumber() const {
    return pageNumber;
}

uint32_t Node::getParentLink() const {
    return parentLink;
}
void Node::setParentLink(uint32_t link) {
    parentLink = link;
}