#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "spchk_header.h"
#define MAX_CONNECTIONS 30
#define DICTIONARY "dictionary_words.txt"
#define NUM_WORDS 99712
#define NUM_LINE 128
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
  char ** dictionary;
  char line[NUM_LINE];
  FILE * file_pointer;
  
  
  size_t i = 0;
  if ((dictionary = malloc(NUM_WORDS * sizeof(char *))) == NULL ){
    perror("ERROR: UNABLE TO ALLOCATE RESOURCES");
    return NULL;
  }
  file_pointer = fopen(file_name, "r");
  while(fgets(line, sizeof(line), file_pointer )){
    if( (dictionary[i] = malloc(strlen(line) * sizeof(char)  +  1)) == NULL){
      perror("ERROR LOADING WORDS");
      return NULL;
    }
    strncpy(dictionary[i++], line, strlen(line) - 1);


  }
  dictionary[i] = NULL;
  return dictionary;

}
int checkSpelling(char ** dictionary, char * word){
  size_t i;
  for(i = 0; i < NUM_WORDS; i++){
    if(strcmp(dictionary[i], word) == 0){
      return 1;
    }

  }
  return 0;

}
int main(int argc, char ** argv){
  /* int test_connection = accept_connection(9150); //just do some testing for connection
     printf("%d\n", test_connection); */ //if it return any postive int --> success in creating a socket descriptor
  char ** words_test = load_dictionary("dictionary_words.txt");
  /*for(int i = 0; words_test[i] != '\0'; i++)
    {
      printf("\n Element is %s", words_test[i]);
    }
  */

}
