# CSCI 4061: Project 3

The third project in CSCI 4061: Intro to Operating Systems.

## Fall 2020

- **Test machine:** atlas.cselabs.umn.edu
- **Date:** 11/15/2020
- **Name:** Andrea Smith, Matt Strapp
- **x500:** smit9523, strap012

The purpose of this program is to create a multi-threaded web server by using POSIX threads. In this project, we wrote the mapper and reducer threads, a logging system 

#### Program structure

```dispatch():```
Repeatedly receives the client request and adds the requests to the queue.

```worker():```
Monitors the queue, retrieve new requests as they come in, and send the result back to the client.

## Extra Credit A

```dynamic_pool_size_update():```

## Extra Credit B

```initCache():```

```isInCache():```

```readFromCache():```

```addIntoCache():```

```deleteCache():```


#### Team Contributions:

For the first draft, Andrea primarily contributed to sendChunkData() and getChunkData() and Matt wrote shuffle() and getInterData(), but the debugging process (the majority of the work) was entirely a joint effort. 