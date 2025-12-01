#include "BPlusTreeHeaderBuffer.h"
#include <fstream>
#include <vector>
#include <cstring>

BPlusTreeHeaderBuffer::BPlusTreeHeaderBuffer() : errorState(false), lastError("") {}

BPlusTreeHeaderBuffer::~BPlusTreeHeaderBuffer()
{
    // Destructor
}

bool BPlusTreeHeaderBuffer::readHeader(const std::string& filename, BPlusTreeHeader& bHeader)
{
     std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        setError("Cannot open file: " + filename);
        std::cerr << getLastError() << std::endl;
        return false;
    }

     // Read first 4 bytes to get headerSize
    uint32_t headerSize = 0;
    file.read(reinterpret_cast<char*>(&headerSize), sizeof(headerSize));
    if (file.gcount() < sizeof(headerSize)) 
    {
        setError("Cannot read header size");
        return false;
    }

    // Read full header based on headerSize 
    std::vector<uint8_t> buffer(headerSize);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), headerSize);
    if (file.gcount() < static_cast<std::streamsize>(headerSize)) 
    {
        setError("Header incomplete");
        std::cerr << getLastError() << std::endl;
        return false;
    }

    file.close();

    // Deserialize header
    bHeader = BPlusTreeHeader::deserialize(buffer.data());
    return true;
}

bool BPlusTreeHeaderBuffer::writeHeader(const std::string& filename, const BPlusTreeHeader& bHeader)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) 
    {
        setError("Cannot create file: " + filename);
        return false;
    }
    
    auto headerData = bHeader.serialize();
    file.write(reinterpret_cast<char*>(headerData.data()), headerData.size());
    file.close();
    
    return true;
}

bool BPlusTreeHeaderBuffer::hasError() const 
{
    return errorState;
}

std::string BPlusTreeHeaderBuffer::getLastError() const 
{
    return lastError;
}

void BPlusTreeHeaderBuffer::setError(const std::string& message) 
{
    errorState = true;
    lastError = message;
}