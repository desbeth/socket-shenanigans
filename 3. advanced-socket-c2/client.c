#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <poll.h>

#define BUFFER_LEN 8192  // Length in bytes of the messages which can be sent and received by the socket
#define IP_CHAR_LEN 20   // 

/*
  Name of Function: createSocket(char* ip_address, int port)
  Parameters in Function: char* ip_address, the IP address which the socket will be connected to
                          int port, the port which the socket will be connected to 
  Return of Function: int, the socket's file descriptor
  Purpose of Function: Initializes the client socket, prompts for an inputted message, and sends it to the client
*/
int createSocket(char* ip_address, int port){
  ip_address = strtok(ip_address, " \n");  // Strips the ip_address of any spaces or new lines

  int server_socket = -1;                 // // Initializes the server socket integer
  server_socket = socket(AF_INET, SOCK_STREAM, 0); // Creates a client socket which is an IPv4 running over TCP
 
  struct sockaddr_in server_addr; // Initializes a socket address

  server_addr.sin_family = AF_INET; // Sets the socket address family to IPv4
  server_addr.sin_port = htons(port); // Sets the port number to communicate over based on the parameter inputted
  server_addr.sin_addr.s_addr = inet_addr(ip_address); // // Sets the IP address to communicate over based on the parameter inputted

  if(connect(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
    perror("Connect failed.\n");  // If success is less than zero, meaning it failed, it prints out "Connect failed" to what was stderr (now client socket) 
    exit(1);               // Exits with exit code 1 (container shutdown either because of a failure or invalid file)
  }

  return server_socket;    // Returns the file descriptor called server_socket
}

/*
  Name of Function: main(void)
  Parameters in Function: none
  Return of Function: int, 0 if successful
  Purpose of Function: Prompt the user for IP address and the port to connect to, makes the connection, then allows for execution of commands on the IP address
  NOTE FROM THE AUTHOR: If you type "quit" and click enter, it will reprompt for a new IP address and port, meaning you can connect to multiple IP addresses, one-at-a-time
*/
int main(void){
  while(1){ 
  char ip[IP_CHAR_LEN]; // Initializes a string of the buffer length called ip
  printf("What IP address would you like to connect to?\t"); // Prints out: "What IP address would you like to connect to?    "
  fgets(ip, IP_CHAR_LEN, stdin); // Receives the input from stdin with length IP_CHAR_LEN and stores it in ip
  fflush(stdin);  // Clears the output buffer

  int port;                     // Initializes an integer called port
  printf("What port would you like to connect to?\t");  // Prints out: "What port would you like to connect to?   "
  scanf("%d", &port);           // Receives the input from stdin as an integer and stores it at the memory address of port

  while ((getchar()) != '\n');  // Clears out the buffer

  int socket = createSocket(ip, port);  // Creates a socket and receives a file descriptor stored in 'socket'

  char msg[BUFFER_LEN];        // Initializes a string of the buffer length called msg
  memset(msg, 0, BUFFER_LEN);  // Sets all of the bytes of msg to 0

  printf("Enter command you would like to send:\t");  // Prints out: "Enter command you would like to enter:   "
  fgets(msg, BUFFER_LEN, stdin);                       // Receives the input from stdin with length BUFFER_LEN and stores it in msg
  while(strcmp(msg, "quit\n") != 0){
      fflush(stdin);                                 // Clears the output buffer
      send(socket, msg, BUFFER_LEN, 0);              // Sends the message from the client_socket from msg

      struct pollfd polling = {socket, POLLIN, 0};   // Creates a poll struct called polling 
      while(poll(&polling, 1, 500) > 0){             // Sees if a file descriptor has something to read, timeout is 500 milliseconds because we're trying to wait for data to come in
      recv(socket, msg, BUFFER_LEN, 0);              // Receives the input from the socket with length BUFFER_LEN and stores it in msg
      printf("%s", msg);                             // Prints msg, or what was received from the client socket
      memset(msg, 0, BUFFER_LEN);                    // Sets all of the bytes of msg to 0
    }

    printf("\nEnter command you would like to send:\t");   // 
    fgets(msg, BUFFER_LEN, stdin);          // Receives the input from stdin with length BUFFER_LEN and stores it in msg
  }
  send(socket, msg, BUFFER_LEN, 0);   // Sends the message from the client_socket from msg
  close(socket);                      //
}
  return 0;
}
