# CSCI 4061: Project 4

The fourth project in CSCI 4061: Intro to Operating Systems.

## Fall 2020

- **Test machine:** atlas.cselabs.umn.edu
- **Date:** 12/10/2020
- **Name:** Andrea Smith, Matt Strapp
- **x500:** smit9523, strap012

The purpose of this program is to create a multi-threaded web server by using POSIX threads. In this project, we wrote the backend using POSIX sockets.

In order to run this program, compile with the included makefile (run ```make``` in the directory with no additional arguments). After that, run ```./web_server``` with the following additional arguments seperated by only a space (with no quotes or brackets):
[Port used (between 1025 and 65535)] [Web server directory] [Number of dispatch threads] [Number of worker threads] [Maximum worker queue length] [NOT IMPLEMENTED (Use 0)] [Maximum cache length]


#### Program structure

```init():```
Binds a port that the server can use to communicate with the outside world.

```accept_connection():```
Makes a one-time use socket for the connection.

```get_request()```
Takes the HTTP request and translates it to a file path that the server can use.

```return_result()```
Formats the return to the HTTP protocol and returns the successful request back to the client.

```return_error()```
Formats the return to the HTTP protocol and returns the unsuccessful request back to the client.


#### Team Contributions:

Matt wrote init(), return_result(), and return_error(). Andrea wrote accept_connection() and get_request(). 