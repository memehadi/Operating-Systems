/*Musab Mehadi
mmehadi@jacobs-university.de*/


//SURVER/CLIENT

#define _POSIX_C_SOURCE 201112L
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include "tcp.h"
#include <event2/event.h>

//CONNECTIONS

typedef struct state {
   int num_guesses;
   int correct;
   char fortune[33];
   char missing[33];
} state_t;

typedef struct node {
   int cfd;
   state_t *state;
   struct event *cev;
   struct node *next;
} conn_node;


conn_node* add_new_conn_node(conn_node **head, int cfd){
   conn_node *curr_node = NULL;
   state_t *curr_state = NULL;

   if(*head == NULL){
      *head = (conn_node*) malloc(sizeof(conn_node));
      if(*head == NULL){
         fprintf(stderr, "error creating new connection: %s", strerror(errno));
         exit(EXIT_FAILURE);
      }
      curr_node = *head;
   } else {
      curr_node = *head;
      while(curr_node->next != NULL){
         curr_node = curr_node->next;
      }
      curr_node->next = (conn_node*) malloc(sizeof(conn_node));
      curr_node = curr_node->next;
      curr_node->next = NULL;
   }
   curr_node->cfd = cfd;
   curr_node->cev = NULL;
   curr_node->state = (state_t*) malloc(sizeof(state_t));

   curr_state = curr_node->state;
   curr_state->num_guesses = 0;
   curr_state->correct = 0;
   curr_state->fortune[0] = '\0';
   curr_state->missing[0] = '\0';

   return curr_node;
}

int remove_conn_node(conn_node **head, int cfd){
   conn_node *curr_node = NULL;
   if(*head == NULL){
      fprintf(stderr, "the connection list is empty\n");
      return EXIT_FAILURE;
   }

   curr_node = *head;
   if(curr_node->cfd == cfd){
      *head = ((*head)->next) ? (*head)->next : NULL;
      event_free(curr_node->cev);
      free(curr_node);
      return EXIT_SUCCESS;
   }

   while(curr_node->next != NULL){
      if(curr_node->next->cfd == cfd){
         conn_node* tmp = curr_node->next;
         curr_node->next = (curr_node->next->next) ? curr_node->next->next : NULL;
         event_free(tmp->cev);
         free(tmp);
         break;
      }
      curr_node = curr_node->next;
   }

   return EXIT_SUCCESS;
}


int remove_conn_node(conn_node **head, int cfd);
conn_node* add_new_conn_node(conn_node **head, int cfd);


//SERVER
static const char* result = {//set of replys to the client
  "C: %s",
   "Wrong guess - expected: %s\n", 
   "Congratulation - challenge passed!\n",
   "M: You mastered %d/%d challenges. Good bye!\n",
   "Unknown command or format\n"
};

static const char *progname = "GuessingGame";
static struct event_base *evb;
static conn_node *connections = NULL;
static char chars_POSIX[] = " \t\r\n\v\f.,;!~`_-"; // POSIX whitespace and punctuation chars 

static void find_question(conn_node* connection);
static void read_question(evutil_socket_t evfd, short evwhat, void *evarg);
static void send_to_client(conn_node* connection, int type);
static void read_from_client(evutil_socket_t evfd, short evwhat, void *evarg);
static void set_connection(evutil_socket_t evfd, short evwhat, void *evarg);

int main(int argc, char* argv[]){

   int fd;
   struct event *tev;
   const char* interfaces[] = {"localhost", NULL, "0.0.0.0", "::"};

//checking for arguments
   if(argc != 2){
      fprintf(stderr, "Not enough arguments. Usage: %s [port]\n", progname);
      exit(EXIT_FAILURE);
   }

//  avoiding zombie processes
   if(signal(SIGCHLD, SIG_IGN) == SIG_ERR){
      fprintf(stderr, "%s: signal() %s\n", progname, strerror(errno));
      exit(EXIT_FAILURE);
   }

   (void) daemon(0, 0);
 //creating the event base
   openlog(progname, LOG_PID, LOG_DAEMON);
   evb = event_base_new();
   if(!evb){
      syslog(LOG_ERR, "creating event base failed");
      exit(EXIT_FAILURE);
   }
//registering the event
   for(int i = 0; interfaces[i]; i++){
      fd = tcp_listen(interfaces[i], argv[1]);
      if(fd < 0){
         fprintf(stderr, "error %s %s: could listen to connection\n", strerror(errno), progname);
         continue;
      }
      tev = event_new(evb, fd, EV_READ | EV_PERSIST, set_connection, NULL);
      event_add(tev, NULL);
   }

//reactions to the events
   if(event_base_loop(evb, 0) == -1){
      syslog(LOG_ERR, "event loop failed");
      event_base_free(evb);
      exit(EXIT_FAILURE);
   }

   closelog();
   event_base_free(evb);

   return EXIT_SUCCESS;
}

static void find_question(conn_node* connection){
   pid_t pid;
   struct event *qev;
   int fd[2];

   (void) pipe(fd);
   qev = event_new(evb, fd[0], EV_READ, read_question, connection);
   event_add(qev, NULL);

   pid = fork();
   if(pid == 0)//child process
   {
      close(fd[0]);
      dup2(fd[1], STDOUT_FILENO);
      close(fd[1]);
      execlp("fortune", "fortune", "-n", "32", "-s", NULL);
      syslog(LOG_ERR, "failure executing fortune\n");
      exit(EXIT_FAILURE);
   }
   else if(pid == -1){
      syslog(LOG_ERR, "forking error!\n");
      connection->state->fortune[0] = '\0';
   else {
      close(pfd[1]);
   }
}
static void read_question(evutil_socket_t evfd, short evwhat, void *evarg){
   conn_node *currentcon = (conn_node*) evarg;
   char buffer[64];
   int no_bytes; 
   int fd = evfd;
   no_bytes = read(fd, buffer, 32);
   if(no_bytes < 0){
      fprintf(stderr, "%s:failed to read %s\n", progname, strerror(errno));
      exit(EXIT_FAILURE);
   } else {
      buffer[no_bytes] = '\0';
      strcpy(currentcon->state->fortune, buffer);
   }

   hiding(currentcon->state);
  send_to_client(currentcon, CHALLENGE);
}


static void hiding(state_t* status){
  char game[50];
   int c;
   char *position;
   char *token = NULL;
   int no;
  
   

   position = status->fortune;
   while((position = strpbrk(position, chars_POSIX)) != NULL){
      position++;
      no++;
   }

   while(token == NULL){

      srand(time(NULL));
      c = rand()%n;
      no= 0;

      strcpy(game, status->fortune);
      token = strtok(game, chars_POSIX);
      while(token){
         if(no == c){
            position = strstr(status->fortune, token);
            if(!position){
               break;
            }
            memset(position, '_', strlen(token));
            strcpy(status->missing, token);
            break;
         }
         no++;
         token = strtok(NULL, chars_POSIX);
      }
   }
}

static void send_to_client(conn_node* connection, int res){
   int no_bytes;
   char buffer[256];
    if(res ==0){
      sprintf(buffer, result[0], connection->state->fortune);}
   else if(res == 1){
      sprintf(buffer, result[1], connection->state->missing);}
   else if(res == 2){
      strcpy(buffer, result[2]);
   }
   else if(res == 3){
      sprintf(buffer, result[3], connection->state->correct, connection->state->num_guesses);
   } else {
      strcpy(buffer, result[4]);
   }

   no_bytes = tcp_write(connection->cfd, buffer, strlen(buffer));
   if(no_bytes < 0){
      fprintf(stderr, "%s: writing failed %s\n", progname, strerror(errno));
      exit(EXIT_FAILURE);
   }
}

static void read_from_client(evutil_socket_t evfd, short evwhat, void *evarg){
   conn_node *current_connection = (conn_node*) evarg;
   int no_bytes;
   char buffer[256];
   char *guess;
   int cfd = evfd;
   no_bytes = read(cfd, buffer, sizeof(buffer));
   if(no_bytes < 0){
      fprintf(stderr, "%s: reading failed: %s\n", progname, strerror(errno));
      exit(EXIT_FAILURE);
   }

   if(strstr(buffer, "Q:") == buffer){
      /* In case the user wants to quit */
      send_to_client(current_connection, 3);
      event_free(current_connection->cev);
      tcp_close(cfd);
      remove_conn_node(&connections, cfd);
   } else 
   {
      if(strstr(buffer, "R:") == buffer){
         /* Response from the user */
         current_connection->state->num_guesses++;
         memmove(buffer, buffer+2, strlen(buffer)-2);
         guess = strtok(buffer, chars_POSIX);
         if(guess == NULL || strcmp(guess, current_connection->state->missing) != 0){
            send_to_client(current_connection, 1);
         } else {
            current_connection->state->correct++;
            send_to_client(current_connection, 2);
         }
         find_question(current_connection);
      } else {
         /* Unrecognized command */
         send_to_client(current_connection, 4);
      }
   }
}

static void set_connection(evutil_socket_t evfd, short evwhat, void *evarg){
   conn_node *new_connection;
   size_t no;
   int cfd;
   char Hello[256];
   

   cfd = tcp_accept((int)evfd);
   if(cfd < 0){
      fprintf(stderr, "%s: failed to connect: %s\n", progname, strerror(errno));
      exit(EXIT_FAILURE);
   }

   strcpy(welcome, "M: Guess the missing ____!\n");
   strcat(welcome, "M: Send your guess in the for 'R: word\\r\\n'.\n");

   no = tcp_write(cfd, Hello, strlen(Hello));
   if(no < 0){
      fprintf(stderr, "%s: %s  failed to write\n", progname, Hello);
      tcp_close(cfd);
      exit(EXIT_FAILURE);
   }

  new_connection = add_new_conn_node(&connections, fd);
  new_connection->cev = event_new(evb, cfd, EV_READ | EV_PERSIST, read_from_client, new_connection);
   event_add(new_connection->cev, NULL);

   find_question(new_connection);
}

