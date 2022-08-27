//MAHFUZ UDDIN

#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include<sys/wait.h>
#include <stdlib.h>//for malloc


int myStringLengthTwo(char * string);
int myStringLength(char * consuming, int CHUNK_SIZE);
int stringToNumber(char * str);
char *myStringCopy(char * destination, const char * source, int n);
char *toString(int number);
int numLen(int number);

int main(int argc, char * argv[]) {
  const int BUFFER_SIZE = stringToNumber(argv[4]);
  int CHUNK_SIZE = stringToNumber(argv[3]);
  const int PAGESIZE = 128;
  const int SIZE = CHUNK_SIZE*BUFFER_SIZE; //the size (in bytes) of shared memory object
  const char * name = "dood"; //name of the shared memory object
  int fd; //shared memory file descriptor
  int f1, f2; //target file and dest file
  char *ptr; //pointer to shared memory obect
  int n = 0;
  char *memMessage = "Shared memory Failed\n";
  char *mapMessage = "Map Failed\n";


  //here we create a pipe for communcation between parent process and child process
  int fdPipe[2];
  if (pipe(fdPipe) == -1) {
    write(STDERR_FILENO, "Pipe Failed!", myStringLengthTwo("Pipe Failed!"));
  }
  const char * in_data_name = "in";
  const char * out_data_name = "out";
  //create shared memory for the inPTR so both parent and child can write to it as well as read updated value of it
  int in_data = shm_open(in_data_name, O_CREAT | O_RDWR, 0666);
  if (in_data == -1) {
    write(STDERR_FILENO, memMessage, myStringLengthTwo(memMessage));
    return -1;
  }
  //create shared memory for the inPTR so both parent and child can write to it as well as read updated value of it
  int out_data = shm_open(out_data_name, O_CREAT | O_RDWR, 0666);
  if (out_data == -1) {
    write(STDERR_FILENO, memMessage, myStringLengthTwo(memMessage));
    return -1;
  }
  //configure the size of the shared memory object for inPTR
  ftruncate(in_data, PAGESIZE); 
  //memory map the shared memory object to inPTR
  int * inPtr = mmap(0, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, in_data, 0);
  if (inPtr == MAP_FAILED) {
    write(STDERR_FILENO, mapMessage, myStringLengthTwo(mapMessage));
    return -1;
  }
  //configure the size of the shared memory object for outPTR
  ftruncate(out_data, PAGESIZE);
  //memory map the shared memory object to outPTR
  int * outPtr = mmap(0, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, out_data, 0);
  if (outPtr == MAP_FAILED) {
    write(STDERR_FILENO, mapMessage, myStringLengthTwo(mapMessage));
    return -1;
  }

  * inPtr = 0;
  * outPtr = 0;

  //create the shared memory object 
  fd = shm_open(name, O_CREAT | O_RDWR, 0666);
  // configure the size of the shared memory object
  ftruncate(fd, SIZE);
  // memory map the shared memory object 
  ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED) {
    write(STDERR_FILENO, mapMessage, myStringLengthTwo(mapMessage));
    return -1;
  }

  int pid = fork();

  if (pid < 0) {
    write(STDERR_FILENO, "Fork Failed!", myStringLengthTwo("Fork Failed!"));
    return -1;

  } else if (pid == 0) {//child process
    if ((f2 = creat(argv[2], 0644)) == -1) {
      write(STDERR_FILENO, "Can't Create Target File!", myStringLengthTwo("Can't Create Target File!"));
      return 3;
    }

    char child_buffer[CHUNK_SIZE];
    int shMemCsrCharCount = 0;
    //loop iterates until we have consumed all the text from shared memory
    //loop breaks when we encounter null terminator in shared memory
    while ("true" == "true") {

      while ( * inPtr == * outPtr)
      ; //buffer is empty
      /*consuming points to the start of memory where we need to read from
      we offset the base pointer by the product of out value, size of char, and chunksize
      after finding the current location, we copy CHUNK_SIZE characters from memory into child_buffer*/
      char *consuming = ptr + ( * outPtr * sizeof(char) * CHUNK_SIZE);
      int sLen = myStringLength(consuming, CHUNK_SIZE);
      shMemCsrCharCount += sLen;

      if (consuming[0] == '\0') break;
      myStringCopy(child_buffer, consuming, sLen);




            
      //In the next few lines of code we create a big bufferr(childOuput), to where we copy our whole output into it for part 1 of the project.
      //We do this because if we write "CHILD: OUT = " individual of "CHILD: ITEM = " the output will get mixed up because the child and parent run at different pace.
      //Hence, to prevent that from happening we add all out output into one big buffer which we will output to the terminal.
      //First we calculate a correct amount of space for the big buffer by adding the length of all our string output 
      //Then we start copying all of the strings to the big buffer using loops
      char * stringOfout = toString( * outPtr);
      int childOutputSize = myStringLengthTwo("\nCHILD: OUT = ") + myStringLengthTwo("\n\tCHILD: ITEM = ") +
        myStringLengthTwo(stringOfout) + sLen;
      char childOuput[childOutputSize];
      int currPosition = 0;
      char * cOut = "\nCHILD: OUT = ";
      char * cItem = "\n\tCHILD: ITEM = ";

      for (int i = 0; i < myStringLengthTwo(cOut); i++) {
        childOuput[currPosition] = cOut[i];
        currPosition++;
      } //"CHILD: OUT = "

      for (int i = 0; i < myStringLengthTwo(stringOfout); i++) {
        childOuput[currPosition] = stringOfout[i];
        currPosition++;
      } // put inPtr in buffer

      for (int i = 0; i < myStringLengthTwo(cItem); i++) {
        childOuput[currPosition] = cItem[i];
        currPosition++;

      } //" CHILD: ITEM = "

      for (int i = 0; i < sLen; i++) {
        childOuput[currPosition] = child_buffer[i];
        currPosition++;
      } // writng child buffer

      write(STDOUT_FILENO, childOuput,childOutputSize);
      write(f2, child_buffer, sLen);
      * outPtr = ( * outPtr + 1) % BUFFER_SIZE;
    }
    //read values from pipe the parernt sent and store it in tshMemPrdCharCount and output it to terminal
    char cShMemPrdCharCount[128];
    int n = read(fdPipe[0], cShMemPrdCharCount, 128);
    write(STDIN_FILENO, "\n\nCHILD: The parent value of shMemPrdCharCount  = ", 
          myStringLengthTwo("\n\nCHILD: The parent value of shMemPrdCharCount  = "));
    write(STDOUT_FILENO, cShMemPrdCharCount,n );


    char * stringShMemCsrCharCount = toString(shMemCsrCharCount);
    char * childCountMessage = "\nCHILD: The child value of shMemCsrCharCount  =";
    write(STDOUT_FILENO, childCountMessage, myStringLengthTwo(childCountMessage));
    write(STDOUT_FILENO, stringShMemCsrCharCount, myStringLengthTwo(stringShMemCsrCharCount));
    
    //close all the file descriptors and both ends of pipes
    close(f2);
    close(fdPipe[1]);
    close(fdPipe[0]);
    //remove the shared memory object 
    if (shm_unlink(name) == -1) {
      write(STDERR_FILENO, "Error Removing Shared Memory!\n", myStringLengthTwo("Error Removing Shared Memory!\n"));
      return -1;
    }

  } else {
    // test to see if source file opens
    if ((f1 = open(argv[1], O_RDONLY, 0)) == -1) {
      write(STDERR_FILENO, "Can't Open Source File!\n", myStringLengthTwo("Can't Open Source File!\n"));
      return 2;
    }

    //added this local buffer that will be filled in with data from read() calls
    char parent_buffer[CHUNK_SIZE];
    int shMemPrdCharCount = 1;  //starts with 1 to account for null terminator we add after the while loop ends

    //keep reading from file until there's nothing left to read, changed second parameter to parent_buffer to store data into
    while ((n = read(f1, parent_buffer, CHUNK_SIZE)) > 0) {

      while ((( * inPtr + 1) % BUFFER_SIZE) == * outPtr)
      ; // do nothing cause buffer is full

      //using pointer arithmithic we find the next availaable location we can write to
      char * producedSpot = ptr + ( *inPtr * CHUNK_SIZE);

      //Copy buffer's bytes into specific spot in shared memory
      //the spot depends on the value of "in" currently, multiplied by the chunk size, and the product of that is added to the base ptr of shared mem
      myStringCopy(producedSpot, parent_buffer, n);
      shMemPrdCharCount += n;//we add the amount of bytes read from memory to keep a total count, which we will later send to the child process 
      
      //In the next 26 lines of code we create a big bufferr(parentOuput), to where we copy our whole output into it for part 1 of the project.
      //We do this because if we write "PARENT: OUT = " individual of "PARENT: ITEM =" the output will get mixed up because the child and paren run and different pace.
      //Hence, to prevent that from happening we add all out output into one big buffer which we will output to the terminal.
      //First we calculate a descent amount of space for the big buffer by adding the length of all our string output and add 20 just to make sure we ways have enough space.
      //Then we start copying all of the strings to the big buffer using loops
      char *stringOfin = toString(*inPtr);
      


      int parentOutputSize = myStringLengthTwo("\nPARENT: IN = ") + myStringLengthTwo("\n\tPARENT: ITEM = ") +
        myStringLengthTwo(stringOfin) + n;
      char parentOuput[parentOutputSize];
      int currPosition = 0;
      char * pOut = "\nPARENT: IN = ";
      char * pItem = "\n\tPARENT: ITEM = ";
      for (int i = 0; i < myStringLengthTwo(pOut); i++) {
        parentOuput[currPosition] = pOut[i];
        currPosition++;
      } //"CHILD: OUT = "

      for (int i = 0; i < myStringLengthTwo(stringOfin); i++) {
        parentOuput[currPosition] = stringOfin[i];
        currPosition++;
      } // put value of inPtr in buffer

      for (int i = 0; i <myStringLengthTwo(pItem); i++) {
        parentOuput[currPosition] = pItem[i];
        currPosition++;

      } //" CHILD: ITEM = "

      for (int i = 0; i < n; i++) {
        parentOuput[currPosition] = parent_buffer[i];
        currPosition++;
      } //copying over frorm childbuffer to outputbuffer
      
      // write(STDOUT_FILENO, parentOuput,parentOutputSize );
      * inPtr = ( * inPtr + 1) % BUFFER_SIZE;
    }

    //next few lines add null terminator to shared memory after reading all text from file and writing to memory
    while ((( * inPtr + 1) % BUFFER_SIZE) == * outPtr); // do nothing cause buffer is full, we don't want to overwrite something by accident
    char * producedSpot = ptr + ( * inPtr * CHUNK_SIZE);
    producedSpot[0] = '\0';
    *inPtr = ( * inPtr + 1) % BUFFER_SIZE;
    
    char * stringShMemPrdCharCount = toString(shMemPrdCharCount);
    char * parentCountMessage = "\nPARENT: The parent value of shMemPrdCharCount  = ";

   
    write(fdPipe[1], stringShMemPrdCharCount, myStringLengthTwo(stringShMemPrdCharCount));
    close(f1);
    close(fdPipe[0]);
    close(fdPipe[1]);

    // parent waits for child to terminate
    wait(NULL);
    //after child terminates we write our final messaage of total count 
    write(STDOUT_FILENO, parentCountMessage, myStringLengthTwo(parentCountMessage));
    write(STDOUT_FILENO, stringShMemPrdCharCount, myStringLengthTwo(stringShMemPrdCharCount));
    write(STDIN_FILENO, "\n", 1);
  }

  return 0;
}

//implementation of string length, here we only want to know if there is CHUNK_SIZE character left in the shared memory
//or return the length till we hit the end of used memory
int myStringLength(char * consuming, int CHUNK_SIZE) {
  int length = 0;
  int i = 0;
  while (length < CHUNK_SIZE && consuming[i] != '\0') {
    ++length;
    ++i;
  }
  return length;
}
//count the characters in a string until we a null character
int myStringLengthTwo(char * string) {
  int length = 0;
  int i = 0;
  while (string[i] != '\0') {
    ++length;
    ++i;
  }
  return length;
}
//convert a string to a number using ascii values 
int stringToNumber(char * str) {
  int result = 0;

  for (int i = 0; str[i] != '\0'; ++i)
    result = result * 10 + str[i] - '0';
  return result;
}
//copy characters from one string pointed by source to destination
char * myStringCopy(char * destination, const char * source, int n) {
  int i = 0;
  if (destination == NULL) {
    return NULL;
  }

  char * ptr = destination;//point to the start of the destination string
  while (i <= n) {
    *destination = *source;//copy charcter from source to destination and then move the pointer over by 1 for both source and destination
    destination++;
    source++;
    i++;
  }
  *destination = '\0';
}
//returns length of a number
int numLen(int number){
  int count =0;
  int n =number;
     while(n!=0)  
   {  
       n=n/10;  
       count++;  
   }
   return count;
}
//converts a integer to a string 
char * toString(int number){
  int length = numLen(number);
  
  char *theNumber = malloc(length);

  char a[length];
  for (int i = length; i > 0; i--) {
    theNumber[i-1] = (number % 10) + '0'; // mod by 10 to get last digit and add '0' to get string represenation of the digit and store it in a[i]
    number = number / 10; //divide by 10 to get rid of last digit
  }
  return theNumber;
}
