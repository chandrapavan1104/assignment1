#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include<sys/wait.h>
#include <stdlib.h>
#include <netinet/in.h>
#include<pwd.h>
#include <string.h>
#include<assert.h>
#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    //Part of Code that sets up the socket and this part has the privileges
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    pid_t f_pid;
    pid_t child_pid = fork(); //Child process is created
    int status = 0;
    
    if( child_pid == 0 ) //Part of code that process the data from client for which privileges are being dropped
    {
        struct passwd *pwd = getpwnam("nobody");

	if(pwd == NULL)
    {
	    perror("Unable to drop privileges, could not get *nobody user*");
	    exit(EXIT_FAILURE);	
	}
        //Privileges are now dropped to that of "nobody" user
        printf("UID of nobody=%ld\n",(long) pwd->pw_uid);
        if (setuid(pwd->pw_uid) < 0)   //Run the program as superuser in order to execute setuid
            perror("setuid() error");
        else
            printf("UID after setuid() = %ld\n",(long) getuid());

        valread = read( new_socket , buffer, 1024); 
        printf("%s\n",buffer ); 
        send(new_socket , hello , strlen(hello) , 0 ); 
        printf("Hello message sent\n");

    }
    else if (child_pid > 0){
     
        //Server waiting for child process to complete
        if((f_pid = wait(&status)) < 0)
        {
            perror("Error in waiting");
            _exit(1);
        }
    }
    else
    {
        perror("Could not create child process");
	    _exit(2);        			     				                                                                                                                                      
    }    
    return 0;
}