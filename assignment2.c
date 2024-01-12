#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

// do not edit, returns sum of numbers within parameter range
int summation(int start, int end) {
	int sum = 0;
	if (start < end) {
		sum = ((end * (end + 1)) - (start * (start - 1))) / 2;
	}
	return sum;
}

// do not edit
int ith_part_start(int i, int N, int M) {
	int part_size = N / M;
	int start = i * part_size;
	return start;
}

// do not edit
int ith_part_end(int i, int N, int M) {
	int part_size = N / M;
	int end = (i < M - 1) ? ((i + 1) * part_size - 1) : N;
	return end;
}

// main (parent) process
int main(int argc, char **argv) {
	// getting inputs (N, M)
	if (argc != 3) {
		printf("inccorect args received\n");
		exit(1);
	}
	int N = atoi(argv[1]);
    int M = atoi(argv[2]);

	// creating pipe
	int port[2];
	int status;
	status = pipe(port);

	// checking for error while creating pipe
	if(status < 0) { // pipe creation failed
		printf("pipe creation failed\n");
		exit(1); // exiting execution
	}

	// printing updates before forking child1
	printf("\nparent(PID %d): process started\n\n", getpid());
	printf("parent(PID %d): forking child_1\n", getpid());

	// 1. Main program will initiate the creation of a child process, child1
	pid_t child_1, parent, child; // initialize variables to hold PID's
	child_1 = fork(); // create child process

	if (child_1 > 0){ // ---- in parent ---- //
		printf("parent(PID %d): fork successful for child_1(PID %d)\n", getpid(), child_1);
		printf("parent(PID %d): waiting for child_1(PID %d) to complete\n", getpid(), child_1);
		// 2. parent process will pause, awaiting the completion of child_1's tasks before it concludes
		wait(NULL);

		// child_1 complete, program completed
		printf("parent(PID %d): parent complete\n\n", getpid());
	} else if (child_1 == 0) { // ---- in child ---- //
		// 3. child_1, the fork() function will be called M times, utilizing a loop to generate M child processes
		printf("\nchild_1(PID %d): process started from parent(PID %d)\n", getpid(), getppid());
		printf("child_1(PID %d): forking child_1.1 .... child_1.%d\n\n", getpid(), M);

		// looping to create M children
		pid_t child1_M; // initialize child
		for(int i = 0; i < M; i++) {
			child1_M = fork(); // forking Mth child
			if (child1_M > 0) { // ---- in parent (child1) ---- //
				printf("child1.%d(PID %d): fork() successful\n", i, getpid());
				wait(NULL); // waiting for child to complete
			} else if (child1_M == 0) { // ---- in child (child1.M) ---- //	
				int start = ith_part_start(i, N, M);
				int end = ith_part_end(i, N, M);
				int sum = summation(start, end);
				write(port[1],&sum, sizeof(sum)); // writing summation to pipe
				printf("child1.%d(PID %d): partial sum: [%d - %d] = %d\n", i, getpid(), start, end, sum);
				break; // as to not create exponential children
			} else { // ---- forking child1.M failed ---- //
				printf("\nchild1_M creation failed\n");
			}
		}

		// all children have competed, read from pipe and print total sum
		if(child1_M > 0) {
			int d; // variable to hold read int
			int sum = 0; // variable to hold total sum
			for(int i = 0; i < M; i++) {
				read(port[0],&d, sizeof(d)); // reading int from pipe
				sum += d; // adding int read (d) to total sum
			}
			printf("\nchild_1(PID %d): total sum = %d\n", getpid(),sum);
			printf("child_1(PID %d): child_1 complete\n\n", getpid());
		}

	} else { // ---- fork failed ---- //
		printf("\nchild_1 creation failed\n");
	}
	return 0; // default return, execution completed
}