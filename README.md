NAME

    filebenchmark  -- tests files creation and deletion performance
    
SYNOPSIS

		filebenchmark <operation> <path> <filePrefix> <numFiles> <numThreads> <reportFreqSec> <blkSizeBytes> <fileSizeBlks>
  
DESCRIPTION

		operation:     c or d. c=create. d=delete
		path:          where files get created
		filePrefx:     beginning of the file name
		numFiles:      how many files should be created by each thread
		numThreads:    how many concurrent threads should process files. 
		               Every thread processes files in a separate directory
		reportFreqSec: how often (in sec) should the processing speed be reported
   		blkSizeBytes:  block size in bytes for new files
        fileSizeBlks:  file size in blocks
    
BUILD PROCEDURE

    cc -o filebenchmark filebenchmark.c -lm -lpthread -std=gnu99
    
EXAMPLES

    filebenchmark c /tmp/testfolder file_ 10000 3 5 0 0
    
		This command will create directories t_0, t_1, t_2 in /tmp/testfolder
		It will launch 3 threads, each creating 10000 files in one of these directories
		Every thread will report how many files it has created every 5 seconds
		Every file will have 0 bytes size
    
VERSION 
    1.0 (Dec, 5 2018)  First implementation, empty files generator
    2.0 (May, 29 2019) Added support for non-empty files creation

AUTHOR

    Gregory Touretsky, gregory.touretsky@gmail.com   Dec, 5 2018
