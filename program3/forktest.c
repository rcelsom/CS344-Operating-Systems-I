#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void main(){
	pid_t spawnpid = -5; 
	int child_exit_method = -5;
	int exit_status;
	int ten =10;
	
	spawnpid = fork();
	switch(spawnpid){
		case -1: 
			perror("hulle breach!\n");	
			exit(1);
			break;
		case 0: 
			ten = ten +1;
			printf("This is the child prcess! This is the vale of ten: %d\n", ten);
			printf("I am the child and this is my PID: %d\n", getpid());
			  printf("waiting\n");
			sleep(2);
			printf("Child %d is converting into \'ls -a\'\n", getpid());
			execlp("ls", "ls", "-al", NULL);
			exit(2); break; 	
		default:
			waitpid(spawnpid, &child_exit_method, 0);
			printf("Lets explore how the child process I just spawned exited!\n");
			if(WIFEXITED(child_exit_method) != 0)
				exit_status = WEXITSTATUS(child_exit_method);		
			if(WIFSIGNALED(child_exit_method) != 0)
				exit_status = WTERMSIG(child_exit_method);
			printf("This is the exit status: %d\n", exit_status);
			ten = ten- 1;
			printf("I am the parent. This is the value of ten: %d\n", ten);
			printf("This is the PID of the child I spawned: %d\n",spawnpid); 	
			exit(0);break; 
	}
	printf("This line will be excuted by both of us! This is the value of ten: %d\n", ten);



}




