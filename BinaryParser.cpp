#include "BinaryParser.h" // Bring the parser declarations, reader, and nested models into this implementation.

#include <limits> // Provide numeric_limits for checking field capacity or testing the largest integer value.
#include <utility> // Provide std::move so a completed record vector can transfer its storage to the document.
#include <vector> // Provide the dynamically sized collection that holds the document's decoded records.

namespace bitreader { // Place these reader, model, or parser definitions in the shared bitreader namespace.
namespace { // Give the following helper functions or test data internal linkage within this source file.

// Connect a field's on-disk width to a model setter, deducing its value type.
template<unsigned Bits, typename Model, typename Value> // Parameterize the next helper by field width, destination class, and setter argument type.
void readField(BitReader& reader, Model& model, void (Model::*setter)(Value)) // Accept the shared reader, target object, and member-function pointer that receives the decoded value.
{ // Begin readField's body; the following statements implement the declaration immediately above.
    static_assert(std::numeric_limits<Value>::is_integer); // Require an integer or bool setter argument at compile time, before attempting a numeric bit conversion.
    static_assert(!std::numeric_limits<Value>::is_signed); // Reject signed setter arguments because this helper does not implement sign extension.
    static_assert(Bits > 0 && Bits <= 64); // Match BitReader's supported widths so invalid field mappings fail during compilation.
    static_assert(Bits <= std::numeric_limits<Value>::digits, // Check that all field bits fit the setter's value type; the diagnostic continues on the next line.
                  "Field does not fit the setter's parameter type"); // Supply the compile-time error message for the capacity check started above.

    (model.*setter)(static_cast<Value>(reader.readBits(Bits))); // Consume this field, convert to the inferred argument type, and invoke the setter on this model.
} // End the readField function body; subsequent lines belong to its enclosing scope.

} // Close the file-local helper namespace; the helpers above remain private to this source file.

FormatInfo readFormatInfo(BitReader& reader) // Define the leaf parser called by readFileHeader to construct its FormatInfo child.
{ // Begin readFormatInfo's body; the following statements implement the declaration immediately above.
    FormatInfo format; // Create an initialized leaf object that the next two field mappings will populate.
    readField<3>(reader, format, &FormatInfo::setVersion); // Consume the next three bits as version and store them through FormatInfo's setter.
    readField<1>(reader, format, &FormatInfo::setTagged); // Consume the bit following version as the tagged flag, completing this four-bit child.
    return format; // Return the completed FormatInfo so readFileHeader can attach it to its parent.
} // End the readFormatInfo function body; subsequent lines belong to its enclosing scope.

FileHeader readFileHeader(BitReader& reader) // Define the header parser used by readDocument or by callers processing records individually.
{ // Begin readFileHeader's body; the following statements implement the declaration immediately above.
    FileHeader header; // Create a parent header ready to receive its FormatInfo child and record count.
    header.setFormat(readFormatInfo(reader)); // Parse the four-bit child first and copy the resulting FormatInfo into this header.
    readField<12>(reader, header, &FileHeader::setRecordCount); // Consume twelve count bits after FormatInfo, completing the sixteen-bit file header.
    return header; // Return the header whose count tells its caller how many records follow.
} // End the readFileHeader function body; subsequent lines belong to its enclosing scope.

RecordMetadata readRecordMetadata(BitReader& reader) // Define the leaf parser that readRecord calls for its metadata child.
{ // Begin readRecordMetadata's body; the following statements implement the declaration immediately above.
    RecordMetadata metadata; // Create a metadata object for the ID, category, and active fields mapped below.
    readField<12>(reader, metadata, &RecordMetadata::setId); // Consume twelve ID bits at the beginning of this record's metadata.
    readField<3>(reader, metadata, &RecordMetadata::setCategory); // Consume the three category bits immediately following the ID.
    readField<1>(reader, metadata, &RecordMetadata::setActive); // Consume the active flag after category, completing sixteen metadata bits.
    return metadata; // Return the completed metadata so readRecord can store it as a child.
} // End the readRecordMetadata function body; subsequent lines belong to its enclosing scope.

Measurement readMeasurement(BitReader& reader) // Define the leaf parser that readRecord calls after reading metadata.
{ // Begin readMeasurement's body; the following statements implement the declaration immediately above.
    Measurement measurement; // Create a measurement object for the magnitude and exponent mappings below.
    readField<20>(reader, measurement, &Measurement::setMagnitude); // Read twenty magnitude bits, continuing across byte boundaries through BitReader.
    readField<4>(reader, measurement, &Measurement::setExponent); // Read the four exponent bits following magnitude, completing twenty-four measurement bits.
    return measurement; // Return the completed measurement for attachment to its Record parent.
} // End the readMeasurement function body; subsequent lines belong to its enclosing scope.

Record readRecord(BitReader& reader) // Define a forty-bit record parser by composing the two child parsers below.
{ // Begin readRecord's body; the following statements implement the declaration immediately above.
    Record record; // Create the parent object that owns its metadata and measurement by value.
    record.setMetadata(readRecordMetadata(reader)); // Parse the next sixteen bits into metadata and attach that child through the parent setter.
    record.setMeasurement(readMeasurement(reader)); // Parse the next twenty-four bits into a measurement and attach it after metadata.
    return record; // Return the assembled record to the document parser or a streaming caller.
} // End the readRecord function body; subsequent lines belong to its enclosing scope.

BinaryDocument readDocument(BitReader& reader) // Define the top-level parser that retains the header and all declared records.
{ // Begin readDocument's body; the following statements implement the declaration immediately above.
    BinaryDocument document; // Create the root object that will own the nested header and record collection.
    document.setHeader(readFileHeader(reader)); // Parse the first sixteen bits into a header and attach it before examining its count.

    std::vector<Record> records; // Stage decoded records locally so the document receives a complete collection.
    records.reserve(document.getHeader().getRecordCount()); // Allocate capacity from the parsed count to avoid reallocating during the following loop.
    for (std::uint16_t i = 0; i < document.getHeader().getRecordCount(); ++i) { // Repeat the record parser exactly as many times as the header declares.
        records.push_back(readRecord(reader)); // Parse and append the next nested record, advancing the same reader by forty bits.
    } // End the declared-record loop; the completed vector can now be attached to the root.
    document.setRecords(std::move(records)); // Transfer the completed vector into the root without copying its elements.
    return document; // Return the assembled hierarchy; a child parse exception bypasses this return.
} // End the readDocument function body; subsequent lines belong to its enclosing scope.

} // Close the shared namespace; callers qualify the definitions above with bitreader::.
