#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h> /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h> /* for atoi() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */

#define MAXPENDING 5 /* Maximum outstanding connection requests */
#define BUFFER_SIZE 512 // size of receive buffer
#define BUF20 20  // smaller size of 20 for buffer

void DieWithError(char *errorMessage); /* Error handling function */
void HandleTCPClient(int clntSocket); /* TCP client handling function */

int main(int argc, char *argv[]) {
	
    int servSock; /* Socket descriptor for server */
    int clntSock; /* Socket descriptor for client */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int clntLen; /* Length of client address data structure */
	char *users[2] = {"Alice", "Bob"};
	char *passwords[2] = {"12345", "56789"};
	int bytesReceived; // will be used for recv() to check if error occurred
	int newClient = 1; // 1 if server is handling a new client, 0 otherwise
	char *bob_messages[10]; // keep track of all messages for bob
	char *alice_messages[10]; // keep track of all messages for alice
	int total_bob_messages = 0; // how many new messages for bob (max 10 )
	int total_alice_messages = 0; // how many new messages for alice (max 10)
	char user_buf[BUF20]; // the user name
	int i; // loop counter
	printf("Trying to get server started....\n");
	
	/* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
         DieWithError( "socket () failed");

	/* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
    echoServAddr.sin_family = AF_INET; /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(8000); /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError ( "bind () failed");
    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");
	
	printf("Server started!\n");
	printf("Listen on 127.0.0.1:8000\n");
	
    for(;;) {
		if (newClient == 1) {
			/* Set the size of the in-out parameter */
			clntLen = sizeof(echoClntAddr);
			/* Wait for a client to connect */
			if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
				DieWithError("accept() failed");
			/* clntSock is connected to a client! */
			printf("Handling client\n");
			newClient = 0;
		}	
		
		
		char choice_buf[BUF20]; // the choice the user enters

		if ((bytesReceived = recv(clntSock, choice_buf, BUF20-1, 0)) < 0)
			DieWithError("recv() failed");
		choice_buf[bytesReceived] = '\0';

		int choice = atoi(choice_buf); 
		
		if (choice == 0) {
			if ((bytesReceived = recv(clntSock, user_buf, BUF20-1, 0)) < 0)
				DieWithError("recv() failed");
			
			user_buf[bytesReceived] = '\0';
			
			char pw_buf[BUF20]; // password for user

			if ((bytesReceived = recv(clntSock, pw_buf, BUF20-1, 0)) < 0)
				DieWithError("recv() failed");
			
			pw_buf[bytesReceived] = '\0';
			
			int isSuccess = 0; // 0 if user was not able to log in successfully, 1 otherwise
			for (i = 0; i < 2; i++) {
				if (strncmp(user_buf, users[i], strlen(users[i])) == 0 && strncmp(pw_buf, passwords[i], strlen(passwords[i])) == 0) {
					printf("Log in successful!\n");
					printf("Username: %s\n", users[i]);
					printf("Password: %s\n", passwords[i]);
					isSuccess = 1;
				}
			}
			
			if (isSuccess == 0) {
				printf("Log in failed.\n");
				isSuccess = 0;
			}
			
			// convert isSuccess to char array to send to client
			char isSuc[2];
			sprintf(isSuc, "%d", isSuccess);
			if (send(clntSock, isSuc, strlen(isSuc), 0) <0)
				DieWithError("send() failed");
			
		} else if (choice == 1) {
			printf("Return user list!\n"); // hard code user list in client (no need to send)
			
		} else if (choice == 2) {
			char u_name[BUF20]; // will hold who the message is for
			if ((bytesReceived = recv(clntSock, u_name, BUF20-1, 0)) < 0)
				DieWithError("recv() failed");
			u_name[bytesReceived] = '\0';
			
			char message[BUFFER_SIZE]; // used for holding message
			if ((bytesReceived = recv(clntSock, message, BUFFER_SIZE-1, 0)) < 0)
				DieWithError("recv() failed");
			message[bytesReceived] = '\0';
			printf("A message to %s", u_name);
			printf("Message is: %s", message);
			
			// check who the message is for and respectively add it to their array and increment total messages
			if (strncmp("Alice", u_name, strlen("Alice")) == 0) {
				alice_messages[total_alice_messages] = message;
				total_alice_messages++;
			} else if (strncmp("Bob", u_name, strlen("Bob")) == 0) {
				bob_messages[total_bob_messages] = message;
				total_bob_messages++;
			}
			
		
		} else if (choice == 3) {
			printf("Send back message(s) for %s", user_buf);
			
			// check who is requesting message and send respective messages back
			if (strncmp("Alice", user_buf, strlen("Alice")) == 0) {
				char msg_count[2];
				sprintf(msg_count, "%d", total_alice_messages);
				if (send(clntSock, msg_count, strlen(msg_count), 0) < 0)
					DieWithError("send() failed");
				
				// send all messages
				for (i = 0; i < total_alice_messages; i++) {
					if (send(clntSock, alice_messages[i], strlen(alice_messages[i]), 0) < 0) 
						DieWithError("send() failed");
					
					sleep(1);
				}
				
				// reset values
				bzero(alice_messages, 10);
				total_alice_messages = 0;
				
			} else if (strncmp("Bob", user_buf, strlen("Bob")) == 0) {
				char msg_count[2];
				sprintf(msg_count, "%d", total_bob_messages);
				if (send(clntSock, msg_count, strlen(msg_count), 0) < 0)
					DieWithError("send failed()");
				
				// send all messages
				for (i = 0; i < total_bob_messages; i++) {
					if (send(clntSock, bob_messages[i], strlen(bob_messages[i]), 0) < 0)
						DieWithError("send() failed");
					
					sleep(1);
				}
				
				// reset values
				bzero(bob_messages, 10);
				total_bob_messages = 0;
			}
			
			
		} else if (choice == 4 || choice == 5) {
			// close connection
			close(clntSock);
			printf("Client has disconnected\n");
			newClient = 1;
		} 
     
    }
/* NOT REACHED */

}
