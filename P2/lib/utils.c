#include "utils.h"

int openQueue(char* path) {
	return msgget(ftok(path, 253), 0666 | IPC_CREAT);
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
	// printf("%ld\n", message.msgType);
	//Queue ID
	int mid = openQueue("map");
	// printf("%d\n", mapperID);
	msgrcv(mid, &message, chunkSize, mapperID, 0);
	// printf("\n%s\n", message.msgText);
	// printf("%d\n", strncmp("END", message.msgText, 3));
	if (strncmp("END", message.msgText, 3) == 0)
	{
		return NULL;
	}
	char* value = message.msgText;
	// printf("%s\n", message.msgText);
	printf("%s\n", value);
	return value;
	//return &(message.msgText);
}

// sends chunks of size 1024 to the mappers in RR fashion
void sendChunkData(char *inputFile, int nMappers) {
	struct msgBuffer message = makeMessage();
	// open message queue
	int msgid = openQueue("map");
	int map = 0;
	FILE* file = fopen(inputFile, "r");
	// construct chunks of 1024 bytes
	while(fgets(message.msgText, chunkSize + 1, file) != NULL) {

		int i = 1023;
		while((validChar(message.msgText[i]))) {
			message.msgText[i] = '\0';
			i--;
		}
		// DEBUG!

		fseek(file, (i - 1023), SEEK_CUR);
		message.msgType = (map++ % nMappers) + 1;
		printf("%s\n\n",message.msgText);
		msgsnd(msgid, &message, map, 0);
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
	int id = openQueue("reduce");
	msgrcv(id, &message, chunkSize, reducerID, 0);
	*Qkey = *message.msgText;
	// printf("%s\n", Qkey);
	return strncmp("END", message.msgText, 3) == 0;
}

void shuffle(int nMappers, int nReducers) {
	struct msgBuffer message = makeMessage();
	//Once again, MAKE SURE THIS WORKS PROPERLY!
	char path[100];
	getcwd(path, 100);
	int id = openQueue("reduce");
	for (int i = 1; i <= nMappers; i++) {
		char newpath[200];
		sprintf(newpath, "%s/output/MapOut/Map_%d", path, i);
		printf("%s\n", newpath);
		DIR *dir = opendir(newpath);
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) {
			if (!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name))
				continue;
			message.msgType = hashFunction(entry -> d_name, nReducers);
			printf("%ld\n", message.msgType);
			msgsnd(id, &message, chunkSize, 0);
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
