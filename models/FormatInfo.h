#pragma once // Include this header only once per translation unit, preventing repeated class or function declarations.

#include <cstdint> // Provide fixed-width integer types used to store and assemble the binary fields.

namespace bitreader { // Place these reader, model, or parser definitions in the shared bitreader namespace.

class FormatInfo { // Define the leaf format description stored inside FileHeader; its accessors expose the fields declared below.
public: // Expose the following constructors or accessors to callers such as the parser and printer.
    std::uint8_t getVersion() const { return version_; } // Return the stored value of the three-bit version populated by readFormatInfo, without changing the object, for the printer and tests.
    void setVersion(std::uint8_t value) { version_ = value; } // Store the supplied value in the three-bit version populated by readFormatInfo; the corresponding getter above exposes it to callers.

    bool getTagged() const { return tagged_; } // Return the stored value of the one-bit tagged flag populated by readFormatInfo, without changing the object, for the printer and tests.
    void setTagged(bool value) { tagged_ = value; } // Store the supplied value in the one-bit tagged flag populated by readFormatInfo; the corresponding getter above exposes it to callers.

private: // Restrict direct access to the state below; callers use the public methods above.
    std::uint8_t version_ = 0; // Own storage for the three-bit version populated by readFormatInfo; initialize it to 0 before its setter receives parsed data.
    bool tagged_ = false; // Own storage for the one-bit tagged flag populated by readFormatInfo; initialize it to false before its setter receives parsed data.
}; // End the FormatInfo class declaration; subsequent lines belong to its enclosing scope.

} // Close the shared namespace; callers qualify the definitions above with bitreader::.
