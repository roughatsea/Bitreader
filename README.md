# Bitreader

A C++20 example that reads binary fields into classes with getters and setters,
then assembles those classes into a hierarchy using composition:

```text
BinaryDocument
├── FileHeader
│   ├── FormatInfo
│   │   ├── version
│   │   └── tagged
│   └── recordCount
└── Record[]
    ├── RecordMetadata
    │   ├── id
    │   ├── category
    │   └── active
    └── Measurement
        ├── magnitude
        └── exponent
```

The model classes live in `models/` and own their children by value. Nested
getters return const references; use setters to replace children. There are no
raw owning pointers or native C++ bit-field layouts.

`io/BitReader` consumes 1–64 bits from an input stream, across byte boundaries,
without loading the entire file. `BinaryParser.cpp` maps unsigned fields to
setters and builds larger models from smaller models:

```cpp
readField<12>(reader, metadata, &RecordMetadata::setId); // Consume twelve bits and pass the numeric value to the metadata object's ID setter. Angle syntax: The <12> is a compile-time template argument selecting a 12-bit field; Model and Value are inferred from the object and setter, and static_assert checks the width.
record.setMetadata(readRecordMetadata(reader)); // Parse the metadata child first and attach it to the parent record through its setter.
record.setMeasurement(readMeasurement(reader)); // Parse the next child with the same advancing reader and attach the measurement to that record.
```

Each `read...` function consumes its model's fields in the order shown below.
The field helper checks at compile time that each width fits the setter's
parameter type. Truncated input causes an exception with the bit position;
the parser does not return an incomplete document. A failed parse can consume
input, so it should not be retried on the same reader without resetting it.

## Demonstration format

This is an illustrative format, not a parser for arbitrary binary files.
Replace the models and mappings with the fields from your actual format.
All fields are unsigned. Bits are read most significant first within each byte,
and the first bit of a field is its most significant bit. There is no padding
between fields. The `tagged` flag and measurement fields are just stored values.

| Container | Field | Bits |
| --- | --- | ---: |
| Header / FormatInfo | version | 3 |
| Header / FormatInfo | tagged | 1 |
| Header | recordCount | 12 |
| Record / RecordMetadata | id | 12 |
| Record / RecordMetadata | category | 3 |
| Record / RecordMetadata | active | 1 |
| Record / Measurement | magnitude | 20 |
| Record / Measurement | exponent | 4 |

A document contains one 16-bit header followed by `recordCount` 40-bit records.
The parser stops after those records; trailing input is left unread.

## Build and run

Open `Bitreader.slnx` in Visual Studio and build with its C++ workload installed.
Run without arguments for a built-in sample, or supply a binary file path:

```powershell
.\x64\Debug\Bitreader.exe # Run the Visual Studio Debug build with no path argument to parse and print the built-in sample.
.\x64\Debug\Bitreader.exe .\sample.bin # Parse the included binary sample file from the project directory.
```

The included `sample.bin` contains the same 12 bytes as the built-in sample:
`B0 02 12 3B AB CD E2 45 64 12 34 57`. It decodes to
version 5, tagged true, and two records: ID 291 with magnitude 703710, and
ID 1110 with magnitude 74565. The existing `test.txt` is plain text and is not
a valid document in this format.

## Processing larger files

`readDocument` keeps all decoded records in a vector. To keep memory bounded,
read the header and process one record at a time instead:

```cpp
bitreader::BitReader reader(input); // Borrow the existing input stream so the header and each record share one consuming bit position.
const auto header = bitreader::readFileHeader(reader); // Parse only the header to learn how many records follow without retaining a full document.
for (std::uint16_t i = 0; i < header.getRecordCount(); ++i) { // Process the number of records declared by the header, one iteration at a time. Angle syntax: The < comparison keeps the zero-based record index below the header's count, processing exactly that many records without attempting an extra one.
    const auto record = bitreader::readRecord(reader); // Assemble one record's nested children from the next forty bits without storing previous records.
    // Process record here, then let it go out of scope.
} // End the loop iteration; its local record is destroyed before the next iteration creates another.
```

The demonstration's 12-bit count permits at most 4095 records; change that
field and its storage type for a format with more records.

## Tests

From a Visual Studio Developer PowerShell, run:

```powershell
msbuild .\tests\ParserTests.vcxproj /p:Configuration=Debug /p:Platform=x64 /verbosity:minimal # Build the x64 Debug test executable from its project using the configured compiler and parser sources.
.\.build\tests\x64\Debug\ParserTests.exe # Run that test build; it exits successfully only if every reader and hierarchy assertion passes.
```

Tests cover nested values, record-by-record parsing, bit ordering, reads across
bytes, 64-bit reads, zero and maximum fields, invalid widths, stream errors,
and every incomplete prefix of the sample document.
