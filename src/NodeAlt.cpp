#include "NodeAlt.h"

NodeAlt::NodeAlt(bool isLeaf, size_t blockSize)
{
    this->isLeaf = isLeaf;
    this->blockSize = blockSize;
    this->maxKeys = calculateMaxKeys(blockSize, isLeaf);
    this->parentRBN = 0;
    this->prevLeafRBN = 0;
    this->nextLeafRBN = 0;
    this->hasError = false;
}

NodeAlt::~NodeAlt()
{

}

uint8_t NodeAlt::isLeafNode() const
{
    return isLeaf;
}

bool NodeAlt::setKeyAt(size_t index, uint32_t key)
{
    if(index >= keys.size())
    {
        setError("Index out of bounds in setKeyAt");
        return false;
    }

    keys[index] = key;
    return true;
}

bool NodeAlt::insertKeyAt(size_t index, uint32_t key)
{
    if(index > keys.size() || isFull())
    {
        setError("Out of bounds or full in setKeyAt");
        return false;
    }

    keys.insert(keys.begin() + index, key);
    return true;
}

bool NodeAlt::removeKeyAt(size_t index)
{
    if(index >= keys.size())
    {
        setError("Index out of bounds in setKeyAt");
        return false;
    }

    keys.erase(keys.begin() + index);
    return true;
}

bool NodeAlt::insertChildRBN(size_t index, uint32_t rbn)
{
    if (index > childRBNs.size() || (!isLeaf && childRBNs.size() >= maxKeys + 1))
    {
        setError("Index out of bounds or too many children in insertChildRBN");
        return false;
    }
    
    childRBNs.insert(childRBNs.begin() + index, rbn);
    return true;
}

bool NodeAlt::removeChildRBN(size_t index)
{
    if(index >= childRBNs.size())
    {
        setError("Index out of bounds in removeChildRBN");
        return false;
    }

    childRBNs.erase(childRBNs.begin() + index);
    return true;
}

bool NodeAlt::isFull() const
{
    return getKeyCount() >= maxKeys;
}

bool NodeAlt::isUnderfull() const
{
    size_t minKeys = (maxKeys + 1) / 2;
    return getKeyCount() < minKeys;
}

bool NodeAlt::setValueAt(size_t index, uint32_t value)
{
    if(index >= values.size())
    {
        setError("Index out of bounds in setValueAt");
        return false;
    }

    values[index] = value;
    return true;
}

bool NodeAlt::insertValueAt(size_t index, uint32_t value)
{
    if(index > values.size() || isFull())
    {
        setError("Out of bounds or full in insertValueAt");
        return false;
    }

    values.insert(values.begin() + index, value);
    return true;
}

bool NodeAlt::removeValueAt(size_t index)
{
    if(index >= values.size())
    {
        setError("Index out of bounds in removeValueAt");
        return false;
    }

    values.erase(values.begin() + index);
    return true;
}

bool NodeAlt::unpack(const std::vector<uint8_t>& data)
{
    if(data.size() < blockSize)
    {
        setError("Buffer too small in node unpack function.");
        return false;
    }

    clear();

    size_t offset = 0;

    // Read Leaf Type
    isLeaf = (data[offset++]);

    // Read Key Count
    uint32_t keyCount;
    memcpy(&keyCount, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Read Parent RBN
    memcpy(&parentRBN, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if(isLeaf == 1)
    {
        // Read Prev Leaf
        memcpy(&prevLeafRBN, data.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Read Next Leaf
        memcpy(&nextLeafRBN, data.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // Read keys and values
        keys.resize(keyCount);
        values.resize(keyCount);

        for(uint32_t i = 0; i < keyCount; ++i)
        {
            memcpy(&keys[i], data.data() + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);

            memcpy(&values[i], data.data() + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
        }
    }
    else
    {
        // Read Keys
        keys.resize(keyCount);
        for(uint32_t i = 0; i < keyCount; ++i)
        {
            memcpy(&keys[i], data.data() + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
        }

        // Read child RBNs
        childRBNs.resize(keyCount + 1);
        for(uint32_t i = 0; i < keyCount + 1; ++i)
        {
            memcpy(&childRBNs[i], data.data() + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
        }
    }

    maxKeys = calculateMaxKeys(blockSize, isLeaf);
    return true;
}

bool NodeAlt::getErrorState() const
{
    return hasError;
}

size_t NodeAlt::getKeyCount() const
{
    return keys.size();
}

size_t NodeAlt::getBlockSize() const
{
    return blockSize;
}

size_t NodeAlt::getMaxKeys() const
{
    return maxKeys;
}

size_t NodeAlt::getChildCount() const
{
    return childRBNs.size();
}

size_t NodeAlt::findChildIndex(uint32_t key) const
{
    if (!isLeaf && keys.empty())
    {
        return 0;
    }
    
    // Find first key >= search key
    for (size_t i = 0; i < keys.size(); ++i)
    {
        if (key < keys[i])
            return i;  // Go to left child
    }
    
    // Key is >= all keys, go to rightmost child
    return keys.size();
}

size_t NodeAlt::calculateMaxKeys(size_t blockSize, bool isLeaf)
{
    // Block structure (maybe placeholder):
    // - 1 byte: node type 
    // - 4 bytes: key count
    // - 4 bytes: parent RBN
    // - Leaf: 8 bytes for prev and next rbn
    // - Index: no extra RBNs
    
    size_t headerSize = 1 + 4 + 4;  // type + count + parent
    
    if (isLeaf)
    {
        headerSize += 8;  // prev + next RBNs
        // Each entry: 4 bytes key + 4 bytes value
        size_t entrySize = 8;
        return (blockSize - headerSize) / entrySize;
    }
    else
    {
        // Each entry: 4 bytes key + 4 bytes child RBN
        // Plus one extra child RBN at the end
        size_t entrySize = 8;
        size_t maxEntries = (blockSize - headerSize - 4) / entrySize;
        if (maxEntries < 1) 
            return 1;
        return maxEntries;
    }
}

uint32_t NodeAlt::getKeyAt(size_t index) const
{
    if(index >= keys.size())
    {
         return uint32_t(-1);
    }
       
    return keys[index];
}

uint32_t NodeAlt::getValueAt(size_t index) const
{
    if(index >= values.size())
    {
         return uint32_t(-1);
    }
    return values[index];
}

uint32_t NodeAlt::getPrevLeafRBN() const
{
    return prevLeafRBN;
}

uint32_t NodeAlt::getNextLeafRBN() const
{
    return nextLeafRBN;
}

uint32_t NodeAlt::getChildRBN(size_t index) const
{
    if(index >= childRBNs.size())
    {
         return uint32_t(-1);
    }
       
    return childRBNs[index];
}

uint32_t NodeAlt::getParentRBN() const
{
    return parentRBN;
}

void NodeAlt::setPrevLeafRBN(uint32_t rbn)
{
    this->prevLeafRBN = rbn;
}

void NodeAlt::setNextLeafRBN(uint32_t rbn)
{
    this->nextLeafRBN = rbn;
}

void NodeAlt::setParentRBN(uint32_t rbn)
{
    this->parentRBN = rbn;
}

void NodeAlt::setChildRBN(size_t index, uint32_t rbn)
{
    childRBNs[index] = rbn;
}

void NodeAlt::setBlockSize(size_t inSize)
{
    this->blockSize = inSize;
}

void NodeAlt::setIsLeaf(uint8_t leaf)
{
    this->isLeaf = leaf;
}

void NodeAlt::clear()
{
    isLeaf = false;
    blockSize = 0;
    maxKeys = 0;
    parentRBN = 0;
    prevLeafRBN = 0;
    nextLeafRBN = 0;
    hasError = false;
    keys.clear();
    values.clear();
    childRBNs.clear();
}

void NodeAlt::pack(std::vector<uint8_t>& data) const
{
    data.clear();
    data.reserve(blockSize);

    // Node Type
    data.push_back(isLeaf);

    // Key Count
    uint32_t keyCount = static_cast<uint32_t>(keys.size());
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&keyCount),
                reinterpret_cast<const uint8_t*>(&keyCount) + sizeof(keyCount));

    // Parent RBN
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&parentRBN),
                reinterpret_cast<const uint8_t*>(&parentRBN) + sizeof(parentRBN));

    // If leaf node write prev and next rbn
    if(isLeaf == 1)
    {
        // Prev Leaf RBN
        data.insert(data.end(), reinterpret_cast<const uint8_t*>(&prevLeafRBN),
            reinterpret_cast<const uint8_t*>(&prevLeafRBN) + sizeof(prevLeafRBN));

        // Next Leaf RBN
        data.insert(data.end(), reinterpret_cast<const uint8_t*>(&nextLeafRBN),
            reinterpret_cast<const uint8_t*>(&nextLeafRBN) + sizeof(nextLeafRBN));

        // Write Keys & Vals
        for(size_t i = 0; i < keys.size(); ++i)
        {
            uint32_t currKey = keys[i];
            uint32_t currVal = values[i];

            data.insert(data.end(), reinterpret_cast<const uint8_t*>(&currKey),
                reinterpret_cast<const uint8_t*>(&currKey) + sizeof(currKey));

            data.insert(data.end(), reinterpret_cast<const uint8_t*>(&currVal),
                reinterpret_cast<const uint8_t*>(&currVal) + sizeof(currVal));
        }
    }
    else
    {
        // Write Keys
        for(uint32_t key : keys)
        {
            data.insert(data.end(), reinterpret_cast<const uint8_t*>(&key),
                reinterpret_cast<const uint8_t*>(&key) + sizeof(key));
        }

        // Write Child RBNs (one more than keyCount)
        for(uint32_t rbn : childRBNs)
        {
            data.insert(data.end(), reinterpret_cast<const uint8_t*>(&rbn),
                reinterpret_cast<const uint8_t*>(&rbn) + sizeof(rbn));
        }
    }

    // Pad to block size
    while(data.size() < blockSize)
    {
        data.push_back(0);
    }
}

void NodeAlt::setError(const std::string& message)
{
    hasError = true;
    errorMessage = message;
}

std::string NodeAlt::getErrorMessage() const
{
    return errorMessage;
}