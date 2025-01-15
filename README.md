# TEE Bad Partitioning Detection Tool

## Overview
This tool is designed to identify **bad partitioning issues** in Trusted Execution Environment (TEE) projects. It leverages **CodeQL** for static code analysis and uses the **gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf** cross-compilation toolchain for building TEE projects.

## Dependencies

- **CodeQL**: [Installation Guide](https://codeql.github.com/docs/)
- **gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf**: [Download](https://releases.linaro.org/components/toolchain/binaries/) and update `config.mk`.

## Directory Structure

- **benchmark**: Contains TEE projects used to evaluate the TEE bad partitioning detection tool.
- **optee_client** and **optee_os**: Dependencies required for building TEE projects.
- **query**: Includes CodeQL query scripts for analyzing TEE projects.

## Usage

### Step 1: Navigate to a TEE Project Directory
For example:
```bash
$ cd benchmark/Lenet5_in_OPTEE
```

### Step 2: Build the CodeQL Database
Run the following command to create a CodeQL database:
```bash
$ codeql database create tee_example --language=c-cpp --command=make --overwrite
```
**Note**: If modifications are made to the project code, ensure you run `make clean` before executing the above command.

A `tee_example` folder will be generated in the current directory.

```bash
Finalizing database at /*/benchmark/Lenet5_in_OPTEE/tee_example.
...
Finished zipping source archive (153.37 KiB).
```

### Step 3: Run the Analysis
Execute the Python script to obtain detection results:
```bash
$ python ../../real_world_analysis.py
...
Unencrypted Data Output: 1
['file:///*/benchmark/Lenet5_in_OPTEE/ta/lenet5_ta.c:156:21:156:21']
Input Validation Weakness: 3
['accesstoarrayfile:///*/benchmark/Lenet5_in_OPTEE/ta/lenet.c:200:42:200:49', 'accesstoarrayfile:///*/benchmark/Lenet5_in_OPTEE/ta/lenet.c:280:25:280:33', 'accesstoarrayfile:///*/benchmark/Lenet5_in_OPTEE/ta/lenet.c:282:35:282:43']
Shared Memory Overwrite: 0
[]
```