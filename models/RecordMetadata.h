#pragma once // Include this header only once per translation unit, preventing repeated class or function declarations.

#include <cstdint> // Provide fixed-width integer types used to store and assemble the binary fields.

namespace bitreader { // Place these reader, model, or parser definitions in the shared bitreader namespace.

class RecordMetadata { // Define the ID, category, and active-state leaf owned by Record; its accessors expose the fields declared below.
public: // Expose the following constructors or accessors to callers such as the parser and printer.
    std::uint16_t getId() const { return id_; } // Return the stored value of the twelve-bit identifier populated by readRecordMetadata, without changing the object, for the printer and tests.
    void setId(std::uint16_t value) { id_ = value; } // Store the supplied value in the twelve-bit identifier populated by readRecordMetadata; the corresponding getter above exposes it to callers.

    std::uint8_t getCategory() const { return category_; } // Return the stored value of the three-bit category populated by readRecordMetadata, without changing the object, for the printer and tests.
    void setCategory(std::uint8_t value) { category_ = value; } // Store the supplied value in the three-bit category populated by readRecordMetadata; the corresponding getter above exposes it to callers.

    bool getActive() const { return active_; } // Return the stored value of the one-bit active flag populated by readRecordMetadata, without changing the object, for the printer and tests.
    void setActive(bool value) { active_ = value; } // Store the supplied value in the one-bit active flag populated by readRecordMetadata; the corresponding getter above exposes it to callers.

private: // Restrict direct access to the state below; callers use the public methods above.
    std::uint16_t id_ = 0; // Own storage for the twelve-bit identifier populated by readRecordMetadata; initialize it to 0 before its setter receives parsed data.
    std::uint8_t category_ = 0; // Own storage for the three-bit category populated by readRecordMetadata; initialize it to 0 before its setter receives parsed data.
    bool active_ = false; // Own storage for the one-bit active flag populated by readRecordMetadata; initialize it to false before its setter receives parsed data.
}; // End the RecordMetadata class declaration; subsequent lines belong to its enclosing scope.

} // Close the shared namespace; callers qualify the definitions above with bitreader::.
