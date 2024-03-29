#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#define QUEUE_SIZE_DEFAULT 20
#define PORT_DEFAULT 8888
//DATA STRUCTURE
typedef struct{
  pthread_mutex_t lock;
  pthread_cond_t not_empty_check;
  pthread_cond_t not_fill_check;
  int client_num;
  int clients_in_socket[QUEUE_SIZE_DEFAULT];

}clientsQueue;

typedef struct{
  pthread_mutex_t lock;
  pthread_cond_t not_empty_check;
  pthread_cond_t not_fill_check;
  char log [QUEUE_SIZE_DEFAULT][300];
  int log_num;

}logsQueue;

//FUNCTIONS PROTOTYPES

int accept_connection(int port_number);
void load_dictionary(char * input_file);
void produce_client(clientsQueue * client_queue, int client);
int get_client(clientsQueue * client_queue);
void produce_log(logsQueue * log_queue, char * item);
char *  get_log(logsQueue * log_queue);
void * thread_work(void *);
void create_threads();
