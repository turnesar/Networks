Sarah Turner CS372
PROJECT 2

1. type chmod +x makefile
2. type 'makefile' (without quotes) to compile server. Of note, this is
throwing a warning and note about the getnameinfo use here because of a 
pointer and cast issue. However, that function works and the program runs.

3. type the following with chosen port number(without arrow).
--> ftpServer <port>

4. run client by typing the following command (without arrow).
The host should match the flip server that server is on, and
the port should match server port. Dataport can be specified by 
user
--> python ftpClient.py <host> <port number> <dataport> 

desired syntax for commands are: 
-l
-g <FILENAME>

NOTE: downloaded moby dick from Gutenberg project for testing large files 
NOTE: used  cmp --silent large.txt 26252713large.txt && echo 'SUCCESS' || echo 'DIFFERENT'
for analysis of file transfer 
NOTE: output of file will have truncated microseconds at front of fileName
NOTE: I am so glad this project is getting turned in 






