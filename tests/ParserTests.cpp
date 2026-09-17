#include "../BinaryParser.h" // Expose the production parser and its models to the tests from the parent directory.

#include <cstdint> // Provide fixed-width integer types used to store and assemble the binary fields.
#include <iostream> // Provide std::cout and std::cerr for normal output and error reporting below.
#include <limits> // Provide numeric_limits for checking field capacity or testing the largest integer value.
#include <sstream> // Provide in-memory input streams so sample bytes can use the same parser as files.
#include <stdexcept> // Provide exception types used to report invalid inputs and failures to the caller.
#include <string> // Provide strings for sample bytes, file paths, and diagnostic messages.

namespace { // Give the following helper functions or test data internal linkage within this source file.

const std::string sample("\xB0\x02\x12\x3B\xAB\xCD\xE2\x45\x64\x12\x34\x57", 12); // Share the twelve-byte fixture whose nested values are checked by the document and truncation tests.

void require(bool condition, const char* message) // Define an assertion helper that remains active in every build configuration.
{ // Begin require's body; the following statements implement the declaration immediately above.
    if (!condition) { // Enter the failure branch when the test condition supplied above evaluates to false.
        throw std::runtime_error(message); // Propagate the supplied failure explanation to the test program's catch block.
    } // End the branch guarded by !condition; subsequent lines belong to its enclosing scope.
} // End the require function body; subsequent lines belong to its enclosing scope.

template<typename Exception, typename Function> // Parameterize the following assertion by expected exception type and callable action.
void requireThrows(Function action, const char* message) // Define a helper that runs an action and requires it to throw the specified exception type.
{ // Begin requireThrows's body; the following statements implement the declaration immediately above.
    try { // Run the following test operation under the exception handler immediately below.
        action(); // Invoke the supplied action; if it returns normally, the missing-exception failure below runs.
    } catch (const Exception&) { // Catch only the expected exception type; unexpected types continue to the outer test runner.
        return; // Mark the expected exception as successfully observed by returning from requireThrows.
    } // End the exception handler; subsequent lines belong to its enclosing scope.
    throw std::runtime_error(message); // Propagate the supplied failure explanation to the test program's catch block.
} // End the requireThrows function body; subsequent lines belong to its enclosing scope.

void testBitReader() // Define tests of ordering, field widths, byte boundaries, and reader errors independently of models.
{ // Begin testBitReader's body; the following statements implement the declaration immediately above.
    std::istringstream input(std::string("\xB3\x6D", 2)); // Provide two known bytes so successive reads can verify ordering and crossing a byte boundary.
    bitreader::BitReader reader(input); // Create a reader over this test's stream; the following assertions observe its consuming operations.
    require(reader.readBits(3) == 5, "MSB-first ordering is incorrect"); // Check that the first three bits of B3 are 101, establishing MSB-first order.
    require(reader.readBits(9) == 0x136, "Cross-byte field is incorrect"); // Check the next nine bits, 100110110, spanning the remainder of B3 and the start of 6D.
    require(reader.readBits(4) == 13, "Remaining bits are incorrect"); // Check that the final nibble of 6D is still available after the cross-byte read.
    require(reader.getBitPosition() == 16, "Bit position is incorrect"); // Verify that the three successful reads consumed exactly both input bytes.
    requireThrows<std::runtime_error>([&] { reader.readBits(1); }, "EOF was not rejected"); // Require another read past those two bytes to throw rather than supply an invented bit.

    std::istringstream wideInput(std::string("\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8)); // Provide eight distinct bytes to test a full-width unsigned read and field significance.
    bitreader::BitReader wideReader(wideInput); // Use a separate reader so invalid-width checks cannot interfere with the earlier ordering test.
    requireThrows<std::invalid_argument>([&] { wideReader.readBits(0); }, "Zero width was accepted"); // Verify that a zero-bit request fails validation before reading the fixture.
    requireThrows<std::invalid_argument>([&] { wideReader.readBits(65); }, "Excessive width was accepted"); // Verify that a request larger than the 64-bit result fails validation.
    require(wideReader.getBitPosition() == 0, "Invalid width consumed input"); // Check that both rejected width requests left the stream position unchanged.
    require(wideReader.readBits(64) == UINT64_C(0x0123456789ABCDEF), "64-bit value is incorrect"); // Read all eight bytes and check that their ordered bits form the expected 64-bit value.

    std::istringstream maximumInput(std::string(8, static_cast<char>(0xFF))); // Provide eight all-one bytes to exercise the largest representable reader result.
    bitreader::BitReader maximumReader(maximumInput); // Create a reader for this maximum-value fixture, separate from previous reader state.
    require(maximumReader.readBits(64) == std::numeric_limits<std::uint64_t>::max(), // Check a full-width read against uint64_t's maximum; its failure message follows.
            "Maximum 64-bit value is incorrect"); // Complete the preceding assertion with a diagnostic for a lost or incorrectly shifted high bit.

    std::istringstream unalignedInput(std::string("\x80", 1) + std::string(8, static_cast<char>(0xFF))); // Provide one leading set bit, seven zero bits, then eight all-one bytes for an unaligned full-width read.
    bitreader::BitReader unalignedReader(unalignedInput); // Start a new reader whose byte cache will become partially consumed by the next assertion.
    require(unalignedReader.readBits(1) == 1, "Leading bit is incorrect"); // Consume the first set bit so the following 64-bit read starts inside a byte.
    require(unalignedReader.readBits(64) == (UINT64_C(1) << 57) - 1, // Check that seven zeros followed by fifty-seven ones form the expected unaligned result.
            "Unaligned 64-bit read is incorrect"); // Complete that assertion with a diagnostic for boundary or shift errors.
    require(unalignedReader.readBits(7) == 127, "Unaligned read consumed extra bits"); // Verify the remaining seven ones were not consumed by the preceding 64-bit read.

    std::istringstream failedInput(sample); // Provide otherwise valid bytes for a test that explicitly simulates stream failure.
    failedInput.setstate(std::ios::badbit); // Mark that stream as broken so the next byte fetch takes the reader's input-error path.
    bitreader::BitReader failedReader(failedInput); // Wrap the broken stream without reading it yet, allowing the assertion below to observe failure.
    requireThrows<std::runtime_error>([&] { failedReader.readBits(1); }, "Stream error was not rejected"); // Require the simulated input error to propagate as an exception from the reader.
} // End the testBitReader function body; subsequent lines belong to its enclosing scope.

void testDocument() // Define checks that the shared fixture is decoded into the correct nested parent and child objects.
{ // Begin testDocument's body; the following statements implement the declaration immediately above.
    std::istringstream input(sample); // Create a fresh stream containing the shared sample so this test starts at its first bit.
    bitreader::BitReader reader(input); // Create a reader over this test's stream; the following assertions observe its consuming operations.
    const auto document = bitreader::readDocument(reader); // Parse the fixture into the root whose header and record children are examined below.
    const auto& header = document.getHeader(); // Borrow the root's header to check both its FormatInfo child and declared record count.
    require(header.getFormat().getVersion() == 5, "Nested format version is incorrect"); // Follow two parent-child getters to verify the fixture's three-bit version.
    require(header.getFormat().getTagged(), "Nested format flag is incorrect"); // Verify the tagged flag was read immediately after version and attached to the header.
    require(header.getRecordCount() == 2 && document.getRecords().size() == 2, // Compare the parsed count and actual owned vector length; both must equal the fixture's two records.
            "Document record count is incorrect"); // Complete the count assertion with its failure explanation.

    const auto& first = document.getRecords()[0]; // Borrow the first parsed record to test its metadata and measurement children.
    require(first.getMetadata().getId() == 0x123, "First record ID is incorrect"); // Check the first record's twelve-bit identifier through its metadata getter.
    require(first.getMetadata().getCategory() == 5 && first.getMetadata().getActive(), // Check the category and active fields that follow the first record's identifier.
            "First record flags are incorrect"); // Complete the preceding first-record metadata assertion with its diagnostic.
    require(first.getMeasurement().getMagnitude() == 0xABCDE, // Check the first record's twenty-bit magnitude through its measurement child.
            "First record magnitude is incorrect"); // Complete the first-record magnitude assertion with its diagnostic.
    require(first.getMeasurement().getExponent() == 2, "First record exponent is incorrect"); // Verify the four exponent bits immediately after the first record's magnitude.

    const auto& second = document.getRecords()[1]; // Borrow the second record to ensure parser state advanced correctly after the first.
    require(second.getMetadata().getId() == 0x456, "Second record ID is incorrect"); // Verify that the second record has its own expected identifier rather than the first record's data.
    require(second.getMetadata().getCategory() == 2 && !second.getMetadata().getActive(), // Check the second record's category and unset active flag, exercising a false Boolean value.
            "Second record flags are incorrect"); // Complete the second-record metadata assertion with its diagnostic.
    require(second.getMeasurement().getMagnitude() == 0x12345, // Verify the second record's distinct magnitude after advancing through its metadata.
            "Second record magnitude is incorrect"); // Complete the second-record magnitude assertion with its diagnostic.
    require(second.getMeasurement().getExponent() == 7, "Second record exponent is incorrect"); // Verify the final exponent field of the sample document.
    require(reader.getBitPosition() == 96, "Document consumed the wrong number of bits"); // Check that one sixteen-bit header and two forty-bit records consumed exactly ninety-six bits.

    // The lower-level entry points also support processing records one at a time.
    std::istringstream streamingInput(sample); // Restart the fixture for the independent record-by-record parsing path below.
    bitreader::BitReader streamingReader(streamingInput); // Share this reader across the header and individual record calls without creating a root document.
    const auto streamingHeader = bitreader::readFileHeader(streamingReader); // Read just the header, leaving the reader positioned at the first record.
    require(streamingHeader.getRecordCount() == 2, "Streaming header is incorrect"); // Verify the header count available to callers that process records without retaining the full document.
    require(bitreader::readRecord(streamingReader).getMetadata().getId() == 0x123, // Parse the first record individually and check its metadata identifier.
            "First streamed record is incorrect"); // Complete the assertion for the first individually parsed record.
    require(bitreader::readRecord(streamingReader).getMetadata().getId() == 0x456, // Parse the next record with the same reader and verify correct advancement.
            "Second streamed record is incorrect"); // Complete the assertion for the second individually parsed record.
} // End the testDocument function body; subsequent lines belong to its enclosing scope.

void testEmptyAndMaximumFields() // Define checks for zero-record documents and the largest values allowed by each mapped field.
{ // Begin testEmptyAndMaximumFields's body; the following statements implement the declaration immediately above.
    std::istringstream emptyInput(std::string(2, '\0')); // Provide a sixteen-bit all-zero header that declares no following records.
    bitreader::BitReader emptyReader(emptyInput); // Create a reader for the zero-field fixture examined by the next assertions.
    const auto emptyDocument = bitreader::readDocument(emptyReader); // Parse only the zero-count header; the document parser's record loop must execute zero times.
    require(emptyDocument.getRecords().empty(), "Zero-record document is incorrect"); // Confirm that the root owns an empty vector when the header declares zero records.
    require(emptyDocument.getHeader().getFormat().getVersion() == 0 // Start checking the zero version in the nested format; the next line also checks its flag.
            && !emptyDocument.getHeader().getFormat().getTagged(), "Zero fields are incorrect"); // Require the nested flag to be false, completing the all-zero format assertion.
    require(emptyReader.getBitPosition() == 16, "Zero-record document consumed extra input"); // Confirm that the zero-count parser stopped immediately after its sixteen-bit header.

    std::istringstream maximumInput(std::string("\xF0\x01\xFF\xFF\xFF\xFF\xFF", 7)); // Provide version seven, tagged true, count one, and one record with all its field bits set.
    bitreader::BitReader maximumReader(maximumInput); // Create a reader for this maximum-value fixture, separate from previous reader state.
    const auto document = bitreader::readDocument(maximumReader); // Parse the maximum-field fixture for comparison with each field's width-derived limit.
    require(document.getHeader().getFormat().getVersion() == 7, "Maximum version is incorrect"); // Verify the largest three-bit version survived conversion to its model storage type.
    const auto& record = document.getRecords().at(0); // Borrow the single record with bounds checking before examining its maximum-valued children.
    require(record.getMetadata().getId() == 4095 && record.getMetadata().getCategory() == 7 // Check the twelve-bit ID and three-bit category maxima; the active-flag check follows.
            && record.getMetadata().getActive(), "Maximum metadata fields are incorrect"); // Require the one-bit active field to be true, completing the maximum metadata assertion.
    require(record.getMeasurement().getMagnitude() == 1048575 // Start checking the largest twenty-bit magnitude; the exponent check continues below.
            && record.getMeasurement().getExponent() == 15, "Maximum measurement fields are incorrect"); // Check the largest four-bit exponent and complete the maximum measurement assertion.
} // End the testEmptyAndMaximumFields function body; subsequent lines belong to its enclosing scope.

void testTruncatedDocuments() // Define checks that every incomplete sample prefix fails instead of returning partial nested data.
{ // Begin testTruncatedDocuments's body; the following statements implement the declaration immediately above.
    // Every incomplete prefix must fail rather than return a partially filled hierarchy.
    for (std::size_t length = 0; length < sample.size(); ++length) { // Try every byte length shorter than the complete fixture, including an entirely empty stream.
        std::istringstream input(sample.substr(0, length)); // Create the current truncated prefix as a fresh stream for this loop iteration.
        bitreader::BitReader reader(input); // Create a reader over this test's stream; the following assertions observe its consuming operations.
        requireThrows<std::runtime_error>([&] { bitreader::readDocument(reader); }, // Require the root parser or one of its child parsers to reject this incomplete input.
                                         "Truncated document was accepted"); // Complete the missing-exception assertion with a diagnostic for incorrectly accepted partial data.
        require(reader.getBitPosition() == length * 8, "Truncation position is incorrect"); // Verify the reader consumed all available prefix bits and no fabricated bits after end-of-file.
    } // End the incomplete-prefix loop after checking every shorter fixture length.
} // End the testTruncatedDocuments function body; subsequent lines belong to its enclosing scope.

} // Close the file-local helper namespace; the helpers above remain private to this source file.

int main() // Define the test executable's entry point that runs each group and reports the overall result.
{ // Begin main's body; the following statements implement the declaration immediately above.
    try { // Run the following test operation under the exception handler immediately below.
        testBitReader(); // Run primitive reader tests before relying on that reader to test higher-level parsing.
        testDocument(); // Run nested-object and record-by-record checks using the known two-record fixture.
        testEmptyAndMaximumFields(); // Run zero-count and maximum-field checks after ordinary document parsing.
        testTruncatedDocuments(); // Run every incomplete-prefix check to verify exceptions prevent incomplete documents.
        std::cout << "All parser tests passed.\n"; // Print success only after all test functions above return without an assertion failure.
    } catch (const std::exception& error) { // Catch a failed assertion or unexpected standard exception from any test group above.
        std::cerr << "Test failure: " << error.what() << '\n'; // Display the exception's diagnostic to identify which check or reader operation failed.
        return 1; // Report a nonzero failure exit code to PowerShell after the caught test failure.
    } // End the exception handler; subsequent lines belong to its enclosing scope.
    return 0; // Report success after every test group has completed.
} // End the main function body; subsequent lines belong to its enclosing scope.
