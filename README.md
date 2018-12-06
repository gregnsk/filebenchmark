NAME
    filebenchmark  -- tests files creation and deletion performance
    
SYNOPSIS
		filebenchmark <operation> <path> <filePrefix> <numFiles> <numThreads> <reportFreqSec>
  
DESCRIPTION
		operation:     c or d. c=create. d=delete
		path:          where files get created
		filePrefx:     beginning of the file name
		numFiles:      how many files should be created by each thread
		numThreads:    how many concurrent threads should create files. Every thread creates files in a separate directory
		reportFreqSec: how often (in sec) should the creation speed be reported
    
BUILD PROCEDURE
    cc -o filebenchmark filebenchmark.c -lm -lpthread
    
EXAMPLES
    filebenchmark c /tmp/testfolder file_ 10000 3 5
    
		This command will create directories t_0, t_1, t_2 in /tmp/testfolder
		It will launch 3 threads, each creating 10000 files in one of these directories
		Every thread will report how many files it has created every 5 seconds
    
AUTHOR
    Gregory Touretsky, gregory.touretsky@gmail.com   Dec, 5 2018
