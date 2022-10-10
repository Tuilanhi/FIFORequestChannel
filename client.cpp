/*
	Original author of the starter code
	Tanzir Ahmed
	Department of Computer Science & Engineering
	Texas A&M University
	Date: 2/8/20

	Please include your Name, UIN, and the date below
	Name: Nhi Vu
	UIN: 230005282
	Date: 09/05/22
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <bits/stdc++.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>

using namespace std;

void delete_chan_helper(FIFORequestChannel *c)
{
	MESSAGE_TYPE quit_channel = QUIT_MSG;
	c->cwrite(&quit_channel, sizeof(MESSAGE_TYPE));
	delete c;
}

int main(int argc, char *argv[])
{
	int m = MAX_MESSAGE;
	char *buff2;

	int opt;
	int p = -1;
	double t = -1;
	int e = -1;
	bool file_flag = false;
	bool new_channel_req = false;
	bool new_buff_req = false;
	char *filename;
	vector<FIFORequestChannel *> channels;

	// get input arguments from the user in the command line
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1)
	{
		switch (opt)
		{
		case 'p':
			p = atoi(optarg);
			break;
		case 't':
			t = atof(optarg);
			break;
		case 'e':
			e = atoi(optarg);
			break;
		case 'f':
			filename = optarg;
			file_flag = true;
			break;
		case 'm':
			m = atoi(optarg);
			buff2 = optarg;
			new_buff_req = true;
			break;
		case 'c':
			new_channel_req = true;
			break;
		}
	}

	// give arguments for the server
	// server needs './server', '-m', '<val for -m arg>', 'NULL'
	// fork
	// In the child, run execvp using the server arguments.

	// Make a child process that is a carbon copy of the parent
	pid_t pid = fork();

	// if the fork() fails
	if (pid < 0)
	{
		cerr << "Fork failed." << endl;
		return 1;
	}
	else if (pid == 0)
	{
		// replace the child process with the server binary
		// check if new_buff_req is enable
		if (new_buff_req)
		{
			char *args[] = {const_cast<char *>("./server"), const_cast<char *>("-m"), buff2, nullptr};
			if (execvp(args[0], args) < 0)
			{
				perror("execvp");
				exit(EXIT_FAILURE);
			}
			execvp(args[0], args);
		}
		else
		{
			char *args[] = {const_cast<char *>("./server"), nullptr};
			if (execvp(args[0], args) < 0)
			{
				perror("execvp");
				exit(EXIT_FAILURE);
			}
			execvp(args[0], args);
		}
	}
	// parent process
	FIFORequestChannel cont_chan("control", FIFORequestChannel::CLIENT_SIDE);
	channels.push_back(&cont_chan);

	// NEW CHANNEL MESSAGE
	if (new_channel_req)
	{

		// send newChannel request to the server
		MESSAGE_TYPE nc = NEWCHANNEL_MSG;
		cont_chan.cwrite(&nc, sizeof(MESSAGE_TYPE));
		// create a variable to hold the name
		char channel_name[30];
		// cread the response from the server
		cont_chan.cread(&channel_name, sizeof(channel_name));
		// call the FIFORequestChannel constructor with the name from the server (new)
		FIFORequestChannel *new_chan = new FIFORequestChannel(channel_name, FIFORequestChannel::CLIENT_SIDE);
		// Push the new channel into the vector
		channels.push_back(new_chan);
	}

	FIFORequestChannel chan = *(channels.back());

	// if p != -1, request 1000 datapoints
	// loop over 1st 1000 lines
	// send request for ecg 1
	// send request for ecg 2
	// write line to received/x1.csv
	// format t, e1, e2
	if (p != -1 && t == -1 && e == -1)
	{

		ofstream points;
		points.open("received/x1.csv");
		for (double time = 0.0; time < 4; time += 0.004)
		{
			datamsg x(p, time, 1);			  // change from hardcoding to user's value
			chan.cwrite(&x, sizeof(datamsg)); // question
			double reply;
			chan.cread(&reply, sizeof(double)); // answer

			datamsg x2(p, time, 2);			   // change from hardcoding to user's value
			chan.cwrite(&x2, sizeof(datamsg)); // question
			double reply2;
			chan.cread(&reply2, sizeof(double)); // answer
			points << time << ',' << reply << ',' << reply2 << endl;
		}
		points.close();
	}

	// single datapoint, only run p.t.e != -1
	// example data point request
	else if (t != -1 && p != -1 && e != -1) // time flag is true and we are requesting only 1 data point at a specific time
	{
		char buf[MAX_MESSAGE]; // 256
		datamsg x(p, t, e);	   // change from hardcoding to user's value

		memcpy(buf, &x, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg)); // question
		double reply;
		chan.cread(&reply, sizeof(double)); // answer
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}

	// new file message
	else if (file_flag == true)
	{
		int64_t offset = 0;
		int64_t length = 0;

		filemsg init_fm = filemsg(offset, length);
		// buff req : size len = sizeof(filemsg) + sizeof(filename)
		int len = sizeof(filemsg) + strlen(filename) + 1;
		char *buf2 = new char[len];
		memcpy(buf2, &init_fm, sizeof(filemsg));
		// we want to append the filename to the buffer, which contains the file message
		strcpy(buf2 + sizeof(filemsg), filename);

		chan.cwrite(buf2, len);
		int64_t filesize;
		chan.cread(&filesize, sizeof(int64_t));
		int num = ceil(double(filesize) / m);
		// accommodate for filesize
		// if filesize is less than 256
		filemsg *file_req = (filemsg *)buf2;

		if (num == 1)
		{
			file_req->offset = 0;		 // set offset in the file
			file_req->length = filesize; // set the length to filesize
		}
		else
		{
			file_req->offset = 0; // set offset in the file
			file_req->length = m; // set the length to filesize
		}

		// buff res : size buff capacity
		int64_t res = filesize - m * (num - 1);

		chan.cwrite(buf2, len);
		char *buf3 = new char[m];
		chan.cread(buf3, m);

		string file_path = string("received/") + string(filename);
		ofstream file;
		file.open(file_path.c_str());
		file.write(buf3, file_req->length);
		// loops over the segments in the file filesize/buff (capacity)(m)
		// create filemsg instance
		for (int i = 1; i < num; i++)
		{

			if (i < num - 1)
			{
				file_req->offset += m;
				chan.cwrite(buf2, len);
				chan.cread(buf3, m);
				file.write(buf3, file_req->length);
			}
			else
			{
				file_req->length = res;
				file_req->offset += m;

				chan.cwrite(buf2, len);
				chan.cread(buf3, m);
				file.write(buf3, file_req->length);
			}
		}

		delete[] buf3;
		delete[] buf2;
	}

	if (new_channel_req)
	{
		// close and delete the new channel
		// not quit the first channel
		// MESSAGE_TYPE quit = QUIT_MSG;
		// chan.cwrite(&quit, sizeof(MESSAGE_TYPE)); for every channel except the chan channel
		delete_chan_helper(channels.back());
		cout << "\n";
	}

	// Close the channel
	MESSAGE_TYPE quit = QUIT_MSG;
	cont_chan.cwrite(&quit, sizeof(MESSAGE_TYPE));

	// use diff to check if the two files are equal in both received file and BIMDC file
	// wait for the child process to finish
	waitpid(pid, nullptr, 0);
}
