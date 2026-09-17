#pragma once // Include this header only once per translation unit, preventing repeated class or function declarations.

#include "Measurement.h" // Define the Measurement child owned by each Record below.
#include "RecordMetadata.h" // Define the RecordMetadata child owned by each Record below.

namespace bitreader { // Place these reader, model, or parser definitions in the shared bitreader namespace.

class Record { // Define the parent owning one RecordMetadata and one Measurement child; its accessors expose the fields declared below.
public: // Expose the following constructors or accessors to callers such as the parser and printer.
    const RecordMetadata& getMetadata() const { return metadata_; } // Return a read-only reference to the metadata child assembled first by readRecord, avoiding a copy when the parser, printer, or tests traverse the hierarchy.
    void setMetadata(const RecordMetadata& value) { metadata_ = value; } // Store the supplied value in the metadata child assembled first by readRecord; the corresponding getter above exposes it to callers.

    const Measurement& getMeasurement() const { return measurement_; } // Return a read-only reference to the measurement child assembled second by readRecord, avoiding a copy when the parser, printer, or tests traverse the hierarchy.
    void setMeasurement(const Measurement& value) { measurement_ = value; } // Store the supplied value in the measurement child assembled second by readRecord; the corresponding getter above exposes it to callers.

private: // Restrict direct access to the state below; callers use the public methods above.
    RecordMetadata metadata_; // Own storage for the metadata child assembled first by readRecord; its child objects start with their declared default values.
    Measurement measurement_; // Own storage for the measurement child assembled second by readRecord; its vector starts empty until setRecords receives the parsed collection.
}; // End the Record class declaration; subsequent lines belong to its enclosing scope.

} // Close the shared namespace; callers qualify the definitions above with bitreader::.
