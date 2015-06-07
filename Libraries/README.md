
OpenCV installation guide is provided in the next link:

http://docs.opencv.org/doc/tutorials/introduction/linux_install/linux_install.html


Steps to use the library cvBlobsLib (using MSVC++ sp 5):

1 - open the project of the library and build it

2 - Add the libraries: in the project where the library should be used, add:
2.1 In "Project/Settings/C++/Preprocessor/Additional Include
directories" add the directory where the blob library is stored
2.2 In "Project/Settings/Link/Input/Additional library path" add
the directory where the blob library is stored and in "Object/Library
modules" add the cvblobslib.lib file

3- Include the file "BlobResult.h" where you want to use blob variables.

4- To see an example on using the blob library, see the file
example.txt inside the zip file.

NOTE: Verify that in the project where the cvblobslib.lib is used, the MFC Runtime Libraries are not mixed: 

1. 	Check in "Project->Settings->C/C++->Code Generation->Use run-time library" of your project and set it to 
	Debug Multithreaded DLL (debug version ) or to Multithreaded DLL ( release version ).
2 	Check in "Project->Settings->General" how it uses the MFC. It should be "Use MFC in a shared DLL". 

Steps to use the library libconfig:

The simplest way to compile this package is:

cd' to the directory containing the package's source code and type
./configure'...

Type `make' to compile the package.

Optionally, type `make check'...

Type `make install' to install the programs and any data files and documentation.

You can remove the program binaries and object files from the source code directory by typing `make clean'... 

You must add the libraries in the Eclipse project as usual

