#include "BitReader.h" // Declare BitReader and its state before defining readBits in this source file.

#include <stdexcept> // Provide exception types used to report invalid inputs and failures to the caller.
#include <string> // Provide strings for sample bytes, file paths, and diagnostic messages.

namespace bitreader { // Place these reader, model, or parser definitions in the shared bitreader namespace.

std::uint64_t BitReader::readBits(unsigned count) // Implement the declared reader operation, returning the next field as an unsigned integer.
{ // Begin readBits's body; the following statements implement the declaration immediately above.
    if (count == 0 || count > 64) { // Reject widths outside the result type's capacity before consuming any input.
        throw std::invalid_argument("Bit width must be between 1 and 64"); // Report an invalid API argument to the caller instead of entering the extraction loop.
    } // End the branch guarded by count == 0 || count > 64; subsequent lines belong to its enclosing scope.

    std::uint64_t value = 0; // Initialize the field accumulator that receives each extracted bit below.
    for (unsigned i = 0; i < count; ++i) { // Extract exactly count bits, fetching additional bytes only when needed.
        if (remainingBits_ == 0) { // Refill the byte cache only when all bits from its previous byte have been consumed.
            char byte; // Provide temporary storage for the next byte read through the stream's char interface.
            if (!input_.get(byte)) { // Attempt to fetch a byte and branch to diagnostics if the stream cannot supply one.
                const auto position = std::to_string(bitPosition_); // Convert the number of already consumed bits to text for both failure messages below.
                if (input_.bad() || !input_.eof()) { // Distinguish an input failure from normal end-of-file before choosing the diagnostic.
                    throw std::runtime_error("Input stream error at bit " + position); // Report a stream failure and its position; this aborts the calling child and parent parsers.
                } // End the branch guarded by input_.bad() || !input_.eof(); subsequent lines belong to its enclosing scope.
                throw std::runtime_error("Unexpected end of input at bit " + position); // Report truncated input rather than fabricate bits or return a partially parsed hierarchy.
            } // End the branch guarded by !input_.get(byte); subsequent lines belong to its enclosing scope.
            currentByte_ = static_cast<std::uint8_t>(static_cast<unsigned char>(byte)); // Preserve the raw eight-bit pattern even when char is signed, then store it in the cache.
            remainingBits_ = 8; // Mark all eight bits of the newly fetched byte as available for extraction.
        } // End the branch guarded by remainingBits_ == 0; subsequent lines belong to its enclosing scope.

        --remainingBits_; // Select the next bit index, descending from seven to zero for MSB-first reading.
        value = (value << 1) | ((currentByte_ >> remainingBits_) & 1u); // Shift earlier field bits left and append the selected cached bit to the least significant position.
        ++bitPosition_; // Record consumption of this bit so later reads and failure diagnostics share the correct position.
    } // End the count-bit extraction loop; return the accumulated field on the following line.
    return value; // Return the assembled unsigned field after the loop has consumed all requested bits.
} // End the readBits function body; subsequent lines belong to its enclosing scope.

} // Close the shared namespace; callers qualify the definitions above with bitreader::.
