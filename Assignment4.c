/*
 * Name:
 * ID #:
 * Programming Assignment 1
 * Description:
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/stat.h> 
#include <time.h>

#define NUM_BLOCKS 4226
#define BLOCK_SIZE 8192
#define NUM_FILES 128
#define NUM_INODES 128
#define MAX_BLOCK_NUM_PER_FILE 32

unsigned char data_blocks[NUM_BLOCKS][BLOCK_SIZE];
int used_blocks[BLOCK_SIZE];

void initialize_dir_inode();
//int intialize_shell();

struct inode {
  time_t date;
  int valid;
  int size;
  int blocks[MAX_BLOCK_NUM_PER_FILE];
};

//struct inode inode_array[NUM_INODES];
struct inode * inode_array_ptr[NUM_INODES];

struct directory_entry {
  char * name;
  int valid;
  int inode_idx;
};

struct directory_entry * directory_ptr;
//struct directory_entry * directory_array_ptr[NUM_FILES];

void initialize_dir_inode() {
  int i;
  directory_ptr = (struct directory_entry * ) & data_blocks[0];

  for (i = 0; i < NUM_FILES; i++) {
    directory_ptr[i].valid = 0;
  }
  int inode_idx = 0;
  for (i = 1; i < 130; i++) {
    inode_array_ptr[inode_idx++] = (struct inode * ) & data_blocks;
  }
}

int findFreeDirectoryEntry() {
  int i;
  int retval = -1;
  for (i = 1; i < 128; i++) {
    if (directory_ptr[i]-> valid == 0) {
      retval = i;
      break;
    }
  }
  return retval;
}

int findFreeInode() {
  int i;
  int retval = -1;
  for (i = 1; i < 128; i++) {
    if (inode_array_ptr[i] - > valid == 0) {
      retval = i;
      break;
    }
  }
  return retval;
}

int findFreeBlock() {
  int i;
  for (i = 130; i < NUM_BLOCKS; i++) {
    if (used_blocks[i] == 0) {
      return i;
    }
  }
}

int findFreeInodeBlockEntry(int inode_index) {
  int i;
  int retval = -1;
  for (i =0; i<MAX_BLOCK_NUM_PER_FILE; i++) {
    if (inode_array_ptr[inode_index]->blocks[i] == 0) {
      retval = i;
      break;
    }
  }
    return retval;
}

int df_function() {
  int count = 0;
  int i;
  for (i = 0; i < NUM_BLOCKS; i++) {
    if (used_blocks[i] == 0) {
      count++;
    }
  }

  return count * BLOCK_SIZE;
}

char * readString(void) {
  int buffer_size = 1024;
  //parse and store
  char * command = malloc(sizeof(char) * buffer_size);
  memset(command, 0, 1024);
  if (!command) {
    exit(EXIT_FAILURE);
  }

  while (1) {
    //takes user input
    if (fgets(command, 1024, stdin) == NULL) return NULL;
    size_t length = strlen(command);

    // replace the end of the file with the null character
    if (command[length - 1] == '\n') {
      command[length - 1] = '\0';
      return command;
    }
    return command;
  }
}

char * * split_command(char * command) {
  int buffer_size = 255;
  char * * tokens = malloc(buffer_size * sizeof(char * ));
  char * token;
  token = strtok(command, " ");
  int i = 0;
  // tokenize the user command to treat it like separate strings.
  while (token != NULL) {
    tokens[i] = token;
    i++;
    token = strtok(NULL, " ");
  }
  tokens[i] = NULL;
  //   for (i =0; i<sizeof(tokens); i++)
  // {
  // printf("%s",tokens[i]);
  // }
  return tokens;

}

void put(char** cmd) {

    int status;
    struct stat buf;
    char * fileName = cmd[1];
    status = stat(fileName, & buf);

    if (status == -1) {
      printf("put error: File not found.\n");
      return;
    } else {
      int count = 0;
      copy_size = buf.st_size;
      if ((copy_size > 32 * BLOCK_SIZE)) {
          printf("File size too big\n");
          return;
        }
        if (copy_size > df()) {
          printf("put error: Not enough disk space.\n");
          return;
        }

          int dir_idx = findFreeDirectoryEntry();

          if (dir_idx == -1) {
            printf("put error: Not enough disk space.\n");
            return;
          }

            FILE * ifp = fopen(cmd[1], "r");
            directory_ptr[dir_idx].valid = 1;
            directory_ptr[dir_idx].name = (char*)malloc(strlen(cmd[1]));
            strncpy(directory_ptr[dir_idx].name,cmd[1],strlen(cmd[1]));

             // Store inode Index
             int inode_idx = findFreeInode();
          if (inode_idx == -1){
            printf("No free Inode");
            return;
          }
            directory_ptr[dir_idx].inode_idx = inode_idx;

            inode_array_ptr[inode_idx]->size = copy_size;
            inode_array_ptr[inode_idx]->date = time(NULL);
            inode_array_ptr[inode_idx]->valid= 1 ;

            int block_index = findFreeBlock();
         if  (block_index == -1) {
            printf("Error Cant find free Block");
            return;
        }

        while( copy_size >= BLOCK_SIZE )
    {
        //printf("Luc ASas ");
         if  (block_index == -1) {
            printf("Error Cant find free Block");
            return;
        }

         used_blocks[block_index]=1;
         int inode_block_entry = findFreeInodeBlockEntry(inode_idx);

        if(inode_block_entry == -1 ){

         printf("Luv Error: Cannot find free node block\n");
         return;
        }
         inode_array_ptr[inode_idx]->blocks[inode_block_entry] = block_index;
      // Index into the input file by offset number of bytes.  Initially offset is set to
      // zero so we copy BLOCK_SIZE number of bytes from the front of the file.  We
      // then increase the offset by BLOCK_SIZE and continue the process.  This will
      // make us copy from offsets 0, BLOCK_SIZE, 2*BLOCK_SIZE, 3*BLOCK_SIZE, etc.
      fseek( ifp, offset, SEEK_SET );

      // Read BLOCK_SIZE number of bytes from the input file and store them in our
      // data array.
      int bytes  = fread( data_blocks[block_index], BLOCK_SIZE, 1, ifp );

      // If bytes == 0 and we haven't reached the end of the file then something is
      // wrong. If 0 is returned and we also have the EOF flag set then that is OK.
      // It means we've reached the end of our input file.
      if( bytes == 0 && !feof( ifp ) )
      {
        printf("An error occured reading from the input file.\n");
        return;
}
        clearerr(ifp);
        copy_size -= BLOCK_SIZE;
        offset    += BLOCK_SIZE;
 }

      if (copy_size > 0) {
         int block_index = findFreeBlock();

         if(block_index == -1) {
            printf("Error Cant find free Block");
            return;
        }
         int inode_block_entry = findFreeInodeBlockEntry(inode_idx);

        // int inode_idx = findFreeInode();
        if(inode_block_entry == -1 ){
         printf("Tannu Error: Cannot find free node b
  int i;
  for (i = 1; i < 128; i++) {
  		if( directory_ptr[i].valid == 1)lock\n");
         return;
        }
         inode_array_ptr[inode_idx]->blocks[inode_block_entry] = block_index;
         used_blocks[block_index]=1;
         fseek(ifp,offset,SEEK_SET);
         int bytes  = fread( data_blocks[block_index], copy_size, 1, ifp );
        }
        fclose(ifp);
        return;
    }
}

   void list()
{
  		{
  			printf("%d\n"inode_array_ptr[i]->size);	
			printf("%s\n",directory_ptr[i].name);    	
			

  		}
	}
}
                int ind;
                int index = findFreeDirectoryEntry();
                for(ind = 0;ind<index;ind++){
                    if(strcmp(directory[ind].name,"\0")!=0){
                        char *time;
 	                       strcpy(time,directory[ind].time);
                        char buffer[5][10];
                        token = strtok(time,"\n\r ");
                        int k = 0;
                        while (token != NULL){
                            strcpy(buffer[k++],token);
                            token = strtok(NULL,"\n\r ");
                        }
                        printf("%d %s %s %s %s\n", directory[ind].size, buffer[1], buffer[2], buffer[3], directory[ind].name);
                    }

                }
            
        }

    int run_mfs(char * * cmd) {
      char * * command = cmd;
      while (1) {
        if (cmd[0] == '\0') return 1;

        if ((strcmp(cmd[0], "exit") == 0) || (strcmp(cmd[0], "quit") == 0)) {
          exit(1);
        }

        if (cmd[2] == NULL) {
          printf("\nPut something after command");
          return 0;
        }

        if (cmd[3] != NULL) {
          printf("\nWrong command");
          return 0;
        }

        if ((strcmp(cmd[0], "put") == 0)) {
          put(command);
          //printf("%s\n",cmd[1]);
          return 0;
        }

      }
    }

    int initialize_shell() {

      char * input_string;
      char * * command;
      int status;
      while (1) {
        printf("\nmfs> ");
        input_string = readString(); // call readString method to read all string
        if (input_string == NULL)
          continue;
        command = split_command(input_string); // call for split_command function which tokenize user command
        status = run_mfs(command); // call for run_msh function

        free(input_string); // releasing memory
        free(command); // releasing memory
      }
      return 0;

    }

    int main() {
      initialize_dir_inode();
      return initialize_shell();
    }