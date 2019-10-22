#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#define QUEUE_SIZE_DEFAULT 20
int accept_connection(int port_number);
char ** load_dictionary(char * input_file);

typedef struct{
  pthread_mutex_t lock;
  pthread_cond_t empty_check;
  pthread_cond_t fill_check;
  int client_num;
  int clients_in_socket[QUEUE_SIZE_DEFAULT];

}clientsQueue;

typedef struct{
  pthread_mutex_t lock;
  pthread_cond_t empty_check;
  pthread_cond_t fill_check;
  char log [QUEUE_SIZE_DEFAULT][300];
  int log_num;

}logsQueue;
