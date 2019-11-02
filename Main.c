#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include "spchk_header.h"
#define MAX_CONNECTIONS 3
#define DICTIONARY "dictionary_words.txt"
#define NUM_WORDS 99712
#define NUM_LINE 256
//GLOBAL VARIABLES TO USE
FILE * log_file;
char ** dictionary;
int dict_size;
clientsQueue client_q;
logsQueue  log_q;
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

void load_dictionary(char * file_name)
{
  char file[NUM_LINE];
  
  strcpy(file, file_name); //copy file name to file to process
  
  FILE *file_pointer = fopen(file, "r");
  
  int count = 0;
  
  if (file_pointer == NULL)
    {
      perror("Unable to allocate resources\n");
    }
  else
    {
      char line [NUM_LINE]; //holder for each word each line
      while (fgets(line, sizeof(line), file_pointer) != NULL )
	{
	  count =  count + 1; //each line has one word so increment by one
	}
      
      fclose(file_pointer);
    }
  dict_size = count; //check how many words are there
  dictionary = (char **) malloc(count*sizeof(char*));
  for (int i = 0; i < count; i++) {
    dictionary[i] = (char*) malloc(256 * sizeof(char));
  }
  file_pointer = fopen(file, "r");
  //reset count to zero
  count = 0;
  
  if (file_pointer == NULL){
      perror("ERROR!");
  }
  else{
      char line[256];
      while (fgets(line, sizeof(line), file_pointer ) != NULL){
	  strcpy(dictionary[count],line);
	  for (int i = 0; i < NUM_LINE; i++) {
	    if (dictionary[count][i] == '\n') {
	      dictionary[count][i] = '\0';
	      break;
	    }
	  }
	  count++;
	}
      fclose(file_pointer);
  }
} 
int checkSpelling(char * word){ //check whether a word is in the list of words or not
  size_t i;
  for(i = 0 ; i < dict_size; i ++){
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
    pthread_cond_wait(&client_queue->not_fill_check, &client_queue->lock);
    
  }
  client_queue->clients_in_socket[client_queue->client_num] = client; // put client in
  client_queue->client_num = client_queue->client_num + 1; //increment items in the buffer
  
  pthread_cond_signal(&client_queue->not_empty_check); //signal that the buffer is not empty!
  pthread_mutex_unlock(&client_queue->lock);

}

int get_client(clientsQueue * client_queue){
  //acquire a lock for this thread
  //run a while loop when a number of clients in client queue is less or equal than 0
  //make the process wait until the empty_check is signal. Until empty_check is signaled, the thread will be suspended
  //consume the client from the end of the  queue
  //decrement the number of client inside the client socket
  //unlock the lock for this thread
  int client;
  
  pthread_mutex_lock(&client_queue->lock);
  
  while(client_queue->client_num <= 0 ){
    pthread_cond_wait(&client_queue->not_empty_check, &client_queue->lock);
  }
  
  client = client_queue->clients_in_socket[client_queue->client_num - 1];
  
  client_queue->client_num  = client_queue->client_num - 1;
  
  pthread_cond_signal(&client_queue->not_fill_check);
  
  pthread_mutex_unlock(&client_queue->lock);
  
  return (client);
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
    pthread_cond_wait(&log_queue->not_fill_check, &log_queue->lock);
  }
  
  strcpy(log_queue->log[log_queue->log_num], log); // copy item from buffer 
  
  log_queue->log_num = log_queue->log_num + 1; 
  
  pthread_cond_signal(&log_queue->not_empty_check); //signal that the buffer is not empty
  
  pthread_mutex_unlock(&log_queue->lock); //unlock
  
  
}


char * get_log(logsQueue * log_queue){
  char * return_log = (char *) malloc(sizeof(char) * 300);
  pthread_mutex_lock(&log_queue->lock); //acquire lock

  while(log_queue->log_num <= 0 ){ //condition variable: while log queue is empty, suspend threads
    pthread_cond_wait(&log_queue->not_empty_check, &log_queue->lock);   
  }
  strcpy(return_log, log_queue->log[log_queue->log_num - 1]); //consume log from queue
  log_queue->log_num = log_queue->log_num - 1;
  pthread_cond_signal(&log_queue->not_fill_check);


  pthread_mutex_unlock(&log_queue->lock);//release lock
  return return_log;

}
void * thread_work(void * vargp){ //this function handles threads between clients and server
  while (1) {
    int byte_check;
    
    char message_receive[NUM_LINE];
    
    char word_holder[NUM_LINE];
    
    int clientSocket = get_client(&client_q);
    
    char msg[] = "Connected Successfully. Please enter a word to check its spelling\n"; //when client connect, this message should appear
    send(clientSocket, msg, strlen(msg), 0);
    while(1){
      memset(message_receive,'\0',1000);
      memset(word_holder,'\0',1000);
      byte_check = recv(clientSocket, message_receive, 1000, 0);
      if(byte_check == -1){
	send(clientSocket, "Unable to receive message!", strlen("Unable to receive message!"), 0);
      }
      else{   
	if(message_receive[0] == 27){ //when user enter ESC 
	    send(clientSocket, "Session ended!", strlen("Session ended!"), 0);
	    close(clientSocket);
	    break;
	  }
	  else{     
	      for (int i = 0; i < NUM_LINE; i++) {
		if (message_receive[i] == '\r' || message_receive[i] == '\n') { //when user press "ENTER" and return newline
		  message_receive[i] = '\0';
		  word_holder[i] = '\0';
		  break;
		}
		word_holder[i] = message_receive[i]; //copy user input into holder
	      }
	      
	      strcpy(word_holder,message_receive); //make a copy of result

	      if (checkSpelling(word_holder) == 1) { //check and send result to user
		strcat(message_receive," CORRECT\n");
		send(clientSocket,message_receive, strlen(message_receive), 0);
	      }
	      else {
		strcat(message_receive," Misspelled\n");
		send(clientSocket, message_receive, strlen(message_receive), 0);
	      }

	      produce_log(&log_q, message_receive);//// produces log
	    }

	}
    }
  }
  return NULL;
}
void * server_log(void * vargp){ //threads for log 
  char * log;
  while(1){
    log = get_log(&log_q);
    log_file = fopen("logs_output.txt" , "a+");
    fprintf(log_file, "%s\n", log);
    fclose(log_file);
  }
}
void create_threads(){  
  pthread_t tid[MAX_CONNECTIONS]; //threads for spellcheckinng
  pthread_t thread_id; //threads for log
  for (int i = 0; i<MAX_CONNECTIONS; i++) {
    pthread_create(&tid[i], NULL, thread_work, (void *)&tid[i]);
  }
  pthread_create(&thread_id, NULL, server_log, (void *)&thread_id);
}



int main(int argc, char ** argv){
  //
  struct sockaddr_in Client;
  int client_length = sizeof(Client);
  int socket_connection;
  int socket_client;
  const char * scan;
  //
  int port = PORT_DEFAULT;
  if(argc == 1) //if only run the command with no argument of .txt or port number, load the default
    {
      load_dictionary(DICTIONARY);
      port = PORT_DEFAULT;
    }
  else
    {
      
      if(argc == 2) //if there are two parameters for the command 
	{
	  scan = strrchr(argv[1], '.');
	  if(!scan)
	    {
	      load_dictionary(DICTIONARY);
	      port = atoi(argv[1]);
	    }
	  else
	    {
	      load_dictionary(DICTIONARY);
	      port = PORT_DEFAULT;
	    }
	}
      else //if there are three paramters 
	{
	  scan = strrchr(argv[1], '.');
	  if(!scan)
	    {
	      load_dictionary(DICTIONARY);
	    
	      port = atoi(argv[1]);
	    }
	  else
	    {
	      load_dictionary(DICTIONARY);
	      port = atoi(argv[2]);
	    }
	}
    }

  
  //There is no port  below 1024 and ports above 65535 .
  if(port > 65535 || port < 1024){
    perror("No such port exists");
    return -1;
  }
  //starting server and accepting connections
  socket_connection = accept_connection(port);

  if(socket_connection == -1){
    perror("Unable to connect to this port");
    return -1;
  }
  puts("Succesfully Establish Connections between Sockets");

  create_threads();

  printf("PORT IN SESSION:  %d", port);
  fflush(stdout);
  printf("\n");
  // listen for input from client
  while(1) {
    socket_client = accept(socket_connection, (struct sockaddr*)&Client, &client_length);
    if(socket_client == -1){
      perror("Cannot connect to client");
      return -1;
    }
    //display localhost machine number
    printf("Accepted connection from telnet host number: %d\n", socket_client);
    fflush(stdout);
    printf("Handling Threads\n");
    fflush(stdout);
    //create client
    produce_client(&client_q, socket_client);
  }

  return 0;
}
