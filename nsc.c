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

#include <stdint.h>  /* uint8_t */

#include <stdio.h>  /* TODO SMAZAT! Z duvodu ladeni */

#define STDIN 0   /**< standardni vstup */
#define STDOUT 1  /**< standardni vystup */
#define STDERR 2  /**< standardni chybovy vystup */

#define NUM_BLOCK_SIZE 120  /**< velikost bloku cisel (cislo delitelne 60) */

#define TRUE 1
#define FALSE 0


/**
 * Blok cisel.
 * Obousmerne vazany seznam.
 */
typedef struct numBlock {
  uint8_t num[NUM_BLOCK_SIZE];  /**< blok cisel */
  unsigned numCount;  /**< pocet ulozenych cisel */
  struct numBlock *prev;
  struct numBlock *next;
} TNumBlock;


/**
 * Ukazatele na zacatek a konec seznamu.
 */
typedef struct {
  TNumBlock *first;
  TNumBlock *last;
} TNum;



/**
 * Kody stavu (predevsim chybovych)
 */
enum codes {
  EOK = 0,      /**< Vse v poradku */
  EMEM,         /**< Chyba pri alokaci pameti */
  EREAD,        /**< Chyba pri cteni ze vstupu */
  EINPUT,       /**< Chybny format vstupnich dat */
  EINPUTBASE,   /**< Vstupni ciselna soustava je mimo rozsah */
  EOUTPUTBASE,  /**< Vystupni ciselna soustave je mimo rozsah */
  EUNKNOWN,     /**< Neznama chyba */
};


/**
 * Chybova hlaseni. Jejich poradi odpovida poradi konstant ve vyctu codes.
 */
const char *MSG[] = {
  "OK.",
  "ERROR! Cannot allocate memory.",
  "ERROR! Read from standard input failed.",
  "ERROR! Bad format of input data.",
  "ERROR! Input radix is out of range <2,36>.",
  "ERROR! Output radix is out of range <2,36>.",
  "ERROR! Unknown error.",
};



/**
 * Vypise chybove hlaseni na standardni chybovy vystup.
 * @param error Kod chyby z vyctu codes.
 */
void printError(int error)
{
  if (error < EOK || error >= EUNKNOWN)
    error = EUNKNOWN;

  unsigned int msgLength = 0;  /* Pocet znaku v chybove hlasce */
  while (MSG[error][msgLength] != '\0')
    msgLength++;

  write(STDERR, MSG[error], msgLength);
  write(STDERR, "\n", 1);
}


/**
 * Vycisti vstupni buffer
 */
void clearBuffer(void)
{
  /* TODO Asi nefunguje moc dobre. */
  char buf;
  while (read(STDIN, &buf, 1) == 1)
    ;
}


/**
 * Inicializuje ukazatele na seznam.
 * @param list Ukazatel na strukturu TNum.
 */
void inicializeList(TNum *list)
{
  list->first = NULL;
  list->last = NULL;
}


/**
 * Prida novy numBlock na konec seznamu.
 * @param list Ukazatel na strukturu TNum.
 * @return Ukazatel na prave pridany numBlock nebo NULL pri chybe.
 */
TNumBlock *addNewNumBlock(TNum *list)
{
  TNumBlock *numBlock = malloc(sizeof(TNumBlock));
  if (numBlock == NULL)  /* chyba pri alokaci */
    return NULL;

  for (int i = 0; i < NUM_BLOCK_SIZE; i++)  /* inicializace cisel */
    numBlock->num[i] = 0;
  numBlock->numCount = 0;

  if (list->first == NULL) {  /* pridavame prvni blok */
    list->first = numBlock;
  }
  else {  /* pridavame dalsi blok */
    list->last->next = numBlock;
  }
  numBlock->prev = list->last;
  numBlock->next = NULL;
  list->last = numBlock;

  return numBlock;
}


/**
 * Zrusi numBlock a opravi navaznosti seznamu
 * @param numBlock Ukazatel na numBlock ke zruseni.
 * @param list Ukazatel na strukturu TNum.
 */
void destroyNumBlock(TNumBlock *numBlock, TNum *list)
{
  if (numBlock->prev != NULL)  /* existuje predchozi prvek */
    numBlock->prev->next = numBlock->next;
  if (numBlock->next != NULL)  /* existuje nasledujici prvek */
    numBlock->next->prev = numBlock->prev;

  /** Odstraneni z ukazatele na seznam */
  if (numBlock == list->first)
    list->first = NULL;
  if (numBlock == list->last)
    list->last = NULL;

  free(numBlock);
}


/**
 * Zrusi seznam a uvolni veskerou pamet.
 * @param list Ukazatel na strukturu TNum.
 */
void destroyList(TNum *list)
{
  /*
     POZOR! Zde je vyuzito ukazatele list->last na strukturu TNumBlock
     jako pomocneho ukazatele z duvodu nepotrebnosti.
     Je mozne si vytvorit novy pomocny ukazatel, coz by bylo reseni
     prehlednejsi, ovsem ne uspornejsi.
  */

  while (list->first != NULL) {
    list->last = list->first;
    list->first = list->first->next;
    free(list->last);
  }

  list->last = NULL;
}


/**
 * Overi, jestli je dany znak cislo
 * @param ch Znak.
 * @return TRUE = je cislo, FALSE = neni cislo
 */
short isNumber(char ch)
{
  if (ch >= '0' && ch <= '9')  /* cislo */
    return TRUE;

  return FALSE;
}


/**
 * Overi, jesti je dany znak pismeno
 * @param ch Znak.
 * @return TRUE = je cislo, FALSE = neni cislo
 */
short isLetter(char ch)
{
  if (ch >= 'A' && ch <= 'Z')  /* pismeno */
    return TRUE;

  return FALSE;
}


/**
 * Prevede cisla z jedne ciselne soustavy do druhe
 * Pouziva standardni vstup a standardni vystup
 */
int convertNumberBases(void)
{
  TNum list;  /**< Obousmerny seznam */
  inicializeList(&list);

  char buf[NUM_BLOCK_SIZE];  /**< nacitaci buffer */
  int readBytes;  /**< Pocet nactenych bytu */
  TNumBlock *numBlock = NULL;  /**< ukazatel na aktualni blok */

  /** Kontrola prvniho vstupniho znaku */
  readBytes = read(STDIN, buf, 1);
  if (readBytes == -1)  /* chyba pri cteni ze vstupu */
    return EREAD;
  if (readBytes == 0)  /* nic jsme nenacetli */
    return EINPUT;
  if (buf[0] != '[')  /* vstpuni data nejsou ve spravnem formatu */
    return EINPUT;

  unsigned short inputNumberBase = 0;  /**< vstupni ciselna soustava */
  unsigned short outputNumberBase = 0;  /**< vystupni ciselna soustava */

  /** Nacitani vstpunich dat */
  while ((readBytes = read(STDIN, buf, NUM_BLOCK_SIZE)) != 0) {
    if (readBytes == -1) {  /* chyba pri cteni ze vstupu */
      destroyList(&list);
      return EREAD;
    }

    numBlock = addNewNumBlock(&list); 
    if (numBlock == NULL) {  /* chyba pri alokaci pameti */
      destroyList(&list);
      return EMEM;
    }

    /** Zpracovani vsech nactenych znaku */
    for (int i = 0; i < readBytes; i++) {

      if (isNumber(buf[i]))  /* nacitame cislo */
        numBlock->num[numBlock->numCount++] = (uint8_t) (buf[i] - '0');
      else if (isLetter(buf[i]))  /* nacitame pismeno */
        numBlock->num[numBlock->numCount++] = (uint8_t) (buf[i] - 'A' + 10);
      else if (buf[i] == ']') {  /* konec nacitaneho cisla */
        /* TODO Nacteni zadanych ciselnych soustav neni idealni! */

        /* minimalne 3 znaky na definovani ciselne soustavy */
        if ((i + 3) >= readBytes) {
          destroyList(&list);
          return EINPUT;
        }
        i++;  /* posunuti za znak ] */

        /** Zpracovani vstupni ciselne soustavy */
        if (isNumber(buf[i]))
          inputNumberBase = (unsigned short) (buf[i] - '0');
        else {
          destroyList(&list);
          return EINPUT;
        }
        i++;  /* posun na dalsi znak */

        if (buf[i] != '=') {  /* pokud je soustava dvojciferna */
          if (isNumber(buf[i])) {
            inputNumberBase = (inputNumberBase * 10)
                              + (unsigned short) (buf[i] - '0');
            i++;  /* posun na dalsi znak */
          }
          else {
            destroyList(&list);
            return EINPUT;
          }
        }

        if (buf[i] != '=') {  /* pokud neexistuje oddelovac soustav */
          destroyList(&list);
          return EINPUT;
        }
        if ((i + 1) >= readBytes) {  /* neni co cist */
          destroyList(&list);
          return EINPUT;
        }
        i++;  /* posun na dalsi znak */

        /** Zpracovani vystupni ciselne soustavy */
        if (isNumber(buf[i]))
          outputNumberBase = (unsigned short) (buf[i] - '0');
        else {
          destroyList(&list);
          return EINPUT;
        }
        /* TODO Nacitani posledniho znaku je podezrele => proverit! */
        if ((i + 1) >= readBytes) {  /* neni co cist */
          destroyList(&list);
          return EINPUT;
        }
        i++;  /* posun na dalsi znak */

        if (isNumber(buf[i]))
          outputNumberBase = (outputNumberBase * 10)
                             + (unsigned short) (buf[i] - '0');

        clearBuffer();
        break;
      }
      else {  /* neakceptovatelny znak */
        destroyList(&list);
        return EINPUT;
      }
    }
  }

  /** Kontrola rozmezi vstupni a vystupni ciselne soustavy */
  if (inputNumberBase < 2 || inputNumberBase > 36) {
    destroyList(&list);
    return EINPUTBASE;
  }
  if (outputNumberBase < 2 || outputNumberBase > 36) {
    destroyList(&list);
    return EOUTPUTBASE;
  }

  /* TODO Chybi osetreni vstupnich cisel dane soustavy ([123]2=10 je chyba) */

  write(STDOUT, "[", 1);  /* zacatek cisla */

  /** Konverze do vystupni ciselne soustavy a vypis na standardni vystup */
  if (inputNumberBase == outputNumberBase) {  /* ciselne soutavy jsou stejne */
    numBlock = list.first;
    while (numBlock != NULL) {
      for (unsigned i = 0; i < numBlock->numCount; i++) {
        if (numBlock->num[i] < 10)
          buf[i] = (char) (numBlock->num[i] + '0');
        else
          buf[i] = (char) (numBlock->num[i] + 'A' - 10);
      }
      write(STDOUT, buf, numBlock->numCount);
      numBlock = numBlock->next;

      if (numBlock != NULL)  /* zruseni zpracovaneho bloku */
        destroyNumBlock(numBlock->prev, &list);
    }
  }

  destroyList(&list);

  write(STDOUT, "]", 1);  /* konec cisla */
  if (outputNumberBase < 10) {  /* jednociferna soustava */
    buf[0] = (char) (outputNumberBase + '0');
    write(STDOUT, buf, 1);
  }
  else {  /* dvouciferna soustava */
    buf[0] = (char) ((outputNumberBase / 10) + '0');
    buf[1] = (char) ((outputNumberBase - ((int) (buf[0] - '0') * 10)) + '0');
    write(STDOUT, buf, 2);
  }
  write(STDOUT, "\n", 1);  /* odradkovani */

  return EOK;
}


/********************************** main() **********************************/

int main(void)
{
  int state = convertNumberBases();
  if (state != EOK) {  /* neco je spatne */
    clearBuffer();
    printError(state);
    return state;
  }

  return EOK;
}

