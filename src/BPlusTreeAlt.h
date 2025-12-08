#ifndef BPLUS_TREE_ALT
#define BPLUS_TREE_ALT

#include "HeaderBuffer.h"
#include "BlockBuffer.h"
#include "BPlusTreeHeaderBufferAlt.h"
#include "NodeAlt.h"
#include "PageBufferAlt.h"
#include <string>
#include <cstdint>
#include <iostream>
#include <vector>

// Structure representing a b plus tree index entry.
struct IndexEntry
{
    uint32_t key;
     uint32_t blockRBN;
};

/**
 * @class BPlusTreeAlt
 * @brief Class responsible for the building and maintain of a B+ tree living on the disk.
 * @details Implements standard algorithms for the build of the tree. Has functions for searching, insertion, removal, and range queries.
 */
class BPlusTreeAlt
{
public:
    /**
     * @brief Default Constructor
     */
    BPlusTreeAlt();
    /**
     * @brief Destructor
     */
    ~BPlusTreeAlt();

    /**
     * @brief Opens the index file and sequence set file while setting up all necessary buffers and dependencies.
     * @param indexFileName The name of the index file that will be created or updated.
     * @param sequenceSetFilename The name of the sequence set file the index was or will be built from.
     * @return True if the files were opened successfully and all buffers were initialized.
     */
    bool open(const std::string& indexFileName, const std::string& sequenceSetFilename);
    /**
     * @brief Checks if the B+ tree index file is open.
     * @return True if the file is open.
     */
    bool isFileOpen() const;
    /**
     * @brief Builds a B+ tree index from a blocked sequence set zcb file.
     * @return True if the B+ tree index was succesfully built and written to disk.
     */
    bool buildFromSequenceSet();
    /**
     * @brief Searches the B+ tree index file to find the RBN in the blocked sequence that contains a given key.
     * @param key The zip code that is being searched for.
     * @param outValue The RBN the zip code is stored in within the sequence set.
     * @return Returns true if the key was found in a leaf node and the rbn was passed via outValue.
     */
    bool search(uint32_t key, uint32_t& outValue);
    /**
     * @brief Exposed insert api that calls insert recursive to add a new key and value to the B+ tree.
     * @param The key that needs to be added to the tree.
     * @param blockRBN The block number the given key resides in within the sequence set file.
     * @return Function returns true if the new key value pair was successfully inserted into the B+ tree.
     */
    bool insert(uint32_t key, uint32_t blockRBN);
    /**
     * @brief Exposed remove api that calls remove recursive to remove a given key from the B+ tree.
     * @param key The key that is to be removed from the B+ tree.
     * @return Function returns true if the key was found and successfully removed.
     */
    bool remove(uint32_t key);
    /**
     * @brief Gets whether the B+ tree class has encountered an error in any of it's processes.
     * @return Function returns true if the error flag is set to true.
     */
    bool hasError() const;
    /**
     * @brief Gets whether the index is stale and needs to be rebuilt. Redundant function.
     * @return True if the index could be stale.
     */
    bool getIsStale() const;

    /**
     * @brief Returns the last error encountered by the B+ tree class.
     * @return Returns the last error as a string.
     */
    std::string getLastError() const;

    /**
     * @brief Given a key findLeafRBN finds the leaf node that should contain that key.
     * @param key The key that should be contained within the RBN the found leaf points to.
     * @return The RBN of the leaf in the B+ tree index.
     */
    uint32_t findLeafRBN(uint32_t key);
    /**
     * @brief Alternative search function using a slightly more efficient recursive search algorithm.
     * @param nodeRBN The node the recursive search is starting from.
     * @param key The key the search is attempting to find.
     * @return The function returns the RBN of the leaf node that contains the pointer to the sequence set block the key is contained in.
     */
    uint32_t searchRecursive(uint32_t nodeRBN, uint32_t key);

    /**
     * @brief Finds the rbn that a key should be placed within the sequence set.
     * @param The key that will be added to B+ tree and represent the ZipCodeRecord.
     * @return Returns the RBN that record will be placed in the sequence set file.
     */
    uint32_t findInsertionBlock(uint32_t key);
    
    /**
     * @brief Searches the B+ tree for all values inbetween to uint32_t's.
     * @details Uses helper function rangeSearch to find the node to start the search at.
     * @param keyStart The lower limit of the search.
     * @param keyEnd The upper limit of the search
     * @return The function returns an vector of the keys inclusive and inbetween to lower and upper search limit.
     */
    std::vector<uint32_t> searchRange(const uint32_t keyStart, const uint32_t keyEnd);

    /**
     * @brief Helper function that prints the B+ tree to the terminal.
     * @details Reccomended to use in test programs ran with "*command*>*dump_output.txt*"
     */
    void printTree();
    /**
     * @brief Closes the files, all buffers, and rewrites the changed data such as the treeHeader.
     */
    void close();

    //void convertIndexToBPlusTree(const std::vector<BlockIndexFile::IndexEntry>& indexEntries, const std::string& bPlusTreeFileName, uint32_t blockSize);

private:
    bool isOpen; // Is the B+ tree file open (redundant with PageBufferAlt?)
    bool errorState; // Error state flag
    bool isStale; // Indicates if the B+ tree is stale

    std::string errorMessage; // Stores the last error encountered.
    std::string sequenceSetFilename;
    std::string indexFilename;


    BPlusTreeHeaderAlt treeHeader; // Stored for necessary index realted metadata
    HeaderRecord sequenceHeader; // Stored for necessary sequence set related metadata

    PageBufferAlt indexPageBuffer; // Buffer for index file pages
    BlockBuffer sequenceSetBuffer; // Buffer for sequence set blocks

    uint32_t sequenceHeaderSize; // Cahced header size for convenience
    uint32_t blockSize; // Cahced block size for convenience

    /**
     * @brief Given a valid node rbn this function loads an active node from the B+ tree file.
     * @param rbn The B+ tree rbn of the node to be loaded.
     * @return A pointer to a newly created and active node in memory.
     */
    NodeAlt* loadNode(uint32_t rbn);
    
    /**
     * @brief Splits a node into two if a node were to exceed the maximum size.
     * @details Handles the updating of node pointers within the B+ tree as well as key promotion.
     * @param nodeRBN The node that is to be split.
     * @param promotedKey The key that needs to be moved up and adjacent within the B+ tree index.
     * @return The new rbn of the node that was split.
     */
    uint32_t splitNode(uint32_t nodeRBN, uint32_t& promotedKey);
    /**
     * @brief Allocates a new node for the B+ tree to work with.
     * @details Created a new RBN by getting the index block count from the header, incrementing it, and setting the new index block count.
     * @return Returns the new rbn allocated by the B+ tree.
     */
    uint32_t allocateTreeBlock();
    /**
     * @brief Helper function that finds the initial position of a range query.
     * @details Traverses the B+ tree until it finds a node whose key is greater than it's key parameter.
     * @param nodeRBN The rbn of the node to load and start the search with.
     * @param key The key that a given node must be greater than to find the initial range query position.
     * @return Returns the RBN of the node to load and start the range quety with.
     */
    uint32_t rangeSearch(uint32_t nodeRBN, uint32_t key);

    /**
     * @brief Serializes the active data of a node to the disk.
     * @param rbn The rbn of the node to be written to the disk.
     * @param node The active node whose data shoud be serialized at the given rbn location.|
     * @return Returns true if the node was succesffuly written to the disk.
     */
    bool writeNode(uint32_t rbn, const NodeAlt& node);
    /**
     * @brief Builds a B+ tree from a vector of IndexEntry structures.
     * @details Called by build sequence set. Calls helper functions buildLeafLevel and buildIndexLevel for tree construction.
     * @param entries A vector of IndexEntry structs passed by reference to be passed to the build helper functions.
     * @return The functions returns true if index entries was succesfully passed and is not empty.
     */
    bool buildTreeFromEntries(const std::vector<IndexEntry>& entries);
    /**
     * @brief Attempts to insert a new key value pair into the B+ tree.
     * @details Called by the public insert function. Recursively traverses the tree finding the proper place to insert the key value pair. 
     * Handles key promotion and splitting for leaf and index nodes.
     * @param nodeRBN The node to insert the key value pair into.
     * @param key The key to be inserted.
     * @param value The value to be inserted.
     * @param newChildRBN The rbn of the new child if a split occurs.
     * @param newPromotedKey The value of the key to be moved up and adjacent to an index if a split occurs.
     * @return Returns true if a split occurred and false if otherwise.
     */
    bool insertRecursive(uint32_t nodeRBN, uint32_t key, uint32_t value, 
                                    uint32_t& newChildRBN, uint32_t& newPromotedKey);
    /** 
     * @brief Recursively attempts to remove a given key from the B+ tree.
     * @details Recursively traverses the tree to find the node to remove. Handles leaf and index edge cases such as underfull or needing to update the parent key.
     * @param nodeRBN The node to start removal and tree traversal from.
     * @param key The key to be removed from the B+ tree.
     * @param parentRBN The RBN of the parent of the noad loaded by nodeRBN.
     * @param indexInParent The index location of the node loaded by nodeRBN within the node loaded by parentRBN.
     * @return the function returns true if key was found and removed succesfully.
    */
    bool removeRecursive(uint32_t nodeRBN, uint32_t key, uint32_t parentRBN, size_t indexInParent);

    /**
     * @brief Attempts to borrow key, values, or children from adjacent nodes to maintain tree balance.
     * @details Handles the borrowing of left and right siblings as well as leaf and index nodes.
     * @param nodeRBN The RBN of the underfull node that needs to attempt borrowing.
     * @param parentRBN The RBN of the parent of the node loaded by nodeRBN.
     * @param indexInParent The index of node loaded by nodeRBN within the children vector of the node loaded by parentRBN.
     */
    bool borrowFromSibling(uint32_t nodeRBN, uint32_t parentRBN, size_t indexInParent);
    /**
     * @brief Attempts to merge two adjacent nodes into one node.
     * @details Handles the merging of left and right siblings as well as leaf and index nodes. 
     * Checks if the parent is underfull after a merge and recursively calls if it is underfull.
     * @param nodeRBN The RBN of the underfull node that needs to be merged.
     * @param parentRBN The RBN of the parent of the node loaded by nodeRBN.
     * @param indexInparent The index of the node loaded by nodeRBN within the children vector of the node loaded by parentRBN.
     * @return Returns true if a merge occurred.
     */
    bool mergeWithSibling(uint32_t nodeRBN, uint32_t parentRBN, size_t indexInParent);

    /**
     * @brief Sets the error state of the B+ tree to true and updates the error message to reflect the most recent error.
     * @param message The error message to be stored.
     */
    void setError(const std::string& message);
    /**
     * @brief Unimplemented. Blocks remain dead and unaccessible until the index is fully rebuilt.
     * @details Just as SQL databases do not truly care data until a full rebuild old nodes and RBN chains remain unnaccessable until rebuild.
     */
    void freeIndexBlock(uint32_t rbn);
    /**
     * @brief Helper function that prints the contents of a node.
     * @param rbn The rbn of the node to print.
     * @param depth The depth of the string indent to use.
     */
    void printNode(uint32_t rbn, int depth);
    /**
     * @brief Traverses a leaf node and inserts a given key and value into the node.
     * @param node The active leaf node to insert the key and value into.
     * @param key The key to be inserted into keys.
     * @param value The value to be inserted into values.
     */
    void insertIntoLeaf(NodeAlt* node, uint32_t key, uint32_t value);
    /**
     * @brief Traverses an index node and inserts a given key and childRBN into the node.
     * @param node The active index node to insert the key and childRBN into.
     * @param key The key to be inserted into keys.
     * @param The childRBN to be inserted into children.
     */
    void insertIntoIndex(NodeAlt* node, uint32_t key, uint32_t childRBN);
    /**
     * @brief Helper function for remove recursive that updates the parent key index with the new seperator key in case of root leaf node edge case.
     * @param parentRBN The RBN of parent node that needs to be updated.
     * @param indexInParent The index of the child node within the parent. Used to find the separator index.
     * @param newKey The new key to be stored at the seperator index.
     */
    void updateParentKey(uint32_t parentRBN, size_t indexInParent, uint32_t newKey);

    /**
     * @brief Helper function responsible for building the leaf level of the B+ tree from a vector of index entries.
     * @details Called by build tree from entries. Uses the standard bottom up leaf building nested loop algorithm.
     * @param entries The vector of index entries the leaf level will be built from.
     * @return Returns a vector of child RBNs to build the index level from.
     */
    std::vector<uint32_t> buildLeafLevel(const std::vector<IndexEntry>& entries);
    /**
     * @brief Helper function responsible for building the index level of the B+ tree from a vector of keys.
     * @details Called by build tree from entries. Uses the standard bottom up index building nested loop algorithm.
     * @param childRBNs A vector of child RBNs used to find the appropriate index block locations.
     * @return A vector or parentRBN's that are used to determine the height of the B+ tree within build tree from entries.
     */
    std::vector<uint32_t> buildIndexLevel(const std::vector<uint32_t>& childRBNs);

    bool freeRBN(uint32_t rbn);
};

#endif

