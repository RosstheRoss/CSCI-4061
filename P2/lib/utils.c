#include "utils.h"

int openQueue() {
	char cwd [50];
	getcwd(cwd, 50);
	return msgget(ftok(cwd, 4061), 0666 | IPC_CREAT);
}
int closeQueue(int id) {
	return msgctl(id, IPC_RMID, NULL);
}

struct msgBuffer makeMessage() {
	struct msgBuffer temp;
	memset(temp.msgText, '\0', MSGSIZE);
	temp.msgType = 0;
	return temp;
}

char *getChunkData(int mapperID) {
	//Message
	struct msgBuffer message = makeMessage();
	//Queue ID
	int mid = openQueue();
	//printf("MAPPER ID:%d\n", mapperID);
	msgrcv(mid, &message, MSGSIZE, mapperID, 0);
	printf("\n%s\n", message.msgText);
	// printf("%d\n", strncmp("END", message.msgText, 3));
	if (strncmp("END", message.msgText, 4) == 0)
		return NULL;
	// char* value = message.msgText;
	// return value;

	// DEBUG! malloc a buffer/return 
	char* value = malloc(1024); // chunkSize or MSGSIZE?
	return value;
	// Free memory

	// printf("%s\n", message.msgText);
	//printf("RECEIVED CHUNK:%s\nRECEIVED VALUE:%ld\n", value, message.msgType);
	
	//return &(message.msgText);
}

// sends chunks of size 1024 to the mappers in RR fashion
void sendChunkData(char *inputFile, int nMappers) {
	struct msgBuffer message = makeMessage();
	// open message queue
	int msgid = openQueue();
	closeQueue(msgid);
	msgid = openQueue();
	// DEBUG! Remove if already exists when opening queue for the first time
	int map = 0;
	FILE* file = fopen(inputFile, "r");
	// construct chunks of 1024 bytes
	while(fgets(message.msgText, chunkSize + 1, file) != NULL) {

		int i = 1023;
		while(validChar(message.msgText[i])) {
			message.msgText[i] = '\0';
			i--;
		}
		// DEBUG!

		fseek(file, (i - 1023), SEEK_CUR);
		message.msgType = (map++ % nMappers) + 1;
		//printf("SENT CHUNK: %s\nSENT CHUNK MAPPER: %ld\n",message.msgText, message.msgType);
		msgsnd(msgid, &message, MSGSIZE, 0);
	}

	for (int i = 1; i <= nMappers; i++) {
		struct msgBuffer END = {i, "END"};
		msgsnd(msgid, &END, MSGSIZE, 0);
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

int getInterData(char *Qkey, int reducerID) {
	struct msgBuffer message= makeMessage();
	//DEBUG! make sure it work.
	// How do we traverse the directory if we're not given it as an arg?
	int id = openQueue();
	msgrcv(id, &message, MSGSIZE, reducerID, 0);
	// printf("INTER DATA: %s\n", message.msgText);
	*Qkey = *message.msgText;
	printf("INTER DATA: %s\n", Qkey);
	return (strncmp("END", message.msgText, 3) == 0);
}

void shuffle(int nMappers, int nReducers) {
	//TODO: Error checking!!!!!!!!!!!!!
	struct msgBuffer message = makeMessage();
	//Once again, MAKE SURE THIS WORKS PROPERLY!
	char path[50];
	getcwd(path, 50);
	int id = openQueue();
	for (int i = 1; i <= nMappers; i++) {
		char newpath[100];
		sprintf(newpath, "%s/output/MapOut/Map_%d", path, i);
		DIR *dir = opendir(newpath);
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) {
			if (!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name))
				continue;
			sprintf(message.msgText, "%s/%s", newpath, entry -> d_name);
			message.msgType = hashFunction(entry -> d_name, nReducers);
			printf("SENT SHUFFLE:%s\n", message.msgText);
			printf("%ld\n", message.msgType);
			msgsnd(id, &message, MSGSIZE, 0);
			}
		closedir(dir);
	}
}

// check if the character is valid for a word
int validChar(char c){
	return (tolower(c) >= 'a' && tolower(c) <='z') ||
					(c >= '0' && c <= '9');
}

char *getWord(char *chunk, int *i){
	char *buffer = (char *)malloc(sizeof(char) * chunkSize);
	memset(buffer, '\0', chunkSize);
	int j = 0;
	while((*i) < strlen(chunk)) {
		// read a single word at a time from chunk
		// printf("%d\n", i);
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
