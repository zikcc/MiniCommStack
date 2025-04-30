# Tests

This directory contains the test suite for the MiniCommStack project.

## Structure

```
tests/
├── CMakeLists.txt        # CMake configuration for tests
└── unit/                 # Unit tests
    └── logger_test.cpp   # Tests for the Logger class
```

## Requirements

- CMake 3.10 or higher
- C++17 compatible compiler
- Internet connection (for downloading Google Test)

## Building the Tests

From the project root directory:

```bash
mkdir build
cd build
cmake ..
make
```

## Running the Tests

After building, you can run the tests using either:

```bash
# From the build directory
ctest

# Or run the test executable directly
./tests/logger_test
```

## Test Coverage

The test suite currently covers:

- Basic logging functionality
- Log level filtering
- Thread safety of the Logger class

## Adding New Tests

To add new tests:

1. Create a new test file in the appropriate directory (e.g., `unit/` for unit tests)
2. Add the test file to `tests/CMakeLists.txt`
3. Follow the Google Test framework conventions for writing tests 