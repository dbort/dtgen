def main():
    import argparse

    from dtgen.device_tree import DeviceTreeParser

    parser = argparse.ArgumentParser(description="Device Tree Generator Tool")
    parser.add_argument("input_file", help="Path to the Device Tree source file")
    parser.add_argument("output_file", help="Path to the output C code file")

    args = parser.parse_args()

    parser = DeviceTreeParser()
    try:
        data = parser.parse(args.input_file)
        code = parser.generate_code(data)

        with open(args.output_file, "w") as f:
            f.write(code)

        print(f"Generated C code has been written to {args.output_file}")
    except Exception as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    main()
