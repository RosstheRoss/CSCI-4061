#include "utils.h"

int openQueue() {
	key_t Qkey = ftok("4061 Project 2 SS", 'S');
	int id = msgget(Qkey, 0666 | IPC_CREAT);
	if (id < 0)
	{
		perror("Cannot open queue.\n");
		return NULL;
	};
	return id;
}

char *getChunkData(int mapperID) {
	//Message
	struct msgBuffer message;
	//Queue ID
	int mid = openQueue();
	msgrcv(mid, &message, sizeof(message.msgText), mapperID, 0);
	/*IMPLEMENT END AND ACK SOON*/
	// if (strcmp("END", message.msgText)) {
	// 	struct msgBuffer ACK = {mapperID, "ACK"};
	// 	msgsnd(mid, &ACK, MSGSIZE, 0);
	// }
	return message.msgText;
}

// sends chunks of size 1024 to the mappers in RR fashion
void sendChunkData(char *inputFile, int nMappers) {
	struct msgBuffer message;
	// open message queue
	int msgid = openQueue();
	// message.msgText = 1;
	FILE *fptr = fopen(inputFile, "r");

	// construct chunks of 1024 bytes
	while(fgets(&message, chunkSize, fptr) != EOF) {

		/*  Go to the end of the chunk, check if final character 
		    is a space if character is a space, do nothing
		    else cut off before that word and put back file      */
		// TODO! help 

		msgsnd(msgid, &message, 11, 0);
	}

	for (int i = 1; i < nMappers; i++) {
		struct msgBuffer END = {i, "END"};
		msgsnd(msgid, &END, MSGSIZE, 0);

		// TODO! does this need to be in another loop or is blocking good enough?
		msgrcv(msgid, &message, MSGSIZE, i, 0);
	}

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
	//make sure it work.
	key_t Qkey = ftok("4061 Project 2 SS", 'S');

}

void shuffle(int nMappers, int nReducers) {
	//Once again, MAKE SURE THIS WORKS PROPERLY!
	key_t Qkey = ftok("4061 Project 2 SS", 'S');

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
