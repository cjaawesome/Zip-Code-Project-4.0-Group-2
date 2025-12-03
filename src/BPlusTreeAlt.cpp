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

bool BPlusTreeAlt::buildFromSequenceSet()
{
    // To Do After Insert
    return true;
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
    // Close Buffers
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

uint32_t BPlusTreeAlt::allocateIndexBlock()
{
    // Get Updated Index Block Count
    uint32_t newRBN = treeHeader.getIndexBlockCount() + 1;
    // Update Index Block Count
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
    return true;
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
    return buildTreeFromEntries(entries);
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
        
        uint32_t leafRBN =  allocateIndexBlock();
        writeNode(leafRBN, leaf);

        if(!leafRBNs.empty())
        {
            NodeAlt* prevLeaf = loadNode(leafRBNs.back());
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
        uint32_t parentRBN = allocateIndexBlock();
        writeNode(parentRBN, indexNode);
        parentRBNs.push_back(parentRBN);
    }
    return parentRBNs;
}


