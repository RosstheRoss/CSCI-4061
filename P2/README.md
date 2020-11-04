# CSCI 4061: Project 2

The second project in CSCI 4061: Intro to Operating Systems.

## Fall 2020

- **Test machine:** apollo.cselabs.umn.edu
- **Date:** 10/24/2020
- **Name:** Andrea Smith, Matt Strapp
- **x500:** smit9523, strap012

The purpose of this program is to recreate the mapreduce programming model for Big Data Analytics. In this project, we wrote the IPC utility functions that support the map and reduce process. In order to compile this program, simply run the included Makefile, which takes no arguments.

#### Program structure

```sendChunkData():```
sendChunkData divides the input file into chunks of size 1024 bytes or less, then distributes the chunks to the mappers in a round robin fashion.

```getChunkData():```
Each mapper in the Map phase calls getChunkData to receieve the 1024 byte chunks from sendChunkData.

```shuffle():```
Shuffle divides the word.txt files in output/MapOut/Map_mapperID folders across nReducers, then sends out the filepaths across nReducers in a random order determined by a hash function.

```getInterData()```
getInterData is used by each reducer to retrieve the filepath for words that it must reduce and compute the total count for. 

#### Team Contributions:

For the first draft, Andrea primarily contributed to sendChunkData() and getChunkData() and Matt wrote shuffle() and getInterData(), but the debugging process (the majority of the work) was entirely a joint effort. 