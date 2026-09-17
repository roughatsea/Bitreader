#pragma once // Include this header only once per translation unit, preventing repeated class or function declarations.

#include "FormatInfo.h" // Define the FormatInfo child stored by value inside FileHeader below.

#include <cstdint> // Provide fixed-width integer types used to store and assemble the binary fields. Angle syntax: The < and > delimit cstdint as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.

namespace bitreader { // Place these reader, model, or parser definitions in the shared bitreader namespace.

class FileHeader { // Define the header parent owning FormatInfo and the count used to parse records; its accessors expose the fields declared below.
public: // Expose the following constructors or accessors to callers such as the parser and printer.
    const FormatInfo& getFormat() const { return format_; } // Return a read-only reference to the FormatInfo child assembled by readFileHeader, avoiding a copy when the parser, printer, or tests traverse the hierarchy.
    void setFormat(const FormatInfo& value) { format_ = value; } // Store the supplied value in the FormatInfo child assembled by readFileHeader; the corresponding getter above exposes it to callers.

    std::uint16_t getRecordCount() const { return recordCount_; } // Return the stored value of the twelve-bit count used by readDocument to control its record loop, without changing the object, for the printer and tests.
    void setRecordCount(std::uint16_t value) { recordCount_ = value; } // Store the supplied value in the twelve-bit count used by readDocument to control its record loop; the corresponding getter above exposes it to callers.

private: // Restrict direct access to the state below; callers use the public methods above.
    FormatInfo format_; // Own storage for the FormatInfo child assembled by readFileHeader; its child objects start with their declared default values.
    std::uint16_t recordCount_ = 0; // Own storage for the twelve-bit count used by readDocument to control its record loop; initialize it to 0 before its setter receives parsed data.
}; // End the FileHeader class declaration; subsequent lines belong to its enclosing scope.

} // Close the shared namespace; callers qualify the definitions above with bitreader::.
