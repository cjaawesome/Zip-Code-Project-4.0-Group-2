#ifndef PAGEBUFFERALT_H
#define PAGEBUFFERALT_H
#include <vector>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

/**
 * @class PageBufferAlt
 * @brief Manages block-based file I/O operations with buffering support.
 * @details Provides an interface for reading and writing fixed-size blocks to a file,
 *          with support for a header region. Uses Relative Block Numbers (RBN) for addressing.
 */
class PageBufferAlt 
{
public:
    /**
     * @brief Default Constructor
     * @details Initializes a PageBufferAlt object in a closed state.
     */
    PageBufferAlt();
    
    /**
     * @brief Default Destructor
     * @details Ensures the file is properly closed before destruction.
     */
    ~PageBufferAlt();

    /**
     * @brief Opens a file for block-based I/O operations.
     * @param filename The path to the file to open or create.
     * @param blockSize The size of each block in bytes.
     * @param headerSize The size of the header region in bytes (precedes block data).
     * @return True if the file was successfully opened. False otherwise.
     */
    bool open(const std::string& filename, size_t blockSize, size_t headerSize);
    
    /**
     * @brief Checks if a file is currently open.
     * @return True if a file is open. False otherwise.
     */
    bool getIsOpen() const;
    
    /**
     * @brief Reads a block of data from the file at the specified RBN.
     * @param rbn The Relative Block Number to read from.
     * @param data Output vector to store the read block data.
     * @return True if the block was successfully read. False on error.
     */
    bool readBlock(uint32_t rbn, std::vector<uint8_t>& data);
    
    /**
     * @brief Writes a block of data to the file at the specified RBN.
     * @param rbn The Relative Block Number to write to.
     * @param data The data to write (must match blockSize).
     * @return True if the block was successfully written. False on error.
     */
    bool writeBlock(uint32_t rbn, const std::vector<uint8_t>& data);
    
    /**
     * @brief Checks if the buffer is in an error state.
     * @return True if an error has occurred. False otherwise.
     */
    bool hasError() const;

    /**
     * @brief Sets the filename for future operations.
     * @param filename The path to the file.
     */
    void setFileName(const std::string& filename);
    
    /**
     * @brief Closes the currently open file.
     * @details Flushes any pending writes and closes the file stream.
     */
    void closeFile();

    /**
     * @brief Gets the current filename.
     * @return The filename string.
     */
    std::string getFileName() const;
    
    /**
     * @brief Gets the most recent error message.
     * @return A string describing the last error, or empty if no error exists.
     */
    std::string getLastError() const;

    /**
     * @brief Gets a reference to the underlying file stream.
     * @details Allows direct manipulation of the file stream if needed.
     * @return Reference to the internal fstream object.
     */
    std::fstream& getFileStream();
   
private:
    std::fstream file;        // File stream for reading and writing blocks
    size_t blockSize;         // Size of each block in bytes
    size_t headerSize;        // Size of the header region preceding block data
    bool isOpen;              // Flag indicating if a file is currently open
    bool errorState;          // Flag indicating if an error has occurred
    std::string lastError;    // Description of the most recent error
    std::string fileName;     // Path to the currently associated file

    /**
     * @brief Sets the buffer into an error state with a descriptive message.
     * @param message The error message to store.
     */
    void setError(const std::string& message);
};

#endif // PAGEBUFFERALT_H