Aidan Becker
Dulal Kar
COSC 3346 Operating Systems 
11/01/2020
Project #4: Process Synchronization Using Pthreads: The Producer / Consumer Problem With Prime Number Detector README
This project is intended for use exactly as the requirements document outlines. 

       Installation:
Extract ABecker2Proj4.cpp and makefile to a directory.
Open a command line, navigate to that directory.
Type “make” and press enter.
To uninstall, delete all files in the directory.

	Usage:
Navigate to the installation directory in the command line.
Type “./proj4 arg1 arg2 arg3 arg4 arg5” and press enter.
Arg1 will be the amount of time that you wish the simulation to run for.
Arg2 will be the amount of time in milliseconds that a thread should wait before attempting to produce/consume an item.
Arg3 will the number of producer threads you wish to create.
Arg4 will be the number of consumer threads you wish to create.
Arg5 will be “yes” or “no”, whether or not you want buffer state output whenever a thread attempts to produce/consume an item.
