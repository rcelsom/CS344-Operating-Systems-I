#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/*************************************************
 * Function:            read_file()
 * Preconditions:       The file must exist
 * Postconditions:      The function will generate a dynamically allocated c-string containing the file contents.
 * Description:         This function takes the name of the file to be opened as a parameter, then opens the file with 
 *                      the open() function. Fseek(SEEK_END) sets the file pointer to the end of a file.
 *                      The ftell() function returns the file pointers position in a given stream, thus by calling ftell() directly
 *                      after calling fseek(SEEK_END), we get a long int containing the total number of characters in the file.
 *                      Memory is then dynamically allocated for a c-string plus one for the null terminator. Using fread, we read 
 *                      in the contents of the file equal to the the file_size and return the c-string. 
 * Return:              char*
 ***********************************************/
char* read_file(char* file_name){
	FILE* fp = fopen(file_name, "r");
	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);  //same as rewind(f);
	char *file_contents = malloc(file_size+1);// add plus one if want a null terminator
	memset(file_contents, '\0', sizeof(file_contents));
	fread(file_contents, file_size, 1, fp);
	fclose(fp);
	return file_contents;
}

/*************************************************
 * Function:            check_error()
 * Preconditions:       Two c-string containing the file contents have been created.
 * Postconditions:      The client terminates and frees the allocated memory or keeps running.
 * Description:         This function checks to see if the file contents have bad characters. The c-strings containing the 
 *                      the file contents are iterated through and an if statement verifies that all characters in the string are 
 *                      capital letters of the space character. If a bad characater is found the function frees the c-string memory,
 *                      prints an error to stderr and exits the function.
 * Return:              N/A 
 ***********************************************/
void check_error(char* contents, char* plaintext, char* key){
	int i;
	//printf("Length: %d\n", strlen(contents));
	for(i = 0; i < strlen(contents)-1; i++){
		if(contents[i] > 91 || contents[i] < 64 && contents[i] != ' '){
			//we have a bad input  file character	
			printf("%d\n", i);
			fprintf(stderr,"Bad characters present in file!\n");
			free(plaintext);
			free(key);
			exit(1);
		}	
	}
	return;	
}

 /*************************************************
 * Function:            check_length()
 * Preconditions:       Two c-strings contianing the file plaintext and the key are created.    
 * Postconditions:      This function will either determine that the file can be read or not 
 * Description:         This function checks to see if the length of the key is greater than the length of the plaintext.
 *                      If this is not the case a OTP cannot be performed and the function exits.
 * Return:              N/A
 ***********************************************/
void check_length( char* plaintext, char* key){
	if(strlen(key) < strlen(plaintext)){
		fprintf(stderr,"Key file is shorter than plaintext file!\n");
		free(plaintext);
		free(key);
		exit(1);
	}
}

 /*************************************************
 * Function:            check_args()
 * Preconditions:       The user calls the function.
 * Postconditions:      This function determines that the proper number of arguments was passed.
 * Description:         This function looks at argc and checks that is it greater than 4.
 * Return:              N/A
 ***********************************************/
void check_args(int args){
 if (args < 4) {
                fprintf(stderr, "Improper number of arguments!\n");
                exit(1);
        }
}

 /*************************************************
 * Function:            error()
 * Preconditions:       An erroroneous input was made, or the network protocol failed.
 * Postconditions:      An error messge is printed and the client terminates
 * Description:         This function prints an error then exits.
 * Return:              N/A
 ***********************************************/
void error(const char *msg) { 
	perror(msg); 
	printf("%s\n\n\n", msg);
	exit(0); 
}

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
        //printf("CLIENT: This is charsRead: %d\n", charsRead);

        while(charsRead != 199999){
                charsRead += recv(establishedConnectionFD, buffer + charsRead, 199999, 0); // Read the client's message from the socket
                if (charsRead < 0)
                        error("ERROR reading from socket");

         //       printf("CLIENT: reading\n");
        }

       // printf("CLIENT: this is the recieve buffer: %s\n", buffer);
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
	//printf("CLIENT: message: %s\n", key);
	strcpy(buffer, key);	
	
	charsWritten= send(socketFD, buffer, 199999, 0); // Write to the server
	//printf("CLIENT: CharsWritten: %d\n", charsWritten);
	
	while(charsWritten != 199999){ 
		charsWritten += send(socketFD, buffer + charsWritten, 199999, 0); // Read the client's message from the socket
		if (charsWritten < 0) error("ERROR reading from socket");			
	}
}

 /*************************************************
 * Function:            send_ack()
 * Preconditions:       valid readable files were passed as arguments.
 * Postconditions:      This function will verify that it is communicating with the proper deamon.
 * Description:         This function simply makes a recv() call and looks for the character recieved to be the letter "b".
 *                      If the letter is "b" then the client knows that it is communicating with the wrong deamon and will exit.  
 * Return:              N/A
 ***********************************************/
void send_ack(int socketFD, char* plaintext, char* key){
	int charsRead;
	char buffer[1];
	char* id = "b";
	memset(buffer, '\0', sizeof(buffer));

	charsRead = recv(socketFD, buffer, 1, 0);
	if(buffer[0] == 'a'){
		free(plaintext);
		free(key);
		fprintf(stderr,"Invalid connection, cannot connect!\n");
		exit(2);
	}
}

/*************************************************
 * Function:            main()
 * Preconditions:       User executes the program.
 * Postconditions:      The user passes in a valid amount of arguments.
 * Description:         The main function first reads the contents of the files passed as arguments, then establishes a socket to communicate with the deamon.
 *                      The main function erforms error checking of he passed files using the above described functions, if the files are valid. Main() then recieves 
 *                      the id character and checks that it is communicating with the proper deamon.
 *                      Main() will concatenate the key and the plaintext with a "@" character as an identifier for seperation. Main then sends the c-string via the send_socket() 
 *                      function. After sending the message the client then calls read_socket(), to recieve the cipher text back from the deamon.
 *                      Finally the client prints the ciphertext to the stdout, so it can be written to files.
 * Return:              N/A
 ***********************************************/
int main(int argc, char *argv[]){

	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;

	check_args(argc);

	char* plaintext = read_file(argv[1]);
	//printf("%s\n", plaintext);

	char* key = read_file(argv[2]);
	//printf("%s\n", key);

	check_length(plaintext, key);

	check_error(key, plaintext, key);
	check_error(plaintext, plaintext, key);

	key[strcspn(key, "\n")] = '\0';
	plaintext[strcspn(plaintext, "\n")] = '\0';

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL){ 
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
		exit(0);
	}

	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) 
		error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	send_ack(socketFD, plaintext, key); 

	//send plaintext and a key.
	char buffer[200000];
	memset(buffer, '\0', sizeof(buffer));


	//printf("CLIENT: key: %s\n", key);
	//printf("CLIENT: plaintext: %s\n", plaintext);

	strcpy(buffer, key);
	strcpy(buffer + strlen(key),"@");
	strcpy(buffer + strlen(key)+1,plaintext);


	send_socket(buffer, socketFD);
	//printf("CLIENT: Done sending the key and plaintext\n\n\n");



	//recieve ciphertext
	//printf("CLIENT: reading back the ciphertext\n");
	memset(plaintext, '\0', sizeof(plaintext));
	read_socket(plaintext, socketFD);
	printf("%s\n", plaintext);

	close(socketFD); // Close the socket
	//free(buffer);
	free(plaintext);
	free(key);
	return 0;
}

