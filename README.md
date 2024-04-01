**Description**:

mem-dump is a C program designed for extracting raw memory contents of a shared library from an Android process and dumping it to a binary file. It utilizes the process_vm_readv system call to read memory from another process, making it suitable for rooted Android devices.

**Key Features**:

- Directly extracts raw memory contents without reconstructing ELF structure.
- Automatically identifies the target process and shared library based on package name and module name specified in the code.
- Performs memory dump in manageable chunks to handle large libraries efficiently.
- Provides error handling for file operations, memory reading, and other potential failures.
- Outputs the memory dump to a binary file for further analysis or use.

**Usage**:

1. Change the target process package name and shared library name in the code to match your requirements.
2. Compile the program for Android.
3. Run the compiled binary on a rooted Android device with appropriate permissions.
4. The program identifies the process, locates the shared library in memory, and dumps the memory contents to a binary file.

**Disclaimer**:

This tool is intended for educational and research purposes only. Unauthorized access to process memory and raw memory dumping may violate platform policies or laws. Use this tool responsibly and with appropriate permissions. The author is not responsible for any misuse or damage caused by the tool.

**Contributing**:

Contributions are welcome! If you find any issues or have suggestions for improvements, please open an issue or submit a pull request.

**License**:

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT).
