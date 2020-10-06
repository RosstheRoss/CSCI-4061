#!/bin/bash
make && cp mapper solutionexe/mapper && cp mapreduce solutionexe/mapreduce && cd solutionexe/ && make run && cp mapper.good mapper && cp mapreduce.good mapreduce && cd ..
