#ifndef BPlus_Tree_Header_Buffer
#define BPlus_Tree_Header_Buffer

#include "BPlusTreeHeader.h"
#include <string>
#include <cstring>

class BPlusTreeHeaderBuffer
{
public:
    /**
     * @brief Default constructor
     * @details Initializes header buffer
     */
    BPlusTreeHeaderBuffer();

    /**
     * @brief Default Destructor
     * @details No resources to clean up
     */
    ~BPlusTreeHeaderBuffer();
    
    /**
     * @brief read header
     * @details Opens file, reads header into memory, interprets bytes, uses the header to guide how you read the rest of the file
     * @param filename name of file being read
     * @param bHeader the header being read too
     * @returns true or false depending on if the header was read successfully or not
     */
    bool readHeader(const std::string& filename, BPlusTreeHeader& bHeader);
    /**
     * @brief read header
     * @details Opens file, writes the header, and closes file
     * @param filename name of file being written too
     * @param bHeader the header being written
     * @returns true or false depending on if the header was write was successfully or not
     */
    bool writeHeader(const std::string& filename, const BPlusTreeHeader& bHeader);
    
    /**
     * @brief has error
     * @details returns buffer errorState
     * @returns true or false depending on errorState
     */
    bool hasError() const;

    /**
     * @brief error message
     * @details returns buffer lastError
     * @returns returns the last error message
     */
    std::string getLastError() const;
    
private:
    bool errorState; // Represents if an error has occurred.
    std::string lastError; // Contains the last error message that occurred.
    
    /**
     * @brief set Error
     * @details sets error state true a
     */
    void setError(const std::string& message);
};

#endif