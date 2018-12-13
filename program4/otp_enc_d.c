#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*************************************************
 * Function:		error()
 * Preconditions:	An error occurs during execution.
 * Postconditions:	An error is printed and the process exits.
 * Description:		This function sends an error and exits.
 * Return:		N/A
 ***********************************************/
void error(const char *msg){ 
	perror(msg); 
	exit(1); 
} // Error function used for reporting issues

/*************************************************
 * Function:            read_socket()
 * Preconditions:       All input is valid.
 * Postconditions:      This function will send a packet of 200000 characters to the server, and ensure that the entire packet is sent. 
 * Description:         This function first allocates a character buffer of 200000 and fills it with null terminators. 
 *                      The function then calls recv() looking for 199999 characters (omit one for a null terminator) and places the recieved contents into
 *                      buffer[]. The function then enters a while loop if the number of chars read is less than 199999. This is done to ensure that the entire
 *                      packet is recieved. The extra recv() calls place the contents into to the buffer at an offset of the characters already read.
 *                      At the end of the function the contents of buffer are copied into "key".
 * Return:              N/A
 ***********************************************/
void read_socket(char* key, int establishedConnectionFD){
	int charsRead;
	char buffer[200000];
	memset(buffer, '\0', sizeof(buffer));
	memset(buffer, '\0', sizeof(key));
	charsRead = recv(establishedConnectionFD, buffer, 199999, 0); // Read the client's message from the socket
	//	printf("SERVER: This is charsRead: %d\n", charsRead);

	while(charsRead != 199999){
		charsRead += recv(establishedConnectionFD, buffer + charsRead, 199999, 0); // Read the client's message from the socket
		if (charsRead < 0) 
			error("ERROR reading from socket");

	}
	strcpy(key, buffer);
}

/*************************************************
 * Function:            send_socket()   
 * Preconditions:       Valid files are input by the user.
 * Postconditions:      This function will send the data in a 200000 character buffer.
 * Description:         Send_socket first creates a 200000 character then copies the message to be sent into the buffer. The buffer is then 
                        sent via the send() call. The function then enters a loop that will continue to iterate until the total number of characters 
                        sent is equal to the 199999. This ensures that the entire packet is sent by the client. 
 * Return:              N/A
 ***********************************************/
void send_socket(char*  key, int socketFD){
	char buffer[200000];
	int charsWritten, charsRead;
	int message_length = strlen(key);
	memset(buffer, '\0', sizeof(buffer));
	//     printf("SERVER: message: %s\n", key);
	strcpy(buffer, key);

	charsWritten= send(socketFD, buffer, 199999, 0); // Write to the server
	//      printf("SERVER: CharsWritten: %d\n", charsWritten);

	while(charsWritten != 199999){
		charsWritten += send(socketFD, buffer + charsWritten, 199999, 0); // Read the client's message from the socket
		if (charsWritten < 0) error("ERROR reading from socket");
	}
}

/*************************************************
 * Function:		encrypt()
 * Preconditions:	The deamon recieves valid input from the client.
 * Postconditions:	The function encrypts the plaintext into ciphertext.
 * Description:		This function takes the key and plaintext as parameters then begins a for loop with a number 
 *			of iterations equal to the length of plaintext. First the function checks to see if the current character of the key or plaintext is a space character.
 *			If so the function will convert it to equal to '91'. The capital letters range between 65 and 90, thus in this OTP implementation I treat the character 91 (27) as the 
 *			space character. The process of converting space characters to the value of '91' is implemented to both the key and the plaintext. The ciphertext is then produced by  
 *			subtracting 130 from the the sum of the key[i] + plaintext[i] and the modulo 27 is taken of the sum, then the 65 offset is added back onto the number. 
 *			Finally the number variable is placed into a buffer, this character is the ciphertext character. The buffer is copied to the plaintext vairabe (it is now ciphertext)
 *			and the memory of the buffer is freed.
 * Return:		N/A
 ***********************************************/
void encrypt(char* key, char* plaintext){
        //algorithm --> loop through each array until 
        int i, num;
	//printf("This is length %d and plain: %s\n",strlen(plaintext), plaintext);
        char* buffer = malloc(strlen(plaintext)*sizeof(char));
        memset(buffer, '\0', sizeof(buffer));
        for(i = 0; i < strlen(plaintext); i++){
         //       printf("%d: %d\n",i, plaintext[i]);
		if(plaintext[i] == 32){
                        plaintext[i] = 91;
                }
                if(key[i] == 32){
                        key[i] = 91;
                }
                
		num = (((plaintext[i] + key[i])-130)%27) + 65;
		
		if(key[i] == 91)
			key[i] = 32;                


		if(num == 91)
                       num = 32;       
                
		buffer[i] = num; 
        }
        strcpy(plaintext, buffer);
        free(buffer);
}

/*************************************************
 * Function:		split()
 * Preconditions:	The server recieves a packet from the client.
 * Postconditions:	The buffer is split at the "@" character and the key and plaintext c-string.
 * Description:		This function splits the recieved buffer into the key and the plaintext. By identifying the "@" character, and string copying the contents into the 
 * 			plaintext and key. 
 * Return:		N/A
 ***********************************************/
void split(char* longStr, char* key, char* plaintext){
	int i;
	for(i = 0; i < strlen(longStr); i++){
		if(longStr[i] == '@'){
			strncpy(key, longStr, i);
			strcpy(plaintext, longStr+i+1); 	
		}		
	}
}

/*************************************************
 * Structure:		pidArray{}	
 * Members:		An array of process id's, an array containing the child process communication socket pointers, an integer containing the number of processes.
 ***********************************************/
struct pidArray{
	pid_t pidArray[5];
	int fd[5];
	int num;
};

/*************************************************
 * Function:		reap_zombies()
 * Preconditions:	The main() function iterates.
 * Postconditions:	This function will clean up the resources of ended child processes.
 * Description:		This function iterates through the pidArray structure looking executing the waitpid() function with the WNOHANG flag. 
 * 			If the process is completed the parent process will clean the processes resources and readjust the pid_t array. 
 * Return:		N/A
 ***********************************************/
void reap_zombies(struct pidArray arr){
        if(arr.num > 0){
                int i, j;// exit_status = -1;
                int child_exit_method =-5;
                for(i = 0; i < arr.num; i++){
                        if(waitpid(arr.pidArray[i], &child_exit_method, WNOHANG) != 0){
                                /*if(WIFEXITED(child_exit_method) != 0)
                                        exit_status = WEXITSTATUS(child_exit_method);
                                if(WIFSIGNALED(child_exit_method) != 0)
                                        exit_status = WTERMSIG(child_exit_method);
                                fflush(stdout);
                                printf("background pid %d is done: terminated by signal %d\n",process[i],exit_status );
                                *num = *num -1;*/
				close(arr.fd[i]);
                                for(j = i; j < arr.num; j++){
                                        arr.pidArray[j] = arr.pidArray[j+1];
					arr.fd[j] = arr.fd[j+1];
                                }
                        }
                }
        }
}

/*************************************************
 * Function:		send_ack() 
 * Preconditions:	The profram is started
 * Postconditions:	This function will send a one character identifier to the client to identify whether or not it is otp_enc_d or otp_dec_d.
 * Description:		This function send a one character identifier via the send() call.
 * Return:		N/A
 ***********************************************/
void send_ack(int fd){
        char* id = "a";
        int charsWritten;
        charsWritten = send(fd, id, 1, 0);
}

/*************************************************
 * Function:		main() 
 * Preconditions:	The user executes the program process. 
 * Postconditions:	This function will execute in the background as a deamon and handle up to five concurrent processes.
 * Description:		This function first establishes a communication socket then enters while loop which will execute indefinitley. 
 * 			Upon every connection request the function will accept() the connection and fork into a child process. In the parent code the function
 * 			will add a new addition to the pidArray{} structure so as to have a mena of cleaning the child processes without having to block execution.
 * 			In the child process the function sends its identifier via send_ack(), then recieves data via the socket with the read_socket call. Up on recievind a 200000 
 * 			character buffer completely the function will call split, to pull out the plaintext/cipher text message and the key from the recieved packet. 
 * 			The function will then call encrypt to perform the one time pad. Finally the function calls send_socket to spit the ciphertet back to the client.
 * Return:		N/A
 ***********************************************/
int main(int argc, char *argv[])
{
	//	printf("SERVER: pid %d\n", getpid());
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;
	struct pidArray connections;
	connections.num = 0;


	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) 
		error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	while(1){
		// Accept a connection, blocking if one is not available until one connects
		//check for completed processes and clean them up if they are done.
		
		reap_zombies(connections);
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept

		if (establishedConnectionFD < 0) 
			fprintf(stderr,"SERVER: ERROR on accept\n");

		//HERE we will call our fork function

		int spawnpid = fork();
		switch(spawnpid){
			case -1:
				perror("child process failed to spawn!\n");
				exit(1);
				break;

			case 0:
				printf("\n");
				//				printf("SERVER: In child\n");
				send_ack(establishedConnectionFD);

				char* key = malloc(100000*sizeof(char));
				char* plaintext = malloc(100000*sizeof(char));	
				memset(key, '\0', sizeof(key));
				memset(plaintext, '\0', sizeof(plaintext));

				char buffer[200000];
				memset(buffer, '\0', sizeof(buffer));

				read_socket(buffer, establishedConnectionFD);	

				//We need to split the strings here
				split(buffer, key, plaintext);				
				//				printf("SERVER: key %s\n", key);
				//				printf("SERVER: plaintext %s\n", plaintext);				

				encrypt(key, plaintext);
				//				printf("SERVER: ciphertext: %s\n", plaintext);

				//send back the encrypted data 

				send_socket(plaintext, establishedConnectionFD);
				free(plaintext);
				free(key);
				exit(0);


			default:
				//printf("This is the spawnpid: %d\n", spawnpid);
				connections.pidArray[connections.num] = spawnpid;
				connections.fd[connections.num] = establishedConnectionFD;
				connections.num++;
		}

	}
	//close(establishedConnectionFD); // Close the existing socket which is connected to the client
	close(listenSocketFD); // Close the listening socket
	return 0;
}
