#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "spchk_header.h"
#define MAX_CONNECTIONS 30
int main(int argc, char ** argv){
  puts("Hello");

}
int accept_connection(int port_number){
  
  struct sockaddr_in server_address;
  
  int socket_desc;

  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if(socket_desc < 0 ){
    perror("Error creating socket");
    return -1;

  }
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_address.sin_port = htons(port_number);


  if (bind(socket_desc, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
    perror("Failed to connect");
    return -1;
  }
  if( listen(socket_desc, MAX_CONNECTIONS) < 0 ){
    return -1;
  }

  return socket_desc;

}
void load_dictionary(char * file_name){
  puts("Nothing here yet!");

}
