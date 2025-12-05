#include "BplusTreeAlt.h"

BPlusTreeAlt::BPlusTreeAlt() : isOpen(false), errorState(false), errorMessage("") 
{
}

BPlusTreeAlt::~BPlusTreeAlt() 
{
    if (isOpen) 
    {
        close();
    }
}

bool BPlusTreeAlt::open(const std::string& indexFileName, const std::string& sequenceSetFilename)
{
    HeaderBuffer headerBuffer;
    BPlusTreeHeaderBufferAlt bPlusTreeHeaderBuffer;

    // Open and read sequence set header
    if (!headerBuffer.readHeader(sequenceSetFilename, sequenceHeader)) 
    {
        setError("Failed to read sequence set header");
        return false;
    }

    // Open and read B+ tree header
    if (!bPlusTreeHeaderBuffer.readHeader(indexFileName, treeHeader))
    {
        setError("Failed to read B+ tree header");
        return false;
    }

    // Open index page buffer
    if (!indexPageBuffer.open(indexFileName, treeHeader.getBlockSize(), treeHeader.getHeaderSize())) 
    {
        setError("Failed to open index page buffer");
        return false;
    }

    // Open sequence set buffer
    if (!sequenceSetBuffer.openFile(sequenceSetFilename, sequenceHeader.getHeaderSize())) 
    {
        setError("Failed to open sequence set buffer");
        return false;
    }

    sequenceHeaderSize = sequenceHeader.getHeaderSize();
    blockSize = sequenceHeader.getBlockSize();
    isOpen = true;

    return true;
}

bool BPlusTreeAlt::isFileOpen() const
{
    return isOpen;
}

bool BPlusTreeAlt::hasError() const
{
    return errorState;
}

bool BPlusTreeAlt::getIsStale() const
{
    return isStale;
}

std::string BPlusTreeAlt::getLastError() const
{
    return errorMessage;
}

void BPlusTreeAlt::close()
{
    if (!isOpen)
        return;
    
    BPlusTreeHeaderBufferAlt headerBuffer;
    headerBuffer.writeHeader(indexPageBuffer.getFileStream(), treeHeader);
    
    sequenceSetBuffer.closeFile();
    indexPageBuffer.closeFile();
    isOpen = false;
}

void BPlusTreeAlt::setError(const std::string& message)
{
    errorState = true;
    errorMessage = message;
}

NodeAlt* BPlusTreeAlt::loadNode(uint32_t rbn)
{
    // Create Data Buffer
    std::vector<uint8_t> buffer;
    // Try to read the block
    if(!indexPageBuffer.readBlock(rbn, buffer))
    {
        return nullptr;
    }
    // Create temp node if block read
    NodeAlt* node = new NodeAlt(false, blockSize);
    // Unpack the data into the new block
    if(!node->unpack(buffer))
    {
        delete node;
        return nullptr;
    }
    // Return the new block
    return node;
}

bool BPlusTreeAlt::writeNode(uint32_t rbn, const NodeAlt& node)
{
    // Create Data Vector
    std::vector<uint8_t> data;
    // Pack The Node
    node.pack(data);
    // Write The Node
    return indexPageBuffer.writeBlock(rbn, data);
}

uint32_t BPlusTreeAlt::allocateTreeBlock()
{
    // Get Updated Index Block Count
    uint32_t newRBN = treeHeader.getIndexBlockCount() + 1;
    treeHeader.setIndexBlockCount(newRBN);
    return newRBN;
}

bool BPlusTreeAlt::search(uint32_t key, uint32_t& outValue)
{
    if(!isOpen)
        return false;
    // Find RBN For Leaf
    uint32_t leafRBN = findLeafRBN(key);
    // Load Leaf Node
    NodeAlt* leaf = loadNode(leafRBN);
    // Check For NULL
    if(leaf == nullptr)
        return false;
    for(size_t i = 0; i < leaf->getKeyCount(); ++i)
    {
        // Find Block Containing Key
        if(key <= leaf->getKeyAt(i))
        {
            outValue = leaf->getValueAt(i);
            delete leaf;
            return true;
        }   
    }
    delete leaf;
    return false;
}

uint32_t BPlusTreeAlt::findLeafRBN(uint32_t key)
{
    uint32_t currentRBN = treeHeader.getRootIndexRBN();
    
    uint32_t maxHeight = treeHeader.getHeight() + 5;
    size_t safetyCount = 0;

    while(safetyCount < maxHeight)
    {
        NodeAlt* node = loadNode(currentRBN);
        
        if (node == nullptr)
        {
            setError("Failed to load node at RBN: " + std::to_string(currentRBN));
            return 0;
        }

        if(node->isLeafNode() == 1)
        {
            uint32_t result = currentRBN;
            delete node;
            return result;
        }
        
        // Find child node to descend to
        size_t childIndex = node->findChildIndex(key);
        currentRBN = node->getChildRBN(childIndex);
        delete node;
        
        ++safetyCount;
    }
    
    setError("Tree traversal exceeded maximum height.");
    return 0;
}

bool BPlusTreeAlt::buildFromSequenceSet()
{
    if(!isOpen)
    {
        setError("Buffers not open.");
        return false;
    }

    std::vector<IndexEntry> entries;

    uint32_t currentRBN = sequenceHeader.getSequenceSetListRBN();
    while(currentRBN != 0)
    {
        ActiveBlock block = sequenceSetBuffer.loadActiveBlockAtRBN(currentRBN, blockSize, sequenceHeaderSize);
        std::vector<ZipCodeRecord> records;
        sequenceSetBuffer.unpackBlockAPI(block.data, records);

        if(!records.empty())
        {
            uint32_t highestKey = records.back().getZipCode();
            IndexEntry entry = {highestKey, currentRBN};
            entries.push_back(entry);
        }
        currentRBN = block.succeedingRBN;
    }

    bool result = buildTreeFromEntries(entries);

    return result;
}

bool BPlusTreeAlt::buildTreeFromEntries(const std::vector<IndexEntry>& entries)
{
    if(entries.empty())
    {
        setError("No entries to build tree from.");
        return false;
    }
    std::vector<uint32_t> currentLevel = buildLeafLevel(entries);
    treeHeader.setHeight(1);

    while(currentLevel.size() > 1)
    {
        currentLevel = buildIndexLevel(currentLevel);
        treeHeader.setHeight(treeHeader.getHeight() + 1);
    }

    treeHeader.setRootIndexRBN(currentLevel[0]);

    return true;
}

std::vector<uint32_t> BPlusTreeAlt::buildLeafLevel(const std::vector<IndexEntry>& entries)
{
    std::vector<uint32_t> leafRBNs;
    size_t maxKeys = NodeAlt::calculateMaxKeys(blockSize, true);

    for (size_t i = 0; i < entries.size(); i += maxKeys)
    {
        NodeAlt leaf(true, blockSize);

        for(size_t j = 0; j < maxKeys && (i + j) < entries.size(); ++j)
        {
            leaf.insertKeyAt(j, entries[i + j].key);
            leaf.insertValueAt(j, entries[i + j].blockRBN);
        }

        if(!leafRBNs.empty())
            leaf.setPrevLeafRBN(leafRBNs.back());
        
        uint32_t leafRBN =  allocateTreeBlock();
        if (!writeNode(leafRBN, leaf))
        {
            return {}; 
        };

        if(!leafRBNs.empty())
        {
            NodeAlt* prevLeaf = loadNode(leafRBNs.back());
            if(prevLeaf == nullptr)
            {
                return {}; 
            }
            prevLeaf->setNextLeafRBN(leafRBN);
            writeNode(leafRBNs.back(), *prevLeaf);
            delete prevLeaf;
        }
        leafRBNs.push_back(leafRBN);
    }
    return leafRBNs;
}

std::vector<uint32_t> BPlusTreeAlt::buildIndexLevel(const std::vector<uint32_t>& childRBNs)
{
    std::vector<uint32_t> parentRBNs;
    size_t maxKeys = NodeAlt::calculateMaxKeys(blockSize, false);

    for(size_t i = 0; i < childRBNs.size(); i += (maxKeys + 1))
    {
        NodeAlt indexNode(false, blockSize);

        indexNode.insertChildRBN(0, childRBNs[i]);

        for(size_t j = 1; j <= maxKeys && (i + j) < childRBNs.size(); ++j)
        {
            NodeAlt* childNode = loadNode(childRBNs[i + j - 1]);
            uint32_t childPromoteKey = childNode->getKeyAt(childNode->getKeyCount() - 1);
            delete childNode;

            indexNode.insertKeyAt(j - 1, childPromoteKey);
            indexNode.insertChildRBN(j, childRBNs[i + j]);
        }
        uint32_t parentRBN = allocateTreeBlock();
        writeNode(parentRBN, indexNode);
        parentRBNs.push_back(parentRBN);
    }
    return parentRBNs;
}

void BPlusTreeAlt::printNode(uint32_t rbn, int depth)
{
    NodeAlt* node = loadNode(rbn);
    std::string indent(depth * 2, ' ');

    std::cout << indent << "Node RBN: " << rbn << (node->isLeafNode() ? " (Leaf)" : " (Index)") << "\n";
    std::cout << indent << "Keys: ";
    for (size_t i = 0; i < node->getKeyCount(); ++i)
    {
        std::cout << node->getKeyAt(i) << " ";
    }
    std::cout << "\n";

    if (node->isLeafNode() == 0)
    {
        for(size_t i = 0; i < node->getChildCount(); ++i)
        {
            printNode(node->getChildRBN(i), depth + 1);
        }
    }
    delete node;
}

void BPlusTreeAlt::printTree()
{
    if (!isOpen)
    {
        std::cout << "B+ Tree is not open.\n";
        return;
    }
    std::cout << "B+ Tree Structure:\n";
    std::cout << "Root RBN: " << treeHeader.getRootIndexRBN() << "\n";
    std::cout << "Tree Height: " << treeHeader.getHeight() << "\n";
    printNode(treeHeader.getRootIndexRBN(), 0);
}

bool BPlusTreeAlt::insert(uint32_t key, uint32_t blockRBN)
{
    return false;
}

uint32_t BPlusTreeAlt::splitNode(uint32_t nodeRBN, uint32_t& promotedKey)
{
    NodeAlt* node = loadNode(nodeRBN);
    if(node == nullptr)
    {
        setError("Failed to load node during split.");
        return 0;
    }

    uint32_t newRBN = allocateTreeBlock();
    NodeAlt* newNode = new NodeAlt(node->isLeafNode() == 1, blockSize);

    size_t splitIndex = (node->getKeyCount() + 1) / 2;

    if(node->isLeafNode() == 1)
    {
        for(size_t i = splitIndex; i < node->getKeyCount(); ++i)
        {
            size_t newIndex = i - splitIndex;
            newNode->insertKeyAt(newIndex, node->getKeyAt(i));
            newNode->insertValueAt(newIndex, node->getValueAt(i));
        }

        newNode->setNextLeafRBN(node->getNextLeafRBN());
        newNode->setPrevLeafRBN(nodeRBN);
        node->setNextLeafRBN(newRBN);

        if(newNode->getNextLeafRBN() != 0)
        {
            NodeAlt* nextLeaf = loadNode(newNode->getNextLeafRBN());
            nextLeaf->setPrevLeafRBN(newRBN);
            writeNode(newNode->getNextLeafRBN(), *nextLeaf);
            delete nextLeaf;
        }

        promotedKey = newNode->getKeyAt(0);

        while (node->getKeyCount() > splitIndex)
        {
            node->removeKeyAt(node->getKeyCount() - 1);
            node->removeValueAt(node->getKeyCount() - 1);
        }
    }
    else
    {
        for(size_t i = splitIndex; i < node->getKeyCount(); ++i)
        {
            size_t newIndex = i - splitIndex - 1;
            newNode->insertKeyAt(newIndex, node->getKeyAt(i));
            newNode->insertChildRBN(newIndex, node->getChildRBN(i));
        }

        newNode->insertChildRBN(newNode->getKeyCount(), node->getChildRBN(node->getKeyCount()));

        promotedKey = node->getKeyAt(splitIndex);

        while (node->getKeyCount() > splitIndex)
        {
            node->removeKeyAt(node->getKeyCount() - 1);
            node->removeChildRBN(node->getChildCount() - 1);
        }
    }

    writeNode(nodeRBN, *node);
    writeNode(newRBN, *newNode);

    delete node;
    delete newNode;

    return newRBN;
}

bool BPlusTreeAlt::insertRecursive(uint32_t nodeRBN, uint32_t key, uint32_t value, 
                                    uint32_t& newChildRBN, uint32_t& newPromotedKey)
{
    NodeAlt* node = loadNode(nodeRBN);
    if(node == nullptr)
    {
        setError("Failed to load node during insertion.");
        return false;
    }
    if(node->isLeafNode() == 1)
    {
        if(!node->isFull())
        {
            insertIntoLeaf(node, key, value);
            writeNode(nodeRBN, *node);
            delete node;
            return false; // No split
        }
        else
        {
            delete node;
            newChildRBN = splitNode(nodeRBN, newPromotedKey);

            if(key < newPromotedKey)
            {
                node = loadNode(nodeRBN);
                insertIntoLeaf(node, key, value);
                writeNode(nodeRBN, *node);
                delete node;
            }
            else
            {
                node = loadNode(newChildRBN);
                insertIntoLeaf(node, key, value);
                writeNode(newChildRBN, *node);
                delete node;
            }
            return true; // Split occurred
        }
    }
    else
    {
        size_t childIndex = node->findChildIndex(key);
        uint32_t childRBN = node->getChildRBN(childIndex);
        delete node;

        uint32_t childPromotedKey, newGrandChildRBN;
        bool childSplit = insertRecursive(childRBN, key, value, newGrandChildRBN, childPromotedKey);

        if(!childSplit)
        {
            return false;
        }

        node = loadNode(nodeRBN);

        if(!node->isFull())
        {
            insertIntoIndex(node, childPromotedKey, newGrandChildRBN);
            writeNode(nodeRBN, *node);
            delete node;
            return false; // No split
        }
        else
        {
            delete node;
            newChildRBN = splitNode(nodeRBN, newPromotedKey);

            if(childPromotedKey < newPromotedKey)
            {
                node = loadNode(nodeRBN);
                insertIntoIndex(node, childPromotedKey, newGrandChildRBN);
                writeNode(nodeRBN, *node);
                delete node;
            }
            else
            {
                node = loadNode(newChildRBN);
                insertIntoIndex(node, childPromotedKey, newGrandChildRBN);
                writeNode(newChildRBN, *node);
                delete node;
            }
            return true; // Split occurred
        }
    }
}

void BPlusTreeAlt::insertIntoLeaf(NodeAlt* node, uint32_t key, uint32_t value)
{
    size_t index = 0;
    while(index < node->getKeyCount() && node->getKeyAt(index) < key)
    {
        ++index;
    }
    node->insertKeyAt(index, key);
    node->insertValueAt(index, value);
}

void BPlusTreeAlt::insertIntoIndex(NodeAlt* node, uint32_t key, uint32_t childRBN)
{
    size_t index = 0;
    while(index < node->getKeyCount() && node->getKeyAt(index) < key)
    {
        ++index;
    }
    node->insertKeyAt(index, key);
    node->insertChildRBN(index + 1, childRBN);
}

bool BPlusTreeAlt::removeRecursive(uint32_t nodeRBN, uint32_t key, bool& underflow)
{
    NodeAlt* node = loadNode(nodeRBN);

    if(node == nullptr)
    {
        setError("Failed to load node during removal.");
        return false;
    }

    if(node->isLeafNode() == 1)
    {
        size_t index = 0;
        while(index < node->getKeyCount() && node->getKeyAt(index) < key)
        {
            ++index;
        }
        if(index < node->getKeyCount() && node->getKeyAt(index) == key)
        {
            node->removeKeyAt(index);
            node->removeValueAt(index);
            writeNode(nodeRBN, *node);
            underflow = node->isUnderfull();
            delete node;
            return true;
        }
        else
        {
            delete node;
            setError("Key not found for removal.");
            return false;
        }
    }
    else
    {
        // Index node removal logic to be implemented
    }
}

bool BPlusTreeAlt::borrowFromSibling(uint32_t nodeRBN, uint32_t parentRBN, size_t indexInParent, bool isLeaf)
{
    NodeAlt* node = loadNode(nodeRBN);

    if(node == nullptr)
    {
        setError("Failed to load node during borrow.");
        return false;
    }

    NodeAlt* parent = loadNode(parentRBN);
    if(parent == nullptr)
    {
        delete node;
        setError("Failed to load parent node during borrow.");
        return false;
    }

    size_t minKeys = (node->getMaxKeys() + 1) / 2;

    if(node->isLeafNode() == 1)
    {
        if(indexInParent < parent->getChildCount() - 1)
        {
            uint32_t rightSiblingRBN = parent->getChildRBN(indexInParent + 1);
            NodeAlt* rightSibling = loadNode(rightSiblingRBN);
            
            if(rightSibling != nullptr && rightSibling->getKeyCount() > minKeys)
            {
                // Borrow first value from right sibling
                uint32_t borrowedKey = rightSibling->getKeyAt(0);
                uint32_t borrowedValue = rightSibling->getValueAt(0);

                // Remove key & value from
                rightSibling->removeKeyAt(0);
                rightSibling->removeValueAt(0);

                // Add to current node
                node->insertKeyAt(node->getKeyCount(), borrowedKey);
                node->insertValueAt(node->getKeyCount(), borrowedValue);

                // Update parent seperator key
                // Parent key at indexInParent seperates this node from right sibling
                // Set it to the highest key in the current node
                parent->setKeyAt(indexInParent, node->getKeyAt(node->getKeyCount() - 1));

                writeNode(nodeRBN, *node);
                writeNode(rightSiblingRBN, *rightSibling);
                writeNode(parentRBN, *parent);

                delete node;
                delete rightSibling;
                delete parent;
                return true;
            }
            delete rightSibling;
        }

        if(indexInParent > 0)
        {
            uint32_t leftSiblingRBN = parent->getChildRBN(indexInParent - 1);
            NodeAlt* leftSibling = loadNode(leftSiblingRBN);

            if(leftSibling != nullptr && leftSibling->getKeyCount() > minKeys)
            {
                // Borrow last key & value from left sibling
                uint32_t borrowedKey = leftSibling->getKeyAt(leftSibling->getKeyCount() - 1);
                uint32_t borrowedValue = leftSibling->getValueAt(leftSibling->getKeyCount() - 1);

                // Remove key & value from left sibling
                leftSibling->removeKeyAt(leftSibling->getKeyCount() - 1);
                leftSibling->removeValueAt(leftSibling->getKeyCount() - 1);

                // Add to beginning of current node
                node->insertKeyAt(0, borrowedKey);
                node->insertValueAt(0, borrowedValue);

                // Update parent seperator key
                // Parent key at indexInParent - 1
                // Set it to the highest key in left sibling after removal
                parent->setKeyAt(indexInParent - 1, leftSibling->getKeyAt(leftSibling->getKeyCount() - 1));

                writeNode(nodeRBN, *node);
                writeNode(leftSiblingRBN, *leftSibling);
                writeNode(parentRBN, *parent);

                delete node;
                delete leftSibling;
                delete parent;
                return true;
            }
            delete leftSibling;
        }
    }
    else
    {
        // Index Node
    }
}