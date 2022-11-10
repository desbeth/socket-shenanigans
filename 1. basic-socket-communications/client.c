#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_LEN 1024        // Length in bytes of the messages which can be sent and received by the socket
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

  connect(client_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));  //

  char msg[BUFFER_LEN];                      // Initializes a string of the buffer length called msg
  printf("Enter message to send:\t");        // Prints the message: "Enter message to send:  "
  fgets(msg, BUFFER_LEN, stdin);             // Receives the input from stdin with length BUFFER_LEN and stores it in msg

  fflush(stdin);             // Clears the output buffer

  send(client_socket, msg, strlen(msg), 0);  // Sends the message (msg) from the client_socket 
}
