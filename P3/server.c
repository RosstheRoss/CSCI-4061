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

// Global variables
int port, workers, dispatchers, dynFlag, maxQlen, maxCSiz, curQSiz, cacheLength, wIndex;
char *path;
pthread_mutex_t Qlock, logLock, cacheLock;
pthread_cond_t QisEmpty, QisFull, cacheIsEmpty;
clock_t start_t, end_t, total_t = 600;
void *worker(void *arg);

/*
  THE CODE STRUCTURE GIVEN BELOW IS JUST A SUGGESTION. FEEL FREE TO MODIFY AS NEEDED
*/

// Structs:
typedef struct request_queue {
  int fd;
  char *request;
  struct request_queue *next;
} request_t;
request_t *Q = NULL; // The request queue

typedef struct cache_entry {
  int len;
  char *request;
  char *content;
  struct cache_entry *next;
} cache_entry_t;
cache_entry_t *dynQ = NULL; //The cache queue

/* ******************** Dynamic Pool Code  [Extra Credit A] **********************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
void *dynamic_pool_size_update(void *arg) {
  return NULL;
}

/**********************************************************************************/

/* ************************ Cache Code [Extra Credit B] **************************/

// Function to check whether the given request is present in cache
// return the index of the request is present in the cache
int isInCache(char *request) {
  cache_entry_t *traverse = dynQ;
  if (dynQ == NULL)
    return -2;
  int index = 0;
  while (traverse != NULL) {
    if (traverse->request == NULL)
      break;
    if (!strcmp(request, traverse->request))
      return index;
    //else
    traverse = traverse->next;
    index++;
  }
  return -1;
}

// Function to traverse cache queue to find cache
int readFromCache(int index, char *buffer) {
  cache_entry_t *traverse = dynQ;
  if (traverse == NULL)
    return -1;
  for (int i = 0; i < index; i++)
    traverse = traverse->next;
  memcpy(buffer, traverse->content, traverse->len);
  return 0;
}

// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory, int memory_size) {
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memory when adding or replacing cache entries

  cache_entry_t *traverse = dynQ;
  if (dynQ == NULL)
    return;
  bool fullCache = false;
  while (traverse->next != NULL) {
    if (cacheLength >= maxCSiz) {
      fullCache = true;
      break;
    }
    traverse = traverse->next;
  }
  char *silence = (char *)malloc(memory_size + 1);
  memcpy(silence, memory, memory_size);
  if (fullCache) {
    //Delete oldest member of cache (the first one on the list)
    cache_entry_t *temp = dynQ;
    dynQ = dynQ->next;
    free(temp->content);
    free(temp);
    free(silence);
    addIntoCache(mybuf, memory, memory_size);
  } else { //Cache is not full
    cache_entry_t *temp = (cache_entry_t *)calloc(1, sizeof(cache_entry_t));
    temp->request = mybuf;
    temp->content = silence;
    temp->len = memory_size;
    if (cacheLength == 0) {
      dynQ = temp;
      pthread_cond_signal(&cacheIsEmpty);
    }
    else {
      traverse->next = temp;
    }
    cacheLength++;
  }
}

// Clear the memory allocated to the cache
void deleteCache()
{
  request_t *tempReq = NULL;
  pthread_mutex_lock(&Qlock);
  while (Q != NULL) {
    tempReq = Q;
    Q = Q->next;
    free(tempReq->request);
    free(tempReq);
  }
  pthread_mutex_unlock(&Qlock);
  // De-allocate/free the cache memory
  cache_entry_t *tempCache = NULL;
  pthread_mutex_lock(&cacheLock);
  while (dynQ != NULL) {
    tempCache = dynQ;
    dynQ = dynQ->next;
    free(tempCache->content);
    free(tempCache->request);
    free(tempCache);
  }
  pthread_mutex_unlock(&cacheLock);
}

// Function to initialize the cache
void initCache(){
  // Allocating memory and initializing the cache array
  dynQ = (cache_entry_t *)calloc(1, sizeof(cache_entry_t));
  wIndex = 0;
  cacheLength = 0;
}

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char *getContentType(char *mybuf) {
  // Should return the content type based on the file type in the request
  // (See Section 5 in Project description for more details)
  char *ext = strrchr(mybuf, '.');

  if (ext == NULL) {
    printf("Filetype not found. Exiting\n");
    return NULL;
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

// Function to get the size of the file in the queue. Will be used to allocate memory for the cache
// (Thanks Matt from stackOverflow)
long getFileSize(char *file){
  char *temp = (char *)malloc(BUFF_SIZE);
  sprintf(temp, ".%s", file);
  FILE *f = fopen(temp, "r");
  free(temp);
  if (f == NULL) {
    perror("File size read error");
    return 0; 
  }
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fclose(f);
  return len;
}

// Function to open and read the file from the disk into memory
int readFromDisk(char *fileName, char *buffer, long fileSize) {
  char *temp = (char *)malloc(BUFF_SIZE + 1);
  sprintf(temp, ".%s", fileName);
  // Open and read the contents of file given the request
  int requestFile = open(temp, O_RDONLY);
  free(temp);
  if (requestFile == -1) {
    perror("Disk read error");
    return -1; // Error handle
  }
  if (read(requestFile, buffer, fileSize) == -1) {
    perror("File read error");
    close(requestFile);
    return -1;
  }
  close(requestFile);
  return 0;
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
void *dispatch(void *arg){
  while (1) {
    // Accept client connection and get the fd
    int newReq = accept_connection();
    if (newReq > INVALID) {
      pthread_mutex_lock(&Qlock);
      while (curQSiz == maxQlen)
        pthread_cond_wait(&QisFull, &Qlock);
      request_t *traverse = Q;

      for (int i = 0; i < maxQlen; i++) {
        if (traverse == NULL || traverse->next == NULL) {
          //Add things to queue. Lock & unlock to prevent a deadlock
          request_t *tempNode = (request_t *)malloc(sizeof(request_t) + 1);
          char *dispatchBuf = (char *)malloc(BUFF_SIZE); // Buffer to store the requested filename
          if (get_request(newReq, dispatchBuf) != 0) {
            free(tempNode);
            free(dispatchBuf);
            pthread_mutex_unlock(&Qlock);
            break; // If get_request fails, ignore the request
          }
          tempNode->fd = newReq;
          tempNode->request = dispatchBuf;
          tempNode->next = NULL;
          if (traverse == NULL) {
            Q = tempNode;
            pthread_cond_signal(&QisEmpty);
          } else {
            traverse->next = tempNode;
          }
          curQSiz++;
          pthread_mutex_unlock(&Qlock);
          break;
        } else {
          traverse = traverse->next;
        }
      }
    }
  }
  return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void *worker(void *arg) {
  int id = *(int *)arg;
  long numbytes;
  unsigned long numReqs = 0;
  while (1) {
    // Get the request from the queue
    pthread_mutex_lock(&Qlock);
    while (curQSiz == 0)
      pthread_cond_wait(&QisEmpty, &Qlock);
    start_t = clock();
    request_t *request = (request_t *)malloc(sizeof(request_t));
    request->fd = Q->fd;
    request->request = Q->request;
    Q = Q->next;
    curQSiz--;
    pthread_cond_signal(&QisFull);
    pthread_mutex_unlock(&Qlock);

    //Get the data from the disk or the cache (extra credit B)
    numbytes = getFileSize(request->request);
    char *workerBuf = (char *)calloc(numbytes + 1, sizeof(char));
    char *bytesError = (char *)malloc(BUFF_SIZE);
    bool fail = false;
    if (numbytes != 0) {
      //SUCC
      sprintf(bytesError, "%ld", numbytes);
    } else {
      //ERR
      fail = true;
      sprintf(bytesError, "%s", strerror(errno));
    }
    char *cacheTest; // HIT/MISS only
    if (!fail) {
      // Make sure nothing is fiddling with cache before fiddling with cache
      pthread_mutex_lock(&cacheLock);
      int test = isInCache(request->request);
      if (test != -1) {
        //In cache, file exists
        cacheTest = "HIT";
        readFromCache(test, workerBuf);
        pthread_mutex_unlock(&cacheLock);
      } else {
        pthread_mutex_unlock(&cacheLock);
        cacheTest = "MISS";
        if (readFromDisk(request->request, workerBuf, numbytes) == -1) {
          // If request not in cache, disk read failed, set flag
          fail = true;
        }
        else {
          //Not in cache, disk read succeeds
          pthread_mutex_lock(&cacheLock);
          addIntoCache(request->request, workerBuf, numbytes);
          pthread_mutex_unlock(&cacheLock);
        }
      }
    } else {
      //Failsafe to add cache being missed
      cacheTest = "MISS";
    }

    // Log the request into the file and terminal
    pthread_mutex_lock(&logLock);
    FILE *log = fopen("../webserver_log", "a");
    fprintf(log, "[%d][%ld][%d][%s][%s][%s]\n", id, ++numReqs, request->fd, request->request, bytesError, cacheTest);
    printf("[%d][%ld][%d][%s][%s][%s]\n", id, numReqs, request->fd, request->request, bytesError, cacheTest);
    fclose(log);
    pthread_mutex_unlock(&logLock);
    free(bytesError);

    // Return the result
    if (fail) {
      // ERROR
      return_error(request->fd, request->request);
    }
    else {
      // SUCCESS
      return_result(request->fd, getContentType(request->request), workerBuf, numbytes);
    }
    // Free things no longer needed
    free(request);
    free(workerBuf);
  }
  return NULL;
}

/**********************************************************************************/

// Flag for when server needs to die nicely
static volatile sig_atomic_t exitFlag = 0;

//Sets e flag so process can die happily and not sad.
static void eggs(int signo) {
  exitFlag |= 1;
}
int main(int argc, char **argv) {

  // Error check on number of arguments
  if (argc != 8) {
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
  maxQlen = atoi(argv[6]);
  //Max cache size
  maxCSiz = atoi(argv[7]);

  /* -- ERROR CHECKING -- */
  if (port < 1025 || port > 65535) {
    printf("Invalid port. Port must be between 1025 and 65535 (inclusive).\n");
    return -1;
  }
  if (dispatchers > MAX_THREADS || dispatchers < 1) {
    printf("Number of dispatchers is invalid. It must be between 1 and %d (inclusive).\n", MAX_THREADS);
    return -1;
  }
  if (workers > MAX_THREADS || workers < 1) {
    printf("Number of dispatchers is invalid. It must be between 1 and %d (inclusive).\n", MAX_THREADS);
    return -1;
  }
  if (maxQlen > MAX_queue_len || maxQlen < 1) {
    printf("Queue length is invalid. It must be between 1 and %d (inclusive).\n", MAX_queue_len);
    return -1;
  }
  if (maxCSiz > MAX_queue_len || maxCSiz < 1) {
    printf("Cache size is invalid. It must be between 1 and %d (inclusive).\n", MAX_queue_len);
    return -1;
  }

  /* -- END ERROR CHECKING -- */

  // Change SIGINT action for graceful termination
  struct sigaction act;
  act.sa_handler = eggs;
  act.sa_flags = 0;
  if (sigemptyset(&act.sa_mask) == -1 || sigaction(SIGINT, &act, NULL) == -1) {
    perror("SIGINT Handler Error");
    return -2;
  }
  // Create instance of logfile
  FILE *logfile = fopen("webserver_log", "w");
  if (logfile == NULL) {
    perror("Logfile creation error");
    return -2;
  }
  fclose(logfile);

  // Change the current working directory to server root directory
  if (chdir(path) == -1) {
    perror("Directory Change error");
    return -2;
  }
  // Initialize cache (extra credit B)
  initCache();

  // Make sure threads are all detached
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  // Start the server
  init(port);
  char threadName[16];
  // Create dispatcher threads
  pthread_t dThreads[dispatchers];
  for (int i = 0; i < dispatchers; i++) {
    pthread_create(&dThreads[i], &attr, dispatch, NULL);
    sprintf(threadName, "Dispatch %d", i);
    pthread_setname_np(dThreads[i], threadName);
  }
  // Create workers
  int *Wargs = (int *)malloc(sizeof(int) * workers);
  pthread_t wID[workers];
  for (int i = 0; i < workers; i++) {
    wIndex++;
    Wargs[i] = i;
    pthread_create(&wID[i], &attr, worker, (void *)&Wargs[i]);
    sprintf(threadName, "Worker %d", i);
    pthread_setname_np(wID[i], threadName);
  }
  free(Wargs);

  // Server loop (RUNS FOREVER)
  while (1) {
    // Terminate server gracefully
    if (exitFlag) {
      printf("\nSIGINT caught, exiting now.\n");
      // Print the number of pending requests in the request queue
      int pendReqs;
      for (pendReqs = 0; pendReqs < maxQlen; pendReqs++) {
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
