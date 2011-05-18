#include <unistd.h>  /* read, write */
#include <stdlib.h>  /* malloc, free */

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define BUFFER_SIZE 30

int main()
{
  char *buf = malloc(BUFFER_SIZE * sizeof(char));

  read(STDIN, buf, 30);
  write(STDOUT, "stdout ", 8);
  write(STDOUT, buf, 4);
  write(STDOUT, "\n", 1);
  write(STDERR, "error\n", 6);

  free(buf);

  return 0;
}

