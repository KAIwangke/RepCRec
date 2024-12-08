# Distributed Database System (RepCRec)

## Overview
A C++ implementation of a distributed database system with replicated concurrency control and recovery. The system implements Serializable Snapshot Isolation (SSI) to manage concurrent transactions across multiple sites while handling site failures and recoveries.

## Features
- Multi-site distributed database architecture
- Serializable Snapshot Isolation (SSI) for transaction management
- Site failure and recovery handling
- Variable replication across sites
- Read-only and read-write transaction support
- Deadlock detection and resolution
- Buffered write operations
- Automated conflict detection and resolution

## System Architecture

### Core Components
1. **TransactionManager**: Coordinates transaction execution and ensures ACID properties
2. **DataManager**: Manages distributed sites and data access
3. **Site**: Represents individual database sites with local variables
4. **Variable**: Implements multiversion concurrency control
5. **CommandParser**: Processes input commands for database operations

### Data Distribution
- Even-indexed variables (x2, x4, etc.) are replicated across all sites
- Odd-indexed variables (x1, x3, etc.) are stored at site 1 + (i mod 10)
- Each site maintains version history for its variables

## Build Instructions

### Prerequisites
- CMake (version 3.10 or higher)
- C++11 compatible compiler
- Make

### Building the Project
```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Running the System
```bash
./RepCRec                    # Interactive mode
./RepCRec input_file.txt     # File input mode
```
or
```bash
make test01                    # cmake single test
make test02                    # cmake single test
make test03_5                    # cmake single test
```


### Testing
The project includes a comprehensive test suite in the `test` directory. Run tests using:
```bash
make test        # Run all tests
make diff_all    # Compare test outputs with expected results
```

### Supported Commands
- `begin(T1)` - Start transaction T1
- `beginRO(T1)` - Start read-only transaction T1
- `R(T1,x1)` - Read variable x1 in transaction T1
- `W(T1,x1,101)` - Write value 101 to variable x1 in transaction T1
- `end(T1)` - End transaction T1
- `dump()` - Display current state of all sites
- `fail(1)` - Mark site 1 as failed
- `recover(1)` - Recover site 1

### Example Usage
```
begin(T1)
begin(T2)
W(T1,x1,101)
W(T2,x2,202)
R(T1,x2)
end(T1)
end(T2)
dump()
```

## Implementation Details

### Transaction Management
- Uses Serializable Snapshot Isolation (SSI) for concurrency control
- Maintains read/write sets for each transaction
- Detects and prevents write-write conflicts
- Handles transaction dependencies and cycle detection

### Site Management
- Tracks site status (UP/DOWN/RECOVERING)
- Maintains variable version history
- Handles site failures and recoveries
- Ensures data consistency during recovery

### Variable Management
- Implements multiversion concurrency control
- Tracks commit timestamps for each version
- Supports consistent reads based on transaction start time
- Handles replicated and non-replicated variables

## Authors
- Ke Wang (kw3484@nyu.edu)
- Siwen Tao (st5297@nyu.edu)
