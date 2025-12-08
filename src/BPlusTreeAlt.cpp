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

bool BPlusTreeAlt::open(const std::string& inIndexFileName, const std::string& inSequenceSetFilename)
{
    HeaderBuffer headerBuffer;
    BPlusTreeHeaderBufferAlt bPlusTreeHeaderBuffer;
    
    this->sequenceSetFilename = inSequenceSetFilename;
    this->indexFilename = inIndexFileName;

    // Open and read sequence set header
    if (!headerBuffer.readHeader(sequenceSetFilename, sequenceHeader)) 
    {
        setError("Failed to read sequence set header");
        return false;
    }

    // Open and read B+ tree header
    if (!bPlusTreeHeaderBuffer.readHeader(indexFilename, treeHeader))
    {
        setError("Failed to read B+ tree header");
        return false;
    }

    // Open index page buffer
    if (!indexPageBuffer.open(indexFilename, treeHeader.getBlockSize(), treeHeader.getHeaderSize())) 
    {
        setError("Failed to open index page buffer");
        return false;
    }
/*
    // Open sequence set buffer
    if (!sequenceSetBuffer.openFile(sequenceSetFilename, sequenceHeader.getHeaderSize())) 
    {
        setError("Failed to open sequence set buffer");
        return false;
    }
*/
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
    
    // sequenceSetBuffer.closeFile();
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

        // Appropriate leaf node found
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

    if(!sequenceSetBuffer.openFile(sequenceSetFilename, sequenceHeader.getHeaderSize()))
    {
        setError("Failed to open sequenceSetFile");
        return false;
    }
    // Create index entry vector
    std::vector<IndexEntry> entries;
    // Start at root of sequence set
    uint32_t currentRBN = sequenceHeader.getSequenceSetListRBN();
    while(currentRBN != 0)
    {
        // Start reading the sequence set
        ActiveBlock block = sequenceSetBuffer.loadActiveBlockAtRBN(currentRBN, blockSize, sequenceHeaderSize);
        std::vector<ZipCodeRecord> records;
        // Unpack records with the exposed record buffer
        sequenceSetBuffer.unpackBlockAPI(block.data, records);
        // Get the highest key in each block
        if(!records.empty())
        {
            uint32_t highestKey = records.back().getZipCode();
            IndexEntry entry = {highestKey, currentRBN};
            entries.push_back(entry);
        }
        currentRBN = block.succeedingRBN;
    }
    sequenceSetBuffer.closeFile();
    // Start building the tree from the index entries
    bool result = buildTreeFromEntries(entries);
    // Return if the build was successful
    return result;
}

bool BPlusTreeAlt::buildTreeFromEntries(const std::vector<IndexEntry>& entries)
{
    // Verify entries has data
    if(entries.empty())
    {
        setError("No entries to build tree from.");
        return false;
    }
    std::vector<uint32_t> currentLevel = buildLeafLevel(entries);
    treeHeader.setHeight(1);
    // While current level has more than 1 child RBN build the index
    while(currentLevel.size() > 1)
    {
        // Build index levels and update current level with parent rbns
        currentLevel = buildIndexLevel(currentLevel);
        // Update tree height after each level
        treeHeader.setHeight(treeHeader.getHeight() + 1);
    }
    // Set root index to the first node in the current level vector
    treeHeader.setRootIndexRBN(currentLevel[0]);

    return true;
}

std::vector<uint32_t> BPlusTreeAlt::buildLeafLevel(const std::vector<IndexEntry>& entries)
{
    // Create leaf rbn vector
    std::vector<uint32_t> leafRBNs;
    size_t maxKeys = NodeAlt::calculateMaxKeys(blockSize, true);
    // For each entry in entries
    for (size_t i = 0; i < entries.size(); i += maxKeys)
    {
        // Create empty leaf node
        NodeAlt leaf(true, blockSize);
        // From j to max keys insert key and value to distribute keys properly
        for(size_t j = 0; j < maxKeys && (i + j) < entries.size(); ++j)
        {
            leaf.insertKeyAt(j, entries[i + j].key);
            leaf.insertValueAt(j, entries[i + j].blockRBN);
        }
        // Update prev rbn
        if(!leafRBNs.empty())
            leaf.setPrevLeafRBN(leafRBNs.back());
        // allocate new node
        uint32_t leafRBN =  allocateTreeBlock();
        if (!writeNode(leafRBN, leaf))
        {
            return {}; 
        };
        // Update next rbn
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
        // Add leaf rbn to leaf vector
        leafRBNs.push_back(leafRBN);
    }
    return leafRBNs;
}

std::vector<uint32_t> BPlusTreeAlt::buildIndexLevel(const std::vector<uint32_t>& childRBNs)
{
    // Create parent rbn vector
    std::vector<uint32_t> parentRBNs;
    // Calculate max keys per node
    size_t maxKeys = NodeAlt::calculateMaxKeys(blockSize, false);
    // For i in childRBN size increment by max keys + 1
    for(size_t i = 0; i < childRBNs.size(); i += (maxKeys + 1))
    {
        // Create empty index node
        NodeAlt indexNode(false, blockSize);
        // Insert child rbn to first index at i
        indexNode.insertChildRBN(0, childRBNs[i]);
        // Start at second index
        for(size_t j = 1; j <= maxKeys && (i + j) < childRBNs.size(); ++j)
        {
            // Load child node from file
            NodeAlt* childNode = loadNode(childRBNs[i + j - 1]);
            // Get promote key from child node
            uint32_t childPromoteKey = childNode->getKeyAt(childNode->getKeyCount() - 1);
            delete childNode;
            // Insert seperator key into it's correct up and adjacent position
            indexNode.insertKeyAt(j - 1, childPromoteKey);
            indexNode.insertChildRBN(j, childRBNs[i + j]);
        }
        // Allocate new parent block
        uint32_t parentRBN = allocateTreeBlock();
        // Write the newly created block
        writeNode(parentRBN, indexNode);
        // Add parent rbn to the list
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
    // Hold results if node splits
    uint32_t newSplitKey = 0;
    uint32_t newRightChildRBN = 0;

    // Prepare to rewrite header
    BPlusTreeHeaderBufferAlt headerBuffer;

    // Handle emptry tree
    if (treeHeader.getRootIndexRBN() == 0)
    {
        // Allocate new block
        uint32_t newRBN = allocateTreeBlock(); 
        // Create leaf node
        NodeAlt* newRoot = new NodeAlt(true, treeHeader.getBlockSize());
        
        if (newRoot == nullptr)
        {
            setError("Failed to create initial root node.");
            return false;
        }
        // Insert data
        newRoot->insertKeyAt(0, key);
        newRoot->insertValueAt(0, blockRBN);
        // Write node
        writeNode(newRBN, *newRoot);

        // Update Tree Header
        treeHeader.setRootIndexRBN(newRBN);
        treeHeader.setHeight(1);
        // Clean
        delete newRoot;
    }
    else 
    {
        // Recursive insert
        uint32_t oldRootRBN = treeHeader.getRootIndexRBN();

        // The recursive function returns true if a split occurred
        // newChildRBN is the right of the split
        // newPromotedKey is the key to insert into the new root/parent
        bool splitOccurred = insertRecursive(oldRootRBN, key, blockRBN, newRightChildRBN,newSplitKey);

        // Insert worked update header
        if (!splitOccurred)
        {
            return headerBuffer.writeHeader(indexPageBuffer.getFileStream(), treeHeader);
        }
        
        // Split occurred
        // Allocate new node
        uint32_t newRootRBN = allocateTreeBlock();
        // Create new index node
        NodeAlt* newRoot = new NodeAlt(false, treeHeader.getBlockSize());

        if (newRoot == nullptr)
        {
            setError("Failed to create new index root node after split.");
            return false;
        }

        // Old root
        newRoot->insertChildRBN(0, oldRootRBN); 
        
        // Separator key
        newRoot->insertKeyAt(0, newSplitKey);
        
        // New node from split
        newRoot->insertChildRBN(1, newRightChildRBN);

        // Write split node
        writeNode(newRootRBN, *newRoot);

        // Update Tree Header
        treeHeader.setRootIndexRBN(newRootRBN);
        treeHeader.setHeight(treeHeader.getHeight() + 1);
        delete newRoot;
    }

    // Rewrite header
    return headerBuffer.writeHeader(indexPageBuffer.getFileStream(), treeHeader);
}

uint32_t BPlusTreeAlt::splitNode(uint32_t nodeRBN, uint32_t& promotedKey)
{
    // Load node to split
    NodeAlt* node = loadNode(nodeRBN);
    if(node == nullptr)
    {
        setError("Failed to load node during split.");
        return 0;
    }
    // Allocate new node for split
    uint32_t newRBN = allocateTreeBlock();
    // Create new node for split
    NodeAlt* newNode = new NodeAlt(node->isLeafNode() == 1, blockSize);
    // Get the split index 
    size_t splitIndex = (node->getKeyCount() + 1) / 2;
    // If leaf node
    if(node->isLeafNode() == 1)
    {
        // Move values from split index into new right sibling split noede
        for(size_t i = splitIndex; i < node->getKeyCount(); ++i)
        {
            size_t newIndex = i - splitIndex;
            newNode->insertKeyAt(newIndex, node->getKeyAt(i));
            newNode->insertValueAt(newIndex, node->getValueAt(i));
        }
        // Update RBN pointers
        newNode->setNextLeafRBN(node->getNextLeafRBN());
        newNode->setPrevLeafRBN(nodeRBN);
        node->setNextLeafRBN(newRBN);
        // If next node is not the last node
        if(newNode->getNextLeafRBN() != 0)
        {
            NodeAlt* nextLeaf = loadNode(newNode->getNextLeafRBN());
            nextLeaf->setPrevLeafRBN(newRBN);
            writeNode(newNode->getNextLeafRBN(), *nextLeaf);
            delete nextLeaf;
        }
        // Get promoted key from the new node
        promotedKey = newNode->getKeyAt(0);
        // Remove the split keys and values from the old node
        while (node->getKeyCount() > splitIndex)
        {
            node->removeKeyAt(node->getKeyCount() - 1);
            node->removeValueAt(node->getKeyCount() - 1);
        }
    }
    else
    {
        // Index node
        // Get promoted key
        promotedKey = node->getKeyAt(splitIndex);
        // Move children from split point into new index node
        for(size_t i = splitIndex; i < node->getChildCount(); ++i)
        {
            size_t newIndex = i - splitIndex;
            newNode->insertChildRBN(newIndex, node->getChildRBN(i));
        }
        // Move keys from split point into new index node
        for(size_t i = splitIndex + 1; i < node->getKeyCount(); ++i)
        {
            size_t newIndex = i - (splitIndex + 1);
            newNode->insertKeyAt(newIndex, node->getKeyAt(i));
        }
        // Remove keys from old node
        while (node->getKeyCount() > splitIndex)
        {
            node->removeKeyAt(node->getKeyCount() - 1);
        }
        // Remove split index key
        node->removeKeyAt(splitIndex);
        // Remove children from old node
        while(node->getChildCount() > splitIndex)
        {
            node->removeChildRBN(node->getChildCount() - 1);
        }
    }
    // Rewrite nodes
    writeNode(nodeRBN, *node);
    writeNode(newRBN, *newNode);
    // Clean
    delete node;
    delete newNode;
    // Return split node rbn
    return newRBN;
}

bool BPlusTreeAlt::insertRecursive(uint32_t nodeRBN, uint32_t key, uint32_t value, 
                                    uint32_t& newChildRBN, uint32_t& newPromotedKey)
{
    // Load node to insert data into
    NodeAlt* node = loadNode(nodeRBN);
    if(node == nullptr)
    {
        setError("Failed to load node during insertion.");
        return false;
    }
    // If leaf node
    if(node->isLeafNode() == 1)
    {
        // Room to insert
        if(!node->isFull())
        {
            insertIntoLeaf(node, key, value);
            writeNode(nodeRBN, *node);
            delete node;
            return false; // No split
        }
        else // Need to split
        {
            delete node;
            newChildRBN = splitNode(nodeRBN, newPromotedKey);

            if(key < newPromotedKey)
            {   // Insert key into original node
                node = loadNode(nodeRBN);
                insertIntoLeaf(node, key, value);
                writeNode(nodeRBN, *node);
                delete node;
            }
            else
            {   // Insert key into new split node
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
        // Index node
        size_t childIndex = node->findChildIndex(key);
        uint32_t childRBN = node->getChildRBN(childIndex);
        delete node;
        // Store promoted key and rbn of node to move up and adjacent
        uint32_t childPromotedKey, newGrandChildRBN;
        // Recursive call to find appropriate index nodes
        bool childSplit = insertRecursive(childRBN, key, value, newGrandChildRBN, childPromotedKey);
        // If no split occurred exit
        if(!childSplit)
        {
            return false;
        }
        // Load original node
        node = loadNode(nodeRBN);
        // If not full after split instert
        if(!node->isFull())
        {
            insertIntoIndex(node, childPromotedKey, newGrandChildRBN);
            writeNode(nodeRBN, *node);
            delete node;
            return false; // No split
        }
        else
        {   // Split node
            delete node;
            newChildRBN = splitNode(nodeRBN, newPromotedKey);
            // Insert into old node
            if(childPromotedKey < newPromotedKey)
            {
                node = loadNode(nodeRBN);
                insertIntoIndex(node, childPromotedKey, newGrandChildRBN);
                writeNode(nodeRBN, *node);
                delete node;
            }
            else
            {   // Insert into newly split node
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
    // Find leaf to insert key and value into
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
    // Find index to insert key and child into
    size_t index = 0;
    while(index < node->getKeyCount() && node->getKeyAt(index) < key)
    {
        ++index;
    }
    node->insertKeyAt(index, key);
    node->insertChildRBN(index + 1, childRBN);
}

bool BPlusTreeAlt::borrowFromSibling(uint32_t nodeRBN, uint32_t parentRBN, size_t indexInParent)
{
    // Load node
    NodeAlt* node = loadNode(nodeRBN);
    if(node == nullptr)
    {
        setError("Failed to load node during borrow.");
        return false;
    }
    // Load node parent
    NodeAlt* parent = loadNode(parentRBN);
    if(parent == nullptr)
    {
        delete node;
        setError("Failed to load parent node during borrow.");
        return false;
    }
    // Calc min keys
    size_t minKeys = (node->getMaxKeys() + 1) / 2;
    bool success = false;

    // Borrow right sibling
    if(indexInParent < parent->getChildCount() - 1)
    {
        uint32_t rightSiblingRBN = parent->getChildRBN(indexInParent + 1);
        NodeAlt* rightSibling = loadNode(rightSiblingRBN);
        
        if(rightSibling != nullptr && rightSibling->getKeyCount() > minKeys)
        {
            if(node->isLeafNode() == 1)
            {
                // Borrow smallest key & value from right
                uint32_t borrowedKey = rightSibling->getKeyAt(0);
                uint32_t borrowedValue = rightSibling->getValueAt(0);

                // Add to current node
                node->insertKeyAt(node->getKeyCount(), borrowedKey);
                node->insertValueAt(node->getKeyCount(), borrowedValue);

                // Remove from right sibling
                rightSibling->removeKeyAt(0);
                rightSibling->removeValueAt(0);

                // Update separator key
                parent->setKeyAt(indexInParent, rightSibling->getKeyAt(0));
            }
            else // Index Node
            {
                uint32_t parentKey = parent->getKeyAt(indexInParent);
                uint32_t rightFirstKey = rightSibling->getKeyAt(0);
                uint32_t rightFirstChild = rightSibling->getChildRBN(0);

                // Add parent key and right first child RBN to current node
                node->insertKeyAt(node->getKeyCount(), parentKey);
                node->insertChildRBN(node->getChildCount(), rightFirstChild);

                // Remove key & child from right sibling
                rightSibling->removeKeyAt(0);
                rightSibling->removeChildRBN(0);

                // Push right first key up to parent
                parent->setKeyAt(indexInParent, rightFirstKey);
            }
            
            // Write all nodes
            writeNode(nodeRBN, *node);
            writeNode(rightSiblingRBN, *rightSibling);
            writeNode(parentRBN, *parent);
            success = true;
        }
        delete rightSibling;
    }
    
    // Try borrow left sibling
    if(!success && indexInParent > 0)
    {
        uint32_t leftSiblingRBN = parent->getChildRBN(indexInParent - 1);
        NodeAlt* leftSibling = loadNode(leftSiblingRBN);

        if(leftSibling != nullptr && leftSibling->getKeyCount() > minKeys)
        {
            if(node->isLeafNode() == 1)
            {
                // Borrow largest key & value from left
                uint32_t borrowedKey = leftSibling->getKeyAt(leftSibling->getKeyCount() - 1);
                uint32_t borrowedValue = leftSibling->getValueAt(leftSibling->getKeyCount() - 1);

                // Remove key & value from left sibling
                leftSibling->removeKeyAt(leftSibling->getKeyCount() - 1);
                leftSibling->removeValueAt(leftSibling->getKeyCount() - 1);

                // Add to beginning of current node
                node->insertKeyAt(0, borrowedKey);
                node->insertValueAt(0, borrowedValue);

                // Update separator key
                parent->setKeyAt(indexInParent - 1, node->getKeyAt(0));
            }
            else // Index Node
            {
                uint32_t parentKey = parent->getKeyAt(indexInParent - 1);
                uint32_t leftLastKey = leftSibling->getKeyAt(leftSibling->getKeyCount() - 1);
                uint32_t leftLastChild = leftSibling->getChildRBN(leftSibling->getChildCount() - 1);

                // Add parent key and left last child RBN to current node
                node->insertKeyAt(0, parentKey);
                node->insertChildRBN(0, leftLastChild);

                // Remove last key and child from left sibling
                leftSibling->removeKeyAt(leftSibling->getKeyCount() - 1);
                leftSibling->removeChildRBN(leftSibling->getChildCount() - 1);

                // Add left last key to parent
                parent->setKeyAt(indexInParent - 1, leftLastKey);
            }

            // Write nodes
            writeNode(nodeRBN, *node);
            writeNode(leftSiblingRBN, *leftSibling);
            writeNode(parentRBN, *parent);
            success = true;
        }
        delete leftSibling;
    }
    
    if (success) 
    {
        return true;
    }
     
    delete node;
    delete parent;
    return false;
}

/*
bool BPlusTreeAlt::borrowFromSibling(uint32_t nodeRBN, uint32_t parentRBN, size_t indexInParent)
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
        if(indexInParent < parent->getChildCount() - 1)
        {
            uint32_t rightSiblingRBN = parent->getChildRBN(indexInParent + 1);
            NodeAlt* rightSibling = loadNode(rightSiblingRBN);

            if(rightSibling != nullptr && rightSibling->getKeyCount() > minKeys)
            {
                // Move parent seperator key down & bring up right sibling key.
                uint32_t parentKey = parent->getKeyAt(indexInParent);
                uint32_t rightFirstKey = rightSibling->getKeyAt(0);
                uint32_t rightFirstChild = rightSibling->getChildRBN(0);

                // Add parent key and right first child key
                node->insertKeyAt(node->getKeyCount(), parentKey);
                node->insertChildRBN(node->getChildCount(), rightFirstChild);

                // Remove first key and child
                rightSibling->removeKeyAt(0);
                rightSibling->removeChildRBN(0);

                // Push right first key up to parent
                parent->setKeyAt(indexInParent, rightFirstKey);

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
    }
    if(indexInParent > 0)
    {
        uint32_t leftSiblingRBN = parent->getChildRBN(indexInParent - 1);
        NodeAlt* leftSibling = loadNode(leftSiblingRBN);

        if(leftSibling != nullptr && leftSibling->getKeyCount() > minKeys)
        {
            // Bring down seperator key & push up left sibling key
            uint32_t parentKey = parent->getKeyAt(indexInParent - 1);
            uint32_t leftLastKey = leftSibling->getKeyAt(leftSibling->getKeyCount() - 1);
            uint32_t leftLastChild = leftSibling->getChildRBN(leftSibling->getChildCount() - 1);

            // Add pareny key and left last child
            node->insertKeyAt(0, parentKey);
            node->insertChildRBN(0, leftLastChild);

            // Remove last key and child
            leftSibling->removeKeyAt(leftSibling->getKeyCount() - 1);
            leftSibling->removeKeyAt(leftSibling->getChildCount() - 1);

            // Add left last key to parent
            parent->setKeyAt(indexInParent - 1, leftLastKey);

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

    delete node;
    delete parent;
    return false;
}
*/

bool BPlusTreeAlt::mergeWithSibling(uint32_t nodeRBN, uint32_t parentRBN, size_t indexInParent)
{
    // Load node
    NodeAlt* node = loadNode(nodeRBN);

    if(node == nullptr)
    {
        setError("Failed to load node in merge with sibling.");
        return false;
    }
    // Load parent
    NodeAlt* parent = loadNode(parentRBN);

    if(parent == nullptr)
    {
        setError("Failed to load parent node in merge with sibling.");
        delete node;
        return false;
    }
    // Store rbn to set new tree head to
    bool success = false;
    uint32_t rbnToReturn = 0;
    // If leaf node
    if(node->isLeafNode() == 1)
    {
        // Try merge with right sibling
        uint32_t rightSiblingRBN = node->getNextLeafRBN();
        NodeAlt* rightSibling = loadNode(rightSiblingRBN);
        // Get start index of values
        size_t startIndex = node->getKeyCount();
        // Chceck right sibling loaded and there is room in the node
        if(rightSibling != nullptr && (node->getKeyCount() + rightSibling->getKeyCount()) <= node->getMaxKeys())
        {   // Merge keys and values
            for(size_t i = 0; i < rightSibling->getKeyCount(); ++i)
            {
                node->insertKeyAt(startIndex + i, rightSibling->getKeyAt(i));
                node->insertValueAt(startIndex + i, rightSibling->getValueAt(i));
            }
            // Update node pointers
            node->setNextLeafRBN(rightSibling->getNextLeafRBN());
            parent->removeKeyAt(indexInParent);
            parent->removeChildRBN(indexInParent + 1);
            // Update right sibling node pointers
            if(rightSibling->getNextLeafRBN() != 0)
            {
                uint32_t oneDivorcedSiblingRBN = rightSibling->getNextLeafRBN();
                NodeAlt* oneDivorcedSibling = loadNode(oneDivorcedSiblingRBN);
                if(oneDivorcedSibling != nullptr)
                {
                    // Write updated node
                    oneDivorcedSibling->setPrevLeafRBN(nodeRBN);
                    writeNode(oneDivorcedSiblingRBN, *oneDivorcedSibling);
                    delete oneDivorcedSibling;
                }
            }
            // Write surviving nodes and clean
            writeNode(nodeRBN, *node);
            writeNode(parentRBN, *parent);
            delete rightSibling;
            success = true;  
        }
        else
        {
            delete rightSibling;
        }

        if(!success)
        {
            // Try left sibling
            uint32_t leftSiblingRBN = node->getPrevLeafRBN();
            NodeAlt* leftSibling = loadNode(leftSiblingRBN);
            // Get start index
            size_t leftStartIndex = leftSibling->getKeyCount();
            // If left sibling not null and has room to merge
            if(leftSibling != nullptr && (node->getKeyCount() + leftSibling->getKeyCount()) <= node->getMaxKeys())
            {   // Move data from node to left sibling
                for(size_t i = 0; i < node->getKeyCount(); ++i)
                {
                    leftSibling->insertKeyAt(leftStartIndex + i, node->getKeyAt(i));
                    leftSibling->insertValueAt(leftStartIndex + i, node->getValueAt(i));
                }
                    // Update left sibling pointers
                    leftSibling->setNextLeafRBN(node->getNextLeafRBN());
                    parent->removeKeyAt(indexInParent - 1);
                    parent->removeChildRBN(indexInParent);

                // If original node next node is not last node update it's pointers
                if(node->getNextLeafRBN() != 0)
                {
                    uint32_t oneDivorcedSiblingRBN = node->getNextLeafRBN();
                    NodeAlt* oneDivorcedSibling = loadNode(oneDivorcedSiblingRBN);

                    if(oneDivorcedSibling != nullptr)
                    {
                        // Write updated node
                        oneDivorcedSibling->setPrevLeafRBN(node->getPrevLeafRBN());
                        writeNode(oneDivorcedSiblingRBN, *oneDivorcedSibling);
                        delete oneDivorcedSibling;        
                    }
                }
                // Write surviving nodes and clean
                writeNode(leftSiblingRBN, *leftSibling);
                writeNode(parentRBN, *parent);
                delete node;
                delete leftSibling;
                node = nullptr;
                // Store rbn for tree update
                success = true;
                rbnToReturn = leftSiblingRBN;
            }
            else
            {
                delete leftSibling;
            }
        }      
    }
    else
    {   // Index nodes
        // Check there is a right sibling
        if(indexInParent + 1 < parent->getChildCount())
        {   // Get right sibling rbn
            uint32_t rightSiblingRBN = parent->getChildRBN(indexInParent + 1);
            // Confirm right sibling is a valid rbn
            if(rightSiblingRBN != 0)
            {
                // load right sibling
                NodeAlt* rightSibling = loadNode(rightSiblingRBN);

                // Get separator key from the parent node
                uint32_t separatorKey = parent->getKeyAt(indexInParent);

                // Check if room for adjacent keys and separator key
                if(rightSibling != nullptr && node->getKeyCount() + rightSibling->getKeyCount() + 1 <= node->getMaxKeys())
                {
                    // Add separator key to the node at the end of keys
                    node->insertKeyAt(node->getKeyCount(), separatorKey);

                    // Move keys
                    for (size_t i = 0; i < rightSibling->getKeyCount(); ++i)
                    {
                        node->insertKeyAt(node->getKeyCount(), rightSibling->getKeyAt(i));
                    }

                    // Move children
                    for(size_t i = 0; i < rightSibling->getChildCount(); ++i)
                    {
                        node->insertChildRBN(node->getChildCount(), rightSibling->getChildRBN(i));
                    }

                    parent->removeKeyAt(indexInParent);
                    parent->removeChildRBN(indexInParent + 1);

                    writeNode(nodeRBN, *node);
                    writeNode(parentRBN, *parent);

                    delete rightSibling;
                    success = true;
                }
                else
                {
                    delete rightSibling;
                }
                    
            }
        }
        
        if(!success && indexInParent > 0)
        {
            // Try left sibling index
            uint32_t leftSiblingRBN = parent->getChildRBN(indexInParent - 1);
            if(leftSiblingRBN != 0)
            {
                // Load left sibling
                NodeAlt* leftSibling = loadNode(leftSiblingRBN);
                // Get separator key
                uint32_t separatorKey = parent->getKeyAt(indexInParent - 1);
                // If left sibling is valid and there is room to merge
                if(leftSibling != nullptr && node->getKeyCount() + leftSibling->getKeyCount() + 1 <= node->getMaxKeys())
                {
                    // Insert separator key
                    leftSibling->insertKeyAt(leftSibling->getKeyCount(), separatorKey);
                    // Move keys from node to left sibling
                    for(size_t i = 0; i < node->getKeyCount(); ++i)
                    {
                        leftSibling->insertKeyAt(leftSibling->getKeyCount(), node->getKeyAt(i));
                    }
                    // Move children from node to left sibling
                    for(size_t i = 0; i < node->getChildCount(); ++i)
                    {
                        leftSibling->insertChildRBN(leftSibling->getChildCount(), node->getChildRBN(i));
                    }
                    // Remove left key (separator) and original child node from parent
                    parent->removeKeyAt(indexInParent - 1);
                    parent->removeChildRBN(indexInParent);
                    // Rewrite surviving nodes
                    writeNode(leftSiblingRBN, *leftSibling);
                    writeNode(parentRBN, *parent);
                    // Clean
                    delete node;
                    delete leftSibling;
                    node = nullptr;
                    // Store node rbn for return and tree update
                    success = true;
                    rbnToReturn = leftSiblingRBN;
                }
                else
                {
                    delete leftSibling;
                }
            }
        }
    }
    // Clean up and parent checks
    if (success)
    {
        // Check if parent is underfull
        if (parent->isUnderfull())
        {
            // Parent is underfull recursive call
            if (parent->getParentRBN() != 0)
            {
                uint32_t grandparentRBN = parent->getParentRBN();
                NodeAlt* grandparent = loadNode(grandparentRBN);

                if(grandparent != nullptr)
                {
                    size_t parentIndexInGrandparent = 0;
                    for(size_t i = 0; i < grandparent->getChildCount(); ++i)
                    {
                        if(grandparent->getChildRBN(i) == parentRBN)
                        {
                            parentIndexInGrandparent = i;
                            break;
                        }
                    }    
                    // Clean up 
                    if (node)
                        delete node;
                    delete parent;
                    delete grandparent;
                        
                    // Recursive update
                    return mergeWithSibling(parentRBN, grandparentRBN, parentIndexInGrandparent);
                }
                else 
                {
                    setError("Failed to load grandparent RBN during recursive merge check.");
                }
            }
            else 
            {
            if (parent->getKeyCount() == 0)
                {
                    // Update root
                    treeHeader.setRootIndexRBN((node == nullptr) ? rbnToReturn : nodeRBN); 
                    treeHeader.setHeight(treeHeader.getHeight() - 1);
                }
            }
        }     
        // Clean up
        if (node) 
            delete node;
        delete parent;
        return true;
    }
    // Clean up
    if (node) 
        delete node;
    if (parent) 
        delete parent;
    return false;
}

uint32_t BPlusTreeAlt::searchRecursive(uint32_t nodeRBN, uint32_t key)
{
    // Load node
    NodeAlt* node = loadNode(nodeRBN);
    if(node == nullptr)
    {
        setError("Failed to load root node in searchRecursive.");
        return 0;
    }

    size_t i = 0;
    // Find starting index for key
    while(i < node->getKeyCount() && key > node->getKeyAt(i))
    {
        ++i;
    }
    // If leaf node return the RBN the key resides in
    if(node->isLeafNode() == 1)
    {
        uint32_t resultRBN = 0;
        if(i < node->getKeyCount() && node->getKeyAt(i) == key)
        {
            resultRBN = nodeRBN;
        }
       delete node;
       return resultRBN;
    }
    else
    {   // If index node is found get the next RBN
        uint32_t nextRBN = node->getChildRBN(i);
        // Clean
        delete node;
        // Verify it is a valid next RBN
        if(nextRBN == 0)
        {
            setError("Tried to access null RBN pointer in search recursive function.");
            return 0;
        }
        // Recursive call with nextRBN and same key to traverse the tree
        return searchRecursive(nextRBN, key);
    }
    return 0;
}

void BPlusTreeAlt::updateParentKey(uint32_t parentRBN, size_t indexInParent, uint32_t newKey)
{
    // Update if not leftmost child
    if (parentRBN != 0 && indexInParent > 0)
    {
        NodeAlt* parent = loadNode(parentRBN);
        if (parent != nullptr)
        {
            // Get separator key
            size_t parentKeyIndex = indexInParent - 1;

            if (parentKeyIndex < parent->getKeyCount())
            {
                parent->setKeyAt(parentKeyIndex, newKey);
                writeNode(parentRBN, *parent);
            }
            delete parent;
        }
        else
        {
            setError("Failed to load parent node for separator update.");
        }
    }
}

bool BPlusTreeAlt::remove(uint32_t key)
{
    // Start from tree root
    uint32_t rootRBN = treeHeader.getRootIndexRBN();
    // Get the result of recursive move
    bool result = removeRecursive(rootRBN, key, 0, 0);
    // If succesful
    if(result)
    {
        // Load root node
        NodeAlt* root = loadNode(rootRBN);
        if(root != nullptr)
        {
            // If root node is not null, root is not a leaf node, and root key count is zero tree has lost a level, update
            if(root->isLeafNode() == 0 && root->getKeyCount() == 0)
            {
                treeHeader.setRootIndexRBN(root->getChildRBN(0));
                treeHeader.setHeight(treeHeader.getHeight() - 1);
            }
            delete root;
        }
    }
    return result;
}

bool BPlusTreeAlt::removeRecursive(uint32_t nodeRBN, uint32_t key, uint32_t parentRBN, size_t indexInParent)
{
    // Load Node
    NodeAlt* node = loadNode(nodeRBN);
    if(node == nullptr)
    {
        setError("Failed to load node in remove recursive.");
        return false;
    }
    // Find Key
    size_t i = 0;
    while(i < node->getKeyCount() && key > node->getKeyAt(i))
    {
        ++i;
    }

    bool success = false;
    // If leaf node remove key & values and check edge cases
    if(node->isLeafNode() == 1)
    {
        if(i < node->getKeyCount() && key == node->getKeyAt(i))
        {
            node->removeKeyAt(i);
            node->removeValueAt(i);
            writeNode(nodeRBN, *node);
            success = true;
            // Parent key needs to be updated
            if(i == 0 && node->getKeyCount() > 0 && parentRBN != 0)
            {
               updateParentKey(parentRBN, indexInParent, node->getKeyAt(0)); 
            }
        }
        // Node is now underfull try to borrow. If borrowing fails try to merge recursively.
        if(success && node->isUnderfull() && parentRBN != 0)
        {
            success = borrowFromSibling(nodeRBN, parentRBN, indexInParent);

            if(!success)
            {
                success = mergeWithSibling(nodeRBN, parentRBN, indexInParent);
            }
        }      
    }
    else
    {
        // Index node
        uint32_t childRBN = node->getChildRBN(i);

        if(childRBN == 0)
        {
            setError("Failed to load child RBN node in recursive search.");
        }
        else
        {
            // Recursive call until key removed from leaf node
            success = removeRecursive(childRBN, key, nodeRBN, i);
            // Check if underfull
            if(success)
            {
                if(node->isUnderfull() && parentRBN != 0)
                {
                    success = borrowFromSibling(nodeRBN, parentRBN, indexInParent);

                    if(!success)
                    {
                        success = mergeWithSibling(nodeRBN, parentRBN, indexInParent);
                    }
                }
            }
        }
    }
    delete node;
    return success;
}

std::vector<uint32_t> BPlusTreeAlt::searchRange(const uint32_t keyStart, const uint32_t keyEnd)
{
    // Create vector to store keys in range
    std::vector<uint32_t> blockRBNsFound;
    // Start at tree root
    uint32_t rootRBN = treeHeader.getRootIndexRBN();
    // Find the starting rbn for the search
    uint32_t currentRBN = rangeSearch(rootRBN, keyStart);

    // Ensure valid RBN
    if(currentRBN == 0)
    {
        setError("Current rbn is 0 after range search in search range.");
        return blockRBNsFound;
    }
    // Load starting node
    NodeAlt* node = loadNode(currentRBN);
    if(node == nullptr)
    {
        setError("Failed to load current node in search range.");
        return blockRBNsFound;
    }
    // While node is not null
    while(node != nullptr)
    {   // If in range get all keys in the current rbn
        bool rangeExceeded = false;
        for(size_t i = 0; i < node->getKeyCount(); ++i)
        {
            // Get current key in the current rbn
            uint32_t currentKey = node->getKeyAt(i);
            // If less than key start skip ahead to next loop
            if(currentKey < keyStart)
            {
                 continue;
            }
            // If greater than keyEnd exit the loop
            if(currentKey > keyEnd)
            {
                rangeExceeded = true;
                break;
            }

            uint32_t blockRBN = node->getValueAt(i);
            blockRBNsFound.push_back(blockRBN);
        }
        // Exit while loop if out of range
        if(rangeExceeded)
        {
            break;
        }
        // Move onto the next leaf node in the chain
        uint32_t nextRBN = node->getNextLeafRBN();
        // Clean
        delete node;
        node = nullptr;
        // Reached end of index
        if(nextRBN == 0)
        {
            break;
        }
        // Try to load next node
        node = loadNode(nextRBN);
        if(node == nullptr)
        {
            setError("Failed to load next node in search range.");
            break;
        }
    }
    // Clean
    if(node != nullptr)
    {
         delete node;
    }
    // Return range
    return blockRBNsFound;
}

uint32_t BPlusTreeAlt::rangeSearch(uint32_t nodeRBN, uint32_t key)
{
    // Load initial node
    NodeAlt* node = loadNode(nodeRBN);
    if(node == nullptr)
    {
        setError("Failed to load root node in searchRecursive.");
        return 0;
    }

    size_t i = 0;
    // Find start index
    while(i < node->getKeyCount() && key > node->getKeyAt(i))
    {
        ++i;
    }
    // If leaf node starting point found
    if(node->isLeafNode() == 1)
    {
        uint32_t resultRBN = nodeRBN;
        delete node;
        return resultRBN;
    }
    else
    {   // Move onto next child rbn
        uint32_t nextRBN = node->getChildRBN(i);
        // Clean
        delete node;
        // If end is reached exit
        if(nextRBN == 0)
        {
            setError("Tried to access null RBN pointer in search recursive function.");
            return 0;
        }
        // Recursive call to next RBN to find the leaf node
        return rangeSearch(nextRBN, key);
    }
}