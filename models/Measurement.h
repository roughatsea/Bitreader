#pragma once // Include this header only once per translation unit, preventing repeated class or function declarations.

#include <cstdint> // Provide fixed-width integer types used to store and assemble the binary fields. Angle syntax: The < and > delimit cstdint as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.

namespace bitreader { // Place these reader, model, or parser definitions in the shared bitreader namespace.

class Measurement { // Define the magnitude and exponent leaf owned by Record; its accessors expose the fields declared below.
public: // Expose the following constructors or accessors to callers such as the parser and printer.
    std::uint32_t getMagnitude() const { return magnitude_; } // Return the stored value of the twenty-bit magnitude populated by readMeasurement, without changing the object, for the printer and tests.
    void setMagnitude(std::uint32_t value) { magnitude_ = value; } // Store the supplied value in the twenty-bit magnitude populated by readMeasurement; the corresponding getter above exposes it to callers.

    std::uint8_t getExponent() const { return exponent_; } // Return the stored value of the four-bit exponent populated by readMeasurement, without changing the object, for the printer and tests.
    void setExponent(std::uint8_t value) { exponent_ = value; } // Store the supplied value in the four-bit exponent populated by readMeasurement; the corresponding getter above exposes it to callers.

private: // Restrict direct access to the state below; callers use the public methods above.
    std::uint32_t magnitude_ = 0; // Own storage for the twenty-bit magnitude populated by readMeasurement; initialize it to 0 before its setter receives parsed data.
    std::uint8_t exponent_ = 0; // Own storage for the four-bit exponent populated by readMeasurement; initialize it to 0 before its setter receives parsed data.
}; // End the Measurement class declaration; subsequent lines belong to its enclosing scope.

} // Close the shared namespace; callers qualify the definitions above with bitreader::.
