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
//GLOBAL VARIABLES TO USE
FILE * log_output;
char ** dictionary;
clientsQueue *  client_q;
logsQueue * log_q;
//END OF GLOBAL VARIABLES

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
char ** load_dictionary(char * file_name){ //input file a list of dictionary words ; output an array of words in the dictionary
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

int checkSpelling(char * word){
  size_t i;
  for(i = 0; i < NUM_WORDS; i++){
    if(strcmp(dictionary[i], word) == 0){
      return 1;
    }

  }
  return 0;

}
void produce_client(clientsQueue * client_queue, int client){
  //acquire a lock for this thread
  //run a while loop when number of clients in client queue is greater or equals to the max default queue size
  //make the process unlocked the mutex lock or wait until fill_check check is signal. Until not_fill is signaled, the thread is suspended 
  //place the client into client queue/socket
  //increment the number of client inside the client socket 
  //unlock the lock for this thread
  
  pthread_mutex_lock(&client_queue->lock);
  
  while(client_queue->client_num >= QUEUE_SIZE_DEFAULT){
    pthread_cond_wait(&client_queue->fill_check, &client_queue->lock);
    
  }
  client_queue->clients_in_socket[client_queue->client_num] = client; // put client in
  client_queue->client_num = client_queue->client_num + 1; //increment items in the buffer
  
  pthread_cond_signal(&client_queue->empty_check); //signal that the buffer is not empty!
  pthread_mutex_unlock(&client_queue->lock);

}

int get_client(clientsQueue * client_queue){
  //acquire a lock for this thread
  //run a while loop when a number of clients in client queue is less or equal than 0
  //make the process wait until the empty_check is signal. Until empty_check is signaled, the thread will be suspended
  //consume the client from the end of the  queue
  //decrement the number of client inside the client socket
  //unlock the lock for this thread
  int client = 0;
  
  pthread_mutex_lock(&client_queue->lock);
  
  while(client_queue->client_num <= 0 ){
    pthread_cond_wait(&client_queue->empty_check, &client_queue->lock);
  }
  
  client = client_queue->clients_in_socket[client_queue->client_num - 1];
  
  client_queue->client_num  = client_queue->client_num - 1;
  
  pthread_cond_signal(&client_queue->fill_check);
  
  pthread_mutex_unlock(&client_queue->lock);
  
  return client;
}

void produce_log(logsQueue * log_queue, char * log){
  //acquire a lock for this thread
  //while number of entries is equals or bigger than default queue size, suspend this thread
  //copy log file from quuee to file
  //increment the number of log in the log queue
  //signal the queue is not empty
  //unlock the lock
  pthread_mutex_lock(&log_queue->lock); //lock
  
  while(log_queue->log_num >= QUEUE_SIZE_DEFAULT){
    pthread_cond_wait(&log_queue->fill_check, &log_queue->lock);
  }
  
  strcpy(log_queue->log[log_queue->log_num], log); // copy item from buffer 
  
  log_queue->log_num = log_queue->log_num + 1; 
  
  pthread_cond_signal(&log_queue->empty_check); //signal that the buffer is not empty
  
  pthread_mutex_unlock(&log_queue->lock); //unlock
  
  
}


char * get_log(logsQueue * log_queue){
  char * return_log = (char *) malloc(sizeof(char) * 300);
  pthread_mutex_lock(&log_queue->lock); //acquire lock

  while(log_queue->log_num <= 0 ){ //condition variable: while log queue is empty, suspend threads
    pthread_cond_wait(&log_queue->empty_check, &log_queue->lock);   
  }
  strcpy(return_log, log_queue->log[log_queue->log_num - 1]); //consume log from queue
  log_queue->log_num = log_queue->log_num - 1;
  pthread_cond_signal(&log_queue->fill_check);



  pthread_mutex_unlock(&log_queue->lock);//release lock
  return return_log;

}
void * thread_work(void * vargp){
  while(1){
    int check_int;
    char message[] = "Connected. Enter a word to check spelling\n";
    char message_received[2000];
    char word_holder[2000];
    int client = get_client(client_q);
    send(client, message, strlen(message), 0);
    while(1){
      check_int = recv(client, message_received, 2000, 0);
      memset(message_received, '\0', 2000);
      memset(word_holder, '\0', 2000);
      if(check_int == -1){
	send(client, "Message not received", strlen("Message not received"), 0);
	
      }else{
	if(message_received[0] == 27){
	  break;

	}else{
	  for(int i = 0; i < 2000; i++){
	    if(message_received[i] == '\r' || message_received[i] == '\n'){
	      message_received[i] = '\0';
	      word_holder[i] = '\0';
	    }
	    word_holder[i] = message_received[i];
	  }
	  strcpy(word_holder, message_received);
	  if(checkSpelling(word_holder)== 0){
	    strcat(message_received, "  : Incorrectly spelled\n");
	    send(client, message_received, strlen(message_received), 0);
	  }else{
	    strcat(message_received, "  : Correct\n");
	    send(client, message_received, strlen(message_received), 0);
	  }
	  produce_log(log_q, message_received);
	}


      }


      

    }


  }
  return NULL;


}
void * server_log(void * vargp){
  char * log;
  while(1){
    log = get_log(log_q);
    log_output = fopen("logs_output.txt" , "a+");
    fprintf(log_output, "%s\n", log);
    fclose(log_output);
  }
}
void create_threads(){  
  pthread_t thread_id[MAX_CONNECTIONS];
  size_t i;
  for(i = 0 ; i < MAX_CONNECTIONS; i++){
    pthread_create(&thread_id[i], NULL, thread_work, (void *)&thread_id[i] );   
  }
  pthread_t thread_ID;
  pthread_create(&thread_ID, NULL, server_log, (void *)&thread_ID);
}



int main(int argc, char ** argv){
  /* int test_connection = accept_connection(9150); //just do some testing for connection
     printf("%d\n", test_connection); */ //if it return any postive int --> success in creating a socket descriptor
  //char ** words_test = load_dictionary("dictionary_words.txt");
  /*for(int i = 0; words_test[i] != '\0'; i++)

    {
      printf("\n Element is %s", words_test[i]);
    }
  */
  //puts("Chwck");
  
}
