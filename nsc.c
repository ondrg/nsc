/**
 * Soubor:    nsc.c
 * Datum:     2011/05/18
 * Autor:     Ondrej Gersl, ondra.gersl@gmail.com
 * Projekt:   Prevod cisla [www.joineset.com]
 * Popis:     Aplikace provadi prevody cisel mezi ciselnymi soustavami.
 *            Muze se jednat o velmi velka cisla, v radu 10^(10^10).
 *            Pro cteni cisla se pouziva standardni vstup,
 *            pro vypis standardni vystup.
 *            Format: [XXXX]Z1=Z2
 *            Napr.: [1012222121310101]4=32 (prevod ze 4-kove do 32 soustavy)
 *            Je mozne pouzit knihovni funkce, avsak pouze
 *            read, write, malloc a free.
 *            http://www.joineset.cz/prevod_cisla.html
 */

#include <stdlib.h>  /* malloc, free */
#include <unistd.h>  /* read, write */

#define STDIN 0   /* standardni vstup */
#define STDOUT 1  /* standardni vystup */
#define STDERR 2  /* standardni chybovy vystup */

#define BUFFER_SIZE 30


/********************************** main() **********************************/

int main(void)
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

