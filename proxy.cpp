#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/select.h>
#include <vector>
#include <algorithm>
#include <cassert>

using namespace std;

int main(int argc, char* argv[])
{
	if(argc != 4)
	{
		cout << "Error: Usage is ./server <listen_port> <server_ip> <server_port>\n";
		return 1;
	}
	int portNum = atoi(argv[1]);
	char *ipserver = argv[2];
	int portNumServer = atoi(argv[3]);

	// Bind server to sd and set up listen server
	int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in self;
	self.sin_family = AF_INET;
	self.sin_addr.s_addr = INADDR_ANY;
	self.sin_port = htons((u_short) portNum);
	int err = ::bind(sd, (struct sockaddr*) &self, sizeof(self));
	if(err == -1)
	{
		cout << "Error binding the socket to the port number\n";
		return 1;
	}

	err = listen(sd, 10);
	if(err == -1)
	{
		cout << "Error setting up listen queue\n";
		return 1;
	}


	// socket connect to server
	int serversd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(serversd == -1)
	{
		std::cout << "Error creating server socket\n";
		exit(1);
	}

	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons((u_short) portNumServer);
	server.sin_addr.s_addr = inet_addr(ipserver);
	int err1 = connect(serversd, (sockaddr*) &server, sizeof(server));
	if(err1 == -1)
	{
		std::cout << "Error on connect to server\n";
		exit(1);
	}


	// Set of file descriptors to listen to
	fd_set readSet;
	// Keep track of each file descriptor accepted
	vector<int> fds;

	while(true)
	{
		// Set up the readSet
		FD_ZERO(&readSet);
		FD_SET(sd, &readSet);
		for(int i = 0; i < (int) fds.size(); ++i)
		{
			FD_SET(fds[i], &readSet);
		}


		int maxfd = 0;
		if(fds.size() > 0)
		{
			maxfd = *max_element(fds.begin(), fds.end());
		}
		maxfd = max(maxfd, sd);

		// maxfd + 1 is important
		int err = select(maxfd + 1, &readSet, NULL, NULL, NULL);
		assert(err != -1);

		if(FD_ISSET(sd, &readSet))
		{
			int clientsd = accept(sd, NULL, NULL);
			if(clientsd == -1)
			{
				cout << "Error on accept" << endl;
			}
			else
			{
				fds.push_back(clientsd);
			}
		}

		for(int i = 0; i < (int) fds.size(); ++i)
		{
			if(FD_ISSET(fds[i], &readSet))
			{
				while(1){
					char buf[1000] = "";
					int bytesRecvd = recv(fds[i], &buf, 1000, 0);
					if(bytesRecvd < 0)
					{
						cout << "Error recving bytes" << endl;
						cout << strerror(errno) << endl;
						exit(1);
					}
					else if(bytesRecvd == 0)
					{
						cout << "Connection closed" << endl;
						fds.erase(fds.begin() + i);
						break;
					}
					else{
						cout<<buf<<endl;
					}

					int bytesSent = send(serversd, &buf, 1000, 0);
					if(bytesSent <= 0)
					{
						std::cout << "Error sending stuff to server" << std::endl;
					}
				} 
			}
		}
	}

}