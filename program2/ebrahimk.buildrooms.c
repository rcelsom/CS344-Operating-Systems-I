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

//Function Prototypes
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

/***************************************************
 *Names:		printRooms()
 *Preconditions:	Function called with room struct array passed as an argument.
 *Postconditions:	The names, type and number of connects for each room is printed to stdin.
 *Description:		This function prints the name, type and number of connections for each 
 *			room in a given array of room structs.
 *Return:		N/A 
 **************************************************/
void printRooms (struct room* arr){
	int i;
	for (i = 0; i < 7; i++){
		printf("%s\n", arr[i].name);
		printf("%s\n", arr[i].type);
		printf("%d\n", arr[i].numConnections);
		printf("\n");
	}
}

/***************************************************
 *Names:		create_direct()
 *Preconditions:	The function is called with a character array passed as an argument.
 *Postconditions:	A new directory is generated with the name of the passed c-string.
 *Description:		This function creates a new directory with the same name as the passed string argument,
 *			and prints an error if it is unable to create the directory.
 *Return:		N/A
 **************************************************/
void create_direct(char* name){
	int check;
	check = mkdir(name, 0777);
	if(check == -1){
		fprintf( stderr, "failed to create a new rooms directory");
		exit(1);
	}
}

/***************************************************
 *Names:		randomize_array()
 *Preconditions:	The function is called with a valid array of c-strings passed as an argument.
 *Postconditions:	The function will randomize the order of the c-string array.
 *Description:		This function is used to generate new random name files from the ten hardcoded
 * 			names created in main(). The algorithm simply swaps name values for each index value. 
 *Return:		N/A 
 **************************************************/
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

/***************************************************
 *Names:		generate_files()
 *Preconditions:	The function is called, an array of room structs, the name of the directory to be accessed 
 *			and the pointer to the directory are passed as arguments.
 *Postconditions:	Files are generated within the passed directory. To these files are written the name of the room they represent
 *			the room type, the name of all of the rooms connections in a specified format.
 *Description:		The directory is first opened, then the current working directory is grabbed and placed within "cwd". Using the sprintf 
 *			command, the name of the directory is appended to "cwd" and the result is copied to "placeholder". A loop is then executed
 *			with appends the name of the file to be created, a value accessed by looping throught the room struct array, and a file is generated 
 *			with the path using fopen. The "w" option open the file for writing. Using fprintf the formatted contents of values contained in 
 *			the room struct are printed to the file. The file is then closed. This processed is iterated seven times for seven room files.
 *			Exiting the loop the directory is closed and the function exits. 
 *Return:		N/A
 **************************************************/
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

/***************************************************
 *Names:		GenerateRooms()
 *Preconditions:	Function is called with a struct room array and an array of c-strings passed as arguments. 	
 *Postconditions:	This function fills the fields of all room structs in the passed array with random names and types.
 *Description:		The function first determines two integer values "start" and "end" which are the indeces of the start and end rooms.
 *			The while loop ensures that the start room index is not equal to the end room index. The function then loops through the room array struct 
 *			assigning names from the name array to each room as well as setting the type field and setting the number of connections equal to zero.
 *Return:		Pass by reference is used to permanently change the contents of the passed arguments.
 **************************************************/
void GenerateRooms(struct room* room_array, char **rand_names){
	srand(time(NULL));
	int i, j,start=0, end=0;
	while(start == end){
		start = rand()%7;
		end = rand()%7; 
	}
	for(i = 0; i < 7; i++){	
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

/***************************************************
 *Names:		IsGraphFull() 
 *Preconditions:	The function is called and an array of room structs is passed as an argument.
 *Postconditions:	This function will return true or false depending on whether or not all rooms have a number of connections greater than three. 
 *Description:		This functions loops through the room struct array and compares each rooms "numConnections" value with three, if one of the rooms 
 *			has a number of connection less than three the function returns false. 
 *Return:		boolean
 **************************************************/
bool IsGraphFull(struct room* room_array){
	int i; 
	for(i = 0; i <7; i++){
		if(room_array[i].numConnections < 3){
			return false;
		}
	}
	return true;
}

/***************************************************
 *Names:		AddRandomConnection() 
 *Preconditions:	The room graph is not completed. 
 *Postconditions:	A connection is generated between two, previously unconnected rooms. 
 *Description:		This function accepts the array of room structs as an argument, grabs two random rooms, verifies that a connection
 * 			can be made between the two, i.e. a connection does not already exist between the two room and the rooms are not the same.
 *Return:		N/A 
 **************************************************/
void AddRandomConnection(struct room* room_array)  {
	struct room *A; 
	struct room *B;  
	while(true)
	{
		A = GetRandomRoom(room_array); 
		int i;
		if (CanAddConnectionFrom(A) == true)
			break;
	}

	do
	{
		B = GetRandomRoom(room_array);
	}
	while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);
	ConnectRoom(A, B);
	ConnectRoom(B, A); 
}

/***************************************************
 *Names:		GetRandomRoom()
 *Preconditions:	AddRandomConnection is called.
 *Postconditions:	A room structure is returned randomly from the array of room structures. 
 *Description:		This function grabs a room structure at random from the room structure array.
 *Return:		A pointer to the room structure within the array. 
 **************************************************/
struct room* GetRandomRoom(struct room* room_array){
	int random_room = rand() % 7;	
	struct room* pointer;
	pointer = &room_array[random_room]; 
	return pointer;
}

/***************************************************
 *Names:		CanAddConnectionFrom()
 *Preconditions:	AddRandomConnection was called. 
 *Postconditions:	A boolean value is returned stating whether or not a connection from room A can be made with room B.
 *Description:		This function verifies that the randomly chosen room has less than 6 connections.
 *Return:		boolean
 **************************************************/
bool CanAddConnectionFrom(struct room*  x){
	if(x->numConnections < 6){
		return true;

	}
	else {
		return false;
	}
}

/***************************************************
 *Names:		ConnectionAlreadyExists() 
 *Preconditions:	AddRandomConnection is called. 
 *Postconditions:	This function returns a bool which indicates whether or not the room arguments already have a connection. 
 *Description:		This function loops through the connections array of room x, checking that the name of the connection does not 
 *			equal the name of room y, if the two equal each other then a connection already exists and the function returns true.
 *Return:		boolean
 **************************************************/
bool ConnectionAlreadyExists(struct room* x,struct room* y)
{
	int i; 
	for(i = 0; i< x->numConnections ;i++){
		if(strcmp(x->connections[i]->name,y->name) == 0)
			return true; 
	}
	return false; 
}

/***************************************************
 *Names:		ConnectRoom()
 *Preconditions:	AddRandomConnection is called. 
 *Postconditions:	This function fills in a connection index of a single room struct. 
 *Description:		This function fills in a connection for a single room by setting x's connection member at the number of x's connections 
 *                      equal to room y. 
 *Return:		N/A
 **************************************************/
void ConnectRoom(struct room* x,struct room* y) {
	x->connections[x->numConnections] = y; //fix this 
	x->numConnections++;
}

/***************************************************
 *Names:		IsSameRoom() 		
 *Preconditions:	AddRandomConnection is called.
 *Postconditions:	This function returns a bool indicating that the two room arguments are the same. 
 *Description:		This function compares the two name members of each struct and checks to see if they are equal.
 *Return:		Boolean 
 **************************************************/
bool IsSameRoom(struct room* x, struct room* y) 
{
	if(strcmp(x->name, y->name) == 0)
		return true;
	else 
		return false; 
}

/***************************************************
 *Names:		Main()
 *Preconditions:	ebrahimk.buidlrooms is executed from the command line.
 *Postconditions:	A directory is generated containing a list of room files containing the data of each room. 
 *Description:		This function calls all other previously declared functions which each perform the task outlined in the assignment. 
 *Return:		0, upon successful completion. 
 **************************************************/
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
	char dir_name[30] = "ebrahimk.rooms.";
	int pid = getpid(); 
	sprintf(dir_name+ strlen(dir_name),"%d", pid);
	create_direct(dir_name);	
	DIR* direct_ptr;
	generate_files(room_array,dir_name, direct_ptr);
	return 0;
}










