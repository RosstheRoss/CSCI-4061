#include "reducer.h"

finalKeyValueDS DS;

// create a key value node
finalKeyValueDS *createFinalKeyValueNode(char *word, int count){
	finalKeyValueDS *newNode = (finalKeyValueDS *)malloc (sizeof(finalKeyValueDS));
	strcpy(newNode -> key, word);
	newNode -> value = count;
	newNode -> next = NULL;
	return newNode;
}

// insert or update an key value
finalKeyValueDS *insertNewKeyValue(finalKeyValueDS *root, char *word, int count){
	finalKeyValueDS *tempNode = root;
	if(root == NULL)
		return createFinalKeyValueNode(word, count);
	while(tempNode -> next != NULL){
		if(strcmp(tempNode -> key, word) == 0){
			tempNode -> value += count;
			return root;
		}
		tempNode = tempNode -> next;
	}
	if(strcmp(tempNode -> key, word) == 0){
		tempNode -> value += count;
	} else{
		tempNode -> next = createFinalKeyValueNode(word, count);
	}
	return root;
}

// free the DS after usage. Call this once you are done with the writing of DS into file
void freeFinalDS(finalKeyValueDS *root) {
	if(root == NULL) return;

	finalKeyValueDS *tempNode = NULL;
	while (root != NULL) {
		tempNode = root;
		root = root -> next;
		free(tempNode);
	}
}

// reduce function
void reduce(char *key) {
	// Calculate the total count of values (1's) for the word from [the_word].txt
	// Store the total count in finalKeyValueDS
	// Lather, rinse, repeat for all the words
	FILE* fptr = fopen(key, "r");
	
	char word[BUFFSIZE] = ""; 
	fgets(word, BUFFSIZE, fptr); // Buffer is currently entire [the_word].txt file
	
	char *parsedKey = strtok(word, " "); // Parses input by whitespaces
	char *parsedWord = parsedKey; // Save the first token, which is the word. It's a surprise tool that will help us later.

	int count = 0;
	while(parsedKey != NULL) {
		count++;
		parsedKey = strtok(NULL, " ");
	}

	fclose(fptr);
	insertNewKeyValue(&DS, parsedWord, count);
}

// write the contents of the final intermediate structure
// to output/ReduceOut/Reduce_reducerID.txt
void writeFinalDS(int reducerID){
	
	finalKeyValueDS *root = &DS;
	finalKeyValueDS *tempNode = root -> next;

	while(tempNode != NULL) {
		// Shove word and number of occurances in a file named word.txt
		char filename[BUFFSIZE] = "";
		sprintf(filename, "output/ReduceOut/Reduce_%d.txt", reducerID);
		FILE* fptr = fopen(filename, "w");
		fprintf(fptr, "%s %d", tempNode -> key, tempNode -> value);
		fclose(fptr);
		tempNode = tempNode -> next;
	}
	freeFinalDS(root -> next);
}

int main(int argc, char *argv[]) {

	if(argc < 2){
		printf("Less number of arguments.\n");
		printf("./reducer reducerID");
	}

	// ###### DO NOT REMOVE ######
	// initialize 
	int reducerID = strtol(argv[1], NULL, 10);

	// ###### DO NOT REMOVE ######
	// master will continuously send the word.txt files
	// alloted to the reducer
	char key[MAXKEYSZ];
	while(getInterData(key, reducerID))
		reduce(key);

	// You may write this logic. You can somehow store the
	// <key, value> count and write to Reduce_reducerID.txt file
	// So you may delete this function and add your logic
	writeFinalDS(reducerID);

	return 0;
}