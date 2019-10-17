#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "spchk_header.h"
#define MAX_CONNECTIONS 30

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
  if(listen(socket_desc, MAX_CONNECTIONS) < 0 ){
    return -1;
  }

  return socket_desc;

}
char ** load_dictionary(char * file_name){
  //puts("Nothing here yet!");
  char line_holder [1000];
  char ** dictionary =  NULL;
  FILE * file_pointer;
  size_t dict_size = 1000000; //approximate for the number of words

  dictionary = malloc(dict_size * sizeof(char *) );
  file_pointer = open(file_name, "r");
  
  if(dictionary == NULL){
    perror("Failed to allocate resources");
    return NULL;
  }
  size_t i = 0;
  while(fgets(line_holder, 1000, file_pointer)){
    strncpy(dictionary[i], line_holder, strlen(line_holder) -  1);
    i++;

  }

  dictionary[i] =  NULL;
  
  

  return my_dictionary;
  
  

}
int main(int argc, char ** argv){
  /* int test_connection = accept_connection(9150); //just do some testing for connection
     printf("%d\n", test_connection); */ //if it return any postive int --> success in creating a socket descriptor
  
}
