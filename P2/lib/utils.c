#include "utils.h"

// Helper functions, open and close the queue
int openQueue() {
	char cwd [50];
	getcwd(cwd, 50);
	return msgget(ftok(cwd, 4061), 0666 | IPC_CREAT);
}
int closeQueue(int id) {
	return msgctl(id, IPC_RMID, NULL);
}

// Creates a message
struct msgBuffer makeMessage() {
	struct msgBuffer temp;
	memset(temp.msgText, '\0', MSGSIZE);
	temp.msgType = 0;
	return temp;
}

// Receives chunks from sendChunkData()
char *getChunkData(int mapperID) {
	//Message
	struct msgBuffer message = makeMessage();
	//Queue ID
	int mid = openQueue();
	msgrcv(mid, &message, MSGSIZE, mapperID, 0);
	if (strncmp("END", message.msgText, 3) == 0)
		return NULL;
	char* value = malloc(1024);
	strcpy(value, message.msgText);
	return value;
}

// Sends chunks of size 1024 to the mappers in RR fashion
void sendChunkData(char *inputFile, int nMappers) {
	struct msgBuffer message = makeMessage();
	// open message queue
	int msgid = openQueue();
	closeQueue(msgid);
	msgid = openQueue();
	int map = 0;
	FILE* file = fopen(inputFile, "r");
	// construct chunks of 1024 bytes
	while(fgets(message.msgText, chunkSize + 1, file) != NULL) {

		int i = 1023;
		while(validChar(message.msgText[i])) {
			message.msgText[i--] = '\0';
		}
		if (fseek(file, (i - 1023), SEEK_CUR) == -1)
			break;
		message.msgType = (map++ % nMappers) + 1;
	
		if (msgsnd(msgid, &message, MSGSIZE, 0) == -1)
			break;
	}
	for (int i = 1; i <= nMappers; i++) {
		struct msgBuffer END = {i, "END"};
		if (msgsnd(msgid, &END, MSGSIZE, 0) == -1)
			break;
	}
	fclose(file);
}

// hash function to divide the list of word.txt files across reducers
//http://www.cse.yorku.ca/~oz/hash.html
int hashFunction(char* Qkey, int reducers){
	unsigned long hash = 0;
    int c;

    while ((c = *Qkey++)!='\0')
        hash = c + (hash << 6) + (hash << 16) - hash;

    return (hash % reducers);
}

// Retrieves file path for the words reducer must reduce and compute the total count for
int getInterData(char *Qkey, int reducerID) {
	struct msgBuffer message= makeMessage();
	//DEBUG! make sure it work.
	int id = openQueue();
	if (id == -1)
		exit(-1);
	if (msgrcv(id, &message, MSGSIZE, reducerID, 0) == -1)
		exit(-1);
	strcpy(Qkey, message.msgText);
	return (strncmp("END", message.msgText, 3) != 0);
}

// Divides the word.txt files in output/MapOut/Map_mapperID folders across nReducers and send filepath 
// across nReducers randomly from a hash function
void shuffle(int nMappers, int nReducers) {
	struct msgBuffer message = makeMessage();
	int id = openQueue();
	if (id == -1)
		exit(-1);
	for (int i = 1; i <= nMappers; i++) {
		char newpath[100];
		sprintf(newpath, "output/MapOut/Map_%d", i); 
		DIR *dir = opendir(newpath);
		if (dir == NULL)
			break;
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) {
			if (!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name))
				continue;
			sprintf(message.msgText, "%s/%s", newpath, entry -> d_name);
			message.msgType = (hashFunction(entry -> d_name, nReducers)+1);
			if (msgsnd(id, &message, MSGSIZE, 0) == -1)
				break;
			}
		closedir(dir);
	}
	for (int i = 1; i <= nReducers; i++) {
		struct msgBuffer END = {i, "END"};
		if (msgsnd(id, &END, MSGSIZE, 0))
			break;
	}
}

// Check if the character is valid for a word
int validChar(char c){
	return (tolower(c) >= 'a' && tolower(c) <='z') ||
					(c >= '0' && c <= '9');
}

// Gets words from the buffer
char *getWord(char *chunk, int *i){
	char *buffer = (char *)malloc(sizeof(char) * chunkSize);
	memset(buffer, '\0', chunkSize);
	int j = 0;
	while((*i) < strlen(chunk)) {
		// read a single word at a time from chunk
		if (chunk[(*i)] == '\n' || chunk[(*i)] == ' ' || !validChar(chunk[(*i)]) || chunk[(*i)] == 0x0) {
			buffer[j] = '\0';
			if(strlen(buffer) > 0){
				(*i)++;
				return buffer;
			}
			j = 0;
			(*i)++;
			continue;
		}
		buffer[j] = chunk[(*i)];
		j++;
		(*i)++;
	}
	if(strlen(buffer) > 0)
		return buffer;
	return NULL;
}

void createOutputDir(){
	mkdir("output", ACCESSPERMS);
	mkdir("output/MapOut", ACCESSPERMS);
	mkdir("output/ReduceOut", ACCESSPERMS);
}

char *createMapDir(int mapperID){
	char *dirName = (char *) malloc(sizeof(char) * 100);
	memset(dirName, '\0', 100);
	sprintf(dirName, "output/MapOut/Map_%d", mapperID);
	mkdir(dirName, ACCESSPERMS);
	return dirName;
}

void removeOutputDir(){
	pid_t pid = fork();
	if(pid == 0){
		char *argv[] = {"rm", "-rf", "output", NULL};
		if (execvp(*argv, argv) < 0) {
			printf("ERROR: exec failed\n");
			exit(1);
		}
		exit(0);
	} else{
		wait(NULL);
	}
}

void bookeepingCode(){
	removeOutputDir();
	sleep(1);
	createOutputDir();
}
