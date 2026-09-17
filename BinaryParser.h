#pragma once // Include this header only once per translation unit, preventing repeated class or function declarations.

#include "io/BitReader.h" // Make the stream reader available to the parsing functions declared below.
#include "models/BinaryDocument.h" // Make the top-level document and its included child models available to parser declarations.

namespace bitreader { // Place these reader, model, or parser definitions in the shared bitreader namespace.

// Each function consumes exactly the fields belonging to its model.
FormatInfo readFormatInfo(BitReader& reader); // Declare the four-bit leaf parser whose result becomes FileHeader's format child.
FileHeader readFileHeader(BitReader& reader); // Declare the sixteen-bit header parser; its count controls subsequent record parsing.
RecordMetadata readRecordMetadata(BitReader& reader); // Declare the sixteen-bit metadata parser used to assemble a Record.
Measurement readMeasurement(BitReader& reader); // Declare the twenty-four-bit measurement parser used after record metadata.
Record readRecord(BitReader& reader); // Declare the forty-bit parser that combines metadata and measurement children.
BinaryDocument readDocument(BitReader& reader); // Declare the root parser that combines a header with a vector of records.

} // Close the shared namespace; callers qualify the definitions above with bitreader::.
