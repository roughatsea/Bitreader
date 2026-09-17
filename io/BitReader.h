#pragma once // Include this header only once per translation unit, preventing repeated class or function declarations.

#include <cstdint> // Provide fixed-width integer types used to store and assemble the binary fields. Angle syntax: The < and > delimit cstdint as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.
#include <istream> // Provide std::istream, the common interface borrowed by BitReader for file or memory input. Angle syntax: The < and > delimit istream as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.

namespace bitreader { // Place these reader, model, or parser definitions in the shared bitreader namespace.

// Borrows the stream. Reads bit 7 first in each byte, then bit 6, and so on.
// The first bit read becomes the most significant bit of the returned value.
class BitReader { // Define the stateful bit reader used by every leaf parser, sharing one stream position.
public: // Expose the following constructors or accessors to callers such as the parser and printer.
    explicit BitReader(std::istream& input) : input_(input) {} // Borrow the supplied stream without copying or owning it; it must outlive this reader.

    // Reads 1..64 bits, continuing across byte boundaries as necessary.
    // Throws on invalid widths, truncated input, or stream errors.
    std::uint64_t readBits(unsigned count); // Declare the consuming operation implemented in BitReader.cpp and called by readField.
    std::uint64_t getBitPosition() const { return bitPosition_; } // Report total consumed bits without changing state, for diagnostics and test expectations.

private: // Restrict direct access to the state below; callers use the public methods above.
    std::istream& input_; // Keep a reference to the caller's stream, which supplies bytes whenever the cached byte runs out.
    std::uint8_t currentByte_ = 0; // Initialize the cached byte; readBits replaces it before consuming the first bit.
    unsigned remainingBits_ = 0; // Start with no cached bits so the first readBits call fetches a byte.
    std::uint64_t bitPosition_ = 0; // Start the total consumed-bit counter at zero and increment it once per extracted bit.
}; // End the BitReader class declaration; subsequent lines belong to its enclosing scope.

} // Close the shared namespace; callers qualify the definitions above with bitreader::.
