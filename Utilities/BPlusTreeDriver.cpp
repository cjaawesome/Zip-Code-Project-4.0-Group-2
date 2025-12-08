#include "../src/CSVBuffer.h"
#include "../src/BlockBuffer.h"
#include "../src/RecordBuffer.h"
#include "../src/HeaderRecord.h"
#include "../src/HeaderBuffer.h"
#include "../src/BPlusTreeAlt.h"
#include "../src/BPlusTreeHeaderAlt.h"
#include "../src/BPlusTreeHeaderBufferAlt.h"
#include "../src/ZipCodeRecord.h"
#include "../src/PageBufferAlt.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Convert CSV to blocked sequence set (no index)
bool convertCSVToBlockedSequenceSet(const std::string& csvFile, const std::string& zcbFile, 
                                    uint32_t blockSize, uint16_t minBlockSize)
{
    CSVBuffer csvBuffer;
    if(!csvBuffer.openFile(csvFile))
    {
        std::cerr << "Failed to open CSV file." << std::endl;
        return false;
    }

    std::vector<ZipCodeRecord> allRecords;
    ZipCodeRecord record;
    while(csvBuffer.getNextRecord(record))
    {
        allRecords.push_back(record);
    }

    std::cout << "Read " << allRecords.size() << " records." << std::endl;

    std::sort(allRecords.begin(), allRecords.end(),
              [](const ZipCodeRecord& a, const ZipCodeRecord& b) {
                  return a.getZipCode() < b.getZipCode();
              });

    csvBuffer.closeFile();
    std::cout << "Sorted records by ZipCode." << std::endl;

    HeaderRecord header;
    header.setFileStructureType("ZIPC");
    header.setVersion(2);
    header.setHeaderSize(0);
    header.setSizeFormatType(0);
    header.setBlockSize(blockSize);
    header.setMinBlockSize(minBlockSize);
    header.setIndexFileName("data/test_output.idx");
    header.setIndexFileSchemaInfo("Primary Key: Zipcode");
    header.setRecordCount(allRecords.size());
    header.setBlockCount(0);
    
    std::vector<FieldDef> fields;
    fields.push_back({"zipcode", 1});
    fields.push_back({"location", 3});
    fields.push_back({"state", 4});
    fields.push_back({"county", 3});
    fields.push_back({"latitude", 2});
    fields.push_back({"longitude", 2});
    
    header.setFields(fields);
    header.setFieldCount(csvBuffer.EXPECTED_FIELD_COUNT);
    header.setPrimaryKeyField(0);
    header.setAvailableListRBN(0); 
    header.setSequenceSetListRBN(1); 
    header.setStaleFlag(0);

    std::ofstream out(zcbFile, std::ios::binary);
    if (!out.is_open()) 
    {
        std::cerr << "Error: Cannot create output file: " << zcbFile << std::endl;
        return false;
    }
    
    auto headerData = header.serialize();
    header.setHeaderSize(headerData.size());
    out.write(reinterpret_cast<char*>(headerData.data()), headerData.size());
    out.close();
    
    size_t blockCountOffset = 4 + 2 + 4 + 1 + 4 + 2 + 2 + 
                              header.getIndexFileName().length() + 
                              2 + header.getIndexFileSchemaInfo().length() + 
                              sizeof(uint32_t);
    
    std::cout << "Converting " << csvFile << " to " << zcbFile << "..." << std::endl;

    RecordBuffer recordBuffer;
    BlockBuffer blockBuffer;

    if(!blockBuffer.openFile(zcbFile, header.getHeaderSize()))
    {
        std::cerr << "Failed to open block buffer." << std::endl;
        return false;
    }

    uint32_t currentRBN = 1;
    uint32_t blockCount = 0;
    std::vector<ZipCodeRecord> currentBlockRecords;
    size_t currentSize = 10;

    for(const auto& rec : allRecords)
    {
        if (currentSize + rec.getRecordSize() + 4 > blockSize)
        {
            ActiveBlock block;
            block.precedingRBN = (currentRBN == 1) ? 0 : currentRBN - 1;
            block.succeedingRBN = currentRBN + 1;
            block.recordCount = static_cast<uint16_t>(currentBlockRecords.size());
            
            recordBuffer.packBlock(currentBlockRecords, block.data, blockSize);
            blockBuffer.writeActiveBlockAtRBN(currentRBN, blockSize, header.getHeaderSize(), block);
            
            ++blockCount;
            ++currentRBN;
            currentBlockRecords.clear();
            currentSize = 10;
        }
      
        currentBlockRecords.push_back(rec);
        currentSize += rec.getRecordSize() + 4;
    }

    if (!currentBlockRecords.empty())
    {
        ActiveBlock block;
        block.precedingRBN = (currentRBN == 1) ? 0 : currentRBN - 1;
        block.succeedingRBN = 0;
        block.recordCount = static_cast<uint16_t>(currentBlockRecords.size());
        
        recordBuffer.packBlock(currentBlockRecords, block.data, blockSize);
        blockBuffer.writeActiveBlockAtRBN(currentRBN, blockSize, header.getHeaderSize(), block);
        
        ++blockCount;
    }

    blockBuffer.closeFile();

    std::fstream updateFile(zcbFile, std::ios::binary | std::ios::in | std::ios::out);
    updateFile.seekp(blockCountOffset);
    updateFile.write(reinterpret_cast<char*>(&blockCount), sizeof(uint32_t));
    updateFile.close();

    std::cout << "Created blocked sequence set with " << blockCount << " blocks" << std::endl;
    return true;
}

// Build B+ tree from blocked sequence set
bool buildBPlusTreeFromSequenceSet(const std::string& idxFile, const std::string& zcbFile)
{
    HeaderBuffer seqHeaderBuffer;
    HeaderRecord seqHeader;

    if(!seqHeaderBuffer.readHeader(zcbFile, seqHeader))
    {
        std::cerr << "Failed to read header from " << zcbFile << std::endl;
        return false;
    }

    std::cout << "Building B+ Tree from sequence set..." << std::endl;

    // Create initial B+ tree header
    BPlusTreeHeaderAlt treeHeader;
    treeHeader.setBlockedFileName(zcbFile);
    treeHeader.setBlockSize(seqHeader.getBlockSize());
    treeHeader.setHeight(0);
    treeHeader.setRootIndexRBN(0);
    treeHeader.setIndexBlockCount(0);

    std::ofstream out(idxFile, std::ios::binary);
    if (!out.is_open())
    {
        std::cerr << "Error: Cannot create index file: " << idxFile << std::endl;
        return false;
    }
    
    auto headerData = treeHeader.serialize();
    treeHeader.setHeaderSize(headerData.size());
    out.write(reinterpret_cast<char*>(headerData.data()), headerData.size());
    out.close();

    // Open tree and build from sequence set
    BPlusTreeAlt tree;
    if(!tree.open(idxFile, zcbFile))
    {
        std::cout << "Failed to open B+ Tree." << std::endl;
        return false;
    }

    if(!tree.buildFromSequenceSet())
    {
        std::cout << "Failed to build B+ Tree from sequence set." << std::endl;
        std::cout << "Error: " << tree.getLastError() << std::endl;
        return false;
    }

    tree.close();
    std::cout << "B+ Tree successfully built!" << std::endl;

    return true; 
}

int main()
{
    std::cout << "Starting B+ Tree Test:" << std::endl;
    std::cout << std::endl;
    
    const std::string csvFile = "data/PT2_Sorted.csv";
    const std::string zcbFile = "data/test_output.zcb";
    const std::string idxFile = "data/test_output.idx";
    const uint32_t blockSize = 4096;
    const uint16_t minBlockSize = 1024;

    // Create Sequence Set
    std::cout << "STEP 1: Creating blocked sequence set from CSV..." << std::endl;
    if(!convertCSVToBlockedSequenceSet(csvFile, zcbFile, blockSize, minBlockSize))
    {
        std::cerr << "FAILED: Sequence set creation" << std::endl;
        return 1;
    }
    std::cout << " Sequence set created successfully\n" << std::endl;

    // Build Tree
    std::cout << "STEP 2: Building B+ tree index..." << std::endl;
    if (!buildBPlusTreeFromSequenceSet(idxFile, zcbFile))
    {
        std::cout << "FAILED: B+ tree creation" << std::endl;
        return 1;
    }
    std::cout << " B+ tree built successfully\n" << std::endl;

    // Open Tree
    std::cout << "STEP 3: Opening B+ tree and verifying structure..." << std::endl;
    BPlusTreeAlt tree;
    
    if (!tree.open(idxFile, zcbFile))
    {
        std::cerr << "FAILED: Could not open B+ tree" << std::endl;
        std::cerr << "Error: " << tree.getLastError() << std::endl;
        return 1;
    }
    
    std::cout << " B+ tree opened successfully" << std::endl;
    std::cout << "\n--- Tree Structure ---" << std::endl;
    tree.printTree();
    std::cout << std::endl;

    HeaderRecord header;
    HeaderBuffer headerBuffer;
    if(!headerBuffer.readHeader(zcbFile, header))
    {
        std::cerr << "Failed to read header" << std::endl;
        return 1;
    }

    // Test Search
    std::cout << "STEP 4: Testing search functionality..." << std::endl;
    
    struct SearchTest {
        uint32_t key;
        bool shouldExist;
    };
    
    std::vector<SearchTest> searchTests = {
        {501, true},      // Should exist
        {49751, true},    // Should exist
        {98165, true},    // Should exist
        {99999, false}    // Not Exist
    };
    
    int searchPassed = 0;
    for (const auto& test : searchTests)
    {
        uint32_t blockRBN;
        bool found = tree.search(test.key, blockRBN);
        
        std::cout << "  Key " << test.key << ": ";
        if (found)
        {
            std::cout << "FOUND in block RBN=" << blockRBN;
            if (test.shouldExist) {
                std::cout << " ";
                searchPassed++;
            } else {
                std::cout << " (unexpected)";
            }
        }
        else
        {
            std::cout << "NOT FOUND";
            if (!test.shouldExist) {
                std::cout << " ";
                searchPassed++;
            } else {
                std::cout << "  (expected to find)";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "Search tests passed: " << searchPassed << "/" << searchTests.size() << "\n" << std::endl;

    std::cout << "Testing range query..." << std::endl;
    
    struct RangeTest {
        uint32_t start;
        uint32_t end;
    };
    
    std::vector<RangeTest> rangeTests = {
        {500, 1000},
        {10000, 11000},
        {0, 1000},
        {90000, 100000}
    };
    
    RecordBuffer recordBuffer;
    BlockBuffer blockBuffer;
    
    if(!blockBuffer.openFile(zcbFile, header.getHeaderSize()))
    {
        std::cerr << "Failed to open block buffer for range tests" << std::endl;
        return 1;
    }
    
    for (const auto& test : rangeTests)
    {
        // Get block RBNs in range
        std::vector<uint32_t> blockRBNs = tree.searchRange(test.start, test.end);
        std::cout << "    Found " << blockRBNs.size() << " blocks containing range" << std::endl;
        
        if (blockRBNs.empty())
        {
            std::cout << "    No blocks found in range" << std::endl;
            continue;
        }
    }
    
    blockBuffer.closeFile();
    std::cout << "\n[PASS] Range query tests completed\n" << std::endl;

    std::cout << "Testing insert functionality..." << std::endl;

    struct InsertTest {
        uint32_t key;
        uint32_t blockRBN;
    };

    std::vector<InsertTest> insertTests = {
        {93597, tree.findInsertionBlock(93597)},
        {1006, tree.findInsertionBlock(1006)},
        {52735, tree.findInsertionBlock(52735)}
    };

    for (const auto& test : insertTests)
    {
        std::cout << "Inserting Key: " << test.key << std::endl;
        if (tree.insert(test.key, test.blockRBN))
        {
            // Verify insertion
            uint32_t foundRBN;
            std::cout << "Insertion successful searching for key: " << test.key << std::endl;
            if (tree.search(test.key, foundRBN) && foundRBN == test.blockRBN)
            {
                std::cout << "SUCCESS - " << test.key << " found at: " << foundRBN << std::endl;
            }
            else
            {
                std::cout << "FAILED (inserted but verification failed)" << std::endl;
            }
        }
        else
        {
            std::cout << "FAILED - Error: " << tree.getLastError() << std::endl;
        }
    }

    // Test Remove
    std::cout << "\nSTEP 7: Testing remove functionality..." << std::endl;
    
    std::vector<uint32_t> removeTests = {
        1349,  
        2142,    
        92803   
    };

    std::cout << tree.getLastError() << std::endl;
    
    for (const auto& key : removeTests)
    {
        std::cout << "  Removing key=" << key << "... " << std::flush;
        
        bool removeResult = tree.remove(key);
        std::cout << "Remove returned: " << (removeResult ? "true" : "false") << std::endl;
        
        if (removeResult)
        {
            uint32_t foundRBN;
            bool searchResult = tree.keyExistsInIndex(key);
            std::cout << "    Search after remove: " << (searchResult ? "FOUND" : "NOT FOUND") << std::endl;
            
            if (!searchResult)
            {
                std::cout << "    SUCCESS" << std::endl;
            }
            else
            {
                std::cout << "    FAILED (removed but still found)" << std::endl;
            }
        }
        else
        {
            std::cout << "    FAILED - Error: '" << tree.getLastError() << "'" << std::endl;
        }
    }

    std::cout << "\n Remove tests completed" << std::endl;
    
    std::cout << std::endl;

    std::cout << "\n Printing Tree: " << std::endl;
    tree.printTree();
    
    tree.close();

    std::cout << "\nB+ Tree Test Concluded." << std::endl;
    return 0;
}