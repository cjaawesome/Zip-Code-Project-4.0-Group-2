#include "Node.h"
#include <cstdint>
#include <stdexcept>


Node::Node(){}

Node::~Node(){}


uint32_t Node::getKeyAt(const size_t& index) const {
    if (index >= keys.size()) {
        throw std::out_of_range("Key index out of range");
    }
    return keys[index];
}

uint32_t Node::getLinkAt(const size_t& index) const {
    if (index >= links.size()) {
        throw std::out_of_range("Link index out of range");
    }
    return links[index];
}
bool Node::setKeyAt(const size_t& index, const uint32_t& key) {
    if (index >= keys.size()) {
        return false;
    }
    keys[index] = key;
    return true;
}
bool Node::setLinkAt(const size_t& index, const uint32_t& link) {
    if (index >= links.size()) {
        return false;
    }
    links[index] = link;
    return true;
}
bool Node::addKey(const uint32_t& key) {
    keys.push_back(key);
    return true;
}
bool Node::addLink(const uint32_t& link) {
    links.push_back(link);
    return true;
}
size_t Node::getKeyCount() const {
    return keys.size();
}
size_t Node::getLinkCount() const {
    return links.size();
}
