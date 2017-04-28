#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */

#define BUFFER_SIZE 512 /* Size of receive buffer */
void DieWithError(char *errorMessage); /* Error handling function */

int main(int argc, char *argv[]) {
    int sock; /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort; /* Echo server port */
	int choice; /* the choice the user enters */
	int isLoggedOn = 0; // 0 is user is not logged on, 1 otherwise
	int bytesReceived; // used when recv() is called
	char buffer[BUFFER_SIZE]; // buffer used for most cases
	char username[BUFFER_SIZE];
	char choice_str[BUFFER_SIZE]; // will hold the user's choice

	for (;;) {
		printf("---------------------------------------------------\n");
		printf("Command:\n");
		printf("0. Connect to the server\n");
		printf("1. Get the user list\n");
		printf("2. Send a message\n");
		printf("3. Get my messages\n");
		printf("4. Initial a chat with my friend\n");
		printf("5. Chat with my friend\n");
		printf("Your option<enter a number>: ");
		fgets(choice_str, BUFFER_SIZE, stdin);
		choice = atoi(choice_str);
		if (choice == 0) {
			char ip_str[BUFFER_SIZE]; // store the ip address
			char port_str[BUFFER_SIZE]; // store the port number
			printf("Please enter the IP address: ");
			fgets(ip_str, BUFFER_SIZE, stdin);
			printf("Please enter the port number: ");
			fgets(port_str, BUFFER_SIZE, stdin);
			printf("Connecting.....\n");
			
			/* Create a reliable, stream socket using TCP */
			if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
				DieWithError("socket () failed");
		

			/* Construct the server address structure */
			memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
			echoServAddr.sin_family = AF_INET; /* Internet address family */
			echoServAddr.sin_addr.s_addr = inet_addr(ip_str); /* Server IP address */
			echoServAddr.sin_port = htons(atoi(port_str)); /* Server port */

			/* Establish the connection to the echo server */
			if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
				DieWithError("connect () failed");	

			printf("Connected!\n");
			printf("Welcome! Please log in.\n");
			printf("Username: ");
			
			fgets(username, BUFFER_SIZE, stdin);

			printf("Password: ");
			char password[BUFFER_SIZE];
			fgets(password, BUFFER_SIZE, stdin);
		
			// send choice to server
			if (send(sock, choice_str, strlen(choice_str), 0) < 0)
				DieWithError("send() sent a different number of bytes than expected for choice");
			
			sleep(1);
			
			// send the username and password for validation to server
			if (send(sock, username, strlen(username), 0) < 0)
				DieWithError("send() sent a different number of bytes than expected for username");
			
			sleep(1);
			
			if (send(sock, password, strlen(password), 0) < 0)
				DieWithError("send() sent a different number of bytes than expected for password");
			
			// get from server whether login was successful or not and convert to int
			char isSuc[2];
			if ((bytesReceived = recv(sock, isSuc, 1, 0)) < 0)
				DieWithError("recv() failed");
			
			isSuc[bytesReceived] = '\0';
			isLoggedOn = atoi(isSuc);
			
			if (isLoggedOn == 0)
				printf("Log in failed.\n");
			else if (isLoggedOn == 1)
				printf("Log in successful.\n");
			
		} else if (choice == 1) { // hard code users
			if (isLoggedOn == 1) {
				if (send(sock, choice_str, strlen(choice_str), 0) < 0)
					DieWithError("send() sent a different number of bytes than expected for choice");
			
				printf("There are 2 users\n");
				printf("Alice\n");
				printf("Bob\n");
			} else {
				printf("You must log in first\n");
			}
		} else if (choice == 2) {
			if (isLoggedOn == 1) {
				printf("Please enter the user name: ");
				char u_name[BUFFER_SIZE]; // will hold the name of user to be sent a message
				fgets(u_name, BUFFER_SIZE, stdin);
				printf("Please enter the message: ");
				bzero(buffer, BUFFER_SIZE);
				fgets(buffer, BUFFER_SIZE, stdin);
				
				if (send(sock, choice_str, strlen(choice_str), 0) < 0)
					DieWithError("send() sent a different number of bytes than expected for choice");
			
				sleep(1);
				
				if (send(sock, u_name, strlen(u_name), 0) < 0)
					DieWithError("send() sent a different number of bytes than expected");
			
				sleep(1);
				
				if (send(sock, buffer, strlen(buffer), 0) < 0)
					DieWithError("send() sent a different number of bytes than expected");
				
				printf("Message sent successfully\n");
			
			} else {
				printf("You must log in first\n");
			}
		
		} else if (choice == 3) {
			if (isLoggedOn == 1) {
				if (send(sock, choice_str, strlen(choice_str), 0) < 0)
					DieWithError("send() sent a different number of bytes than expected for choice");
			
				// get the number of new messages from the user and convert to int
				char num_msgs[2];
				if ((bytesReceived = recv(sock, num_msgs, 1, 0)) < 0)
					DieWithError("recv() failed");
				
				num_msgs[bytesReceived] = '\0';
				int messages = atoi(num_msgs);
				printf("You have %d message(s)!\n", messages);
				int i;
				for (i = 0; i < messages; i++) {
					bzero(buffer, BUFFER_SIZE);
					if ((bytesReceived = recv(sock, buffer, BUFFER_SIZE-1, 0)) < 0)
						DieWithError("recv() failed");
					
					buffer[bytesReceived] = '\0';
					printf("%s", buffer);
				} 
			} else {
				printf("You must log in first\n");
			}
		} else if (choice == 4) {
			if (isLoggedOn == 1) {
				if (send(sock, choice_str, strlen(choice_str), 0) < 0)
					DieWithError("send() sent a different number of bytes than expected for choice");
				close(sock);
				printf("-------Disconnected from server-------\n");
				int servSock; /* Socket descriptor for server */
				int clntSock; /* Socket descriptor for client */
				struct sockaddr_in echoServAddr; /* Local address */
				struct sockaddr_in echoClntAddr; /* Client address */
				unsigned int clntLen; /* Length of client address data structure */
				printf("Enter the port number you want to listen on: ");
				char portNo[BUFFER_SIZE];
				fgets(portNo, BUFFER_SIZE, stdin);
				if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
					DieWithError( "socket () failed");

				/* Construct local address structure */
				memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
				echoServAddr.sin_family = AF_INET; /* Internet address family */
				echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
				echoServAddr.sin_port = htons(atoi(portNo)); 

				/* Bind to the local address */
				if (bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
					DieWithError ( "bind () failed");
				/* Mark the socket so it will listen for incoming connections */
				if (listen(servSock, 5) < 0)
					DieWithError("listen() failed");
				
				printf("Listening on 127.0.0.1:%d\n", atoi(portNo));
				
				clntLen = sizeof(echoClntAddr);
				/* Wait for a client to connect */
				if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
					DieWithError("accept() failed");
				
				char friend[BUFFER_SIZE]; // will hold the name of the friend
				if ((bytesReceived = recv(clntSock, friend, BUFFER_SIZE-1, 0)) < 0) 
					DieWithError("recv() failed");
				
				friend[strcspn(friend, "\r\n")] = 0; // remove new line
				send(clntSock, username, strlen(username), 0);
				username[strcspn(username, "\r\n")] = 0; // remove new line

				printf("%s is connected\n", friend);
				for (;;) { 
					// recv
					// display
					// get input
					// send
					
					printf("<Type \"Bye\" to stop the conversation>\n");
	
					bzero(buffer, BUFFER_SIZE);
					if ((bytesReceived = recv(clntSock, buffer, BUFFER_SIZE-1, 0)) < 0) 
						DieWithError("recv() failed");
					
					buffer[bytesReceived] = '\0';
					printf("%s: %s", friend, buffer);
					
					if (strncmp(buffer, "Bye", 3) == 0) {
						printf("Disconnected from my friend!\n");
						close(clntSock);
						break;
					}
				
					printf("%s: ", username);
					bzero(buffer, BUFFER_SIZE);
					fgets(buffer, BUFFER_SIZE, stdin);

					if (send(clntSock, buffer, strlen(buffer), 0) < 0) 
						DieWithError("send() failed");
					
					
					if (strncmp(buffer, "Bye", 3) == 0) {
						printf("Disconnected from my friend!\n");
						close(clntSock);
						break;
					}
				}
			} else {
				printf("You must log in first\n");
			}
			
		} else if (choice == 5) {
			if (isLoggedOn == 1) {
				printf("-------Disconnected from server-------\n");
				close(sock);
				char ip_str[BUFFER_SIZE];
				char port_str[BUFFER_SIZE];
				printf("Please enter the IP address of your friend: ");
				fgets(ip_str, BUFFER_SIZE, stdin);
				printf("Please enter the port number of your friend: ");
				fgets(port_str, BUFFER_SIZE, stdin);
				printf("Connecting.....\n");
				
				/* Create a reliable, stream socket using TCP */
				if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
					DieWithError("socket () failed");
			

				/* Construct the server address structure */
				memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
				echoServAddr.sin_family = AF_INET; /* Internet address family */
				echoServAddr.sin_addr.s_addr = inet_addr(ip_str); /* Server IP address */
				echoServAddr.sin_port = htons(atoi(port_str)); /* Server port */

				/* Establish the connection to the echo server */
				if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
					DieWithError("connect () failed");	
					
				printf("Connected!\n");
				send(sock, username, strlen(username), 0);
				username[strcspn(username, "\r\n")] = 0; // remove new line

				char friend[BUFFER_SIZE]; // will hold the name of the friend
				recv(sock, friend, BUFFER_SIZE-1, 0);
				friend[strcspn(friend, "\r\n")] = 0; // remove new line

				
				for (;;) {
					// get input
					// send
					// recv
					// display
					
					printf("<Type \"Bye\" to stop the conversation>\n");
					printf("%s: ", username);
					fgets(buffer, BUFFER_SIZE, stdin);
	
					if (send(sock, buffer, strlen(buffer), 0) < 0) 
						DieWithError("send() failed");
					
					
					if (strncmp(buffer, "Bye", 3) == 0) {
						printf("Disconnected from my friend!\n");
						close(sock);
						break;
					}
					
		
					bzero(buffer, BUFFER_SIZE);
					
					if ((bytesReceived = recv(sock, buffer, BUFFER_SIZE-1, 0)) < 0) 
						DieWithError("recv() failed");
					
					buffer[bytesReceived] = '\0';
					printf("%s: %s", friend, buffer);
					
					if (strncmp(buffer, "Bye", 3) == 0) {
						printf("Disconnected from my friend!\n");
						close(sock);
						break;
					}
				}
			} else {
				printf("You must log in first\n");
			}
				
		}


	}
	


    printf("\n"); /* Print a final linefeed */

    close(sock);
    exit(0);
}
