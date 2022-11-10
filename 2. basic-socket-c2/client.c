#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_LEN 4096        // Length in bytes of the messages which can be sent and received by the socket
#define IP_ADDR "127.0.0.1"    // Loopback address used for testing
#define PORT 4444              // Testing port to be used for communication between the client and server

/*
  Name of Function: main(void)
  Parameters in Function: void
  Return of Function: int, 0 if successful
  Purpose of Function: Initializes the client socket, prompts for an inputted message, and sends it to the client
*/
int main(void){
  fflush(stdin);             // Clears the output buffer
  int client_socket = -1;    // Initializes the client socket integer

  struct sockaddr_in server_addr;                     // Initializes a socket address 
  client_socket = socket(AF_INET, SOCK_STREAM, 0);    // Creates a client socket which is an IPv4 running over TCP

  server_addr.sin_family = AF_INET;                   // Sets the socket address family to IPv4
  server_addr.sin_port = htons(PORT);                 // Sets the port number to communicate over to 4444
  server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);   // Sets the IP address to communicate over to the loopback address

  connect(client_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));  // Connects the client socket to the server address

  char msg[BUFFER_LEN];                      // Initializes a string of the buffer length called msg
  printf("Enter message to send:\t");        // Prints the message: "Enter message to send:  "
  fgets(msg, BUFFER_LEN, stdin);             // Receives the input from stdin with length BUFFER_LEN and stores it in msg
  while(strcmp(msg, "quit\n") != 0){
    fflush(stdin);                             // Clears the output buffer
    
    send(client_socket, msg, BUFFER_LEN, 0);   // Sends the message from the client_socket from msg
    memset(msg, 0, BUFFER_LEN);  
    recv(client_socket, msg, BUFFER_LEN, 0);   // Receives the message from the client_socket and stores it in msg
    printf("%s\n", msg);                       // Prints msg, or what was received from the client socket

    memset(msg, 0, BUFFER_LEN);                // Sets all of the bytes of buffer to 0
    printf("Enter message to send:\t");        // Prints the message: "Enter message to send:  "
    fgets(msg, BUFFER_LEN, stdin);             // Receives the input from stdin with length BUFFER_LEN and stores it in msg
  }
  close(client_socket);
}
