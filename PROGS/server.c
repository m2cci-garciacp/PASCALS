#include "server.h"
#include "dimensions.h"
#include "data.h"
#include "html.h"
#include "init_cells.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>


char client_message[CLIENT_MESSAGE_LENGTH];
char server_message[SERVER_MESSAGE_LENGTH];

// Handle answer of the form GET  /nblines HTTP ...
// extract nblines if present

int analyze_client_request () {
  int found,nblines;

  found = sscanf(client_message,"GET /%d",&nblines);
  if (found != 1) return 8;
  return nblines;
}

  
void create_answer () {
  int size;

  char *text = server_message;

  number_of_lines = analyze_client_request ();
  
  // compute size of html figure
  compute_cells_values ();
  compute_cells_strings ();
  size = generate_html(server_message);

  text += sprintf (text,"HTTP/1.0 200 OK\n");
  text += sprintf (text,"Content-type: text/html\n");
  text += sprintf (text,"Content-length : %d\n",size);

  // generate html
  text += generate_html(text);
}
  

int main (void) {
  int socket_desc;
  int answer_socket;
	int pid ; 
  ssize_t length;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len;

  socket_desc = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
 
  if(socket_desc < 0){
          fprintf(stderr,"Error while creating socket\n");
          return -1;
     }
    fprintf(stderr,"Socket created successfully\n");

    // Set port and IP:
    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);     
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    // Accept from all address instead of 127.0.0.1
    // Especially when running in docker
    server_addr.sin_addr.s_addr = INADDR_ANY;

    const int enable = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
      fprintf (stderr,"Set REUSEADDR socket option failed\n");
      exit(EXIT_FAILURE);
    }

    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        fprintf(stderr,"Couldn't bind to the port\n");
        return -1;
    }
    fprintf(stderr,"Done with binding\n"); 

    if (listen(socket_desc,3) <0) {
        fprintf (stderr,"FAILURE : listen\n");
        return -1;
    }

    fprintf (stderr,"listen ok\n");
    while (1) {
        answer_socket = accept(socket_desc,(struct sockaddr*)&client_addr,&client_addr_len);
        if (answer_socket<0)  {
            fprintf (stderr,"FAILURE : accept\n");
            return -1;
        }

        fprintf (stderr,"accept ok\n");
        pid =fork();
        if (pid ==0){
            close(socket_desc);
            // receive client message
            length = read(answer_socket, client_message, sizeof(client_message));
            
            if (length <0)  {
                printf ("reception failure\n");
                switch (errno) {
                case EAGAIN : fprintf (stderr,"EAGAIN\n");break; 
                case EBADF : fprintf (stderr,"EBADF\n");break; 
                case EFAULT : fprintf (stderr,"EFAULT\n");break; 
                case EIO : fprintf (stderr,"EIO\n");break; 
                case EINTR : fprintf (stderr,"EINTR\n");break; 
                case EINVAL : fprintf (stderr,"EINVAL\n");break; 
                }
                perror ("error type : ");
                return -1;
            }
        
            fprintf (stderr,"received ok : 0x%lx\n",length);

            // Add end of string to message
            client_message[length] = 0;

            // Print client and message 
            fprintf(stderr,"Received message from IP: %s and port: %i\n",
                    inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            fprintf(stdout,"Message from client : <<\n %s", client_message);
            fprintf(stdout,">>End of message from client\n\n");

            create_answer ();

            length = write(answer_socket, server_message, strlen(server_message));
            close(answer_socket);
            exit(0);
        } else {
            close(answer_socket);
        }
    }
    close(socket_desc);
    
    return 0;
}
