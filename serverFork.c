/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>	/* for the waitpid() system call */
#include <signal.h>	/* signal name macros, and the kill() prototype */

//Additional includes
#include <string.h>
#include <sys/stat.h>
#include <time.h>

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void dostuff(int); /* function prototype */
char* fileType(char*);
char* currentTime();
char* lastModified(char*);
int contentLength(char*);
char* parseMessage(char*);
void debug();

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, pid;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     struct sigaction sa;          // for signal SIGCHLD

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
      
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     
     listen(sockfd,5);
     
     clilen = sizeof(cli_addr);
     
     /****** Kill Zombie Processes ******/
     sa.sa_handler = sigchld_handler; // reap all dead processes
     sigemptyset(&sa.sa_mask);
     sa.sa_flags = SA_RESTART;
     if (sigaction(SIGCHLD, &sa, NULL) == -1) {
         perror("sigaction");
         exit(1);
     }
     /*********************************/
     
     while (1) {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
         
         if (newsockfd < 0) 
             error("ERROR on accept");
         
         pid = fork(); //create a new process
         if (pid < 0)
             error("ERROR on fork");
         
         if (pid == 0)  { // fork() returns a value of 0 to the child process
             close(sockfd);

             debug();
            
             dostuff(newsockfd);  //makes the connection wait for a input from client

             exit(0);
         }
         else //returns the process ID of the child process to the parent
             close(newsockfd); // parent doesn't need this 
     } /* end of while */
     return 0; /* we never get here */
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/


void dostuff (int sock)
{
   int n;
   char buffer[256];      
   //declaring our variables
   char* filePath;

   bzero(buffer,256);
   n = read(sock,buffer,255);
   if (n < 0) error("ERROR reading from socket");
   printf("Here is the message: %s\n",buffer);

   //testing parseMessage()
   filePath = parseMessage(buffer);
   printf("Here is the URL parsed from the request: %s\n", filePath);

   n = write(sock,"I got your message",18);
   if (n < 0) error("ERROR writing to socket");
}

//Returns the file extension of the file (ex. hi.dogs-cats.html should return .html)
/* Declare a char* variable and set it as the output of the function */
char* fileType (char *input)
{
  char *ret = strrchr(input, '.');
  if (!ret || ret == input)
    return "";
  else
    return ret;
}

//Returns a timestamp of the response message
char* currentTime()
{
  return 0;
}

//Returns the last modified date of the file we're looking at
char* lastModified (char* path)
{
  struct stat fileState;

  int retVal = stat(path, &fileState);

  if (retVal != 0)
  {
    printf("Error determining last modified. Errno %d \n", retVal);
    return 0;
  }

  //Access the last modified timestamp
  time_t rawModTime = fileState.st_mtime;
  time(&rawModTime);

  char* lastModTime = ctime(&rawModTime);

  return lastModTime;
}

//Returns the content length of the file we're looking at
int contentLength (char* path)
{
  //Passes in the path to function stat and gets the size
  //Ex. a file named "dog.html" in our directory will have a char* path of "dog.html"

  //Initialize the struct and run stat
  struct stat fileState;

  int retVal = stat(path, &fileState);

  if (retVal != 0)
  {
    printf("Error determining file state. Errno %d \n", retVal);;
    return 0;
  }

  //Access the size value
  int size = (int)(fileState.st_size);

  return size;
}


// this function will parse the file we want out of the
// get request and return a c-string of the url.
  // ex: GET /derp.html HTTP/1.1 ...
  // returns /derp.html
char* parseMessage(char* buffer) {
  // buffer = "DERP /derp.html DERP/1.1\0";
  int end, start = 0, size = 0, spCount = 0;
  for (end = 0; end < 256; end++) {
    if (buffer[end] == ' ') {
      if (spCount == 0) {
        start = end;
        spCount++;
      } else if (spCount == 1) {
        size = end - start;
        spCount++;
        break;
      }
    }
  }
  char* output;
  output = malloc(sizeof(char) * size);
  memcpy(output, buffer+start+1, size);
  output[size] = '\0';
  return output;
}


//For my sanity...
void debug()
{
  //TEST EXTENSION
  char* testString = "dog.dog.dog.cat-meow.doc";
  char* type = fileType(testString);
  printf("The extension should be .doc: %s \n\n", type);

  testString = "hihihihi!!!hiH!!!!fdm";
  type = fileType(testString);
  printf("The extension should be nothing: %s \n\n", type);

  //TEST LENGTH
  int lenTest = contentLength("lick.html");
  printf("The size of lick.html should be about 162kb: %d \n\n", lenTest);

  lenTest = contentLength("lots_of_boobs.html");
  printf("There should be an error above because the file doesn't exist: %d \n\n", lenTest);

  //TEST LAST MODIFIED
  char* lastMod = lastModified("lick.html");
  printf("Last modified date: %s \n", lastMod);

}

