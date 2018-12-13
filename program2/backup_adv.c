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

struct room{
	char name[32];
	int numConnections;
	char* connections[6];
	char type[32];
};

void directory_name(char newestDirName[]){
	int newestDirTime = -1;
	char* targetDirPrefix = "ebrahimk.rooms.";//41215 
	//char newestDirName[256];
	memset(newestDirName, '\0', sizeof(newestDirName));
	DIR* dirToCheck;
	struct dirent *fileInDir;
	struct stat dirAttributes;

	dirToCheck = opendir(".");

	if (dirToCheck > 0) // Make sure the current directory could be opened
	{
		while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
		{
			if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
			{
				//	printf("Found the prefex: %s\n", fileInDir->d_name);
				stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

				if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
				{
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(newestDirName));
					strcpy(newestDirName, fileInDir->d_name);
					//printf("Newer subdir: %s, new time: %d\n",fileInDir->d_name, newestDirTime);
				}
			}
		}
	}

	closedir(dirToCheck); // Close the directory we opened

	//	printf("Newest entry found is: %s\n", newestDirName);
}

void printRooms (struct room* arr){
	int i,j;
	for (i = 0; i < 7; i++){
		printf("NAME: %s\n", arr[i].name);
		printf("TYPE: %s\n", arr[i].type);
		printf("NUMBER OF CONNECTIONS: %d\n", arr[i].numConnections);
		for(j =0; j<arr[i].numConnections; j++){
			printf("CONNECTION %d: %s\n",j, arr[i].connections[j]);
		} 
		printf("#####################################\n");
	}
}

bool check(char** names, struct dirent* fileInDir){
	int i;
	for(i = 0; i <10; i++){
		if(strcmp(names[i], fileInDir->d_name) == 0)
			return true;
	} 
	return false;
}

void parse_data(struct room* curRoom, char* buffer, int* j){
	char subbuff[32];
	memset(subbuff, '\0', sizeof(subbuff));
	int i;
	for(i = 0; i < strlen(buffer); i ++){
		if(buffer[i] == ':'){
			memcpy(subbuff, &buffer[i+2], strlen(buffer));
			//subbuff[strlen(subbuff)] = '\0';	
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
	//printf("This is cwd: %s\n", cwd); 

	if(dirToCheck > 0){ //file opened 
		while((fileInDir = readdir(dirToCheck)) != NULL){		//go through each file in thewest directory 
			if(check(names, fileInDir) == true){
				strcpy(placeholder, cwd);
				//printf("This is the pointer name: %s\n", fileInDir->d_name);
				sprintf(placeholder+strlen(placeholder), "/%s",fileInDir->d_name);
				//printf("filepath: %s\n", placeholder);
				fp = fopen(placeholder, "r");
				if(fp != NULL){ //file opened successfully
					//printf("opened: %s\n", fileInDir->d_name);
					while (fgets(buffer,sizeof(buffer)/sizeof(buffer[0]), fp) != NULL)
					{
						//!!!printf("%s\n", buffer); //parse in all fields to room data structs 
						parse_data(&room_array[i], buffer, jptr);
						//printf("Outside loop name: %s\n",room_array[i].name);
						//printf("The address of the fucking pointer: %d\n", &room_array[i].connections[0]);

					}
					if (feof(fp))
					{
						fclose(fp);
					}
				}
				j = 0; 
				//printf("The address of the fucking pointer: %d\n", &room_array[i].connections[0]);
				//printf("Outside loop name: %s\n",room_array[i].name);
				//printf("this is i: %d\n", i);
				i++;
			}
			//printf("This is a connections: %d\n", room_array[0].connections[0]);
			//printf("This is a connection 2: %d\n", room_array[0].connections[1]);
		}
	}
	closedir(dirToCheck);			
	//printf("This is a connections last: %s\n", room_array[1].connections[2]);

}

void read_time(){
	pthread_mutex_lock(&myMutex);
	char* file_name = "currentTime.txt", cwd[256], buffer[100];
	FILE* fd;	

	getcwd(cwd, sizeof(cwd));
	sprintf(cwd+strlen(cwd), "/%s",file_name);
	fd = fopen(cwd, "r"); 

	fgets(buffer,sizeof(buffer)/sizeof(buffer[0]), fd);
	printf("\n%s\n\n", buffer);
	pthread_mutex_unlock(&myMutex);
}



void* write_time(void *param){
	pthread_mutex_lock(&myMutex);
	char* file_name = "currentTime.txt",time_display[100], cwd[256];
	FILE* fd;

	struct tm* display;
	time_t time_data;
	time(&time_data);
	display = localtime(&time_data);

	getcwd(cwd, sizeof(cwd));
	//printf("CWD: %s\n", cwd);
	sprintf(cwd+strlen(cwd), "/%s",file_name);

	//open the file for writing, new entries will overwrite old ones
	fd = fopen(cwd, "w");
	strftime(time_display,100,"%I:%M%p, %A, %B %d, %Y", display);

	fprintf(fd,"%s", time_display);
	fclose(fd);
	pthread_mutex_unlock(&myMutex);
}



int verify(struct room* room_array, char* buffer, int* steps/*, pthread_t* threadID*/){
	int i, resultID; 

	for(i = 0; i < 7; i++){
		if(strcmp(buffer,room_array[i].name)==0){
			printf("\n");
			*steps = *steps +1;
			return i;
		}
		else if(strcmp(buffer,"time")==0){
			//blocks the main thread until the thread with threadID is complete
			int resultInt;
			pthread_t myThreadID;
			resultInt = pthread_create(&myThreadID, NULL, write_time, NULL);
			//verify that we did create the thread.
			assert(0 == resultInt);
			resultInt = pthread_join(myThreadID,NULL);//<-we want the actual thread structure 
			assert(0 == resultInt);

			read_time();
			//pthread_mutex_lock(myMutex);
			//HERE our main thread needs to try and grab the content of the time file,
			pthread_cancel(myThreadID);
			return -1;
		}
	}
	printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
	return -1;
}

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
		placeholder = verify(room_array, buffer, steps/*, threadID*/);		
		free(buffer);
	}
	while(placeholder == -1);	
	path[*steps-1]=room_array[location].name;  
	location = placeholder;
	return location; 
}



//verify that im not missing anythin here 
int get_starting_location(struct room* room_array){
	int i;
	char* start_room = "START_ROOM";
	for(i = 0; i < 7; i++){
		if(strcmp(room_array[i].type, start_room) == 0){
			return i; 
		}
	}	
}

int get_ending_location(struct room* room_array){
	int i;
	for(i = 0; i < 7; i++){

		if(strcmp(room_array[i].type, "END_ROOM") == 0){
			return i;
		}
	}
}

void play_game(struct room* room_array, int location, int end ){
	char* path[100];
	path[0] = room_array[location].name;

	/*
	////////////////////
	//prepare thread variable
	int resultInt;
	pthread_t myThreadID;
	resultInt = pthread_create(&myThreadID, NULL, write_time, NULL); 
	//verify that we did create the thread.
	assert(0 == resultInt);
	printf("thread has been created\n");
	//to kill the thread: pthread_exit()
	//////////////////////////
	*/

	int steps =0, i;
	while(location != end){
		location = interface(room_array, location, &steps, path/*, &myThreadID*/); 
	}
	path[steps] = room_array[location].name;

	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
	for(i =0;i < steps+1; i++){
		printf("%s\n", path[i]); 
	}
	//	pthread_cancel(myThreadID); //might need to be the address of myThreaID !!!!!!!!!!!!!!!!!1
}



void free_mem(struct room* room_array){
	int i, j;
	for(i = 0; i < 7; i++){
		for(j = 0; j < room_array[i].numConnections; j++){
			free(room_array[i].connections[j]);
		}
	}
}


int main(){

	char* names[10];// = declare_arr(names);
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

	//printf("names: &s\n", names[1]);
	char newestDirName[256]; 
	directory_name(newestDirName);
	//read all of the informaiton into structs
	//get into directory


	/*
	//prepare thread variable
	int resultInt;
	pthread_t myThreadID;
	resultID = pthread_create(&myThreadID, NULL, write_time, NULL); 
	//verify that we did create the thread.
	assert(0 == resultInt);
	//to kill the thread: pthread_exit()
	//lets initialize the second thread which will create and write to the time text file 
	*/


	//	printf("#################################\n");
	struct room room_array[7];
	read_in(newestDirName, room_array,names);
	//	printRooms(room_array);


	int start, end;
	start =get_starting_location(room_array);
	end = get_ending_location(room_array);
	//printf("This is starting location: %d\n", start); //room_array[start].name);
	play_game(room_array, start, end);
	free_mem(room_array);
	pthread_mutex_destroy(&myMutex);
	return 0;
}


