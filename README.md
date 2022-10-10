# FIFORequestChannel

## Run application

"make" to compile client.cpp, common.cpp, FIFORequestChannel.cpp, server.cpp

## FIFORequestChannel class (FIFORequestChannel.cpp/.h)

These files implements a pipe-based communication channel. The client and server processes use this class to communicate with each other. This class has a read and a write function to receive and send data from/to another process, respectively. The usage of the function is demonstrated in the given client.cpp.

## Server program  (server.cpp) 

This file contains the server logic. When compiled with the makefile, an executable called server is made. You need to run this executable to start the server. Refer to the server to understand the server protocol and then implement the client functionality based on that. 

## Client program (client.cpp)

This file contains the client logic. The starter code can connect to the server using the FIFORequestChannel class; the client sends a sample message to the server and receives a response. When compiled with the makefile, an executable file client is generated. You need to run this executable to start the client.

## Utilities (common.h and common.cpp)

These files contain different useful classes and functions shared between the server and the client. Classes for different types of messages (e.g., a data message, a file message) are defined here.

## Help with implementation

You will find the request format in common.h as a datamsg. The client requests a data point by constructing a datamsg object and then sending this object across the channel through a buffer. A datamsg object is constructed with the following fields:
-p Patient ID is simply specified as a number. There are 15 patients total. The required data type is an int with a value in the range [1,15].
-t Time in seconds. The type is a double with range [0.00,59.996].
-e ECG record: 1 or 2, indicating which record (ecg1 or ecg2) the client should be sent. The data type is an integer.
