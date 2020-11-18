#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include "util.h"
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

#define MAX_THREADS 100
#define MAX_queue_len 100
#define MAX_CE 100
#define INVALID -1
#define BUFF_SIZE 1024

int port, workers, dispatchers, dynFlag, qLen, cSiz = 0;
char* path;
pthread_mutex_t Qlock, logLock;

/*
  THE CODE STRUCTURE GIVEN BELOW IS JUST A SUGGESTION. FEEL FREE TO MODIFY AS NEEDED
*/

// structs:
typedef struct request_queue {
   int fd;
   char *request;
   struct request_queue* next;
} request_t;
request_t *Q = NULL; // The request queue

typedef struct cache_entry {
    int len;
    char *request;
    char *content;
    struct cache_entry* next;
} cache_entry_t;
cache_entry_t *dynQ = NULL; //The cache queue

/* ******************** Dynamic Pool Code  [Extra Credit A] **********************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
void * dynamic_pool_size_update(void *arg) {
  while(1) {
    // Run at regular intervals
    // Increase / decrease dynamically based on your policy
  }
}
/**********************************************************************************/

/* ************************ Cache Code [Extra Credit B] **************************/

// Function to check whether the given request is present in cache
int getCacheIndex(char *request){
  /// return the index if the request is present in the cache
  cache_entry_t *traverse = dynQ;
  int index = 0;
  while (traverse != NULL) {
    if (!strcmp(request, traverse->request)) {
      return index;
    }
    index++;
  }
  return -1;
}

// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory , int memory_size){
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memory when adding or replacing cache entries
}

// clear the memory allocated to the cache
void deleteCache(){
  
  request_t *tempReq = NULL;
  while (Q != NULL) {
    tempReq = Q;
    free(tempReq->request);
    Q = Q->next;
    free(tempReq);
  }
  // De-allocate/free the cache memory
  cache_entry_t *tempCache = NULL;
  while (dynQ != NULL)
  {
    tempCache = dynQ;
    dynQ = dynQ->next;
    free(tempCache);
  }
}

// Function to initialize the cache
void initCache(){
  // Allocating memory and initializing the cache array
  
}

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char * mybuf) {
  // Should return the content type based on the file type in the request
  // (See Section 5 in Project description for more details)
  char* ext = strrchr(mybuf, '.');

  if (ext == NULL) {
      printf("Filetype not found. Exiting\n");
      exit(-1);
  }
  else if ((strcmp((ext + 1), "htm") == 0) || (strcmp((ext + 1), "html") == 0)) {
    return "text/html";
  }
  else if (strcmp((ext + 1), "jpg") == 0) {
    return "image/jpeg";
  }
  else if (strcmp((ext + 1), "gif") == 0) {
    return "image/gif";
  }
  else {
    return "text/plain";
  }
}

// Function to get the size of the file in the queue
// (Thanks Matt from stackOverflow)
unsigned long getFileSize(char *file) {
  char* temp = malloc(BUFF_SIZE);
  sprintf(temp, ".%s", file);
  FILE *f = fopen(temp, "r");
  free(temp);
  if (f == NULL)
    return 0;
  fseek(f, 0, SEEK_END);
  unsigned long len = (unsigned long)ftell(f);
  fclose(f);
  return len;
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
int readFromDisk(char* fileName, char* buffer, long fileSize) {
    char* temp = malloc(BUFF_SIZE);
    sprintf(temp, ".%s", fileName);
    // Open and read the contents of file given the request
    int requestFile = open(temp, O_RDONLY);
    free(temp);
    if (requestFile == -1) {
      return -1; // Error handle
    }
    read(requestFile, buffer, fileSize);
    close(requestFile);
    return 0;
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
void * dispatch(void *arg) {

  while (1) {
    // Accept client connection and get the fd
    int newReq = accept_connection();
    if (newReq > INVALID) {
      request_t* traverse = Q;
      
      // Get request from the client
      // Add the request into the queue
      for(int i = 0; i < qLen; i++) {
        if (traverse == NULL) {
          //Add things to queue. Lock & unlock to prevent a deadlock
          pthread_mutex_lock(&Qlock);
          request_t * tempNode = (request_t*) malloc(sizeof(request_t)); 
          char* dispatchBuf = (char *) malloc(BUFF_SIZE); // Buffer to store the requested filename 
          if (get_request(newReq, dispatchBuf) != 0) {
            pthread_mutex_unlock(&Qlock);
            free(dispatchBuf);
            continue; // If get_request fails, try again
          } 
          tempNode->fd = newReq;
          tempNode->request = dispatchBuf;
          Q = tempNode;
          pthread_mutex_unlock(&Qlock);
          break;
        } else {
          traverse = traverse -> next;
        }
      }
    }
   }
   return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {
  int id = *(int*) arg;
  unsigned long numbytes;
  unsigned long long numReqs = 0;
  while (1) {
    // Get the request from the queue
    pthread_mutex_lock(&Qlock);
    if (Q == NULL) {
      pthread_mutex_unlock(&Qlock);
      continue;
    }
    
    //Make copy of request and get rid of old one.
    request_t *request = NULL;
    request = (request_t*) malloc(sizeof(request_t));
    request->fd = Q->fd;
    request->request = Q->request;
    Q = Q->next;
    pthread_mutex_unlock(&Qlock);

    // TODO! Get the data from the disk or the cache (extra credit B)
    numbytes = getFileSize(request->request);
    char *workerBuf = (char *)calloc(numbytes, sizeof(char));
    char *bytesError = malloc(BUFF_SIZE); 
    if (numbytes != 0) {
      //SUCC
      sprintf(bytesError, "%ld", numbytes);
    } else {
      //ERR
      sprintf(bytesError, "%s", strerror(errno));
    }

    // Log the request into the file and terminal
    pthread_mutex_lock(&logLock);
    FILE* log = fopen("../webserver_log", "a");
    fprintf(log, "[%d][%lld][%d][%s][%s][%s]\n", id, ++numReqs, request->fd, request->request, bytesError, "CACHE");
    printf("[%d][%lld][%d][%s][%s][%s]\n", id, numReqs, request->fd, request->request, bytesError, "CACHE");
    fclose(log);
    pthread_mutex_unlock(&logLock);
    free(bytesError);

    // Return the result
    if (readFromDisk(request->request, workerBuf, numbytes) != 0) {
      //ERR
      return_error(request->fd, request->request);
    } else {
      //SUCC
      return_result(request->fd, getContentType(request->request), workerBuf, numbytes);
    }
    //Free things no longer needed
    free(request);
    free(workerBuf);
  }
  return NULL;
}

/**********************************************************************************/

//Flag for when server needs to die nicely
static volatile sig_atomic_t exitFlag = 0;

//Sets exit flag so process can die happily and not sad.
static void eggs(int signo) {
  exitFlag |= 1;
}
int main(int argc, char **argv) {
  // Error check on number of arguments
  if(argc != 8){
    printf("usage: %s port path num_dispatcher num_workers dynamic_flag queue_length cache_size\n", argv[0]);
    return -1;
  }
  // Get the input args
  //Port
  port = atoi(argv[1]);
  //Webroot path
  path = argv[2];
  //(static) number of dispatchers
  dispatchers = atoi(argv[3]);
  //(static) number of workers
  workers = atoi(argv[4]);
  //Dynamic worker flag
  dynFlag = atoi(argv[5]);
  //Queue Length
  qLen = atoi(argv[6]);
  //Max cache size
  cSiz = atoi(argv[7]);

  /* -- ERROR CHECKING -- */
  if (port < 1025 || port > 65535) {
    printf("Invalid port. Port must be greater than 1025 or less than 65535 (inclusive).\n");
    return -1;
  }
  if (dispatchers > MAX_THREADS || dispatchers < 1) {
    printf("Number of dispatchers is invalid. It must be greater than 1 or less than %d (inclusive).\n", MAX_THREADS);
    return -1;
  }
  if (workers > MAX_THREADS || workers < 1) {
    printf("Number of dispatchers is invalid. It must be greater than 1 or less than %d (inclusive).\n", MAX_THREADS);
    return -1;
  }
  if (qLen > MAX_queue_len || qLen < 1) {
    printf("Queue length is invalid It must be greater than 1 or less than %d (inclusive).\n", MAX_queue_len);
    return -1;
  }
  /* -- END ERROR CHECKING -- */

  // Change SIGINT action for graceful termination
  struct sigaction act;
  act.sa_handler = eggs;
  act.sa_flags = 0;
  if (sigemptyset(&act.sa_mask) == -1 ||
      sigaction(SIGINT, &act, NULL) == -1) {
        perror("SIGINT Handler Error");
        return -2;
  }
  // Create instance of logfil
  FILE* logfile = fopen("webserver_log", "w");
  fclose(logfile);

  // Change the current working directory to server root directory
  if (chdir(path) == -1) {
    perror("Directory Change error");
    return -2;
  }
  // Initialize cache (extra credit B)
  initCache();

  //Make sure threads are all detached
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  // Start the server
  init(port);
  // Create dispatcher threads (make detachable????)
  pthread_t dThreads[dispatchers];
  for (int i=0; i<dispatchers; i++) {
    pthread_create(&dThreads[i], &attr, dispatch, NULL); // DEBUG! figure out last arg
  }
  //Create workers (make detachable?????)
  pthread_t wThreads[workers];
  int* Wargs = malloc(sizeof(int)*workers);
  for (int i = 0; i < workers; i++) {
    Wargs[i]=i;
    pthread_create(&wThreads[i], &attr, worker, (void *) &Wargs[i]); //TODO: Worker arguments
  }
  free(Wargs);
  // Create dynamic pool manager thread (extra credit A)
  if (dynFlag) {
    pthread_t pThread;
    pthread_create(&pThread, &attr, dynamic_pool_size_update, NULL); //TODO: possible arguments
  }
 
  //Server loop (RUNS FOREVER)
  while (1) {
    //TODO: Add something else?
    // Terminate server gracefully
    if (exitFlag){
      printf("\nSIGINT caught, exiting now.\n");
      // Print the number of pending requests in the request queue
      int pendReqs;
      for (pendReqs = 0; pendReqs < qLen; pendReqs++) {
        if (Q == NULL || Q->next == NULL)
          break;
      }
      printf("There are %d pending requests left in the queue.\n", pendReqs);
      // Remove cache (extra credit B)
      deleteCache();
      printf("Cache has been deleted.\n");
      return 0;
    }
  }
  printf("\n\nThis should never be printed.\n\n");
  return 42;
}
