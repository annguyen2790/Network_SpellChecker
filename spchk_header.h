#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

int accept_connection(int port_number);
char ** load_dictionary(char * input_file);
