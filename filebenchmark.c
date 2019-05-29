//File system benchmarking tool - parallel creates and deletes
//Gregory Touretsky, gregory.touretsky@gmail.com 2018.12.05

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_TEST_THREADS 50
#define CREATE 1
#define DELETE 2

int mypid;

// Parameters for testThread
typedef struct {
	int task_id;
	int task_fileNum;
	int task_reportFreqSec;
	char task_path[255];
	char task_prefix[20];
	int task_operation;
    long task_blkSizeBytes;
    long task_fileSizeBlks;
} testThreadParams_struct;

long getmSecSinceEpoch() {
	long ms; //milliseconds
	time_t s; //seconds
	struct timespec spec;


    clock_gettime(CLOCK_REALTIME, &spec);
    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999) {
        s++;
        ms = 0;
    }

    return (s*1000+ms);

}

void *testThread(void *vargp) {
	long ms,secLast,secCurrent;
	int i, k, iLast;
	char fname[512];
    char *block;
	testThreadParams_struct *testThreadParams = vargp;

    if( mkdir(testThreadParams->task_path,0777)) {
    	printf("PID: %d. TID: %d. Failed to create directory %s, maybe it is already there...\n",
    		mypid,testThreadParams->task_id,testThreadParams->task_path);
    }
    if(testThreadParams->task_blkSizeBytes > 0) {
        block = (char *)malloc(testThreadParams->task_blkSizeBytes);
    }
    ms = getmSecSinceEpoch();

    printf("PID: %d. TID: %d. Processing %d files in %s. Started at %jd milliseconds since the Epoch\n",
    	mypid,testThreadParams->task_id,testThreadParams->task_fileNum,testThreadParams->task_path,ms);

    secLast = round(ms/1.0e3);iLast=0;
    for(i=0; i<testThreadParams->task_fileNum; i++) {
    	secCurrent=round(getmSecSinceEpoch()/1.0e3);
    	if(secCurrent >= secLast + testThreadParams->task_reportFreqSec) {
    		printf("PID: %d. TID: %d. %ld: processed %d files in %ld seconds\n",mypid,testThreadParams->task_id,secLast,i-iLast,secCurrent-secLast);
    		secLast=secCurrent;
    		iLast=i;
    	}

    	sprintf(fname,"%s/%s_%09d",testThreadParams->task_path,testThreadParams->task_prefix,i);
    	if(testThreadParams->task_operation == CREATE) {
    	//printf("%s\n",fname);
    		FILE *fp = fopen(fname, "wb");
  	  		if (!fp) {
    			perror("Failed to open a file");
    			free(testThreadParams);
    			return NULL;
    		}
            //write data if file size > 0
            if((testThreadParams->task_blkSizeBytes > 0) && (testThreadParams->task_fileSizeBlks > 0)) {
                for(k=0; k<testThreadParams->task_fileSizeBlks; k++) {
                    fwrite(block, testThreadParams->task_blkSizeBytes, 1, fp);
                }
            }
    		fclose(fp);
    	} else if(testThreadParams->task_operation == DELETE) {
    		if(remove(fname) !=0) {
    			printf("Can not remove file %s, continue\n",fname);
    		}
    	}
    }

    printf("PID: %d. TID: %d. Processed %d files in %ld milliseconds\n",mypid,testThreadParams->task_id,i,getmSecSinceEpoch()-ms);
    if(testThreadParams->task_blkSizeBytes > 0) {
        free(block);
    }

    free(testThreadParams);
	return NULL;
}

int main(int argc, char **argv) {

	pthread_t thread_id[MAX_TEST_THREADS];
	int threadNum,fileNum,i;
	long reportFreqSec,fileSizeBlks,blkSizeBytes;
	char path[255],prefix[20];
	int operation;

	printf("Testing files creation and deletion by gtouretsky@infinidat.com 2018.12.05\n");
	if(argc != 9) {
		printf("Wrong usage. Run %s <operation> <path> <filePrefix> <numFiles> <numThreads> <reportFreqSec> <blkSizeBytes> <fileSizeBlks>\n",argv[0]);
		printf("   operation: c or d. c=create. d=delete\n");
		printf("   path: where files get processed\n");
		printf("   filePrefx: beginning of the file name\n");
		printf("   numFiles: how many files should be processed by each thread\n");
		printf("   numThreads: how many concurrent threads should process files.\n"); 
        printf("               Every thread creates files in a separate directory\n");
		printf("   reportFreqSec: how often (in sec) should the processing speed be reported\n");
        printf("   blkSizeBytes: block size in bytes for new files\n");
        printf("   fileSizeBlks: file size in blocks\n");
		printf("\n\n");
		printf("Usage example: %s c /tmp/testfolder file_ 10000 3 5 0 0\n", argv[0]);
		printf("   This command will create directories t_0, t_1, t_2 in /tmp/testfolder.\n");
		printf("   It will launch 3 threads, each creating 10000 files in one of these directories.\n");
		printf("   Every thread will report how many files it has created every 5 seconds.\n");
        printf("   Every file will have 0 bytes size.\n");
		return(-1);
	}

	if(strcmp(argv[1],"c") == 0) {
		operation = CREATE;
	} else if(strcmp(argv[1],"d") == 0) {
		operation = DELETE;
	} else {
		printf("Unsupported operation %s, exiting\n",argv[1]);
		return(-5);

	}
    strcpy(path,argv[2]);
    strcpy(prefix,argv[3]);
    fileNum=atoi(argv[4]);
    threadNum = atoi(argv[5]);
    reportFreqSec=atoi(argv[6]);
    blkSizeBytes = atoi(argv[7]);
    fileSizeBlks = atoi(argv[8]);

    if(threadNum > MAX_TEST_THREADS) {
    	printf("Too many threads specified, can not exceed %d\n",MAX_TEST_THREADS);
    	return(-4);
    }

    DIR* dir = opendir(path);
    if(dir)
    {
    	closedir(dir);
    }
    else if (ENOENT == errno) {
    	printf("Directory %s does not exist\n",path);
    	return (-2);
    }
    else
    {
    	printf("Can not open directory %s\n",path);
    	return (-2);
    }


    mypid = getpid();


    for(int i=0;i<threadNum;i++) {
		printf("Starting thread %i\n",i);
		testThreadParams_struct *args = malloc(sizeof *args);
		sprintf(args->task_path,"%s/t_%d",path,i);
		strcpy(args->task_prefix,prefix);
		args->task_id = i;
		args->task_fileNum = fileNum;
		args->task_reportFreqSec = reportFreqSec;
		args->task_operation = operation;
        args->task_blkSizeBytes = blkSizeBytes;
        args->task_fileSizeBlks = fileSizeBlks;
		if(pthread_create(&thread_id[i], NULL, testThread, args)) {
			free(args);
			perror("Failed to launch thread");
			return(0);
		};
	}

	// wait for all threads completion
	for(int i=0;i<threadNum;i++) {
		pthread_join(thread_id[i],NULL);
	}

	printf("All threads finished\n");


	return(0);


}



