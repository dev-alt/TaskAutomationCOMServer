# Task Automation COM Server

## Overview

The Task Automation COM Server is a powerful tool designed to automate repetitive file management tasks such as renaming files and organizing directories. This project includes a COM server implemented in C++ and a Windows Explorer shell extension for easy access to file management features directly from the context menu.

## Features

- Batch rename files with customizable options
- Organize directories based on various criteria
- Windows Explorer integration via right-click context menu
- COM interface for integration with other applications (e.g., C# clients)

## System Requirements

- Windows 10 or later
- .NET Framework 4.7.2 or later (for C# clients)
- Visual C++ Redistributable for Visual Studio 2019 (or compatible version)

## Installation

1. Download the latest release from the [Releases](https://github.com/yourusername/task-automation-com-server/releases) page.
2. Run the installer package and follow the on-screen instructions.
3. The installer will deploy and register both the COM server and the shell extension.

## Usage

### Via Windows Explorer

1. Right-click on a file or folder in Windows Explorer.
2. Select one of the custom options from the context menu:
   - "Batch Rename Files"
   - "Organize Directory"
3. Follow the prompts in the dialog box to specify parameters for the operation.

### Via COM Interface (for Developers)

```csharp
// C# example
var fileManagement = new TaskAutomation.FileManagementClass();
fileManagement.BatchRenameFiles(@"C:\MyFiles", "_", true, "*.txt");
```

Refer to the [API Documentation](docs/API.md) for detailed information on available methods and parameters.

## Building from Source

1. Clone the repository:
   ```
   git clone https://github.com/yourusername/task-automation-com-server.git
   ```
2. Open the solution in Visual Studio 2019 or later.
3. Build the solution in Release mode.
4. Use `regsvr32` to register the COM server and shell extension DLLs.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

```
MIT License

Copyright (c) [year] [fullname]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## Acknowledgments

- [Microsoft COM Documentation](https://docs.microsoft.com/en-us/windows/win32/com/component-object-model--com--portal)
- [Windows Shell Extension Documentation](https://docs.microsoft.com/en-us/windows/win32/shell/shell-exts)

