/******************************************************
 *Name: Sarah Turner 
 *PROJECT 1, CS372
 *Last modified 28 October 2019
 *requested syntax: chatClient <host> <port>  
 *Description: This is the chat client. After connecting 
 *to the socket, prints a messages with the host and port.
 *Then sends PORTNUM to server, receives serverName, 
 *sends clientName.  Then enters looping chat function
 *until user on either side closes connection with /quit. 
 *References: heavy use of my own program from CS344 project 4
 *Primarily in reference to the socket setup itself, and basic
 *erro checking.  Additional references in individual functions.   
 * *************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>

#define MAXLINE 501        //define our maxline for buffer 
void doTheThing(int socketFD, char* clientName, char* serverName);  //function prototype

/******************************
 *Name: void error(const char* msg, int e)
 *Description: takes a msg and exit number. Prints
 *message to stderr and then exits according to 
 *the defined number
 * *****************************/
void error(const char* msg, int e) {fprintf(stderr,"%s\n",msg); exit(e);}

/******************************
 *Name: struct addrinfo* createAddress(char* host, char* port)
 *Description: creates the address and specifies version/TCP
 *heavy reference to Beej's guide
 *****************************/
struct addrinfo* createAddress(char* host, char* port){             
	struct addrinfo hints;
    struct addrinfo *res;
    int check;
	
	memset(&hints, 0, sizeof hints);        
	hints.ai_family = AF_INET;                  
	hints.ai_socktype = SOCK_STREAM;    

	if((check = getaddrinfo(host, port, &hints, &res)) != 0){   
		error("CLIENT:error\n", 1);
	}
	
	return res;
}

/**********************************
 *Name: main(int argc, char *argv[])
 *Description: desired syntax: otp_enc  <host><port> 
 *Main sets up client connection on specified, connects and sends initial packet.
 *Then exchanges server and client name.  Once the server-client names have
 *been exchanged, a separate function is called to send  
 *chats between server/client.
 *Socket references from Beej's guide to C and Beej's guide to network programming 
 * **************************/
int main(int argc, char *argv[]){
    char clientName[25];                                //used for clientName
    char serverName[11];                                //used for serverName received

    memset(clientName, '\0', sizeof(clientName));       //initialize strings
    memset(serverName, '\0', sizeof(serverName));

    int socketFD, portNumber, charWrite, charRead, checkSock;  //setting up for our connection 
    if(argc !=3) { error("incorrect arguments", 1);}           //error catch
    
    struct addrinfo* res = createAddress(argv[1], argv[2]);    //struct for our client setup
    //create socket
    if ((socketFD = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
		error("CLIENT: Socket was not created.\n",1);
    }
    //connect socket  
    if((checkSock = connect(socketFD, res->ai_addr, res->ai_addrlen))== -1){
        error("CLIENT: error connecting\n",1);
    }
    
     //print out hostname and port 
    printf("Client connection via ./chatClient hostname: %s port:%s\n", argv[1], argv[2]);  //woot we are in 
    fflush(stdout);
    
    //get the clientName that is correct length
    do{
        memset(clientName, '\0', sizeof(clientName));
        printf("enter a username 10 characters or less:\n");
        fflush(stdout);
        fgets(clientName, sizeof(clientName), stdin);
    }while (strlen(clientName) > 10 || strlen(clientName) == 0);
    strtok(clientName, "\n"); //remove newline

    strcpy(serverName, "PORTNUM");                                //sending first packet per project specs 
    charWrite = send(socketFD, serverName, sizeof(serverName), 0);          
    if(charWrite <0) error("CLIENT: Error sending", 1);

    memset(serverName, '\0', sizeof(serverName));           //reset serverName
    charRead = recv(socketFD, serverName, 10,0);            //recv serverName
    if(charRead <0) error("CLIENT: Error receiving",1); 
    
    charWrite = send(socketFD, clientName, 10,0);           //send clientName
    if(charWrite <0) error("CLIENT: ERROR sending",1);
             
    doTheThing(socketFD, clientName, serverName);           //now we do our chat with the server 
    freeaddrinfo(res);
    return 0;
}  

/******************************
 *Name: void doTheThing(int socketFD, char* clientName, char* serverName)
 *Description: Recieves the socketFD from main, the 
 *names of client and server.
 *runs a loop that alternates between sending and receiving with the server.
 *Client initiates the send and continues until it gets sentinel from 
 *client user or server closes connection.
 *references: piazza for strstr 
 https://stackoverflow.com/questions/2843277/c-winsock-p2p/2920787#2920787
 used as a refesher for status numbers for error checking on recv status 
 lecture for send/recv
 * ******************************/
void doTheThing(int socketFD, char* clientName, char* serverName){
    int charWrite, charRead;    //check for send/recv
    char buffer[MAXLINE];       //buffer for read/write 
       
    //printf("in client do the thing \n");
    //fflush(stdout);

     while(1){
        memset(buffer, '\0', sizeof(buffer));   //initialize the buffer  
        printf("%s> ", clientName);
        fgets(buffer, MAXLINE, stdin);
        strtok(buffer, "\n");                   //get the user input and remove newline  
        if(strstr(buffer, "\\quit")){           //if user wants to quit 
            printf("quitter....\n");
            break; 
        }
        charWrite = send(socketFD, buffer, MAXLINE-1, 0);  //send after checking for \quit
        if(charWrite <0) error("CLIENT: ERROR sending",1);
        
        memset(buffer, '\0', sizeof(buffer));              //reset buffer             
        charRead = recv(socketFD, buffer, MAXLINE-1,0);    //now we recv from server
        if(charRead <0) error("CLIENT: Error receiving",1);//if error
        else if (charRead == 0){                           //else if the server closed connection
            printf("server quit...\n");
            break;
        }
        else{    
            printf("%s> %s\n",serverName, buffer);           //else we print to stdout 
            fflush(stdout);
        }
    }
    close(socketFD);                                        //close the socket
    printf("connection closed, buh-bye\n");
    return;
}
