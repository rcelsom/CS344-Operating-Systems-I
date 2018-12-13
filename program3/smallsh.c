#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>

bool mode = false;

//I need to make it so the message is displayed immegiately after the the last foreground process stops!!! THEN IM DONE

//Error when running SIGSTSP while in the middle of a sleep cal



void catchSIGINT(int signo){
	//int child_exit_method = -5;
	//wait(&child_exit_method);
	//waitpid(-1, &child_exit_method, WNOHANG);
	char* message = "terminated by signal 2\n";
	write(STDOUT_FILENO, message, 23);
}

void catchSIGTSTP(int signo){
  	char* line ="\n";
	//char* message1 = "Entering foreground-only mode (& is now ignored)\n";
        //char* message2 = "Exiting foreground-only mode\n";
        write(STDOUT_FILENO, line, 1);
	if(!mode){
                mode = true;
		//write(STDOUT_FILENO, message1, 49);
        }
        else if(mode){
                mode = false;

                //write(STDOUT_FILENO, message2, 29);
        }
	 
}

//initializes the sigaction strutures 
void init_signal_handler(struct sigaction* sigint, struct sigaction* sigtstp, /*struct sigaction* sigchld,*/ struct sigaction* ignore){


	ignore->sa_handler = SIG_IGN;

	sigint->sa_handler = catchSIGINT;
	sigfillset(&sigint->sa_mask);		//3.3------35:15
	sigint->sa_flags = 0;

	sigtstp->sa_handler = catchSIGTSTP;
	sigfillset(&sigtstp->sa_mask);             //3.3------35:15
	sigtstp->sa_flags = 0;

	sigaction(SIGINT, sigint, NULL);
	sigaction(SIGTSTP, sigtstp, NULL);	
}

void set_ignore_default(struct sigaction* sigint, /**/ struct sigaction* sigtstp, int foreground, int background){
	//We have a foreground process and we want it to be set to SIG_DFL
	if(foreground){
		sigint->sa_handler = SIG_DFL;
		sigtstp->sa_handler = SIG_IGN;
		sigaction(SIGTSTP, sigtstp, NULL); 
		sigaction(SIGINT, sigint, NULL);
	}
	if(background){
		sigint->sa_handler = SIG_IGN;
		sigtstp->sa_handler = SIG_IGN;
		sigaction(SIGTSTP, sigtstp, NULL);
		sigaction(SIGINT, sigint, NULL);
	}
}

char* promptUser(char input_buffer[]){
	int numCharsEntered = -5; // How many chars we entered
	int currChar = -5; // Tracks where we are when we print out every char
	size_t bufferSize = 0; // Holds how large the allocated buffer is
	char* lineEntered = NULL; // Points to a buffer allocated by getline() that holds our entered string + \n + \0
	// Get input from the user
	while(1){
		//printf("Enter in a line of text (CTRL-C to exit):");
		fflush(stdout);
		printf(":");
		numCharsEntered = getline(&lineEntered, &bufferSize, stdin);
		if (numCharsEntered == -1)
			clearerr(stdin);
		else
			break; // Exit the loop - we've got input
	}
	lineEntered[strcspn(lineEntered, "\n")] = '\0';
	strcpy(input_buffer, lineEntered);

	free(lineEntered);
	lineEntered = NULL;
}

void grab_arguments(char input[], char* args[]){
	char* token;
	int arg_num = 0; 
	token = strtok(input, " ");
	args[arg_num] = token;
	arg_num = arg_num +1;
	while(token != NULL){
		token = strtok(NULL, " ");
		args[arg_num] = token;
		arg_num = arg_num +1;
	}
}

void print_args(char* args[]){
	printf("\n");
	int i = 0;
	while(args[i] != NULL){
		printf("%s\n", args[i]);
		i++;
	}
	printf("\n");
}


void kill_background(pid_t pids[], int num){
	int i = 0, status =-5;
	while(pids[i] != -1){
		//kill the signal then call waitpid to clean its resources
		kill(pids[i], SIGTERM);
		waitpid(pids[i],&status,0);
		i++;
	}	
}

//run this and then hit a switch statement 
int check_built_in(char* args[], pid_t pids[], int num, int child_exit_method){
	//check for empty
	int status, exit_status;
	char cwd [256];
	char* check = args[0];

	memset(cwd, '\0', sizeof(cwd));
	
	if( args[0] == NULL)
		return 1;
	
	else if(check[0] == '#')
		return 1;
	
	else if(strcmp(args[0],"exit")==0){
		//KILL all background childrem
		kill_background(pids, num);
		fflush(stdout);
		printf("leaving the small shell.\n");
		exit(0);		
	}

	//check for cd 
	else if(strcmp(args[0],"cd")==0){
		if(args[1] == NULL){
			status =chdir(getenv("HOME"));

			//printf("This is the status of the chdir() function %d\n", status);
			getcwd(cwd, sizeof(cwd));
			//	printf("We are now in %s\n", cwd);
		}
		else{
			status =chdir(args[1]);
			if(status == -1){
				printf("file %s does not exist\n", args[1]);
				fflush(stdout);
			}
			else{
				getcwd(cwd, sizeof(cwd));
				//	printf("We are now in %s\n", cwd);
			}
		}
		return 1;
	}

	else if(strcmp(args[0],"status")==0){
		if(WIFEXITED(child_exit_method) != 0){
			exit_status = WEXITSTATUS(child_exit_method);
			fflush(stdout);
			printf("exit value %d\n", exit_status);
		}
		else if(WIFSIGNALED(child_exit_method) != 0){
			exit_status = WTERMSIG(child_exit_method);
			fflush(stdout);
			printf("terminated by signal %d\n", exit_status);	
		}
		return 1;
	}

	else 
		return -1;
}

void execute_bash(char** args){
	if(execvp(*args, args) < 0){
		fflush(stdout);
		perror("exec call failed!");
		exit(1);
	}
}

//Return the index of the file to be read from
int get_read_redirection(char* args[]){
	int i = 0;
	while(args[i] != NULL){
		if(strcmp(args[i],"<")==0 && args[i+1] != NULL)
			return i+1;
		i++;
	}
	//stdin is not redirected
	return -1;
}

//Return the index of the file to be written to 
int get_write_redirection(char* args []){
	int i = 0;
	while(args[i] != NULL){
		if(strcmp(args[i],">")==0 && args[i+1] != NULL)
			return i+1;
		i++;
	}
	//stdout is not redirected
	return -1;
}


int check_background(char* args[]){
	int length = 0;
	while(args[length+1] != NULL){
		length++;
	}
	if(strcmp(args[length],"&") == 0)
		return length;

	return -1;
}

void remove_direction(char* args[], int location, int times){
	int j;
	for(j=0; j< times; j++){

		int i = location;
		while(args[i+1] != NULL){
			args[i] = args[i+1];
			i++;
		}
		args[i] = NULL;
	}
}


void reap_zombies(pid_t process[], int* num){
	if(num > 0){
		int i, j, exit_status = -1;
		int child_exit_method =-5;
		for(i = 0; i < *num; i++){
			if(waitpid(process[i], &child_exit_method, WNOHANG) != 0){
				if(WIFEXITED(child_exit_method) != 0)
					exit_status = WEXITSTATUS(child_exit_method);
				if(WIFSIGNALED(child_exit_method) != 0)
					exit_status = WTERMSIG(child_exit_method);
				fflush(stdout);
				printf("background pid %d is done: terminated by signal %d\n",process[i],exit_status );
				*num = *num -1;
				for(j = i; j < *num; j++){
					process[j] = process[j+1];
				}	
			} 
		}
	}
}

void convert(char input[]){
	pid_t pid = getpid();
	char str_pid[15];
	memset(str_pid, '\0', sizeof(str_pid));
	sprintf(str_pid,"%d", getpid());
	while(strstr(input,"$$") != NULL){
		char* location = strstr(input,"$$");	
		 strcpy(location + strlen(str_pid), location + 2);
		 strncpy(location, str_pid, strlen(str_pid));
	}
}


void main(){
	//ADDITION --> flush all filestream fflush(stdout)

	struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0},/* SIGCHLD_action ={0},*/ ignore_action ={0};
	init_signal_handler(&SIGINT_action, &SIGTSTP_action,/* &SIGCHLD_action,*/ &ignore_action);

	char input[2048]; //also need to ensure that the user cannot input more than 512 arguments 
	memset(input, '\0', sizeof(input));

	pid_t spawnpid =-5;
	int child_exit_method = -5, exit_status;

	//ADDITION--> make this a dynamic array later. 
	pid_t processPID[100];
	int num_process =0;
	memset(processPID, -1, sizeof(processPID));

	//code can only handle up to 512 arguments 
	char* arguments[512] ={ NULL };
	int built_in_cmd;

	//redirection variable
	int dest, src, read_in = 0, write_in =0 ;

	//STATUS command 
	int exit_buffer;

	//SIG_TSTP
	bool prev = false;

	while(1){
		reap_zombies(processPID, &num_process);
		
		promptUser(input);
		convert(input);
		//printf("This is the converted input : %s\n", input);
		grab_arguments(input, arguments);

		if(check_built_in(arguments, processPID, num_process, exit_buffer) == -1){	

			int background_index = check_background(arguments);
			spawnpid = fork();
			switch(spawnpid){
				case -1:
					perror("child process failed to spawn!\n");
					exit(1);
					break;
				case 0:
					//this is code run by child process
					read_in = get_read_redirection(arguments);


					if(read_in != -1){
						src = open(arguments[read_in], O_RDONLY, 0600);
						if(src < 0){
							perror("Could not open file!");
							exit(1);
						}
						else{
							dup2(src,0); //redirect stdin
							remove_direction(arguments, read_in-1, 2);
						}
					}

					write_in =get_write_redirection(arguments);
					if(write_in != -1){
						dest = open(arguments[write_in], O_CREAT | O_TRUNC | O_WRONLY, 0600);
						if(dest < 0){
							perror("Could not open file!");
							exit(1);
						}

						else{
							dup2(dest,1); //redirect stdout
							remove_direction(arguments, write_in-1, 2);
						}
					}



					if(background_index != -1){
						remove_direction(arguments,background_index,1);
						if(!mode){
							set_ignore_default(&SIGINT_action, &SIGTSTP_action, 0 , 1);
							if(write_in == -1){
								//no specified file to write to, /dev/null
								dest = open("/dev/null", O_WRONLY);	
								dup2(dest,1);
							}			//ADDITION --> error checking 
							if(read_in == -1){
								src = open("/dev/null", O_RDONLY);	
								dup2(src,0);
							}
						}
						else{
							set_ignore_default(&SIGINT_action, &SIGTSTP_action,1 , 0);
						}
					}
					else{
						set_ignore_default(&SIGINT_action, &SIGTSTP_action, 1 , 0);
					}

					execute_bash(arguments);

				default:
					if(background_index != -1 && mode == false){
						fflush(stdout);
						printf("background PID is %d\n", spawnpid);
						//waitpid(spawnpid, &child_exit_method, WNOHANG);
						processPID[num_process]=spawnpid;
						num_process = num_process + 1; 
					}

					else{
						child_exit_method = -5;
						waitpid(spawnpid, &child_exit_method, 0);
						exit_buffer = child_exit_method;	
						waitpid(spawnpid, &child_exit_method, 0);
						if(mode && !prev){
							printf("Entering foreground-only mode (& is now ignored)\n");
						}
						else if(!mode && prev){
							printf("Exiting foreground-only mode\n");
						}		
						prev = mode;
						//we dont get the message if we CTRL-Z then throw something in the background we wont hear the message 
					}
					break;
			}
		}
		memset(input, '\0', sizeof(input));	
	}
}
