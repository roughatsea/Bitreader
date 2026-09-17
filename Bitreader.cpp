#include "BinaryParser.h" // Bring the parser declarations, reader, and nested models into this implementation.

#include <fstream> // Provide std::ifstream for reading an external file as the parser's input stream. Angle syntax: The < and > delimit fstream as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.
#include <iostream> // Provide std::cout and std::cerr for normal output and error reporting below. Angle syntax: The < and > delimit iostream as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.
#include <sstream> // Provide in-memory input streams so sample bytes can use the same parser as files. Angle syntax: The < and > delimit sstream as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.
#include <stdexcept> // Provide exception types used to report invalid inputs and failures to the caller. Angle syntax: The < and > delimit stdexcept as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.
#include <string> // Provide strings for sample bytes, file paths, and diagnostic messages. Angle syntax: The < and > delimit string as a header name for the preprocessor's configured include search paths, bringing in the library declarations used below.

namespace { // Give the following helper functions or test data internal linkage within this source file.

void printDocument(const bitreader::BinaryDocument& document) // Define a printer that traverses the assembled hierarchy without copying or modifying it.
{ // Begin printDocument's body; the following statements implement the declaration immediately above.
    const auto& header = document.getHeader(); // Borrow the root's header for reading its nested format and record count below.
    const auto& format = header.getFormat(); // Borrow the header's FormatInfo child to display its leaf fields below.
    std::cout << "BinaryDocument\n" // Start one chained output expression by printing the root model's name. Angle syntax: Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
              << "  Header\n" // Continue the expression with indentation that shows the header belongs to the root. Angle syntax: Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
              << "    Format: version=" << static_cast<unsigned>(format.getVersion()) // Display the nested version numerically; converting uint8_t avoids treating it as a character. Angle syntax: The <unsigned> names the explicit conversion's target type; this makes the eight-bit model value print as a number instead of a character. Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
              << ", tagged=" << std::boolalpha << format.getTagged() << '\n' // Display the format's flag as true or false, enabling that formatting for subsequent bool output. Angle syntax: Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
              << "    Record count: " << header.getRecordCount() << '\n'; // Finish the header output expression with the count that guided record parsing. Angle syntax: Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.

    for (const auto& record : document.getRecords()) { // Visit every record owned by the root, borrowing each element instead of copying it.
        const auto& metadata = record.getMetadata(); // Borrow this record's metadata child for the following ID, category, and flag output.
        const auto& measurement = record.getMeasurement(); // Borrow this record's measurement child for the following magnitude and exponent output.
        std::cout << "  Record\n" // Start a new output chain for the current record with root-child indentation. Angle syntax: Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
                  << "    Metadata: id=" << metadata.getId() // Continue the record output by displaying its metadata child's ID. Angle syntax: Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
                  << ", category=" << static_cast<unsigned>(metadata.getCategory()) // Display the three-bit category as a number rather than a uint8_t character. Angle syntax: The <unsigned> names the explicit conversion's target type; this makes the eight-bit model value print as a number instead of a character. Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
                  << ", active=" << metadata.getActive() << '\n' // Finish the metadata line using the bool formatting enabled in the header output. Angle syntax: Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
                  << "    Measurement: magnitude=" << measurement.getMagnitude() // Continue the same output chain with the record's measurement magnitude. Angle syntax: Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
                  << ", exponent=" << static_cast<unsigned>(measurement.getExponent()) // Display the four-bit exponent numerically, avoiding character output. Angle syntax: The <unsigned> names the explicit conversion's target type; this makes the eight-bit model value print as a number instead of a character. Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
                  << '\n'; // Terminate the current record's output chain and move the next output to a new line. Angle syntax: Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
    } // End the record-printing loop after displaying each pair of child objects.
} // End the printDocument function body; subsequent lines belong to its enclosing scope.

} // Close the file-local helper namespace; the helpers above remain private to this source file.

int main(int argc, char* argv[]) // Define the program entry point; argc and argv select sample input or a supplied file path.
{ // Begin main's body; the following statements implement the declaration immediately above.
    if (argc > 2) { // Reject more than one user argument because the program accepts only an optional input path. Angle syntax: The > comparison tests whether argc exceeds two, counting the executable name too; that rejects more than one user-supplied file path.
        std::cerr << "Usage: Bitreader [binary-file]\n"; // Explain the accepted command syntax when the argument-count check above fails. Angle syntax: Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
        return 1; // Return a failure exit code to PowerShell for the invalid arguments or caught error above.
    } // End the branch guarded by argc > 2; subsequent lines belong to its enclosing scope. Angle syntax: The > in the existing comment repeats the argument-count guard above; it explains which branch ends here and is prose rather than another executed comparison.

    try { // Start the protected input-and-parsing path so the catch block below can report exceptions.
        if (argc == 2) { // Select external file input when the user supplies exactly one path argument.
            std::ifstream input(argv[1], std::ios::binary); // Open that path as raw bytes; the BitReader below borrows this stream.
            if (!input) { // Check that file opening succeeded before creating a reader or parsing fields.
                throw std::runtime_error("Could not open input file: " + std::string(argv[1])); // Send the failed path to the shared catch block instead of parsing an unusable stream.
            } // End the branch guarded by !input; subsequent lines belong to its enclosing scope.
            bitreader::BitReader reader(input); // Wrap the current file or memory stream in a reader that all nested parsers will share.
            printDocument(bitreader::readDocument(reader)); // Parse the complete hierarchy first, then pass the resulting document to the printer above.
        } else { // Switch from the external-file branch to the built-in example when no path was supplied.
            // Demonstration format: one 16-bit header and two 40-bit records.
            const std::string sample("\xB0\x02\x12\x3B\xAB\xCD\xE2\x45\x64\x12\x34\x57", 12); // Store the exact twelve sample bytes encoding one header and two records in the documented format.
            std::istringstream input(sample); // Expose the sample bytes through a stream so the same BitReader and parser can consume them.
            bitreader::BitReader reader(input); // Wrap the current file or memory stream in a reader that all nested parsers will share.
            std::cout << "Parsing the built-in sample. Pass a file path to parse a file.\n"; // Explain which input branch is running before printing its parsed hierarchy. Angle syntax: Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
            printDocument(bitreader::readDocument(reader)); // Parse the complete hierarchy first, then pass the resulting document to the printer above.
        } // End the built-in-sample branch; subsequent lines belong to its enclosing scope.
    } catch (const std::exception& error) { // Handle standard exceptions from file opening or any nested parser after the try block ends.
        std::cerr << "Parse error: " << error.what() << '\n'; // Print the caught diagnostic, including the consumed-bit position for reader failures. Angle syntax: Each << is the output stream's insertion operator: it sends the following value or formatting manipulator to the same stream and returns that stream so the expression can be chained.
        return 1; // Return a failure exit code to PowerShell for the invalid arguments or caught error above.
    } // End the exception handler; subsequent lines belong to its enclosing scope.
    return 0; // Report success to the calling shell after the selected input has been parsed and printed.
} // End the main function body; subsequent lines belong to its enclosing scope.
