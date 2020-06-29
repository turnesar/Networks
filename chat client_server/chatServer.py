#!/usr/bin/python

#Sarah Turner CS372, PROJECT 1 
# last modified on 28 October 2019 
#resources used:
#1-https://docs.python.org/2/library/string.html#string-constants
#2-https://tutorialspoint.com/python/
#3-https://docs.python.org/2/library/string.html#formatstrings
#4-https://docs.python.org/2/library/socket.html
#5-https://docs.python.org/2/howto/sockets.html#socket-howto
# Description: syntax desired is python chatServer.py <desired port>
# THis is the chat server. It is listening for clients and then will
# exchange chats with a client until client/server closes socket. 
# Continues to wait for clients until signal recieved
#####################################################


import string                               #using this for our string constant
import sys                                  #for sys calls 
from socket import *                        #for socket magic 

# ####################################
# Name: chittyChat(userName, clientName, connectionClient
# Description: Receives userName of server, client and socket 
# and waits for client to initiate.  Goes back and forth
# until client or server closes connection by typing
# the sentinel.
# Resources: listed above and in addition:
# https://stackoverflow.com/questions/16745409/what-does-pythons-socket-recv-return-for-non-blocking-sockets-if-no-data-is-r
# this was primarily used for determining when the client closes connection
# #################################    
def chittyChat(userName, clientName, connectionClient):
    buffer = ""                                  #used for server Buffer 
    #print("in chittychat")
    while 1:
        buffer = connectionClient.recv(501)       #recv from client
        if not buffer:                            #client closed  
            print("client closed")
            break
        print('{}> {}' .format(clientName,buffer))  #print proper format of client chat
        buffer = ""                                #empty buffer 
        while len(buffer)==0 or len(buffer)>500:   #get the user input 
            buffer = raw_input ("{}> ".format(userName))
        if buffer == "\quit":
            print "server is donesies"  
            break
        connectionClient.send(buffer)    

#out of defined function and into main-like python
# #################################3
# Name main()
# Description:  This is the main function in the server.
# First sets up socket according to lecture notes. Then gets
# user name.  Then receives initial string from client, sends
# server Name, then recieves client name. Calls chat function,
# and closes socket after done chatting. Continues to wait 
# for clients until killed with signal  
# ####################################
def main():
    portNumber = sys.argv[1]                            #user specificied port 
    serverSocket = socket(AF_INET, SOCK_STREAM)         #setting up our TCP socket
    serverSocket.bind(('', int(portNumber)))            #binding to the port 
    serverSocket.listen(1)                              #ready for clients!
    #print "server ready"
    userName = ""
    while len(userName) == 0 or len(userName) > 10:     #getting the user name 
        userName = raw_input("enter username that is 10 characters or less\n")

    print("All set up and waiting for clients")

    while 1:
        connectionSocket, addr = serverSocket.accept()
        #print ("Connected on host %s and port %d." % (addr, portNumber)) 
        userClient = connectionSocket.recv(25)          #initial receive
        #print("%s recieved" % userClient)
        connectionSocket.send(userName)                 #send server Name 
        clientName = connectionSocket.recv(25)          #recv client Name
        #print("%s clientName" % clientName)
              
        chittyChat(userName, clientName, connectionSocket) #call chat function
        connectionSocket.close()                           #close socket after chat  
        print("Waiting for client") 

##############################################
# https://stackoverflow.com/questions/419163/what-does-if-name-main-do
# https://stackabuse.com/command-line-arguments-in-python/
# we are not operating as a module and want to verify correct arguments before calling main
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "no port specified, please type python chatServer.py <desired port>"
        exit(1)
    main()    


   

