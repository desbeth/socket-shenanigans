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
  Name of Function: handle_client(int client_socket)
  Parameters in Function: int client_socket, representing the established socket which the client has accepted the server socket's connection
  Return of Function: 0 if successful

  Purpose of Function: Receive a message from the client socket and print it out onto the server's command line
*/
int handle_client(int client_socket){
  char buffer[BUFFER_LEN];                      // Initializes a string of the buffer length called buffer
  recv(client_socket, buffer, BUFFER_LEN, 0);   // Receives the message from the client_socket and stores it in buffer
  printf("%s\n", buffer);                       // Prints out the message (buffer) received from the socket

  return 0;                                     // Returns 0 if successful
}

/*
  Name of Function: handle_server(void)
  Parameters in Function: void
  Return of Function: int, 0 if successful

  Purpose of Function: Initialize the server and client sockets, and then proceed to handling the client
*/
int handle_server(void){
  int server_socket = -1;    // Initializes the server socket integer
  int client_socket = -1;    // Initializes the client socket integer

  struct sockaddr_in server_addr;                     // Initializes a socket address 
  server_socket = socket(AF_INET, SOCK_STREAM, 0);    // Creates a server socket which is an IPv4 running over TCP

  server_addr.sin_family = AF_INET;                   // Sets the socket address family to IPv4
  server_addr.sin_port = htons(PORT);                 // Sets the port number to communicate over to 4444
  server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);   // Sets the IP address to communicate over to the loopback address

  bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr));  // Binds the server socket to the information specified in the server address
  listen(server_socket, 5);                           // Listens on the server socket with up to 5 possible connections

  client_socket = accept(server_socket, NULL, NULL);  // Creates a new socket with accepts a new connection on the server socket
  handle_client(client_socket);                       // Calls the handle_client method with the client socket

  return 0;
  }

/*
  Name of Function: main(void)
  Parameters in Function: void
  Return of Function: int, 0 if successful

  Purpose of Function: Calls the handle_server() function
*/
int main(void){
  handle_server();
}
