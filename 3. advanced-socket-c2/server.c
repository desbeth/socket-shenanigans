#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_LEN 8192        // Length in bytes of the messages which can be sent and received by the socket
#define IP_ADDR "0.0.0.0"      // Broadcast address
#define PORT 4444              // Testing port to be used for communication between the client and server

/*
  Name of Function: parse_command(char *cmd)
  Parameters in Function: char *cmd, 
  Return of Function: char** args, the command parsed by spaces 
  Purpose of Function: Takes a command and parses it by the spaces, then returns the parsed array
*/
char** parse_command(char *cmd){
  // We don't malloc everything at the beginning because we don't know how many tokens there will be
  int token_num = 0;                 // Initializes the token_num integer
  char **args = NULL;                // Initializes the double-pointer argument array
  char *token = strtok(cmd, " \n");  // Initializes the token character pointer to the instance before a space or new line in cmd, then replaces the first instance with a null terminator
  
  while (token != NULL){
    args = realloc(args, (token_num + 1) * sizeof(char*));  // Since we're increasing the character pointer, we have to reallocate to account for it in args
    args[token_num++] = token;                              // Assigns the current value of args[token_num] to token, then increases token_num by 1
    token = strtok(NULL, " \n");                            // Using the same string (cmd), we move the token pointer to the next instance of a new line or space, and replace the next instance with a null terminator
  }
  args = realloc(args, (token_num + 1) * sizeof(char *));   // For the final instance, we reallocate to account for one more char pointer
  args[token_num] = NULL;                                   // We null-terminate the array
  return args;                                              // If successful, it returns the parsed arguments
}

/*
  Name of Function: handle_client(int client_socket)
  Parameters in Function: int client_socket, representing the established socket which the client has accepted the server socket's connection
  Return of Function: 0 if successful
  Purpose of Function: Receive a message from the client socket and print it out onto the server's command line
*/
int handle_client(int client_socket){
  int fd[3] = {0,1,2};          // Instantiates an array with file descriptors: 0 (stdin), 1 (stdout), and 2 (stderr)
  dup2(client_socket, fd[1]);   // Redirects stdout output to the client socket
  dup2(client_socket, fd[2]);   // Redirects stderr output to the client socket

  char buffer[BUFFER_LEN];                      // Initializes a string of the buffer length called buffer
  memset(buffer, 0, BUFFER_LEN);                // Sets all of the bytes of buffer to 0
  recv(client_socket, buffer, BUFFER_LEN, 0);   // Receives the message from the client_socket and stores it in buffer

  while(strcmp(buffer, "quit\n") != 0){
    char **args = parse_command(buffer);        // Parses the command, buffer, and stores the double character pointer in args

    pid_t success = fork();                     // Forks the parent process, creating a new child process with PID (process ID) success
    if (success < 0){                        
      perror("Fork failed.");                   // If success is less than zero, meaning it failed, it prints out "Fork failed" to what was stderr (now client socket) 
      exit(1);                                  // Exits with exit code 1 (container shutdown either because of a failure or invalid file)
    } else{
      if (success == 0){ 
        int execresult = execvp(args[0], args); // 
        if(execresult < 0){
          perror("Exec failed.");               // If execresult is less than zero, meaning it failed, it prints out "Exec failed" to what was stderr (now client socket) 
          exit(1);                              // Exits with exit code 1 (container shutdown either because of a failure or invalid file)
        }
      } else{
        wait(NULL);                             // Waits for a state change in a child process then obtains information about the child
      }
    }

  memset(buffer, 0, BUFFER_LEN);                // Sets all of the bytes of buffer to 0
  recv(client_socket, buffer, BUFFER_LEN, 0);   // Receives the message from the client_socket and stores it in buffer
  }

  close(client_socket);                         // Closes the connection to the client socket before returning
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

  while (1){
    client_socket = accept(server_socket, NULL, NULL);  // Creates a new socket with accepts a new connection on the server socket
    handle_client(client_socket);                       // Calls the handle_client method with the client socket
    
    close(client_socket);                               // Closes the client socket so we don't run out of file descriptors
 }

  return 0;      // If successful, the function returns 0
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

