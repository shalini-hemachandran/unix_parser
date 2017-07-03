#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFF_SIZE 1024

char temp[] = "temp.txt";

int trail_trim_command(char* command) {
  int i;
  for (i = strlen(command) - 1; i > 0; --i) {
    if (command[i] == '\n' || command[i] == ' ') {
      command[i] = '\0';
    } else {
      break;
    }
  }
}

int first_nonspace(char* command) {
  int i;
  for (i = 0; i < strlen(command); ++i) {
    if (command[i] != ' ') {
      return i;
    }
  }
  return strlen(command);
}

void copy_file(const char* source, const char* dest, const char* dest_mode) {
  FILE* source_fp = fopen(source, "r");
  FILE* dest_fp = fopen(dest, dest_mode);

  char ch;
  while(1) {
    ch = fgetc(source_fp);
    if (ch == EOF) {
      break;
    } else {
      putc(ch, dest_fp);
    }
  }

  fclose(source_fp);
  fclose(dest_fp);
}

void process_read(char* command) {
  // Find <
  int i;
  for (i = 0; i < strlen(command); ++i) {
    if (command[i] == '<') {
      break;
    }
  }

  // Reach first non-space character.
  for (++i; i < strlen(command); ++i) {
    if (command[i] != ' ') {
      break;
    }
  }

  char source[BUFF_SIZE];
  int j;
  for (j = 0; i < strlen(command); ++i, ++j) {
    source[j] = command[i];
  }
  source[j] = '\0';

  copy_file(source, temp, "w");
}

void process_write(char* command) {
  // Find >
  int i;
  for (i = 0; i < strlen(command); ++i) {
    if (command[i] == '>') {
      break;
    }
  }

  // Reach first non-space character.
  for (++i; i < strlen(command); ++i) {
    if (command[i] != ' ') {
      break;
    }
  }

  char dest[BUFF_SIZE];
  int j;
  for (j = 0; i < strlen(command); ++i, ++j) {
    dest[j] = command[i];
  }
  dest[j] = '\0';

  copy_file(temp, dest, "w");
}

void process_append(char* command) {
  // Find >>
  int i;
  for (i = 1; i < strlen(command); ++i) {
    if (command[i - 1] == '>' && command[i] == '>') {
      break;
    }
  }

  // Reach first non-space character.
  for (++i; i < strlen(command); ++i) {
    if (command[i] != ' ') {
      break;
    }
  }

  char dest[BUFF_SIZE];
  int j;
  for (j = 0; i < strlen(command); ++i, ++j) {
    dest[j] = command[i];
  }
  dest[j] = '\0';

  copy_file(temp, dest, "a");
}

void process_command(char* command) {
  int i = first_nonspace(command);
  if (command[i] == 'r') {
    trail_trim_command(command);
    process_read(command);
  } else if (command[i] == 'w') {
    trail_trim_command(command);
    process_write(command);
  } else if (command[i] == 'a') {
    trail_trim_command(command);
    process_append(command);
  } else {
    printf("Invalid command!\n");
  }
}

int main() {
  while (1) {
    // Request and echo the command.
    printf("Please enter a command: ");
    char *command = NULL;
    size_t size;
    getline(&command, &size, stdin);
    trail_trim_command(command);
    printf("The command you entered is: %s\n", command);

    if (strcmp(command, "exit") == 0) {
      printf("Exiting...\n");
      exit(0);
    }

    // Fork a child process and execute the command in the child process.
    int child_pid = fork();
    if (child_pid != 0) {
      // Parent branch
      int return_status = 0;
      waitpid(child_pid, &return_status, 0);
      if (return_status != 0) {
        return return_status;
      }
    } else {
      // Child branch
      char *part;
      part = strtok(command, "|");
      while (part != NULL) {
        process_command(part);
        part = strtok(NULL, "|");
      }
      break;
    }
  }
  return 0;
}
