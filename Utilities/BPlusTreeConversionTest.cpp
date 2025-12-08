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
#include "../src/BlockIndexFile.h"
#include <fstream>
#include <iostream>
#include <string>

bool convertCSVToBlockedSequenceNoIndex(const std::string& csvFile, const std::string& zcbFile, 
                                    uint32_t blockSize = 1024, uint16_t minBlockSize = 256)
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
                [](const ZipCodeRecord& a, const ZipCodeRecord& b)
            {
                return a.getZipCode() < b.getZipCode();
            });

    csvBuffer.closeFile();

    std:: cout << "Sorted records by ZipCode." << std::endl;

    HeaderRecord header;

    header.setFileStructureType("ZIPC");
    header.setVersion(2);
    header.setHeaderSize(0); // Set In Serialization Process
    header.setSizeFormatType(0);
    header.setBlockSize(blockSize);
    header.setMinBlockSize(minBlockSize);
    header.setIndexFileName("data/zipcode_data.idx"); // Placeholder
    header.setIndexFileSchemaInfo("Primary Key: Zipcode"); // Placeholder
    header.setRecordCount(allRecords.size()); // This will need to be tracked and updated after if sorting changes
    header.setBlockCount(0); // Update After Conversion
    
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
    }

    uint32_t currentRBN = 1;
    uint32_t blockCount = 0;
    std::vector<ZipCodeRecord> currentBlockRecords;
    size_t currentSize = 10;  // metadata

    for(const auto& rec : allRecords)
    {
        // Check if adding this record would overflow
        if (currentSize + rec.getRecordSize() + 4 > blockSize)
        {
            // Write current block
            ActiveBlock block;
            block.precedingRBN = (currentRBN == 1) ? 0 : currentRBN - 1;
            block.succeedingRBN = currentRBN + 1;  // Temp - will fix last block later
            block.recordCount = static_cast<uint16_t>(currentBlockRecords.size());
            
            recordBuffer.packBlock(currentBlockRecords, block.data, blockSize);
            blockBuffer.writeActiveBlockAtRBN(currentRBN, blockSize, header.getHeaderSize(), block);
            
            ++blockCount;
            ++currentRBN;
            currentBlockRecords.clear();
            currentSize = 10;  // Reset to metadata size
        }
      
        // Add record to current block
        currentBlockRecords.push_back(rec);
        currentSize += rec.getRecordSize() + 4;
    }

    // Write final block
    if (!currentBlockRecords.empty())
    {
        ActiveBlock block;
        block.precedingRBN = (currentRBN == 1) ? 0 : currentRBN - 1;
        block.succeedingRBN = 0;  // Last block
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

    return true;
}

bool convertBlockedSequenceSetToBPlusTree(const std::string& idxFile, const std::string& zcbFile)
{
    HeaderBuffer seqHeaderBuffer;
    HeaderRecord seqHeader;

    if(!seqHeaderBuffer.readHeader(zcbFile, seqHeader))
    {
        std::cerr << "Failed To Read Header From " << zcbFile << std::endl;
        return false;
    }

    std::cout << "Creating B+ Tree Index From Blocked Sequence Set File." << std::endl;

    std::vector<IndexEntry> indexEntries;

    BlockBuffer blockBuffer;
    RecordBuffer recordBuffer;

    if(!blockBuffer.openFile(zcbFile, seqHeader.getHeaderSize()))
    {
        std::cerr << "Failed To Open Block Buffer." << std::endl;
        return false;
    }

    uint32_t currentRBN = seqHeader.getSequenceSetListRBN();
    uint32_t maxBlocks = seqHeader.getBlockCount();
    uint32_t safetyCount = 0;
    while(currentRBN != 0 && safetyCount < maxBlocks + 1)
    {
        ActiveBlock block = blockBuffer.loadActiveBlockAtRBN(currentRBN, seqHeader.getBlockSize(), 
                                                                seqHeader.getHeaderSize());

        std::vector<ZipCodeRecord> records;
        recordBuffer.unpackBlock(block.data, records);

        if(!records.empty())
        {
            IndexEntry entry;
            entry.key = records.back().getZipCode();
            entry.blockRBN = currentRBN;
            indexEntries.push_back(entry);
        }
        currentRBN = block.succeedingRBN;
        safetyCount++;
    }

    if(safetyCount >= maxBlocks + 1)
    {
        std::cerr << "Error: Exceeded maximum block count while scanning sequence set." << std::endl;
        return false;
    }

    blockBuffer.closeFile();

    std::cout << "Scanned " << indexEntries.size() << " blocks from sequence set" << std::endl;

    //create B+ tree file
    
    BPlusTreeHeaderAlt treeHeader;
    treeHeader.setBlockedFileName(zcbFile);
    treeHeader.setBlockSize(seqHeader.getBlockSize());
    treeHeader.setHeight(0);
    treeHeader.setRootIndexRBN(0);

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

    BPlusTreeAlt tree;

    if(!tree.open(idxFile, zcbFile))
    {
        std::cerr << "Failed To Open B+ Tree." << std::endl;
        return false;
    }

    if(!tree.buildFromSequenceSet())
    {
        std::cerr << "Failed To Build B+ Tree From Sequence Set." << std::endl;
        return false;
    }

    std::cout << "B+ Tree Index Successfully Created." << std::endl;
    tree.close();

    std::fstream seqFile(zcbFile, std::ios::binary | std::ios::in | std::ios::out);
    uint8_t staleFlag = 0;
    size_t flagOffset = seqHeader.getHeaderSize() - 1;

    seqFile.seekp(flagOffset);
    seqFile.write(reinterpret_cast<char*>(&staleFlag), sizeof(uint8_t));
    seqFile.close();

    return true; 
}

bool convertCSVToBlockedSequenceSet(const std::string& csvFile, const std::string& zcbFile, 
                                    uint32_t blockSize = 1024, uint16_t minBlockSize = 256)
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
                [](const ZipCodeRecord& a, const ZipCodeRecord& b)
            {
                return a.getZipCode() < b.getZipCode();
            });

    csvBuffer.closeFile();

    std:: cout << "Sorted records by ZipCode." << std::endl;

    HeaderRecord header;

    header.setFileStructureType("ZIPC");
    header.setVersion(2);
    header.setHeaderSize(0); // Set In Serialization Process
    header.setSizeFormatType(0);
    header.setBlockSize(blockSize);
    header.setMinBlockSize(minBlockSize);
    header.setIndexFileName("data/zipcode_data.idx"); // Placeholder
    header.setIndexFileSchemaInfo("Primary Key: Zipcode"); // Placeholder
    header.setRecordCount(allRecords.size()); // This will need to be tracked and updated after if sorting changes
    header.setBlockCount(0); // Update After Conversion
    
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
    }

    uint32_t currentRBN = 1;
    uint32_t blockCount = 0;
    std::vector<ZipCodeRecord> currentBlockRecords;
    size_t currentSize = 10;  // metadata

    for(const auto& rec : allRecords)
    {
        // Check if adding this record would overflow
        if (currentSize + rec.getRecordSize() + 4 > blockSize)
        {
            // Write current block
            ActiveBlock block;
            block.precedingRBN = (currentRBN == 1) ? 0 : currentRBN - 1;
            block.succeedingRBN = currentRBN + 1;  // Temp - will fix last block later
            block.recordCount = static_cast<uint16_t>(currentBlockRecords.size());
            
            recordBuffer.packBlock(currentBlockRecords, block.data, blockSize);
            blockBuffer.writeActiveBlockAtRBN(currentRBN, blockSize, header.getHeaderSize(), block);
            
            ++blockCount;
            ++currentRBN;
            currentBlockRecords.clear();
            currentSize = 10;  // Reset to metadata size
        }
      
        // Add record to current block
        currentBlockRecords.push_back(rec);
        currentSize += rec.getRecordSize() + 4;
    }

    // Write final block
    if (!currentBlockRecords.empty())
    {
        ActiveBlock block;
        block.precedingRBN = (currentRBN == 1) ? 0 : currentRBN - 1;
        block.succeedingRBN = 0;  // Last block
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

    BlockIndexFile index;
    if(index.createIndexFromBlockedFile(zcbFile, blockSize, header.getHeaderSize(), 
                                        header.getSequenceSetListRBN()))
    {
        std::cout << "Index Succesfully Created. Now Writing Index." << std::endl;
        if(index.write(header.getIndexFileName()))
        {
            std::cout << "Index Successfully Written" << std::endl;
            std::fstream outFile(zcbFile, std::ios::binary | std::ios::in | std::ios::out);
            uint8_t staleFlag = 0;
            size_t flagOffset = header.getHeaderSize() - 1;
            
            outFile.seekp(flagOffset);
            outFile.write(reinterpret_cast<char*>(&staleFlag), sizeof(uint8_t));
            
            outFile.close();
        }
        else
        {
            std::cerr << "Error: Failed to write index file" << std::endl;
            return false;
        }
    }
    return true;
}

int main()
{
    std::cout << "=== B+ Tree Conversion Test ===" << std::endl;
    std::cout << std::endl;
    
    // Test files
    const std::string csvFile = "data/PT2_Sorted.csv";
    const std::string zcbFile = "data/test_output.zcb";
    const std::string idxFile = "data/test_output.idx";
    const std::string FILE_PATH_IN_RAND = "data/test_output.zcb";
    
    // Parameters
    const uint32_t blockSize = 4096;
    const uint16_t minBlockSize = 1024;

    std::cout << "Convert Blocked File With Index:" << std::endl;
    if(!convertCSVToBlockedSequenceSet(csvFile, zcbFile, blockSize, minBlockSize))
    {
        std::cerr << "FAILED: CSV to blocked sequence set conversion with index" << std::endl;
        return 1;
    }

    std::cout << "SUCCESS: Created " << zcbFile << " with index" << std::endl;
    std::cout << std::endl;
    /*
    // Step 1: Convert CSV to blocked sequence set
    std::cout << "Step 1: Converting CSV to blocked sequence set..." << std::endl;
    if (!convertCSVToBlockedSequenceNoIndex(csvFile, zcbFile, blockSize, minBlockSize))
    {
        std::cerr << "FAILED: CSV to blocked sequence set conversion" << std::endl;
        return 1;
    }
    std::cout << "SUCCESS: Created " << zcbFile << std::endl;
    std::cout << std::endl;
    */

    std::cout << "=== Blocked Sequence Set Test Program ===\n\n";
    
    // Test 1: Read header and index
    std::cout << "--- Test 1: Header and Index Loading ---\n";
    HeaderRecord header;
    HeaderBuffer headerBuffer;

    if(!headerBuffer.readHeader(FILE_PATH_IN_RAND, header))
    {
        std::cerr << "Failed To Read Header From " << FILE_PATH_IN_RAND << std::endl;
        return 1;
    }
    
    std::cout << "Header loaded successfully\n";
    std::cout << "  Block Size: " << header.getBlockSize() << "\n";
    std::cout << "  Block Count: " << header.getBlockCount() << "\n";
    std::cout << "  Record Count: " << header.getRecordCount() << "\n";
    std::cout << "  Stale Flag: " << (header.getStaleFlag() ? "STALE" : "VALID") << "\n\n";

    BlockIndexFile index;

    if(header.getStaleFlag())
    {
        std::cout << "Index is stale - rebuilding...\n";
        if(!(index.createIndexFromBlockedFile(FILE_PATH_IN_RAND, header.getBlockSize(), 
                                        header.getHeaderSize(), header.getSequenceSetListRBN())))
        {
            std::cerr << "Failed To Build Index.\n";
            return 1;
        }
        std::cout << "Index successfully rebuilt.\n\n";
    }
    else
    {
        if(index.read(header.getIndexFileName()))
        {
            std::cout << "Index file exists and was successfully read.\n\n";
        }
        else
        {
            std::cerr << "Failed To Read Existing Index File.\n";
            return 1;
        }
    }

    BlockBuffer blockBuffer;

     if(!blockBuffer.openFile(FILE_PATH_IN_RAND, header.getHeaderSize()))
    {
        std::cerr << "Failed to open block buffer\n";
        return 1;
    }

    std::cout << "--- Detailed Block Inspection ---\n";
    std::vector<uint32_t> rbnsToInspect = {1, 2, 3};  // Check first 3 blocks

    RecordBuffer recBuf;
    for(const auto& rbn : rbnsToInspect) 
    {
        std::cout << "\nInspecting Block RBN " << rbn << ":\n";
        ActiveBlock block = blockBuffer.loadActiveBlockAtRBN(rbn, header.getBlockSize(), header.getHeaderSize());
        
        std::cout << "  recordCount field: " << block.recordCount << "\n";
        std::cout << "  precedingRBN: " << block.precedingRBN << "\n";
        std::cout << "  succeedingRBN: " << block.succeedingRBN << "\n";
        std::cout << "  block.data.size(): " << block.data.size() << "\n";
        std::cout << "  getTotalSize(): " << block.getTotalSize() << "\n";
        
        std::vector<ZipCodeRecord> records;
        recBuf.unpackBlock(block.data, records);
        
        std::cout << "  Unpacked record count: " << records.size() << "\n";
        std::cout << "  Records in block:\n";
        for(size_t i = 0; i < records.size(); i++) {
            std::cout << "    [" << i << "] " << records[i].getZipCode() << " - " 
                    << records[i].getLocationName() << "\n";
        }
    }

    blockBuffer.closeFile();

    // Test 2: Search for specific records
    std::cout << "--- Test 2: Record Search ---\n";
    std::vector<uint32_t> zipToFind = {17111, 31836, 8053};

    if(!blockBuffer.openFile(FILE_PATH_IN_RAND, header.getHeaderSize()))
    {
        std::cerr << "Failed to open block buffer\n";
        return 1;
    }

    for(const auto& zip : zipToFind)
    {
        uint32_t rbn = index.findRBNForKey(zip);
        std::cout << "Searching for ZIP " << zip << " (RBN: " << rbn << ")... ";
        
        ZipCodeRecord tempRecord;
        if(blockBuffer.readRecordAtRBN(rbn, zip, header.getBlockSize(), 
                                      header.getHeaderSize(), tempRecord))
        {
            std::cout << "FOUND\n";
            std::cout << "  " << tempRecord << "\n";
        }
        else
        {
            std::cout << "NOT FOUND\n";
        }
    }
    std::cout << "\n";

    blockBuffer.closeFile();

    // Test 3: Physical dump
    std::cout << "--- Test 3: Physical Order Dump ---\n";
    if(!blockBuffer.openFile(FILE_PATH_IN_RAND, header.getHeaderSize()))
    {
        std::cerr << "Failed to reopen block buffer\n";
        return 1;
    }
    
    blockBuffer.dumpPhysicalOrder(std::cout, header.getSequenceSetListRBN(),
                                  header.getAvailableListRBN(), header.getBlockCount(),
                                  header.getBlockSize(), header.getHeaderSize());
    std::cout << "\n";
    
    // Test 4: Logical dump
    std::cout << "--- Test 4: Logical Order Dump ---\n";
    blockBuffer.dumpLogicalOrder(std::cout, header.getSequenceSetListRBN(),
                                 header.getAvailableListRBN(), header.getBlockSize(),
                                 header.getHeaderSize());
    std::cout << "\n";

    blockBuffer.closeFile();

    std::cout << "\n=== All Tests Completed Successfully! ===\n";

    // Step 2: Create B+ tree index from sequence set
    std::cout << "Step 2: Creating B+ tree index from sequence set..." << std::endl;
    if (!convertBlockedSequenceSetToBPlusTree(idxFile, zcbFile))
    {
        std::cerr << "FAILED: B+ tree index creation" << std::endl;
        return 1;
    }
    std::cout << "SUCCESS: Created " << idxFile << std::endl;
    std::cout << std::endl;
    
    // Step 3: Verify by opening and checking tree
    std::cout << "Step 3: Verifying B+ tree..." << std::endl;
    BPlusTreeAlt tree;
    
    if (!tree.open(idxFile, zcbFile))
    {
        std::cerr << "FAILED: Could not open B+ tree" << std::endl;
        std::cerr << "Error: " << tree.getLastError() << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: B+ tree opened successfully" << std::endl;
    
    // Print tree structure
    std::cout << std::endl;
    std::cout << "=== Tree Structure ===" << std::endl;
    tree.printTree();
    std::cout << std::endl;
    
    // Step 4: Test search functionality
    std::cout << "Step 4: Testing search..." << std::endl;
    
    // Test a few searches
    uint32_t testKeys[] = {501, 10001, 50000, 99999};
    
    for (uint32_t key : testKeys)
    {
        uint32_t blockRBN;
        if (tree.search(key, blockRBN))
        {
            std::cout << "  Key " << key << " found in block RBN=" << blockRBN << std::endl;
        }
        else
        {
            std::cout << "  Key " << key << " not found" << std::endl;
        }
    }
    
    tree.close();
    
    std::cout << std::endl;
    std::cout << "=== All Tests Passed! ===" << std::endl;
    
    return 0;
}