#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include<assert.h>

//Global Variables 
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

//Struct room 
struct room{
	char name[32];
	int numConnections;
	char* connections[6];
	char type[32];
};

/***************************************************
 *Names:		directory_name()                
 *Preconditions:      	ebrahimk.buildrooms is executed creating a file from which this function can read.
 *Postconditions:    	The most recent directory with the prefix, "ebrahimk.rooms" is opened and a DIR pointer is esablished to point to the 
 *			directory. 
 *Description:      	This function first declares the prefix of the directory to be opened. In the case of this instnace the prefix is "ebrahimk.rooms.". 
 *			The function first opens the current working directory. Then loops through each file within the directory. When their are no more files to be read 
 *			readdir will return NULL. The function then checks to see if the given file under inspection has an instance of the directory prefix in its name. 
 *			If so then the directory pointed to by fileInDir is one of the files created by running the buildrooms program. 
 *			The funciton then stats the directory and stores the results of the stat() operation in a stat structure. The function then compares the st_mtime 
 *			member of the stat structure, the time of last modification of the given file, with the newestDirTime variable. If the time is greater then 
 *			we have a found a directory with the proper prefix created more recently than any of the previously found directories. In this instance 
 *			newestDirTime is set to the value contained in st_mtime, newestDirName is filled with null terminators and the current directory name is copied into newestDirName.
 *Return:          	This function uses spass by referenece to permanently change the contents of newestDirName() so that it contains the name of the 
 *			most recently created directory with the prefix "ebbrahimk.rooms.".
 **************************************************/
void directory_name(char newestDirName[]){
	int newestDirTime = -1;
	char* targetDirPrefix = "ebrahimk.rooms.";//41215 
	memset(newestDirName, '\0', sizeof(newestDirName));
	DIR* dirToCheck;
	struct dirent *fileInDir;
	struct stat dirAttributes;

	dirToCheck = opendir(".");

	if (dirToCheck > 0) 
	{
		while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
		{
			if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
			{
				stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry
				if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
				{
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(newestDirName));
					strcpy(newestDirName, fileInDir->d_name);
				}
			}
		}
	}
	closedir(dirToCheck); // Close the directory we opened
}

/***************************************************
 *Names:		check()               
 *Preconditions:      	A DIR* pointer has been created and points to the most reently created directory with the prefix "ebrahimk.room".
 *Postconditions:    	This function will return true if a file with a name within the room directory exists
 *Description:      	This function iterates ten times comparing the names contained within the file with the names of the hardcoded name array.
 *Return:          	This function returns a boolean value. 
 **************************************************/
bool check(char** names, struct dirent* fileInDir){
	int i;
	for(i = 0; i <10; i++){
		if(strcmp(names[i], fileInDir->d_name) == 0)
			return true;
	} 
	return false;
}

/***************************************************
 *Names:               	parse_data()
 *Preconditions:       	A file with the prefix "ebrahimk.rooms" is present in the current working directory.	
 *Postconditions:      	This file will will read in the contents of the files created within the room directory and fill the members of a room struct
 *		       	array with the corresponding fields of contained in the files. 
 *Description:         	This function first declares a c-string buffer named subbuff. The fuction then loops a number of times equal to the length of 
 *			the buffer parameter. The buffer parameter is a string containining an individual line from one of the files within the new directory. 
 *			If the buffer c-string at one of its indecies is equal to ":" then we know that the succeeding data will contain the actual contents of the file 
 *			that we want to grab. When we hit the ":" character, the contents of the buffer from i+2 to the null terminator is copied to subbuff. 
 *			Subbuff thus contains the contents of the information we want to read into our structure array. 
 *			To determine which piece of data subbuff contains the strstr function is used. This function returns a pointer ot the first instance 
 *			of some substring contained in the buffer, if no instance of the substring occurs the function returns NULL. 
 *			Using a simple if statement we can check to see if buffer contains an instance of "NAME:", "TYPE:" or "CONNECTION". 
 *Return:          	This funciton uses pass by reference to permanently manipulate the contents of arguments. 
 **************************************************/
void parse_data(struct room* curRoom, char* buffer, int* j){
	char subbuff[32];
	memset(subbuff, '\0', sizeof(subbuff));
	int i;
	for(i = 0; i < strlen(buffer); i ++){
		if(buffer[i] == ':'){
			memcpy(subbuff, &buffer[i+2], strlen(buffer));
			if(strstr(buffer, "NAME:") != NULL){	 
				strcpy(curRoom->name, subbuff);
				curRoom->name[strlen(curRoom->name)-1] = '\0';
			}
			else if(strstr(buffer, "TYPE:") != NULL){
				strcpy(curRoom->type,subbuff);
				curRoom->type[strlen(curRoom->type)-1] = '\0';
			}
			else if(strstr(buffer, "CONNECTION")){
				char* newCpy=(char*) malloc((strlen(subbuff))*sizeof(char));
				memset(newCpy, '\0', sizeof(newCpy));
				strcpy(newCpy,subbuff);
				newCpy[strlen(newCpy)-1] = '\0'; 
				curRoom->connections[*j]=newCpy;
				*j = *j +1;
				curRoom->numConnections = *j;
			}
			break;
		}
	}
}

/***************************************************
 *Names:             	read_in() 
 *Preconditions:     	A directory has been opened and the pointer to this directory has been established. 
 *Postconditions:    	This function will read the contents of each file with in the directory with the prefix "ebrahimk.rooms". 
 *			The array of room structs will have each field completely filled.
 *Description:      	This function first opens the directory of the most recently created room directory using the opendir function.
 *			A file pointer and a character string buffer are declared, so the contents of the files have a bucket to be read into.
 *			Furthermore a placeholder array of character is a lso declared. The current working directory is read into "cwd", and the name of the 
 *			directory ot be opened is appended to the cwd string using sprintf. The function then loops through equal to the number of files present 
 *			within the directory. For each file we call the check() function to verify that the file has the name of a hardcoded room. 
 *			If the file does have the proper name then placeholder is filled with the same contents as cwd and placeholder has the name of the given file appened 
 *			to it. We then call fopen() with the "r" option to open the file for reading. Using the fgets() we read in the data contained in each file line by line. 
 *			Fgets() looks for newline characters and fills the buffer variable with each lines contents. Thes econtents are then passed to parse_data(). 
 *			If the feof(fp) is true, then the end of the file has been reached and the file is closed. This process is completed once for eahc file within the directory.  
 *Return:          	N/A 
 **************************************************/
void read_in(char * newDir, struct room* room_array, char** names){
	DIR* dirToCheck = opendir(newDir);
	struct dirent *fileInDir;
	FILE* fp; 
	char buffer[256];
	//memset(buffer, '\0', sizeof(buffer));
	int i =0, j =0;
	int* jptr = &j;
	char cwd[256], placeholder[256];
	getcwd(cwd, sizeof(cwd));
	sprintf(cwd+strlen(cwd), "/%s",newDir);	
	if(dirToCheck > 0){ //file opened 
		while((fileInDir = readdir(dirToCheck)) != NULL){
			if(check(names, fileInDir) == true){
				strcpy(placeholder, cwd);
				sprintf(placeholder+strlen(placeholder), "/%s",fileInDir->d_name);
				fp = fopen(placeholder, "r");
				if(fp != NULL){ //file opened successfully
					while (fgets(buffer,sizeof(buffer)/sizeof(buffer[0]), fp) != NULL)
					{
						parse_data(&room_array[i], buffer, jptr);
					}
					if (feof(fp))
					{
						fclose(fp);
					}
				}
				j = 0; 
				i++;
			}
		}
	}
	closedir(dirToCheck);			
}

/***************************************************
 *Names:  		read_time()             
 *Preconditions:      	The user enters "time" at the prompt
 *Postconditions:    	This function reads a line from a file named "currentTime.txt".
 *Description:      	This function opens a file and uses fgets() to read in a line of the file. 
 *			The current working directory is first grabbed using the getcwd() function. 
 *			The string contained in file_name is then appended to the current working directory and a file with the
 *			particular pather is opened for reading. Fgets() readsi none line of the file into a character buffer. 
 *Return:         	N/A
 **************************************************/
void read_time(){
	char* file_name = "currentTime.txt", cwd[256], buffer[100];
	FILE* fd;	
	getcwd(cwd, sizeof(cwd));
	sprintf(cwd+strlen(cwd), "/%s",file_name);
	fd = fopen(cwd, "r"); 
	fgets(buffer,sizeof(buffer)/sizeof(buffer[0]), fd);
	printf("\n%s\n\n", buffer);
}

/***************************************************
 *Names: 		write_time()              
 *Preconditions:      	A thread is created and calls this function.
 *Postconditions:    	A file named "currentTime.txt" is created and the time, in a specific formatted manner, is printed to it. 
 *Description:      	This function initializes a pointer to a tm struct and fills the values of the struct with informaiton ertaining to the current time.
 *			This function then grabs the current working directory using the getcwd function and appends the name of the file to be written to/created to 
 *			file path contained in the cwd variable. Using the strftime() function the current time is printed out in a specific format.
 *			Finally the file is closed and mutex is unlocked.	
 *Return:          	N/A 
 **************************************************/
void* write_time(void *param){
	
	pthread_mutex_lock(&myMutex);
	
	char* file_name = "currentTime.txt",time_display[100], cwd[256];
	FILE* fd;
	struct tm* display;
	time_t time_data;
	time(&time_data);
	display = localtime(&time_data);
	getcwd(cwd, sizeof(cwd));
	sprintf(cwd+strlen(cwd), "/%s",file_name);
	fd = fopen(cwd, "w");
	strftime(time_display,100,"%I:%M%p, %A, %B %d, %Y", display);
	fprintf(fd,"%s", time_display);
	fclose(fd);
	
	pthread_mutex_unlock(&myMutex);

}

/***************************************************
 *Names:               	verify()
 *Preconditions:      	The function Interface has been called. 
 *Postconditions:    	This function will sort user input to determine if the input is valid.
 *Description:      	The buffer argument ot the function contains the users input from the prompt. 
 *			This function loops through each connection element of the players current location checking to see if
 *			the user input a valid destination room, i.e. buffer must equal to one of the current room locations conneciton
 *			elements. This function also checks to see if the user input a "time" as an option. If so the function locks 
 *		 	and creates a new thread which creates the curretnTime.txt file, then the funciton calls read_time(). 
 *		 	If the user neither entered time or a valid name of a room conneciton the function returns -1.
 *Return:          	integer of the index at which the users destination rooms is located or -1.
 **************************************************/
int verify(struct room* room_array, char* buffer, int* steps, int location/*, pthread_t* threadID*/){
	int i, resultID; 
	for(i = 0; i < room_array[location].numConnections; i++){
		if(strcmp(buffer,room_array[location].connections[i])==0){
			printf("\n");
			*steps = *steps +1;
			return i;
		}
		else if(strcmp(buffer,"time")==0){
			int resultInt;
			pthread_t myThreadID;
			pthread_mutex_lock(&myMutex);
			resultInt = pthread_create(&myThreadID, NULL, write_time, NULL);
			assert(0 == resultInt);
			pthread_mutex_unlock(&myMutex);
			resultInt = pthread_join(myThreadID,NULL);//<-we want the actual thread structure 
			assert(0 == resultInt);
			read_time();	
			return -1;
		}
	}
	printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
	return -1;
}

/***************************************************
 *Names:            	interface()
 *Preconditions:      	The function play_game() is called. 
 *Postconditions:    	This function waits until the user enters a valid destination room string, up which is exits after updating the users location.
 *Description:      	This function firsts prints the user interface as specified by the assignment by printing the members of room structures at the "location" index. 
 *			The function then uses getline() to grab user input from stdin. The verify() function is then called and verify's return value is assigned to 
 *			"placeholder". This entire operation exists within a do while loop so execution will occur at least once. If placeholder is equal to -1 
 *			i.e. the user input "time" or invalid input then the loop executes again. Placeholder then contains the index of the current
 *			room locaiton connections array at which the user traveled. To set the "locaiton" variable equal to the destination rooms index within the room
 *			array a for loop is entered which finds where the destination rooms index is and sets location equal to that index. 
 *Return:          	The funciton returns the index of the room struct array at whic hthe user is located. 
 **************************************************/
int interface(struct room* room_array, int location, int* steps, char** path/*, pthread_t* threadID*/){
	int i;
	char* buffer;
	size_t bufsize = 32;
	int placeholder = 0;
	char** temp;

	do{
		buffer = (char *)malloc(bufsize * sizeof(char));
		if( buffer == NULL)
		{
			perror("Unable to allocate buffer");
			exit(1);
		}


		printf("CURRENT LOCATION: %s\n", room_array[location].name);
		printf("POSSIBLE CONNECTIONS: ");
		for(i = 0; i < room_array[location].numConnections; i++){
			printf("%s",room_array[location].connections[i]); 	
			if(i == room_array[location].numConnections-1){
				printf(".\n");
			}
			else	 
				printf(", ");
		}	
		printf("WHERE TO? >");
		getline(&buffer,&bufsize,stdin);
		buffer[strlen(buffer)-1] = '\0';
		placeholder = verify(room_array, buffer, steps, location/*, threadID*/);		
		free(buffer);
	}
	while(placeholder == -1);	
	path[*steps-1]=room_array[location].name;  
	//exiting verify placeholder contains the index at room_array[locations].connection[] that contains the destination room name// really bad code 
	for(i = 0; i < 7; i++){
		if(strcmp(room_array[i].name,room_array[location].connections[placeholder])==0)
			return i; 
	}
}

/***************************************************
 *Names:             	get_starting_location()  
 *Preconditions:      	The function is called.
 *Postconditions:    	The index of the starting room is found and returned.
 *Description:      	This function loops through the type member of each room in the room struct array and compares the string with "START_ROOM".
 *			If a room is found that matches this type, the index of that room is returned. 
 *Return:          	Integer of the index of the start room. 
 **************************************************/
int get_starting_location(struct room* room_array){
	int i;
	char* start_room = "START_ROOM";
	for(i = 0; i < 7; i++){
		if(strcmp(room_array[i].type, start_room) == 0){
			return i; 
		}
	}	
}

/***************************************************
 *Names:             	get_ending_location() 
 *Preconditions:      	The function is called.
 *Postconditions:    	The index of the end room is found and returned.
 *Description:      	This function loops through the type member of each room in the room struct array and compares the string with "END_ROOM".
 *                      If a room is found that matches this type, the index of that room is returned.
 *Return:          	Integer of the index of the end room. 
 **************************************************/
int get_ending_location(struct room* room_array){
	int i;
	for(i = 0; i < 7; i++){

		if(strcmp(room_array[i].type, "END_ROOM") == 0){
			return i;
		}
	}
}

/***************************************************
 *Names:    		play_game()           
 *Preconditions:      	The function is called, and the start and end room indeces have been located.
 *Postconditions:    	The function will execute the entire flow of the game.
 *Description:      	This function first creates a path variable which tracks the users progress throughout the game. The function then enters a while loop 
 *			which iterates until the location variable of the user is equal to the index of the end room. After completing the game the function prints
 *			the number of steps the player took and the room order the player traversed.
 *Return:          	N/A
 **************************************************/
void play_game(struct room* room_array, int location, int end ){
	char* path[100];
	path[0] = room_array[location].name;
	int steps =0, i;
	while(location != end){
		location = interface(room_array, location, &steps, path/*, &myThreadID*/); 
	}
	path[steps] = room_array[location].name;
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
	for(i =0;i < steps+1; i++){
		printf("%s\n", path[i]); 
	}
}

/***************************************************
 *Names:              	free_mem() 
 *Preconditions:      	The room struct array's connection members have been dynamically allocated. 
 *Postconditions:    	The memory contained by the connection members has been freed.
 *Description:      	This function loops through for each room within the room struct array. A nested loop iterates for each individual room structure 
 *			freeing the memory allocated for that room structures connection member. 
 *Return:          	N/A
 **************************************************/
void free_mem(struct room* room_array){
	int i, j;
	for(i = 0; i < 7; i++){
		for(j = 0; j < room_array[i].numConnections; j++){
			free(room_array[i].connections[j]);
		}
	}
}

/***************************************************
 *Names:             	main()
 *Preconditions:  	ebrahimk.adventure has been called from the commandline 
 *Postconditions:    	The game is executed and the player has won.
 *Description:      	This function calls the previously described functions to execute one cycel of the game.
 *Return:          	zero upon successful completion. 
 **************************************************/
int main(){
	char* names[10];
	names[0]="Barracks";
	names[1]="Dungeon";
	names[2]="Throne";
	names[3]="Armory";
	names[4]="Hall";
	names[5]="Chapel";
	names[6]="Kitchen";
	names[7]="Lavatory";
	names[8]="Cellar";
	names[9]="Solar";
	char newestDirName[256]; 
	directory_name(newestDirName);
	struct room room_array[7];
	read_in(newestDirName, room_array,names);
	int start, end;
	start =get_starting_location(room_array);
	end = get_ending_location(room_array);
	play_game(room_array, start, end);
	free_mem(room_array);
	pthread_mutex_destroy(&myMutex);
	return 0;
}


