# CSCI 4061: Project 3

The third project in CSCI 4061: Intro to Operating Systems.

## Fall 2020 

- **Test machine:** atlas.cselabs.umn.edu
- **Date:** 11/15/2020
- **Name:** Andrea Smith, Matt Strapp
- **x500:** smit9523, strap012

The purpose of this program is to create a multi-threaded web server by using POSIX threads. In this project, we wrote the worker and dispatcher threads, a logging system, and helper functions that implement caching to improve runtime performance.

In order to run this program, compile with the included makefile (run ```make``` in the directory with no additional arguments). After that, run ```./web_server``` with the following additional arguments seperated by only a space (with no quotes or brackets):
[Port used (between 1025 and 65535)] [Web server directory] [Number of dispatch threads] [Number of worker threads] [Maximum worker queue length] [Unimplemented Flag (Use 0)] [Maximum cache length]


#### Program structure

```dispatch():```
Repeatedly receives the client request and adds the requests to the queue.

```worker():```
Monitors the queue, retrieve new requests as they come in, and send the result back to the client.

```readFromDisk()```
Opens and reads the request's file from the disk into memory.

```getContentType()```
Gets the content type of the file in the request.

## Extra Credit B (Helper functions for worker() to implement a cache)

```initCache():```
Allocates memory and intializes the cache array.

```isInCache():```
Checks whether the given request is present in the cache and on success, returns the index of the request.

```readFromCache():```
Traverses the cache queue to find the needed cache.

```addIntoCache():```
Adds the request and its file content into the cache.

```deleteCache():```
Clears/frees up the memory allocated to the cache.

```getFileSize()```
Gets the size of the file in the queue in order to be able to allocate buffers for the cache.


#### Team Contributions:

We work on projects together using LiveShare on VSCode. Because of this, it is difficult to say exactly how the work is distributed-- there are only two of us, so we typically are working at the same by going back and forth over a chunk of code together. The workload for worker() and dispatch() was evenly distributed. Matt also primarily wrote the Cache Code for Extra Credit B. Andrea handled the interim report, README, and other code documentation.