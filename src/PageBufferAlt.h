#ifndef PAGEBUFFERALT_H
#define PAGEBUFFERALT_H
#include <vector>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

class PageBufferAlt 
{
public:
    PageBufferAlt();
    ~PageBufferAlt();

    bool open(const std::string& filename, size_t blockSize, size_t headerSize);
    bool getIsOpen() const;
    bool readBlock(uint32_t rbn, std::vector<uint8_t>& data);
    bool writeBlock(uint32_t rbn, const std::vector<uint8_t>& data);
    bool hasError() const;


    void setFileName(const std::string& filename);
    void closeFile();

    std::string getFileName() const;
    std::string getLastError() const;

    std::fstream& getFileStream();
   
private:
    std::fstream file;
    size_t blockSize;
    size_t headerSize;
    bool isOpen;
    bool errorState;
    std::string lastError;
    std::string fileName;

    void setError(const std::string& message);
};

#endif // PAGEBUFFERALT_H