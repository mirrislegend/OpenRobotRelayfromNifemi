#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

//relay entries
typedef struct {
	int position;
	struct sockaddr_in relay_addr;
	int port_number;
	int occupied;
} relay_entry;

relay_entry relay_table[10];


//channels
typedef struct{
	char *channel_name;
	char *chanbuff;
	relay_entry subscribers[10];
} channel;

channel channel_table[10];

int relayflag=0;

//"relay entry" = "entry in relay table"
void setUpRelayTable(relay_entry table[])
{
	int capacity = 10;
	int n;
	for(n=0; n<capacity; n++)
	{
		table[n].position = n;
		//printf("%d\n", table[n].position);
		table[n].port_number = 34000+n;
		table[n].occupied = 0;

		/*
		keep an eye on first parameter of the memset method below, the & is gone because table_entry is already an address
		*/
		memset(&table[n].relay_addr, 0, sizeof(table[n].relay_addr));
		table[n].relay_addr.sin_family = AF_INET;
		table[n].relay_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		table[n].relay_addr.sin_port = htons(table[n].port_number);
	}
}

void setUpChannelTable(channel table[])
{
	
	//setting up the channel names
	table[0].channel_name = "A";
	table[1].channel_name = "B";
	table[2].channel_name = "C";
	table[3].channel_name = "D";
	table[4].channel_name = "E";
	table[5].channel_name = "F";
	table[6].channel_name = "G";
	table[7].channel_name = "H";
	table[8].channel_name = "I";
	table[9].channel_name = "J";

	//intializing subscriber relay entries
	const int capacity = 10;
	int i, j;
	for (i = 0; i < capacity; ++i)
	{
		for (j = 0; j < capacity; ++j)
		{	
			relay_entry * table_entry = &table[i].subscribers[j];
			table_entry->occupied = 0;
			table_entry->position = j;
			memset(&table_entry->relay_addr, 0, sizeof(table_entry->relay_addr));
			
		}
		table[i].chanbuff="";
		
	}
}

//this method is not called in the code written in this file
/*
void subscribe_to_channel(char *chann_req, struct sockaddr_in *subscriber_addr)
{
	int capacity = 10;
	int n;
	for(n = 0; n<capacity; ++n){
		if(*channel_table[n].channel_name == *chann_req){
			int j = 0;
			while(channel_table[n].subscribers[j].occupied==1){
				++j;
				if(j==10){
					fprintf(stderr, "%s\n", "The requested channel is full.");
					exit(1);
				}
			}
			relay_entry * table_entry = &channel_table[n].subscribers[j];
			table_entry->occupied = 1;
			table_entry->relay_addr = *subscriber_addr;
			table_entry->port_number = ntohs(subscriber_addr->relay_addr.sin_port);
			break;

		}
	}
}
*/



//add to relay table
struct sockaddr_in add_to_relay(struct sockaddr_in *addr)
{

	int j = 0;
	while(relay_table[j].occupied==1)
	{
		j++;		
		if(j==10)
		{	
			relayflag=1;
			fprintf(stderr, "%s\n", "The relay is full.");
			exit(1);
		}
	}
	
	//printf("%d",j);

	relay_entry * table_entry = &relay_table[j];
	table_entry->occupied = 1;
	table_entry->relay_addr = *addr;
	int x = table_entry->port_number;
	table_entry->relay_addr.sin_port = htons(x);

	return table_entry->relay_addr;
}


void serve(int fd, struct sockaddr_in *addr)
{	
	
	int relay_socket;	

	//setting up new socket for relay
	if ((relay_socket = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		perror("socket");
		exit(1);
	}

	//adding client to relay table and obtaining new port for allocation
	struct sockaddr_in new_addr = add_to_relay(addr);
	int x = new_addr.sin_port;
	int allocated_port = ntohs(x);
	printf("Client will now be given the new port number on which to connect: %d\n", allocated_port);

	//writing the new port number to the client while handling possible errors
	if(write(fd, &allocated_port, sizeof(allocated_port))==-1)
	{
		perror("write");
		exit(1);
	}

//	printf("Did client connect on: %d?\n", allocated_port);

	//binding new address to the new socket
	if (bind(relay_socket, (struct sockaddr *)&new_addr, sizeof(new_addr)) < 0) 
	{
		perror("bind");
		exit(1);
	}


	//put new socket into listening mode
	if (listen(relay_socket, 5)==-1)
	{
		perror("listen");
		exit(1);
	}

	//accepting new connection to new socket

	int csock;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	if((csock = accept(relay_socket,
		(struct sockaddr *)&client_addr, &client_len))==-1)
	{
		perror("accept");
		exit(1);
	}

/*
lets build the data juggling: fd varies by socket

	
	while(1)
	{
		int n;
		for (n=0; n< ;n++)
		{
			
		}
	}
*/

//everything after this comment thru end of serve method is just proof of basic functions	
	printf("New socket has received connection from %s\n",		
		inet_ntoa(client_addr.sin_addr));

	

	//testing new socket connection by writing to its socket
	if (write(csock, &allocated_port, sizeof(allocated_port))==-1)
	{
		perror("write");
		exit(1);
	}

	//receive a message from client
	char buff[1024];
	if (read(csock, buff, sizeof(buff))==-1)
	{
		perror("read");
		exit(1);
	}

	printf("%s\n", buff);




	/*
	By now, a new connection should have been established with the client using the new port number. This is where we ask the client for which channel they would like to broadcast on. Once we have this we then call subscribe_to_channel using this channel and the address stored in new_addr above.

	*/
}



int main(int argc, char const *argv[])
{

	
	void setUpRelayTable(relay_entry[]);
	void setUpChannelTable(channel[]);
	

	//setting up
	setUpRelayTable(relay_table);
	setUpChannelTable(channel_table);


	//declaring variables and methods for later use
	int lsock;

	//setting up our address
	struct sockaddr_in my_addr;
	memset(&my_addr, 0, sizeof(my_addr));

	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	my_addr.sin_port = htons(atoi(argv[1]));

	//error-handling
	
	if (argc != 2)
	{
		fprintf(stderr, "%s\n", "Usage: server port");
		exit(1);
	}

	//creating our socket
	if ((lsock = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		perror("socket");
		exit(1);
	}

	//binding address to socket
	if(bind(lsock, (struct sockaddr *)&my_addr, sizeof(my_addr))==-1)
	{
		perror("bind");
		exit(1);
	}

	//putting socket into listening mode
	if (listen(lsock, 5)==-1)
	{
		perror("listen");
		exit(1);
	}
	
	printf("Please initialize client now \n");

	//accepting connection
	while(1)
	{
		int csock;
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		//setting up client socket and accepting pending connection
		csock = accept(lsock, (struct  sockaddr *)&client_addr, &client_len);

		//acknowledgement
		printf("Received connection from %s\n", inet_ntoa(client_addr.sin_addr));

		//forking
		switch(fork())
		{
			case -1:
				perror("fork");
				exit(1);
			case 0:
				serve(csock, &client_addr);
				//closing the connection to the old socket
				if (close(csock)==-1)
				{
					perror("close");
					exit(1);
				}
				if (relayflag == 1)
				{
					close(lsock);
				}
				break;
			default:
				return 0;
				close(csock);
				break;
		}
		
	}
	return 0;
}

