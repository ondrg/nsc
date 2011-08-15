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

#include <stdint.h>  /* uint*_t */
#include <stdbool.h>  /* bool */

#include <stdio.h>  /* TODO SMAZAT! Z duvodu ladeni */

#define STDIN 0   /**< standardni vstup */
#define STDOUT 1  /**< standardni vystup */
#define STDERR 2  /**< standardni chybovy vystup */

#define MIN_NUMBER_BASE 2   /**< minimalni ciselna soustava (min. je 2) */
#define MAX_NUMBER_BASE 36  /**< maximalni ciselna soustava (max. je 36) */

#define NUM_BLOCK_SIZE 600  /**< velikost bloku cisel (cislo delitelne 60) */


/**
 * Blok cisel.
 * Obousmerne vazany seznam.
 */
typedef struct listBlock {
  uint8_t num[NUM_BLOCK_SIZE];  /**< blok cisel */
  uint16_t numCount;  /**< pocet ulozenych cisel */
  struct listBlock *prev;
  struct listBlock *next;
} TListBlock;


/**
 * Struktura seznamu.
 * Ukazatele na zacatek a konec seznamu.
 */
typedef struct {
  TListBlock *first;
  TListBlock *last;
} TList;


/**
 * Struktura s kompletnim obsahem dat o cisle i soustavach.
 */
typedef struct {
  uint8_t inputNumberBase;  /**< ve ktere soustave je cislo */
  uint8_t outputNumberBase;  /**< do ktere soustavy se ma konvertovat */
  TList list;  /**< seznam obsahujici vstupni cislo */
} TNum;



/**
 * Kody stavu (predevsim chybovych).
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
  "",                                           /* EOK */
  "ERROR! Cannot allocate memory.\n",           /* EMEM */
  "ERROR! Read from standard input failed.\n",  /* EREAD */
  "ERROR! Bad format of input data.\n",         /* EINPUT */
  "ERROR! Input radix is out of range.\n",      /* EINPUTBASE */
  "ERROR! Output radix is out of range.\n",     /* EOUTPUTBASE */
  "ERROR! Unknown error.\n",                    /* EUNKNOWN */
};



/**
 * Vypise chybove hlaseni na standardni chybovy vystup.
 * @param error Kod chyby z vyctu codes.
 */
void printError(int error)
{
  if (error < EOK || error >= EUNKNOWN)
    error = EUNKNOWN;

  uint8_t msgLength = 0;  /* Pocet znaku v chybove hlasce */
  while (MSG[error][msgLength] != '\0')
    msgLength++;

  write(STDERR, MSG[error], msgLength);
}


/**
 * Vycisti vstupni buffer
 */
void clearBuffer(void)
{
  char buf[100];
  while (read(STDIN, &buf, 100) == 1)
    ;
}


/**
 * Inicializuje ukazatele na seznam.
 * @param list Ukazatel na strukturu TList.
 */
void inicializeList(TList *list)
{
  list->first = NULL;
  list->last = NULL;
}


/**
 * Inicializuje strukturu TNum
 * @param num Ukazatel na strukturu TNum
 */
void inicializeNum(TNum *num)
{
  num->inputNumberBase = 0;
  num->outputNumberBase = 0;
  inicializeList(&num->list);
}


/**
 * Prida novy listBlock na konec seznamu.
 * @param list Ukazatel na strukturu TList.
 * @return Ukazatel na prave pridany listBlock nebo NULL pri chybe.
 */
TListBlock *addNewListBlock(TList *list)
{
  TListBlock *listBlock = malloc(sizeof(TListBlock));
  if (listBlock == NULL)  /* chyba pri alokaci */
    return NULL;

  for (uint16_t i = 0; i < NUM_BLOCK_SIZE; i++)  /* inicializace cisel */
    listBlock->num[i] = 0;
  listBlock->numCount = 0;

  if (list->first == NULL)  /* pridavame prvni blok */
    list->first = listBlock;
  else  /* pridavame dalsi blok */
    list->last->next = listBlock;
  listBlock->prev = list->last;
  listBlock->next = NULL;
  list->last = listBlock;

  return listBlock;
}


/**
 * Zrusi listBlock a opravi navaznosti seznamu
 * @param listBlock Ukazatel na listBlock ke zruseni.
 * @param list Ukazatel na strukturu TList.
 */
void destroyListBlock(TListBlock *listBlock, TList *list)
{
  if (listBlock->prev != NULL)  /* existuje predchozi prvek */
    listBlock->prev->next = listBlock->next;
  if (listBlock->next != NULL)  /* existuje nasledujici prvek */
    listBlock->next->prev = listBlock->prev;

  /** Odstraneni z ukazatele na seznam */
  if (listBlock == list->first)
    list->first = listBlock->next;
  if (listBlock == list->last)
    list->last = listBlock->prev;

  free(listBlock);
}


/**
 * Zrusi seznam a uvolni veskerou pamet.
 * @param list Ukazatel na strukturu TList.
 */
void destroyList(TList *list)
{
  /*
     Zde je vyuzito ukazatele list->last na strukturu TListBlock
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
 * Vypocita jestli je jedna ciselna soustava mocninou druhe ciselne soustavy
 * @param baseOne Prvni ciselna soustava.
 * @param baseTwo Druha ciselna soustava.
 * @return n-ta mocnina, jinak false.
 */
uint8_t isPowerOfNumberBase(uint8_t baseOne, uint8_t baseTwo)
{
  /* TODO Slo by mozna napsat lepe pomoci prevodni tabulky */

  /** baseOne musi byt mensi nebo stejna nez baseTwo */
  if (baseOne > baseTwo) {
    /* prohozeni promennych pomoci operace XOR */
    baseTwo = baseOne ^ baseTwo;
    baseOne = baseTwo ^ baseOne;
    baseTwo = baseOne ^ baseTwo;
  }

  /** zjisteni urovne mocniny */
  uint8_t power = baseOne;  /**< uroven mocniny soustavy baseOne */
  uint8_t nPower = 1;  /**< n-ta mocnina */
  /* dokud uroven mocniny nepresahne maximalni ciselnou soustavu */
  while (power <= MAX_NUMBER_BASE) {
    if (power == baseTwo)  /* uroven mocniny nalezena */
      return nPower;  /* vracime hodnotu urovne */

    /* posun na dalsi uroven mocniny */
    power *= baseOne;
    nPower++;
  }

  return false;
}


/**
 * Overi, jestli je dany znak cislo
 * @param ch Znak.
 * @return true = je cislo, false = neni cislo
 */
bool isNumber(char ch)
{
  if (ch >= '0' && ch <= '9')  /* cislo */
    return true;

  return false;
}


/**
 * Overi, jesti je dany znak pismeno
 * @param ch Znak.
 * @return true = je cislo, false = neni cislo
 */
bool isLetter(char ch)
{
  if (ch >= 'A' && ch <= 'Z')  /* pismeno */
    return true;

  return false;
}


/**
 * Nacte data ze vstupu do struktury TNum
 * @param num Ukazatel na strukuturu typu TNum.
 * @return Kod z vyctu codes.
 */
uint8_t readInput(TNum *num)
{
  char buf[NUM_BLOCK_SIZE];  /**< nacitaci buffer */
  short readBytes;  /**< Pocet nactenych bytu */

  /** Kontrola prvniho vstupniho znaku */
  readBytes = read(STDIN, buf, 1);
  if (readBytes == -1)  /* chyba pri cteni ze vstupu */
    return EREAD;
  if (readBytes == 0)  /* nic jsme nenacetli */
    return EINPUT;
  if (buf[0] != '[')  /* vstpuni data nejsou ve spravnem formatu */
    return EINPUT;

  TListBlock *listBlock = NULL;  /**< ukazatel na aktualni blok */
  uint16_t i;  /**< iterator cyklu for */

  /** Nacitani vstpunich dat */
  while ((readBytes = read(STDIN, buf, NUM_BLOCK_SIZE)) != 0) {
    if (readBytes == -1) /* chyba pri cteni ze vstupu */
      return EREAD;

    listBlock = addNewListBlock(&num->list); 
    if (listBlock == NULL)  /* chyba pri alokaci pameti */
      return EMEM;

    /** Zpracovani vsech nactenych znaku */
    for (i = 0; i < readBytes; i++) {

      if (isNumber(buf[i])) {  /* nacitame cislo */
        listBlock->num[listBlock->numCount++] = (uint8_t) (buf[i] - '0');
      }
      else if (isLetter(buf[i])) {  /* nacitame pismeno */
        listBlock->num[listBlock->numCount++] = (uint8_t) (buf[i] - 'A' + 10);
      }
      else if (buf[i] == ']') {  /* konec nacitaneho cisla */
        /* FIXME Lze nacist "zadne cislo": []2=10 */
        /* FIXME Chybi osetredni vstupnich cisel dane soustavy
                 ([123]2=10 je chyba) */

        i++;  /* posunuti za znak ']' */

        /** Donacteni pripadnych nenactenych znaku */
        uint8_t lastReadBytes = readBytes - i;  /* zbyva znaku (pocet) */

        if (readBytes == NUM_BLOCK_SIZE) {  /* zrejme mame stale co nacitat */
          /* presun zbytku z konce na zacatek */
          uint8_t k = 0;
          while (i < readBytes && k <= 5)  /* max. 5 znaku je relevantnich */
            buf[k++] = buf[i++];
          i = 0;  /* nastaveni iteratoru na zacatek */

          /* nacteni zbytku (max. 5 znaku je relevantnich) */
          readBytes = read(STDIN, (buf + k), 5);
          lastReadBytes += readBytes;
          readBytes = 0;  /* priznak pouziti lastReadBytes */
        }

        /** Overeni poctu poslednich znaku (musi byt vice nez 3) */
        if (lastReadBytes < 3)
          return EINPUT;

        /** Zpracovani vstupni ciselne soustavy */
        if (isNumber(buf[i]))
          num->inputNumberBase = (uint8_t) (buf[i] - '0');
        else
          return EINPUT;

        i++;  /* posun na dalsi znak */

        if (buf[i] != '=') {  /* pokud je soustava dvojciferna */
          if (isNumber(buf[i])) {
            num->inputNumberBase = (num->inputNumberBase * 10)
                                   + (uint8_t) (buf[i] - '0');
            i++;  /* posun na dalsi znak */
          }
          else {
            return EINPUT;
          }
        }

        if (buf[i] != '=') {  /* pokud neexistuje oddelovac soustav */
          return EINPUT;
        }
        i++;  /* posun na dalsi znak */

        /* pokud neni co cist */
        if ((readBytes != 0 && (i + 1) >= readBytes) ||
            (readBytes == 0 && (i + 1) >= lastReadBytes)) {
          return EINPUT;
        }

        /** Zpracovani vystupni ciselne soustavy */
        if (isNumber(buf[i]))
          num->outputNumberBase = (uint8_t) (buf[i] - '0');
        else {
          return EINPUT;
        }

        i++;  /* posun na dalsi znak */

         /* pokud existuje posledni znak */
        if ((readBytes != 0 && (i + 1) < readBytes) ||
            (readBytes == 0 && (i + 1) < lastReadBytes)) {
          if (isNumber(buf[i])) {
            num->outputNumberBase = (num->outputNumberBase * 10)
                                    + (uint8_t) (buf[i] - '0');
          }
        }

        clearBuffer();
        break;
      }
      else {  /* neakceptovatelny znak */
        return EINPUT;
      }
    }
  }

  /** Kontrola rozmezi vstupni a vystupni ciselne soustavy */
  if (num->inputNumberBase < MIN_NUMBER_BASE ||
      num->inputNumberBase > MAX_NUMBER_BASE) {
    return EINPUTBASE;
  }
  if (num->outputNumberBase < MIN_NUMBER_BASE ||
      num->outputNumberBase > MAX_NUMBER_BASE) {
    return EOUTPUTBASE;
  }

  return EOK;
}


/**
 * Vypise obsah struktury TNum na vystup
 * @param num Ukazatel na strukuturu typu TNum.
 * @return Kod z vyctu codes.
 */
uint8_t printNumbers(TNum *num)
{
  char buf[NUM_BLOCK_SIZE];  /**< nacitaci buffer */
  uint16_t i;  /**< iterator cyklu for */

  write(STDOUT, "[", 1);  /* zacatek cisla */

  TListBlock *listBlock = num->list.first;
  while (listBlock != NULL) {
    for (i = 0; i < listBlock->numCount; i++) {
      if (listBlock->num[i] < 10)
        buf[i] = (char) (listBlock->num[i] + '0');
      else
        buf[i] = (char) (listBlock->num[i] + 'A' - 10);
    }
    write(STDOUT, buf, listBlock->numCount);
    listBlock = listBlock->next;

    if (listBlock != NULL)  /* zruseni zpracovaneho bloku */
      destroyListBlock(listBlock->prev, &num->list);
  }

  destroyList(&num->list);

  write(STDOUT, "]", 1);  /* konec cisla */
  if (num->outputNumberBase < 10) {  /* jednociferna soustava */
    buf[0] = (char) (num->outputNumberBase + '0');
    write(STDOUT, buf, 1);
  }
  else {  /* dvouciferna soustava */
    buf[0] = (char) ((num->outputNumberBase / 10) + '0');
    buf[1] = (char) ((num->outputNumberBase
                      - ((int) (buf[0] - '0') * 10)) + '0');
    write(STDOUT, buf, 2);
  }
  write(STDOUT, "\n", 1);  /* odradkovani */

  return EOK;
}


/**
 * Prevod pro cisla z nichz je jedna n-tou mocninou druhe
 * @param num Ukazatel na strukuturu typu TNum.
 * @param power N-ta mocnina jedne ze soustav.
 * @return Kod z vyctu codes.
 */
uint8_t powerConvert(TNum *num, uint8_t power)
{
  /* FIXME Nefunguje */

  char buf[NUM_BLOCK_SIZE];  /**< nacitaci buffer */
  TListBlock *listBlock = NULL;  /**< ukazatel na aktualni blok */
  uint16_t i;  /**< iterator cyklu for */
  uint8_t k;  /**< iterator cyklu for */

  /** Nastaveni spravnych poctu nacitanych a vypisovanych cisel */
  uint8_t inputNumbersCount = power;
  uint8_t outputNumbersCount = 1;
  if (num->inputNumberBase > num->outputNumberBase) {
    inputNumbersCount = 1;
    outputNumbersCount = power;
  }

  listBlock = num->list.first;

  while (listBlock != NULL) {
    for (i = 0; i < listBlock->numCount; i += inputNumbersCount) {
      for (k = 0; k < inputNumbersCount; k++) {
        if (listBlock->num[i] < 10)
          buf[i] = (char) (listBlock->num[i] + '0');
        else
          buf[i] = (char) (listBlock->num[i] + 'A' - 10);
      }
    }
    listBlock = listBlock->next;

    if (listBlock != NULL)  /* zruseni zpracovaneho bloku */
      destroyListBlock(listBlock->prev, &num->list);
  }

  return EOK;
}


/**
 * Univerzalni prevod mezi cisel soustavami
 * @param num Ukazatel na strukturu TNum.
 * @return Kod z vyctu codes.
 */
uint8_t universalConvert(TNum *num)
{
  /* FIXME Nefunguje */

  char buf[NUM_BLOCK_SIZE];  /**< nacitaci buffer */
  TListBlock *listBlock = NULL;  /**< ukazatel na aktualni blok */
  uint16_t i;  /**< iterator cyklu for */

  listBlock = num->list.first;
  while (listBlock != NULL) {
    for (i = 0; i < listBlock->numCount; i++) {
      if (listBlock->num[i] < 10)
        buf[i] = (char) (listBlock->num[i] + '0');
      else
        buf[i] = (char) (listBlock->num[i] + 'A' - 10);
    }
    listBlock = listBlock->next;

    if (listBlock != NULL)  /* zruseni zpracovaneho bloku */
      destroyListBlock(listBlock->prev, &num->list);
  }

  return EOK;
}


/**
 * Prevede cisla z jedne ciselne soustavy do druhe
 * Pouziva standardni vstup a standardni vystup
 */
uint8_t convertNumberBases(void)
{
  uint8_t state;  /**< navratovy kod funkci */

  TNum num;  /**< struktura pro zpracovani dat */
  inicializeNum(&num);

  /** Nacteni dat ze vstupu */
  state = readInput(&num);
  if (state != EOK) {  /* nacitani ze vstupu selhalo */    
    destroyList(&num.list);
    return state;
  }

  /** Konverze do vystupni ciselne soustavy */

  uint8_t power = 0;  /**< n-ta mocnina soutavy */

  /* pokud jsou ciselne soustavy stejne, dojde pouze k vypisu */
  if (num.inputNumberBase != num.outputNumberBase) {
    /* jedna z ciselnych soustav je n-tou mocninou te druhe */
    if ((power = isPowerOfNumberBase(num.inputNumberBase,
                                          num.outputNumberBase)) != false) {
      state = powerConvert(&num, power);
      if (state != EOK) {  /* prevod cisla selhal */
        destroyList(&num.list);
        return state;
      }
    }
    /* univerzalni prevod mezi ciselnymi soustavami */
    else {
      state = universalConvert(&num);
      if (state != EOK) {  /* prevod cisla selhal */
        destroyList(&num.list);
        return state;
      }
    }
  }

  /** Vypis na standardni vystup */

  state = printNumbers(&num);
  if (state != EOK) {  /* vypis na vystup selhal */
    destroyList(&num.list);
    return state;
  }

  return EOK;
}


/********************************** main() **********************************/

int main(void)
{
  uint8_t state = convertNumberBases();
  if (state != EOK) {  /* neco je spatne */
    clearBuffer();  /* TODO Musi tu byt? Overit! */
    printError(state);
    return state;
  }

  return EOK;
}
