#pragma once // Include this header only once per translation unit, preventing repeated class or function declarations.

#include "FileHeader.h" // Define the FileHeader child stored by value inside BinaryDocument below.
#include "Record.h" // Define the Record element type stored in BinaryDocument's vector below.

#include <utility> // Provide std::move so a completed record vector can transfer its storage to the document. Angle syntax: The < and > delimit utility as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.
#include <vector> // Provide the dynamically sized collection that holds the document's decoded records. Angle syntax: The < and > delimit vector as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.

namespace bitreader { // Place these reader, model, or parser definitions in the shared bitreader namespace.

class BinaryDocument { // Define the root owning one FileHeader and a vector of Record objects; its accessors expose the fields declared below.
public: // Expose the following constructors or accessors to callers such as the parser and printer.
    const FileHeader& getHeader() const { return header_; } // Return a read-only reference to the header child assembled before records by readDocument, avoiding a copy when the parser, printer, or tests traverse the hierarchy.
    void setHeader(const FileHeader& value) { header_ = value; } // Store the supplied value in the header child assembled before records by readDocument; the corresponding getter above exposes it to callers.

    const std::vector<Record>& getRecords() const { return records_; } // Return a read-only reference to the collection of complete records assembled by readDocument, avoiding a copy when the parser, printer, or tests traverse the hierarchy. Angle syntax: The <Record> supplies vector's element type, so this collection or accessor holds complete Record parents with their nested children.
    void setRecords(std::vector<Record> value) { records_ = std::move(value); } // Move the supplied vector into the collection of complete records assembled by readDocument; the corresponding getter above exposes it to callers. Angle syntax: The <Record> supplies vector's element type, so this collection or accessor holds complete Record parents with their nested children.

private: // Restrict direct access to the state below; callers use the public methods above.
    FileHeader header_; // Own storage for the header child assembled before records by readDocument; its child objects start with their declared default values.
    std::vector<Record> records_; // Own storage for the collection of complete records assembled by readDocument; its vector starts empty until setRecords receives the parsed collection. Angle syntax: The <Record> supplies vector's element type, so this collection or accessor holds complete Record parents with their nested children.
}; // End the BinaryDocument class declaration; subsequent lines belong to its enclosing scope.

} // Close the shared namespace; callers qualify the definitions above with bitreader::.
