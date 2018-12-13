#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <unistd.h>

struct room{
        char* name ;
        int numConnections;
        struct room* connections[6];
        char* type;
};

void printRooms (struct room* arr);
void printArray (char* arr[], int n);
void randomize_array(char **array, int n);
void GenerateRooms(struct room* room_array, char **rand_names);
bool IsGraphFull(struct room* room_array);
void AddRandomConnection(struct room* room_array);
struct room *GetRandomRoom(struct room* room_array);
bool CanAddConnectionFrom(struct room* x);
bool ConnectionAlreadyExists(struct room* x,struct room* y);
void create_direct(char* name);
void generate_files(struct room* room_array, char* direct_name, DIR* direct_ptr);

void ConnectRoom(struct room* x,struct room* y);
bool IsSameRoom(struct room* x, struct room* y);


void printRooms (struct room* arr){
	int i;
	for (i = 0; i < 7; i++){
		printf("%s\n", arr[i].name);
		printf("%s\n", arr[i].type);
		printf("%d\n", arr[i].numConnections);
		printf("\n");
	}
}

void create_direct(char* name){
	int check;
	check = mkdir(name, 0777);
	if(check == -1){
		fprintf( stderr, "failed to create a new rooms directory");
		exit(1);
	}
}

void printArray (char* arr[], int n)
{
	int i;
	for (i = 0; i < n; i++)
		printf("%s\n", arr[i]);
}

void randomize_array(char **array, int n){
	srand(time(NULL));
	int i;
	char *temp; 
	for(i = n-1; i > 0; i--){
		int j = rand() % (i+1);
		temp = array[i];
		array[i] = array[j];
		array[j] = temp;
	}
}


void generate_files(struct room* room_array,char* dir_name,DIR* direct_ptr){
	direct_ptr = opendir(dir_name);
	int i;
	
	char cwd[256], placeholder[256];
        getcwd(cwd, sizeof(cwd));
        sprintf(cwd+strlen(cwd), "/%s",dir_name);
	
	strcpy(placeholder, cwd);
	int j;
	FILE* fd;
	for(i =0; i < 7; i++){
		strcpy(placeholder, cwd);
		sprintf(placeholder+strlen(placeholder), "/%s",room_array[i].name);
		fd = fopen(placeholder, "w+"); 
			
			fprintf(fd, "ROOM NAME: %s\n",room_array[i].name);
			
			for(j = 0; j < room_array[i].numConnections; j++){
				fprintf(fd, "CONNECTION %d: %s\n",j+1,room_array[i].connections[j]->name);
			}

			fprintf(fd, "ROOM TYPE: %s\n",room_array[i].type);

		fclose(fd);
	}

}

//determine start and stop outside of the function call and pass them as parameters to the function
void GenerateRooms(struct room* room_array, char **rand_names){
	srand(time(NULL));
	int i, j,start=0, end=0;

	//make sure the starting room is not the same as the ending room. 
	while(start == end){
		start = rand()%7;
		end = rand()%7; 
	}

	//loop through initializing each room.
	for(i = 0; i < 7; i++){

		//	room_array[i]= struct room;
		//you need to make sure the start and end are in the seven rooms used for the game	
		room_array[i].name = rand_names[i];
		room_array[i].numConnections =0;
		
		if(i == start)
			room_array[i].type = "START_ROOM";
		else if(i == end)
			room_array[i].type = "END_ROOM";
		else
			room_array[i].type = "MID_ROOM";
	}
} 


// Returns true if all rooms have 3 to 6 outbound connections, false otherwise

//this function is always returning false
bool IsGraphFull(struct room* room_array){
	int i; 
	for(i = 0; i <7; i++){
		if(room_array[i].numConnections < 3){
			return false;
		}
	}
	return true;
}


// Adds a random, valid outbound connection from a Room to another Room
void AddRandomConnection(struct room* room_array)  
{
	struct room *A;  // Maybe a struct, maybe global arrays of ints
	struct room *B;  //maybe not a pointer, either a pointer to a room, or a room initialzed as a copy of another room
	while(true)
	{
		A = GetRandomRoom(room_array); //A now points to a random room 
		int i;
		
		if (CanAddConnectionFrom(A) == true)
			break;
	}

	do
	{
		B = GetRandomRoom(room_array);
            
	}
	while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);
	
	ConnectRoom(A, B);  // TODO: Add this connection to the real variables, 
	ConnectRoom(B, A);  //  because this A and B will be destroyed when this function terminates

}



// Returns a random Room, does NOT validate if connection can be added
struct room* GetRandomRoom(struct room* room_array){
	int random_room = rand() % 7;	//return number between 0 and 6
	struct room* pointer;
            //    printf("address of randomly chosen location: %d\n", &room_array[random_room]);
	pointer = &room_array[random_room]; //might need to dereference!!!!!!
	  //      printf("address of randomly chosen location: %d\n", pointer);
	return pointer;
}

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
bool CanAddConnectionFrom(struct room*  x){
	if(x->numConnections < 6){
		return true;
	
	}else {
		return false;
}
}


// Returns true if a connection from Room x to Room y already exists, false otherwise
bool ConnectionAlreadyExists(struct room* x,struct room* y)
{
	int i; 
	for(i = 0; i< x->numConnections ;i++){
		if(strcmp(x->connections[i]->name,y->name) == 0)	//connection already exists
			return true; 
	}
	return false; 
}

// Connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(struct room* x,struct room* y) {
	x->connections[x->numConnections] = y; //fix this 
	x->numConnections++;
}

// Returns true if Rooms x and y are the same Room, false otherwise
bool IsSameRoom(struct room* x, struct room* y) 
{
	if(strcmp(x->name, y->name) == 0)
		return true;
	else 
		return false; 
}


int main(){
	srand(time(NULL));

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

	char* types[3];
	types[0]="START_ROOM";
	types[1]="MID_ROOM";
	types[2]="END_ROOM";

	int n = sizeof(names)/sizeof(names[0]);
	randomize_array(names, n);

	struct room room_array[7];
	GenerateRooms(room_array, names);

	while (IsGraphFull(room_array) == false){
		AddRandomConnection(room_array);
	}
	
	//make the directory <OSU_ONID.rooms.PID> 
	char dir_name[30] = "ebrahimk.rooms.";
	int pid = getpid(); 
	sprintf(dir_name+ strlen(dir_name),"%d", pid);

	//now to make a directory 	
	create_direct(dir_name);	
	

/*	int i;
	for(i=0; i < 7; i++){
			int j;
				
			printf("Number of connections in room: %s\n", room_array[i].name);
			for(j = 0; j < room_array[i].numConnections; j++){
                                
				printf("CONNECTION %d: %s\n",j+1,room_array[i].connections[j]->name);
                        }
	}*/

        DIR* direct_ptr;
	generate_files(room_array,dir_name, direct_ptr);



}










