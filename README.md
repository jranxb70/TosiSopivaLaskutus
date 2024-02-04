TosiSopivaLaskutus
==========================
This project makes possible to add customer, invoice and invoice line data to a database. If some items are added to the database, they also can be printed on command prompt.

The database driver is Microsoft ODBC driver

### Installation

NOTE: the project can be opened as a CMake project but also as a Visual Studio 2022 solution.

To create a Microsoft Visual Studio 2022 Solution for the project's code, follow the steps described in here:

Create a directory named a'build' and call cmake from there on the command line, indicating where the CMakeLists.txt file is:

	TosiSopivaLaskutus\build> cmake ../src -G "Visual Studio 17 2022"
	
After this step, a solution file is created and it can be found from the build directory. Openin the solution file opens two projects: an app and an engine.

Change the target build environment as appropriate.

Open the solution file in Visual Studio, and build the ALL_BUILD project.

### Initial Configuration (Visual Studio)

Before this will run in Visual Studio, fix the following glitches:

1) By default, ALL_BUILD will be the startup project. Right click on the app project and click `Set as Startup Project`.

2) The app.exe executable depends on the engine.dll that we built with ALL_BUILD, but the system doesn't know how to find it. Build the INSTALL project. This will copy app.exe and engine.dll to the bin directory in the build folder.

3) Running app.exe through Visual Studio still won't work, because the command is referencing the copy of app.exe in the `app/Debug` folder, rather than the new copy in the bin directory. Right click on the app project, and select Properties -> Debugging. Set the command to `..\bin\app.exe` and set Command Arguments to some number (e.g. 200 and connectionstring.txt).

Now, when you tell Visual Studio to call app.exe, it is able to find engine.dll, because there is a copy of it in the same folder.

### Debugging

Be sure to re-run INSTALL after each build. Otherwise, when you run through the IDE, you would be running the previous version of the code in the bin directory, and you will not be able to see the effects of your changes.


NOTE ALSO: If the Visual Studio 2022 solution is used to manage the TosiSopilaLaskutus, the command line arguments must be added to the project properties for the project 'app'.

If CMake project is used to manage the project put a file named launch.vs.json with the content of:

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
        "33","connectionstring.txt"
      ]
    }
  ]
}

to a folder named '.vs'.