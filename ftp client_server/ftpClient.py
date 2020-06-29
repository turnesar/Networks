#!/usr/bin/python

#Sarah Turner CS372, PROJECT 2
# last modified on 30 NOV 2019
#resources used:
#1-https://realpython.com/python-sockets/
#2-https://tutorialspoint.com/python/
#3-https://docs.python.org/2/library/string.html#formatstrings
#4-https://docs.python.org/2/library/socket.html
#5-https://docs.python.org/2/howto/sockets.html#socket-howto
#6-https://christophergs.github.io/python/2017/01/15/python-errno/
#7-https://stackoverflow.com/questions/14425401/catch-socket-error-errno-111-connection-refused-exception
# Description: desired syntax: 
#   python ftpClient.py <host> <port number> <dataport> 
# This is the ftp Client. It connects to server  and then will
# exchange commands with the server.  Upon validated command
# establishes second TCP connection with server for data transfer.
# Data transfer ends when server closes connection. 
# Client continues to send commands until signal received.
# Personal note - I once had the file transfer working without any null
# characters but the connection was dying.  I can't get it back so I 
# just removed the null characters
#####################################################


import string                               #using this for our string constant
import sys                                  #for sys calls 
from socket import *                        #for socket magic 
import re                                   #for some string magic                       
import os                                   #for directory manipulation  
import array                                #some array functions
import errno                                #error catching 
from socket import error as socket_error 
import time                                 #create unique fileNames

# ####################################
# Name: chittyChat(clientSocket, serverHost, serverP
# Description: Receives clientSocket, serverHost and dataPort.  
# Then takes in command from user.  Sends command to server 
# and if valid response is received, it sets up a data
# connection and calls a nested function to receive the
# data. 
#
# Resources: listed above and in addition:
# https://stackoverflow.com/questions/16745409/what-does-pythons-socket-recv-return-for-non-blocking-sockets-if-no-data-is-r
# this was primarily used for determining when socket is closed 
# https://www.guru99.com/python-regular-expressions-complete-tutorial.html
# https://pypi.org/project/parse/
# https://www.w3schools.com/python/ref_string_split.asp
# #################################    
def chittyChat(clientSocket, serverHost, dataPort):
    buffer = "" 
    ftpName = "ftpClient"                                 #used for server Buffer 
    fileTransfer = 0
    listDirectory = 0
    #print("in chittychat")

   
    buffer = clientSocket.recv(2)
    #if not buffer:                            #client closed  
     #   print("socket closed")
      #  return
    clientSocket.send(dataPort)
    buffer = clientSocket.recv(2)
    buffer = ""
    flag = 1  
   

    if flag:
        while len(buffer)==0 or len(buffer)>500:          #get our command 
            buffer = raw_input ("{}> ".format(ftpName))
        x = buffer.split()    #lets us set flags for what the command was, if command wasn't formatted correctly, the server will kick back
        if x[0] == '-g':
            #if re.search(".txt$", x[1]):
            buffer = "%s %s" % (x[0], x[1])
            #print (buffer)
            fileTransfer = 1
            #else:
             #   print "incorrect syntax, need -g <filename>, file must exist and be .txt"    
        elif x[0] == '-l':
            listDirectory = 1  
            buffer = x[0]
        #buffer = "%s %s" % (buffer, dataPort)  #adding our dataport to the end
        clientSocket.send(buffer)   #sending our command
        buffer = ""
        buffer = clientSocket.recv(500);   #receive response back 
        if not buffer:                            #client closed  
            print("socket closed")
            return
        result = buffer.find('requested on port')                   #recv response   
        if result != -1:         #woot a good response   
            print('{}> {}' .format('ftpServer', buffer))      #tell the client    
            srvrSocket = socket(AF_INET, SOCK_STREAM)         #set up our data connection 
            try:
               srvrSocket.bind(('', int(dataPort))) 
            except:
               print("socket won't bind")                     #usually means port in use   
               sys.exit(2)
            srvrSocket.listen(1)
            dataSocket, addr = srvrSocket.accept()

            if fileTransfer:                                   #call nested function  
                receiveFile(dataSocket)
                flag = 1 
            elif listDirectory:
                listDir(dataSocket)
                flag = 1                                     
        else:
            print('{}> {}' .format('ftpServer', buffer))       #catch for invalid command sent to server 
            fileTransfer = 0
            listDirectory =0

# ####################################
# Name: listDirectory(dataSocket)
# Description: Receives dataSocket and receives
# confirmation from server that it is going to send the 
# directory information. Sends back the word that lets
# the server know we are ready.  Loops through 
# and prints the directory until socket closes.  Used 
# reference at top for error catching 
# #################################                             
def listDir(dataSocket):
    buffer = ""
    buffer = dataSocket.recv(500)                  #handshake because they are sending 
    print('{}> {}' .format('ftpServer', buffer))
    buffer = "boomdiggity"
    dataSocket.send(buffer)
    buffer = ""
    result = ""
   
    try:                                           #receive and print the directory, catch errno 9 
        while True:
            buffer = dataSocket.recv(500) 
            if not buffer: 
                break
            print (" ")
            print(buffer)
    except socket.error,e:
        if e.errno == 9:
            print ("data connection closed")
            dataSocket.close()
        else:
            raise
    print("end of directory")   
    dataSocket.close()                              #close our socket 
             


# ####################################
# Name: listDirectory(dataSocket)
# Description: Receives dataSocket and receives
# confirmation from server that it is going to send the 
# directory information. Sends back the word that lets
# the server know we are ready.  Loops through 
# and prints the directory until socket closes
#  
# https://stackoverflow.com/questions/37427683/python-search-for-a-file-in-current-directory-and-all-its-parents
# https://www.guru99.com/reading-and-writing-files-in-python.html
# https://stackoverflow.com/questions/17667903/python-socket-receive-large-amount-of-data
# https://www.mkyong.com/python/python-how-to-split-a-string/
# http://stupidpythonideas.blogspot.com/2013/05/sockets-are-byte-streams-not-message.html
# https://docs.python.org/3/library/time.html
# #################################  
def receiveFile(dataSocket):
    buffer = ""
    buffer = dataSocket.recv(500)       #recv the word that it is getting ready to send 
    x = buffer.split()                  #split to get the filename 
    #fileName = x[1]                     
    ts = time.time()
    ts = int(ts)/60                        #yes this is a big number...but its unique!
    fileName = "%s%s" % (ts, x[1])         #make our file unique 
    
    print('{}> {}' .format('ftpServer', buffer)) #now print the incoming message 
    dataSocket.send('boomdiggity')               #let them know we are ready 
    buffer = ""
    fragments = []
    f = open(fileName, "a+")                    #create/append file 
    
    try:
        while True:                                  #loop through and recv the file  
            buffer = dataSocket.recv(500)
            if not buffer:
                break
            fragments.append(buffer)                #build them back together 
    except socket.error as e:                       #catch the error 
        if e.errno == 9:
            print ("data connection closed")
            dataSocket.close()
        else:
            raise
    allPieces = "".join(fragments) 
    finalPiece = allPieces.replace('\000', "")    #this gets rid of null characters 
    f.write("%s" % finalPiece)                    #writes to file   
    f.close()
   
    print("end of file transfer")

    dataSocket.close()
    
#out of defined functions and into main-like python

# #################################
# Name main()
# Description:  
# Desired syntax is python ftpClient.py <host> <port number> <dataport> when running
# https://realpython.com/python-sockets/
# ####################################
def main():
    portNumber = sys.argv[2]                            #user specificied port 
    clientSocket = socket(AF_INET, SOCK_STREAM)         #setting up our TCP socket
    clientSocket.connect((sys.argv[1], int(portNumber)))#connecting to the port     
    chittyChat(clientSocket, sys.argv[1], sys.argv[3]) #call chat function
    clientSocket.close()                           #close socket after chat  
        

##############################################
# https://stackoverflow.com/questions/419163/what-does-if-name-main-do
# https://stackabuse.com/command-line-arguments-in-python/
# we are not operating as a module and want to verify correct arguments before calling main
if __name__ == "__main__":
    if len(sys.argv) != 4:
        print "no port or host specified, please type python ftpClient.py <host> <port> <desired data port>"
        exit(1)
    main()    


   

