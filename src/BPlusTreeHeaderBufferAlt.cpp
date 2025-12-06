#include "BPlusTreeHeaderBufferAlt.h"

BPlusTreeHeaderBufferAlt::BPlusTreeHeaderBufferAlt() : errorState(false), lastError("") {}

BPlusTreeHeaderBufferAlt::~BPlusTreeHeaderBufferAlt()
{
    // Destructor
}

bool BPlusTreeHeaderBufferAlt::readHeader(const std::string& filename, BPlusTreeHeaderAlt& bHeader)
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
    bHeader = BPlusTreeHeaderAlt::deserialize(buffer.data());
    return true;
}

bool BPlusTreeHeaderBufferAlt::writeHeader(std::fstream& file, BPlusTreeHeaderAlt& bHeader)
{
    // Overwrite header via stream passed from page buffer
    // Seek to beginning of file
    file.seekp(0, std::ios::beg); 
    // Serialize tree header
    auto headerData = bHeader.serialize();
    // rewrite header
    file.write(reinterpret_cast<char*>(headerData.data()), headerData.size());
    return true;
}

bool BPlusTreeHeaderBufferAlt::hasError() const 
{
    return errorState;
}

std::string BPlusTreeHeaderBufferAlt::getLastError() const 
{
    return lastError;
}

void BPlusTreeHeaderBufferAlt::setError(const std::string& message) 
{
    errorState = true;
    lastError = message;
}