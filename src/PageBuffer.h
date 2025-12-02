#ifndef PAGEBUFFER_H
#define PAGEBUFFER_H
#include <vector>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <algorithm>

class PageBuffer {
public:
    /**
     * @brief Default constructor
     * @details Initializes PageBuffer
     */
    PageBuffer();
    /**
     * @brief Destructor
     * @details Cleans up PageBuffer
     */
    ~PageBuffer() = default;
    /**
     * @brief Load Page
     * @details Loads a page from disk at the specified page number
     * @param pageNumber the page number to load
     * @returns pointer to loaded page data (caller responsible for deletion)
     */
    char* loadPage(uint32_t pageNumber) const;
    /**
     * @brief Write Page
     * @details Writes a page to disk at the specified page number
     * @param pageNumber the page number to write to
     * @param data the page data to write
     * @returns true if write was successful
     */
    bool writePage(uint32_t pageNumber, const char* data);
    /**
     * @brief Open Page File
     * @details Opens the page file for reading and writing
     * @param filename the name of the page file to open
     * @returns true if file was opened successfully
     */
    bool open(const std::string& filename);
    /**
     * @brief Close Page File
     * @details Closes the page file
     */
    void close();
    /**
     * @brief Is Open
     * @details Checks if the page file is currently open
     * @returns true if file is open, false otherwise
     */
    bool isOpen() const;
    private:
    std::fstream pageFile; // File stream for page file
};



#endif // PAGEBUFFER_H