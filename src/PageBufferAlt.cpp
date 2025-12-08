#include "PageBufferAlt.h"

PageBufferAlt::PageBufferAlt() : isOpen(false), errorState(false), lastError("") 
{
}

PageBufferAlt::~PageBufferAlt() 
{
    if (isOpen) 
    {
        file.close();
    }
}

std::string PageBufferAlt::getFileName() const 
{
    return fileName;
}

bool PageBufferAlt::open(const std::string& filename, size_t blockSize, size_t headerSize) 
{
    this->blockSize = blockSize;
    this->headerSize = headerSize;
    setFileName(filename);
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) 
    {
        setError("Failed to open file: " + filename);
        return false;
    }
    isOpen = true;
    return true;
}

void PageBufferAlt::setFileName(const std::string& filename)
{
    this->fileName = filename;
}

bool PageBufferAlt::getIsOpen() const
{
    return isOpen;
}

bool PageBufferAlt::readBlock(uint32_t rbn, std::vector<uint8_t>& data)
{
    if (!isOpen) 
    {
        setError("File not open for reading");
        return false;
    }

    data.resize(blockSize);
    file.seekg(headerSize + rbn * blockSize, std::ios::beg);
    file.read(reinterpret_cast<char*>(data.data()), blockSize);
    if (file.gcount() < static_cast<std::streamsize>(blockSize)) 
    {
        setError("Failed to read full block at RBN: " + std::to_string(rbn));
        return false;
    }
    return true;
}

bool PageBufferAlt::writeBlock(uint32_t rbn, const std::vector<uint8_t>& data)
{
    if (!isOpen) 
    {
        setError("File not open for writing");
        return false;
    }
    if (data.size() != blockSize) 
    {
        setError("Data size does not match block size");
        return false;
    }

    file.seekp(headerSize + rbn * blockSize, std::ios::beg);
    file.write(reinterpret_cast<const char*>(data.data()), blockSize);
    if (!file) 
    {
        setError("Failed to write full block at RBN: " + std::to_string(rbn));
        return false;
    }
    
    file.flush();

    return true;
}

bool PageBufferAlt::hasError() const 
{
    return errorState;
}

std::string PageBufferAlt::getLastError() const 
{
    return lastError;
}

void PageBufferAlt::closeFile()
{
    file.close();
}

void PageBufferAlt::setError(const std::string& message)
{
    errorState = true;
    lastError = message;
}

std::fstream& PageBufferAlt::getFileStream()
{
    return file;
}