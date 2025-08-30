import io
import os
import pathlib
import tempfile
import unittest

from dtgen import generator


class TestGenerator(unittest.TestCase):

    def setUp(self):
        self._test_dir = tempfile.TemporaryDirectory()
        self.temp_dir = self._test_dir.name
        self.this_dir = pathlib.Path(__file__).parent

    def tearDown(self):
        self._test_dir.cleanup()

    def create_temp_file(self, content: str, filename: str = "temp.dts") -> str:
        # Create a temp file with the provided content in the temp directory
        file_path = os.path.join(self.temp_dir, filename)
        with open(file_path, "w") as f:
            f.write(content)
        return file_path

    def test_parse_valid_succeeds(self):
        # Smoke test that we can create a Generator with minimal valid DTS.
        dts_file = self.create_temp_file("/dts-v1/; / { };")
        generator.Generator(dts_file)

    def test_parse_invalid_fails(self):
        # Smoke test that we can't create a Generator with invalid DTS.
        dts_file = self.create_temp_file("NOT VALID")
        with self.assertRaises(generator.ParseError):
            generator.Generator(dts_file)

    def test_parse_missing_fails(self):
        # Smoke test that we can't create a Generator with invalid DTS.
        with self.assertRaises(OSError):
            generator.Generator("NONEXISTENT.dts")

    def test_parse_example_succeeds(self):
        # Smoke test that the example file parses correctly.
        generator.Generator(self.this_dir / "example.dts")

    def test_gen_example(self):
        # Round-trip test using the example file.
        gen = generator.Generator(self.this_dir / "example.dts")
        with io.StringIO() as f:
            gen.generate_code(f)
            code: str = f.getvalue()
            self.assertEqual(code, "")


if __name__ == "__main__":
    unittest.main()
