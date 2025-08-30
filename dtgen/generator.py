from pathlib import Path
from typing import Iterable, TextIO

from devicetree import dtlib


class Error(Exception):
    """Raised when encountering Generator-specific errors."""


class ParseError(Error):
    """Raised when failing to parse .dts input."""


class Generator:
    def __init__(self, dts_file: Path | str, include_path: Iterable[str] = ()):
        """Parses a .dts file and prepares for code generation.

        Args:
          dts_file: Path to the .dts data to read.
          include_path: Paths to search for /include/d and /incbin/'d files.
              By default, files are only looked up relative to the .dts file
              that contains the /include/ or /incbin/.

        Throws:
            OSError: If the .dts file cannot be read.
            ParseError: If parsing the .dts file fails.
        """
        try:
            self._dt = dtlib.DT(str(dts_file), include_path=include_path)
        except dtlib.DTError as e:
            raise ParseError(f"Failed to parse .dts file: {e}") from e

    def generate_code(self, outfp: TextIO) -> None:
        """Generates C code from the parsed .dts file.

        Args:
            outfp: A writable text file-like object to write the generated code
                to.
        """
