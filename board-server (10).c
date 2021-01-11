#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */
#include <arpa/inet.h>     /* only needed on my mac */
#include <sys/time.h>
#include <signal.h>

int main(int argc, char *argv[]) 
{
    //declare here
    int new_socket;
    int act, valread;
    char og_message[78], post[78];

    strcpy(post, "\n"); // maybe put something inside later
    // create socket

    int listen_soc = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_soc == -1) {
        fprintf(stderr, "server: socket");
        exit(1);
    }

    if (argc < 2)
    {
        exit(1);
    }
    int port = atoi(argv[1]);
    if(port < 1 || port > 65535)
    {
       //error msg
       exit(1);
    }


    //initialize server address    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);  
    memset(&server.sin_zero, 0, 8);
    server.sin_addr.s_addr = INADDR_ANY;

    // bind socket to an address
    if (bind(listen_soc, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) == -1) 
    {
      fprintf(stderr, "server: bind");
      close(listen_soc);
      exit(1);
    } 


    // Set up a queue in the kernel to hold pending connections.
    if (listen(listen_soc, 5) < 0) {
        // listen failed
        fprintf(stderr, "listen");
        exit(1);
    }
   
    struct sockaddr_in client_addr;
    unsigned int client_len = sizeof(struct sockaddr_in);
    client_addr.sin_family = AF_INET;

    // 
    
    fd_set master;
    fd_set temp;
    FD_ZERO(&temp);
    FD_ZERO(&master);         /* initialize  fd_set */
    FD_SET(listen_soc, &master);  /* put listener into fd_set */
    //fd_max = listen_soc; // og the first one is the biggest
   
   signal(SIGPIPE, SIG_IGN);
   while (1)
    {
        temp = master;

    
        act = select(FD_SETSIZE, &temp, NULL, NULL, NULL);
        if (act == -1)
        {
             fprintf(stderr, "Select has failed");
             exit(1);
        }

        for(int i = 0; i < FD_SETSIZE ; i++) 
        {
            if (FD_ISSET(i, &temp))
            {
                if ( i == listen_soc )  /* is it the listener socket?? */
                {
                    
                    new_socket = accept(listen_soc, (struct sockaddr *)&client_addr, &client_len);
                    if (new_socket == -1) 
                    {
                        fprintf(stderr, "accept");
                        exit(1);
                    } 
                    FD_SET(new_socket, &master);  /* put into fd_set */
                        
                }
                else
                {   
                    valread = read(i, og_message, 77);    
                    if(valread <= 0)
                    {
                        close(i);
                        FD_CLR(i, &master);
                    }
                            
                    if(valread == 2 && (og_message[0] == '?') && (og_message[1] == '\n'))
                    {
                        write(i, post, strlen(post));
            
                    }
                                
                    if(valread <= 76 && og_message[0] == '!')
                    {
                        
                        for(int k = 1; k < 76; k++)
                           {
                            if(og_message[k] == '\n')
                            {
                                strncpy(post, &og_message[1], k);
                                post[k] = '\0';
                                break; 
                            }
                           } 
                    }
                }

            }


        }

                
    }
    close(listen_soc);
    return 0;
}
