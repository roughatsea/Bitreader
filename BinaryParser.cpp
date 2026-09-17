#include "BinaryParser.h" // Bring the parser declarations, reader, and nested models into this implementation.

#include <limits> // Provide numeric_limits for checking field capacity or testing the largest integer value. Angle syntax: The < and > delimit limits as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.
#include <utility> // Provide std::move so a completed record vector can transfer its storage to the document. Angle syntax: The < and > delimit utility as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.
#include <vector> // Provide the dynamically sized collection that holds the document's decoded records. Angle syntax: The < and > delimit vector as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.

namespace bitreader { // Place these reader, model, or parser definitions in the shared bitreader namespace.
namespace { // Give the following helper functions or test data internal linkage within this source file.

// Connect a field's on-disk width to a model setter, deducing its value type.
template<unsigned Bits, typename Model, typename Value> // Parameterize the next helper by field width, destination class, and setter argument type. Angle syntax: The < and > enclose template parameter declarations; they let the following helper be instantiated for the particular bit width, model, and setter argument type used at each call.
void readField(BitReader& reader, Model& model, void (Model::*setter)(Value)) // Accept the shared reader, target object, and member-function pointer that receives the decoded value.
{ // Begin readField's body; the following statements implement the declaration immediately above.
    static_assert(std::numeric_limits<Value>::is_integer); // Require an integer or bool setter argument at compile time, before attempting a numeric bit conversion. Angle syntax: The <Value> specializes numeric_limits for Value, so this line uses the setter argument's actual integer properties or capacity.
    static_assert(!std::numeric_limits<Value>::is_signed); // Reject signed setter arguments because this helper does not implement sign extension. Angle syntax: The <Value> specializes numeric_limits for Value, so this line uses the setter argument's actual integer properties or capacity.
    static_assert(Bits > 0 && Bits <= 64); // Match BitReader's supported widths so invalid field mappings fail during compilation. Angle syntax: The > comparison requires a strictly positive field width, ruling out zero-bit mappings at compile time. The <= comparison allows widths up to and including sixty-four, matching readBits and its uint64_t result.
    static_assert(Bits <= std::numeric_limits<Value>::digits, // Check that all field bits fit the setter's value type; the diagnostic continues on the next line. Angle syntax: The <Value> specializes numeric_limits for Value, so this line uses the setter argument's actual integer properties or capacity. The <= comparison permits a field only when its width is at most the setter argument's usable bit capacity, preventing narrowing from discarding field bits.
                  "Field does not fit the setter's parameter type"); // Supply the compile-time error message for the capacity check started above.

    (model.*setter)(static_cast<Value>(reader.readBits(Bits))); // Consume this field, convert to the inferred argument type, and invoke the setter on this model. Angle syntax: The <Value> names the explicit conversion's target type; this converts the uint64_t reader result to the setter's inferred argument type.
} // End the readField function body; subsequent lines belong to its enclosing scope.

} // Close the file-local helper namespace; the helpers above remain private to this source file.

FormatInfo readFormatInfo(BitReader& reader) // Define the leaf parser called by readFileHeader to construct its FormatInfo child.
{ // Begin readFormatInfo's body; the following statements implement the declaration immediately above.
    FormatInfo format; // Create an initialized leaf object that the next two field mappings will populate.
    readField<3>(reader, format, &FormatInfo::setVersion); // Consume the next three bits as version and store them through FormatInfo's setter. Angle syntax: The <3> is a compile-time template argument selecting a 3-bit field; Model and Value are inferred from the object and setter, and static_assert checks the width.
    readField<1>(reader, format, &FormatInfo::setTagged); // Consume the bit following version as the tagged flag, completing this four-bit child. Angle syntax: The <1> is a compile-time template argument selecting a 1-bit field; Model and Value are inferred from the object and setter, and static_assert checks the width.
    return format; // Return the completed FormatInfo so readFileHeader can attach it to its parent.
} // End the readFormatInfo function body; subsequent lines belong to its enclosing scope.

FileHeader readFileHeader(BitReader& reader) // Define the header parser used by readDocument or by callers processing records individually.
{ // Begin readFileHeader's body; the following statements implement the declaration immediately above.
    FileHeader header; // Create a parent header ready to receive its FormatInfo child and record count.
    header.setFormat(readFormatInfo(reader)); // Parse the four-bit child first and copy the resulting FormatInfo into this header.
    readField<12>(reader, header, &FileHeader::setRecordCount); // Consume twelve count bits after FormatInfo, completing the sixteen-bit file header. Angle syntax: The <12> is a compile-time template argument selecting a 12-bit field; Model and Value are inferred from the object and setter, and static_assert checks the width.
    return header; // Return the header whose count tells its caller how many records follow.
} // End the readFileHeader function body; subsequent lines belong to its enclosing scope.

RecordMetadata readRecordMetadata(BitReader& reader) // Define the leaf parser that readRecord calls for its metadata child.
{ // Begin readRecordMetadata's body; the following statements implement the declaration immediately above.
    RecordMetadata metadata; // Create a metadata object for the ID, category, and active fields mapped below.
    readField<12>(reader, metadata, &RecordMetadata::setId); // Consume twelve ID bits at the beginning of this record's metadata. Angle syntax: The <12> is a compile-time template argument selecting a 12-bit field; Model and Value are inferred from the object and setter, and static_assert checks the width.
    readField<3>(reader, metadata, &RecordMetadata::setCategory); // Consume the three category bits immediately following the ID. Angle syntax: The <3> is a compile-time template argument selecting a 3-bit field; Model and Value are inferred from the object and setter, and static_assert checks the width.
    readField<1>(reader, metadata, &RecordMetadata::setActive); // Consume the active flag after category, completing sixteen metadata bits. Angle syntax: The <1> is a compile-time template argument selecting a 1-bit field; Model and Value are inferred from the object and setter, and static_assert checks the width.
    return metadata; // Return the completed metadata so readRecord can store it as a child.
} // End the readRecordMetadata function body; subsequent lines belong to its enclosing scope.

Measurement readMeasurement(BitReader& reader) // Define the leaf parser that readRecord calls after reading metadata.
{ // Begin readMeasurement's body; the following statements implement the declaration immediately above.
    Measurement measurement; // Create a measurement object for the magnitude and exponent mappings below.
    readField<20>(reader, measurement, &Measurement::setMagnitude); // Read twenty magnitude bits, continuing across byte boundaries through BitReader. Angle syntax: The <20> is a compile-time template argument selecting a 20-bit field; Model and Value are inferred from the object and setter, and static_assert checks the width.
    readField<4>(reader, measurement, &Measurement::setExponent); // Read the four exponent bits following magnitude, completing twenty-four measurement bits. Angle syntax: The <4> is a compile-time template argument selecting a 4-bit field; Model and Value are inferred from the object and setter, and static_assert checks the width.
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

    std::vector<Record> records; // Stage decoded records locally so the document receives a complete collection. Angle syntax: The <Record> supplies vector's element type, so this collection or accessor holds complete Record parents with their nested children.
    records.reserve(document.getHeader().getRecordCount()); // Allocate capacity from the parsed count to avoid reallocating during the following loop.
    for (std::uint16_t i = 0; i < document.getHeader().getRecordCount(); ++i) { // Repeat the record parser exactly as many times as the header declares. Angle syntax: The < comparison keeps the zero-based record index below the header's count, processing exactly that many records without attempting an extra one.
        records.push_back(readRecord(reader)); // Parse and append the next nested record, advancing the same reader by forty bits.
    } // End the declared-record loop; the completed vector can now be attached to the root.
    document.setRecords(std::move(records)); // Transfer the completed vector into the root without copying its elements.
    return document; // Return the assembled hierarchy; a child parse exception bypasses this return.
} // End the readDocument function body; subsequent lines belong to its enclosing scope.

} // Close the shared namespace; callers qualify the definitions above with bitreader::.
