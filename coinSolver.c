#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>


#define TARGET_PORT 9007

int binaryTree(int fd,int start, int end, int *chances,int *skip);
int craftMessage(int start,int end, int fd);

int main(void) {
  int targetfd,status;
  struct sockaddr_in *target_addr;
  struct addrinfo hints, *rp;
  memset(&hints,0,sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;  
  // Setting up Target Address
  status = getaddrinfo("pwnable.kr","9007",&hints,&rp);
  if (status < 0) {
    printf("Failed to get addrinfo for pwnable\n");
  }
  //Fd
  targetfd = socket(rp->ai_family,rp->ai_socktype,0);
  if (targetfd == -1) {
    printf("failed  to make targetfd socket\n");
    return 1;
  }
  //Addr storage
  target_addr = (struct sockaddr_in *)rp->ai_addr;  
  target_addr->sin_port = htons(TARGET_PORT);
  // Connecting to target
  if (connect(targetfd, (struct sockaddr *)target_addr,sizeof(*target_addr)) == -1) {
    printf("Failed to connect to site..");
    close(targetfd);
    freeaddrinfo(rp);
    return 1;
  }
  else {
    printf("Connected to site\n");
  }
  //Listening, and Replying to Site.
  char throwawayBuffer[2000];
  int bytes_received = recv(targetfd,throwawayBuffer,sizeof(throwawayBuffer)-1,0);
  if (bytes_received < 0) {
    printf("Failed to receive");
    close(targetfd);
    return 1;
  }
  throwawayBuffer[bytes_received] = '\0';
  printf("The starting message of the server %s\n",throwawayBuffer);
  sleep(5);

  int solved = 0;
  char buffer[20];
  int line = 0;
  int n,c,j = 0;
  char tempVal[10];
  // Binary Tree Variables
   memset(tempVal,'\0',sizeof(tempVal));
  while (solved != 1) {
    //Parse out N and C
    line = recv(targetfd,&buffer,sizeof(buffer),0);
    buffer[line] = '\0';
    printf("buffer is %s\n",buffer);
    for (int i = 2;i<line;i++) {
      if (buffer[i] != ' ') { // It's a number
        tempVal[j] = buffer[i];
        j++;
      }
      else {
        i = i+2;
        j = 0;
        n = atoi(tempVal);
        memset(tempVal,'\0',sizeof(tempVal));
      }
    }
    j = 0;
    c = atoi(tempVal);
    memset(tempVal,'\0',sizeof(tempVal));
    memset(buffer,'\0',sizeof(buffer));
    // Binary Tree time

    //Crafting the message
    printf("N = %d\n",n);
    printf("C = %d\n", c);
    int *ptr = &c;
    int rangeStart = 0;
    int rangeEnd = n-1;
    int done = 0;
    int skipValue = 0;
    int *skip = &skipValue;
    done = binaryTree(targetfd, rangeStart,(rangeEnd/2),ptr,skip); // Left Branch
    // Children of Right Branch
    printf("big loop, r branch\n");
    if (done == 0)
        done =binaryTree(targetfd,(rangeEnd/2)+1,rangeEnd,ptr,skip);
    }
    

  //Clean up
  close(targetfd);
  freeaddrinfo(rp);
  return 0;
}

int binaryTree(int fd,int start,int end,int *chances,int *skip) {
  int sum;
  int done;
  if (*(chances) == -1) {
    return 0;
  }
  if (*(skip) == 0) {
    sum =craftMessage(start,end,fd);
  if (sum == -1) {
    printf("returning 1\n");
    *(chances) = -1;
    return 1;
  }
  *(chances) = *(chances)-1;
  printf("sum = %d\n",sum);
  }
  else{
    printf("skipping start = %d\t end = %d\n",start,end);
    *(skip) = 0;
    sum = 19;
    printf("Sum = skipped\n");
  }
  printf("chances = %d\n",*(chances));
  if (sum == 9) { // Answer found, send whatever value of start/end is to program
    int size = 0;
    printf("Answer found!\n");
    if (start >= 100) {
      size = 4;
    }
    else if ((start >= 10) && (start < 100)) {
      size = 3;
    }
    else {
      size = 2;
    }
    if (*(chances) > 0) {
    char tempAnswer[size];
    char tempReply[100];
    printf("Sum found, chances greater than zero, sending values until chances = 0\n");
    for (int i = *(chances); i > 0; i--) {
        sprintf(tempAnswer,"%d\n",start);
        write(fd,tempAnswer,sizeof(tempAnswer));
        recv(fd,tempReply,sizeof(tempReply),0);
      }
      
    }
    //Submit Answer
    char writeAnswer[size];
    char *writePtr = writeAnswer;
    sprintf(writePtr,"%d\n",start);
    char reply[2048];
    write(0,writeAnswer,sizeof(writeAnswer));
    send(fd,writeAnswer,sizeof(writeAnswer),0);
    recv(fd,reply,sizeof(reply),0);
    printf("reply after answer = %s\n",reply); 
    *(chances) = -1;
    return 1;
  }
  else if ((sum%10) == 9) { // Keep on this branch
    //call craftmessage(left branch)
    printf("Trying Left Branch..\n");
    done = binaryTree(fd,start,(((end-start)/2)+start),chances,skip);
    printf("done = %d\n",done);
    if (done == 0)
      printf("Trying right branch..\n");
      done = binaryTree(fd,(((end-start)/2)+start)+1,end,chances,skip);
    
    if (done == 1) {
      return 1;
    }
    }
  else  if (sum == 0) {
    exit(1);
  }
  else { // Sum%10 = 0, bad branch
    *(skip) = 1;
    return 0;
  }

    return 0;
}
int craftMessage(int start, int end, int fd) {
    
  char writeBuffer[2048];
  printf("The start is %d, the end is %d\n",start, end);
  char *writePtr = writeBuffer;
  char reply[2048];
  int ans = 0;
  int nbytes = 0;
  int totalBytes = 0;
  memset(writeBuffer,'\0',sizeof(writeBuffer));
  memset(reply,'\0',sizeof(reply));

  for (int i = start; i <= end; i++) {
    sprintf(writePtr,"%d %n",i,&nbytes);
    writePtr +=nbytes;
    totalBytes +=nbytes;
    if (i == end) {
      sprintf(writePtr,"\n%n",&nbytes);
      totalBytes +=nbytes;
    }
  }
  char sendTo[totalBytes];
  for (int i = 0; i <totalBytes;i++) {
    sendTo[i] = writeBuffer[i];
  }
  write(0,sendTo,sizeof(sendTo));
  write(fd,sendTo,sizeof(sendTo));
  ans = recv(fd,reply,sizeof(reply),0);
  printf("reply = %s\n",reply);
  if(reply[0] == 'C') {
    printf("Reply is  = Correct!\n");
    write(0,reply,sizeof(reply));
    return -1;
  }
  return (atoi(reply));
}
