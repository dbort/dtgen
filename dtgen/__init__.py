# This file initializes the dtgen library. It can include metadata about the package and import necessary components from other modules.

__version__ = "0.1.0"
__author__ = "Dave Bort"
__description__ = (
    "A command-line tool for reading Device Tree .dts files and generating C code."
)

from .generator import Generator

__all__ = ["Generator"]
