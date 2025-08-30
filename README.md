# dtgen - Generate code from Device Tree files

## Overview
`dtgen` is a library and command-line tool designed to read Device Tree `.dts` files and generate C code based on them.

## Installation
Clone the repo and run:

```bash
pip install .
```

## Usage

```bash
dtgen <input>.dts
```

## Running Tests
Find and run all tests in the `tests` directory:

```bash
python -m unittest discover tests
```

## Development Dependencies

```bash
pip install .[dev]
```

## Code Formatting

Format all code in the repo:

```bash
ufmt format .
```

Check formatting without making changes:

```bash
ufmt check .
```

## License
This project is licensed under the MIT License. See the LICENSE file for more details.