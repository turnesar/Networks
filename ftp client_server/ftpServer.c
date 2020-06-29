/******************************************************
 *Name: Sarah Turner 
 *PROJECT 2, CS372
 *Last modified 30 November 2019
 *requested syntax: ftpServer <port>  
 *Description: This is the ftp server.  First validates command
 *line parameters.  Then waits for clients on specficied port. Will
 *establish TCP control on specified port with clients. Server 
 *waits on commands from the client. From there the client can
 *list content of directory or get files from server. Server validates
 *command, creates data connection, performs handshake with client,
 *adjudicates request, and closes data connection. 
 *References: heavy use of Beej's guide and my project 1 from 
 *this class. Additional references in individual functions.   
 * *************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <dirent.h>


#define MAXLINE 501        //define our maxline for buffer 
//function declarations 
void error(const char* msg, int e, int f);                             //error and exit  
void OpenForBusiness(int socketFD, char* clientAddress);        //used to parse client commands on control connection and call additional functions
struct addrinfo* createAddress(char* port, char* address);      //create address for sockets
struct addrinfo* createAddressNOIP(char* port);      //create address for sockets
int finishSocket(struct addrinfo* res, int connectOrNo);                //make the socket ready for listen
int validateFile(char* fileName);                               //make sure server can locate the file specified by client
void sendFile(char* fileName, char* dataPort, char* address, int socketFD);     //send the file to client over data connection 
void listDirectory(char* dataPort, char* address, int socketFD); 
         //send the directory contents over data connection 

/**********************************
 *Name: main(int argc, char *argv[])
 *Description: desired syntax: ftpServer <port> 
 *Main sets up server connection on specified port, outputs that the connection
 *Then calls function to do ftp exchanges with client.
 *Socket references from Beej's guide to C and Beej's guide to network programming 
 *http://man7.org/linux/man-pages/man3/inet_ntop.3.html
 * **************************/
int main(int argc, char *argv[]){
    int socketFD, checkSock;  //setting up for our connection 
    struct sockaddr_storage their_addr;   
                       //hold client address
    socklen_t addr_size;                                     //size of client address 
    int new_fd; 
                                                   //server socket creation
    if(argc <2) { error("incorrect arguments", 1,1);}           //error catch
    
    struct addrinfo* res = createAddressNOIP(argv[1]);       //struct for our server setup, null value for address

    //does all the socket setup for connection
    socketFD = finishSocket(res, 0);
    
    //print out our welcome
    printf("Server open on port:%s\n", argv[1]);  //woot we are in 
    fflush(stdout);

    //listen on socket
    if ((checkSock = listen(socketFD, 5))==-1){
        close(socketFD);
        error("SERVER: deaf server\n",1,1);
    }
  
    //now we wait for clients and accept them
    //Beej's guide and: 
    //https://stackoverflow.com/questions/12810587/extracting-ip-address-and-port-info-from-sockaddr-storage
    //this does through a warning and note for getnameinfo because of cast and pointer.  
    while(1){
        addr_size = sizeof(their_addr);
        new_fd = accept(socketFD, (struct sockaddr *)&their_addr, &addr_size); 
        if(new_fd == -1){
            continue;
        }      
        char clientAddress[NI_MAXHOST];
        char clientPort[NI_MAXSERV];
        int gotIt = getnameinfo((struct sockaddr *)&their_addr, &addr_size, clientAddress, sizeof(clientAddress), 
        clientPort, sizeof(clientPort), NI_NUMERICHOST | NI_NUMERICSERV);
        if(!gotIt){
            printf("server: got connection from %s %s\n", clientAddress, clientPort);
            fflush(stdout);
            OpenForBusiness(new_fd, clientAddress);   //this is where we do all the exchanges and pass our client info and their address
        }
    
       close(new_fd);
    }  
    close(socketFD); 
    freeaddrinfo(res);
    
}
    
/******************************
 *Name: void error(const char* msg, int e, int f)
 *Description: takes a msg and exit number. Prints
 *message to stderr and then exits according to 
 *the defined number. Last argument will tell them
 *to exit or not 
 * *****************************/
void error(const char* msg, int e, int f) {
    fprintf(stderr,"%s\n",msg);
    if(f){
       exit(e); 
    } 
}

/******************************
 *Name: struct addrinfo* createAddress(char* host, char* port)
 *Description: creates the address and specifies version/TCP.
 *has to be separate because of the flags.
 *heavy reference to Beej's guide
 *https://linux.die.net/man/3/getaddrinfo for additional information
 *on flags 
 *****************************/
struct addrinfo* createAddress(char* port, char* host){             
	struct addrinfo hints;
    struct addrinfo *res;
    int check;
	
    
	memset(&hints, 0, sizeof hints);        
	hints.ai_family = AF_INET;       //versioning            
	hints.ai_socktype = SOCK_STREAM; //tcp 
    
   
	    if((check = getaddrinfo(host, port, &hints, &res)) != 0){   
		    error("SERVER: won't create address\n", 1, 1);
        }
   

	
    return res;
} 
/******************************
 *Name: struct addrinfo* createAddressNOIP(char* port)
 *Description: creates the address and specifies version/TCP
 *heavy reference to Beej's guide. This is the one without host. 
 *https://linux.die.net/man/3/getaddrinfo for additional information
 *on flags 
 *****************************/
struct addrinfo* createAddressNOIP(char* port){             
	struct addrinfo hints;
    struct addrinfo *res;
    int check;
	
    
	memset(&hints, 0, sizeof hints);        
	hints.ai_family = AF_INET;       //versioning            
	hints.ai_socktype = SOCK_STREAM; //tcp 
    hints.ai_flags = AI_PASSIVE;
   
   
	    if((check = getaddrinfo(NULL, port, &hints, &res)) != 0){   
		    error("Server:cannot create address to client\n", 1,0);
        }
   

	return res;
}  
/******************************
 *Name: int finishSocket(struct addrinfo* res, int connectOrNo)
 *Performs the rest of the setup for the socket.  Does bind or connect
 *depending on if they are the data or control connection 
 *Same as used in project 1, just more modularized. Beej's guide reference 
 *****************************/
int finishSocket(struct addrinfo* res, int connectOrNo){
    int socketFD,checkSock;
    int yes = 1;
    //create socket
    if ((socketFD = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
		error("SERVER: Socket was not created.\n",1,1);
    }
    if(connectOrNo){
        if ((checkSock = connect(socketFD, res->ai_addr, res->ai_addrlen)) == -1){
		error("SERVER: Error connecting socket\n",1,0);
        }
    }
  
    //bind socket
    if(!connectOrNo){
        if ((checkSock = bind(socketFD, res->ai_addr, res->ai_addrlen))==-1){
            close(socketFD);
            error("SERVER: error binding\n",1,1);
        }
    }    
    return socketFD;
}

/******************************
 *Name: void OpenForBusiness(int socketFD, char* clientAddress)
 *Description: this takes the clients command and parses it using sscanf.  It then 
 *will call additional functions to list directories, send files (after validating),
 *or send an error message to the client. 
 *https://www.tutorialspoint.com/c_standard_library/c_function_sscanf.htm for the sscanf piece
  *****************************/
void OpenForBusiness(int socketFD, char* clientAddress){
    int charWrite, charRead;
    char buffer[MAXLINE];
    char fileName[MAXLINE];
    int fileInvalid = 0;
    char dataPort[10];
    
    //let them know they are connected 
    strcpy(buffer, "g");
    charWrite = send(socketFD, buffer, strlen(buffer)+1,0);
    if(charWrite <0) error("SERVER: ERROR sending",1,0);
    //now we recieve the port 
    charRead = recv(socketFD, dataPort, 10,0);
    if(charRead <0) error("SERVER: Error receiving",1,0); 
    //printf("%s", dataPort);
    //fflush(stdout);
    //let them know we got it 
    memset(buffer, '\0', sizeof(buffer));
    strcpy(buffer, "g");
    charWrite = send(socketFD, buffer, strlen(buffer)+1, 0);  
    if(charWrite <0) error("SERVER: ERROR sending",1,0);


   
        memset(buffer, '\0', sizeof(buffer));
        charRead = recv(socketFD, buffer, MAXLINE-1,0);            //recv serverName
        if(charRead <0) error("SERVER: Error receiving",1,0); 
        if(buffer[1] == 'g'){
            sscanf(buffer, "%*s %s ", fileName);
            memset(buffer, '\0', sizeof(buffer));
            fileInvalid = validateFile(fileName);
            if (fileInvalid){
                strcpy(buffer, "invalid file name");
                charWrite = send(socketFD, buffer, MAXLINE-1, 0);  
                if(charWrite <0) error("SERVER: ERROR sending",1,0);
                fileInvalid =0;
            }
            else{
                snprintf (buffer, MAXLINE, "File %s requested on port %s\n", fileName, dataPort);
                charWrite = send(socketFD, buffer, MAXLINE-1, 0);  
                if(charWrite <0) error("SERVER: ERROR sending",1,0);
                sendFile(fileName, dataPort, clientAddress, socketFD);    
            }
        }
        else if(buffer[1] == 'l'){
            snprintf (buffer, MAXLINE, "list directory requested on port %s\n", dataPort);
            charWrite = send(socketFD, buffer, MAXLINE-1, 0);  
            if(charWrite <0) error("CLIENT: ERROR sending",1,0);
            listDirectory( dataPort, clientAddress, socketFD);
        } 
        else{
            strcpy(buffer, "invalid command syntax");
            charWrite = send(socketFD, buffer, MAXLINE-1, 0);  
            if(charWrite <0) error("SERVER: ERROR sending",1,0);
        } 
                                                       
        memset(buffer, '\0', sizeof(buffer));
        printf("request handled\n");
        
               
         
}
/******************************
 *Name: int validateFile(char* fileName)
 *Description: this makes sure the file can be opened and read
 *before we open up our data connection and send it.
 *https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.bpxbd00/rtacc.htm
 *
  *****************************/
int validateFile(char* fileName){
  if( access(fileName, F_OK) ==-1){   //it doesn't exist 
    return 1;
  }else{                                                    
      return 0;                       
  }

}

/******************************
*void sendFile(char* fileName, int dataPort, char* address, int socketFD)
*Description: this takes the desired fileName, dataport and client address.
*First it creates the second data connection and validates that it is good.
*Then it lets the client know it is going to send the file.  If the
*server receives a 'boomdiggity' back from the client then we send the file.
*The server loops through the file until EOF and then closes the socket to 
*alert the client that they are done.
*https://stackoverflow.com/questions/1835986/how-to-use-eof-to-run-through-a-text-file-in-c
*used for EOF in the while loop 
*https://stackoverflow.com/questions/21180248/fgets-to-read-line-by-line-in-files/21180478
********************/
void sendFile(char* fileName, char* dataPort, char* address, int socketFD){
  
  char buffer[MAXLINE];
  int charWrite, charRead;
  FILE *fileToSend;
  int dataSocket;

     sleep(2);
     //create and validate connection 
     struct addrinfo* res = createAddress(dataPort, address);
     dataSocket = finishSocket(res,1);
    
     if(!dataSocket){
         return;
     }
     memset(buffer, '\0', MAXLINE);  
     //printf("in send file post data connection");
     //fflush(stdout);
     snprintf (buffer, MAXLINE, "Sending %s on port %s\n", fileName, dataPort);
     charWrite = send (dataSocket, buffer, sizeof(buffer)-1,0);
     if(charWrite <0) error("ERROR writing to socket",1,0); 
    //handshake first to let the client know to get ready 
      memset(buffer, '\0', MAXLINE);
      charRead = recv(dataSocket, buffer, MAXLINE-1,0);
      if(charRead <0) error("ERROR reading from socket",1,0);
      if(strcmp(buffer,"boomdiggity")!=0){
           error("ERROR with handshake before file transfer",1,0);
      }   
   
    //now we send the file as a stream
    fileToSend = fopen(fileName, "r");
    while(1){
            memset(buffer, '\0', sizeof(buffer)); 
            if(fgets(buffer, MAXLINE-1, fileToSend)==NULL) break;
            charWrite = send(dataSocket, buffer, MAXLINE-1, 0);  //send file in directory via buffer            
            if(charWrite <0) error("CLIENT: Error sending", 1,0);    //reset buffer      
    }
                 
    //close file and data socket 
    fclose(fileToSend);
    close(dataSocket);
    freeaddrinfo(res);
}
/***********************************
*void listDirectory(int dataPort, char* address){
*Description: this takes the desired dataport and client address.
*First it creates the second data connection and validates that it is good.
*Then it lets the client know it is going to send the directory data.  If the
*server receives a 'boomdiggity' back from the client then we send the directory list.
*The server loops through the directory until all files are passed and then closes the socket to 
*alert the client that they are done.
*references: heavily used my turnesar.buildrooms.c program from CS344, it was the second project and I'm using
*a modified version of the directory navigation function 
***********************************/
void listDirectory(char* dataPort, char* address, int socketFD){
    DIR* currentDir;                            //used to navigate directories   
    struct dirent *fileDir;                     //struct used to access name of our subdirectory
    int t = 0;
    char buffer[MAXLINE];
    int charWrite, charRead, dataSocket; 
    //set up and validate data connection 
    sleep(2);
    struct addrinfo* res = createAddress(dataPort, address);
    dataSocket = finishSocket(res,1);
    if(!dataSocket){
        return;
    }
    
     memset(buffer, '\0', MAXLINE);
     snprintf (buffer, MAXLINE, "Sending directory contents on port %s\n", dataPort);
     //printf("%s\n", buffer);
     //fflush(stdout);
     charWrite = send (dataSocket, buffer, MAXLINE-1,0);
     if(charWrite <0) error("ERROR writing to socket",1,0); 
    //handshake first to let the client know to get ready 
      memset(buffer, '\0', MAXLINE);
      charRead = recv(dataSocket, buffer, MAXLINE-1,0);
      if(charRead <0) error("ERROR reading from socket",1,0);
      if(strcmp(buffer,"boomdiggity")!=0){
           error("ERROR with handshake before listing directory",1,0);
      } 
      //printf("%s\n", buffer);
       memset(buffer, '\0', MAXLINE); 
    currentDir = opendir(".");                  //open current directory    
    if(currentDir){
        while((fileDir = readdir(currentDir)) != NULL){
            if (t >1){                                       //we don't want the . and the .. -->so we skip those
                strcpy(buffer, fileDir->d_name);   //copy each file in the directory into the buffer  
                charWrite = send(dataSocket, buffer, MAXLINE-1, 0);  //send file in directory via buffer            
                if(charWrite <0) error("CLIENT: Error sending", 1,0); 
                memset(buffer, '\0', sizeof(buffer));           //reset buffer
            }
        t = t+1; 
        }
    }
  

    closedir(currentDir);
    sleep(2);
    close(dataSocket);
    freeaddrinfo(res);     
} 
    