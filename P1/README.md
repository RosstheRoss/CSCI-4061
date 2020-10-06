# CSCI 4061: Project 1

The first project in CSCI 4061: Intro to Operating Systems.

## Fall 2020

- **Test machine:** apollo.cselabs.umn.edu
- **Date:** 10/3/2020
- **Name:** Andrea Smith, Matt Strapp
- **x500:** smit9523, strap012

The purpose of this program is to recreate the mapreduce programming model for Big Data Analytics. In order to compile this program, simply run the included Makefile, which takes no arguments.

#### Program structure

```mapreduce.c:```
Mapreduce is the master phase of this program. It is responsible for calling the mapper and reducer processes. It will wait for each process to finish executing before exiting the code.

```mapper.c:```
 Mapper receives a chunk of data and emits it into an intermediate data structure, which is a two-level linked list. writeIntermediateDS() formats the data, writing each word and its associated "1's" in the form "word 1 1 1..." to its own file, word.txt.

```reducer.c:```
Reducer receives the word.txt files and "reduces" them into n files (where n is an input) containing the words and amount of 1's associated with the word in the format "word number_of_1's"

#### Team Contributions:

As can be seen by the number of back-and-forth GitHub commits, this project was thoroughly a team effort. However, for the purposes of ~~appropriating praise from the professors and TA's~~ giving credit where it is due, Matt wrote mapreduce, Andrea wrote reducer, and mapper was written by both of us.
