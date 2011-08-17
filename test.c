/**
 * Soubor:    generate-test.c
 * Datum:     2011/05/28
 * Autor:     Ondrej Gersl, ondra.gersl@gmail.com
 * Projekt:   Prevod cisla [www.joineset.com]
 * Popis:     Generuje testovaci data pro aplikaci nsc slouzici k prevodu mezi
 *            ciselnymi soustavami.
 *            Vystupni format: [XXXX]Z1=Z2
 *            Napr.: [1012222121310101]4=32 (prevod ze 4-kove do 32 soustavy)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_NUMBER_BASE 2   /**< minimalni ciselna soustava (min. je 2) */
#define MAX_NUMBER_BASE 36  /**< maximalni ciselna soustava (max. je 36) */


/**
 * Kody stavu (predevsim chybovych)
 */
enum codes {
  EOK = 0,      /**< Vse v poradku */
  EPARAM,       /**< Spatne zadane parametry */
  EUNKNOWN,     /**< Neznama chyba */
};


/**
 * Chybova hlaseni. Jejich poradi odpovida poradi konstant ve vyctu codes.
 */
const char *MSG[] = {
  "OK.",
  "ERROR! Bad parameters.",
  "ERROR! Unknown error.",
};


/**
 * Znakove zastoupeni ciselnych sad
 */
const char numbers[] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z',
};



/**
 * Vypise chybove hlaseni na standardni chybovy vystup.
 * @param error Kod chyby z vyctu codes.
 */
void printError(int error)
{
  if (error < EOK || error >= EUNKNOWN)
    error = EUNKNOWN;

  fprintf(stderr, "%s\n", MSG[error]);
}


/**
 *
 */
char getRandomNumber(unsigned short base)
{
  return numbers[rand() % base];
}



/********************************** main() **********************************/

int main(int argc, char *argv[])
{
  /** Musi byt presne 3 argumenty */
  if (argc != 4) {
    printError(EPARAM);
    return EPARAM;
  }

  unsigned short fromBase = 0;
  unsigned short toBase = 0;
  unsigned long long int countOfNumbers = 0;

  srand((unsigned int) time(NULL));

  /** Prevod znaku na cisla */
  if (sscanf(argv[1], "%hu", &fromBase) != 1 ||
      sscanf(argv[2], "%hu", &toBase) != 1 ||
      sscanf(argv[3], "%llu", &countOfNumbers) != 1) {
    printError(EPARAM);
    return EPARAM;
  }

  /** Kontrola rozsahu hodnot */
  if (fromBase < MIN_NUMBER_BASE || fromBase > MAX_NUMBER_BASE ||
      toBase < MIN_NUMBER_BASE || toBase > MAX_NUMBER_BASE ||
      countOfNumbers <= 0) {
    printError(EPARAM);
    return EPARAM;
  }

  /* Zobrazi nactene hodnoty
  printf("fromBase: %u\ntoBase: %u\ncountOfNumbers: %llu\n",
         fromBase, toBase, countOfNumbers);
  */

  /** Vypis na vystup */
  putchar('[');

  /* Prvni cislo nesmi byt 0 */
  char firstNumber;
  do
    firstNumber = getRandomNumber(fromBase);
  while (firstNumber == '0');
  putchar(firstNumber);
  countOfNumbers--;

  /* Generovani zbytku cisel */
  while (countOfNumbers-- > 0)
    putchar(getRandomNumber(fromBase));
  printf("]%u=%u\n", fromBase, toBase);

  return EOK;
}

