TosiSopivaLaskutus
==========================
This project makes possible to add customer, invoice and invoice line data to a database. If some items are added to the database, they also can be printed on command prompt.

This repository can be cloned from: https://github.com/jranxb70/TosiSopivaLaskutus by using Visual Studio 2022  (as of 2024-04-10)

### Prerequirements:

The database driver is Microsoft ODBC driver verion 18.
An SQL Server database. The tested product is Azure Sql Database available in Microsoft Azure.
A file named 'connectionstring.txt' must be available in the working directory. This file must contain a valid connection string to a database.

The software is developed by using Visual Studio 2022. These components must be installed with it:
- Desktop development with C++

### Installation

The project can be opened as a CMake project.


### Initial Configuration (Visual Studio)

The build will install the files app.exe and engine.dll into a directory named: ..\TosiSopivaLaskutus\out\build\x64-Debug\bin


### Debugging

CMake project is used to manage the project put a file named launch.vs.json with the content of:

{
  "version": "0.2.1",
  "defaults": {},
  "configurations": [
    {
      "type": "default",
      "project": "CMakeLists.txt",
      "projectTarget": "app.exe (Install) (bin\\app.exe)",
      "name": "app.exe (Install) (bin\\app.exe)",
      "args": [
        "connectionstring.txt"
      ]
    }
  ]
}

to a folder named '.vs'.