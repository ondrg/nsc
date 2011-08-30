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

#define STDIN 0   /**< standardni vstup */
#define STDOUT 1  /**< standardni vystup */
#define STDERR 2  /**< standardni chybovy vystup */

/**< nahrada parametr funkce addNewListBlock smysluplnejsim nazvem */
#define FIRST true
#define LAST false

#define MIN_NUMBER_BASE 2   /**< minimalni ciselna soustava (min. je 2) */
#define MAX_NUMBER_BASE 36  /**< maximalni ciselna soustava (max. je 36) */

#define NUM_BLOCK_SIZE 600  /**< velikost bloku cisel (cislo delitelne 60) */


/**
 * Blok cisel.
 * Obousmerne vazany seznam.
 */
typedef struct listBlock {
/* TODO uint16_t je prilis velky datovy typ = plytvani pameti,
        nejvetsi cislo, ktere potrebujeme ulozit je 35 * 36 */
  uint16_t num[NUM_BLOCK_SIZE];  /**< blok cisel */
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
  EOK = 0,       /**< Vse v poradku */
  EMEM,          /**< Chyba pri alokaci pameti */
  EREAD,         /**< Chyba pri cteni ze vstupu */
  EINPUT,        /**< Chybny format vstupnich dat */
  EINPUTNUMBER,  /**< Chybne vstupni cislo */
  EINPUTBASE,    /**< Vstupni ciselna soustava je mimo rozsah */
  EOUTPUTBASE,   /**< Vystupni ciselna soustave je mimo rozsah */
  EUNKNOWN,      /**< Neznama chyba */
};


/**
 * Chybova hlaseni. Jejich poradi odpovida poradi konstant ve vyctu codes.
 */
const char *MSG[] = {
  "",                                           /* EOK */
  "ERROR! Cannot allocate memory.\n",           /* EMEM */
  "ERROR! Read from standard input failed.\n",  /* EREAD */
  "ERROR! Bad format of input data.\n",         /* EINPUT */
  "ERROR! Bad input number.\n",                 /* EINPUTNUMBER */
  "ERROR! Input radix is out of range.\n",      /* EINPUTBASE */
  "ERROR! Output radix is out of range.\n",     /* EOUTPUTBASE */
  "ERROR! Unknown error.\n",                    /* EUNKNOWN */
};



/**
 * Pole pro prevod cisel na znaky
 */
const char num2char[] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z',
};



/**
 * Vypise chybove hlaseni na standardni chybovy vystup.
 * @param error Kod chyby z vyctu codes.
 */
void printError(uint8_t error)
{
  if (error > EUNKNOWN)
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
 * Prida novy listBlock na konec nebo zacatek seznamu.
 * @param list Ukazatel na strukturu TList.
 * @param last Pridani noveho bloku na konec (true) nebo na zacatek (false).
 * @return Ukazatel na prave pridany listBlock nebo NULL pri chybe.
 */
TListBlock *addNewListBlock(TList *list, bool last)
{
  TListBlock *listBlock = malloc(sizeof(TListBlock));
  if (listBlock == NULL)  /* chyba pri alokaci */
    return NULL;

  for (uint16_t i = 0; i < NUM_BLOCK_SIZE; i++)  /* inicializace cisel */
    listBlock->num[i] = 0;
  listBlock->numCount = 0;

  /** pridani noveho bloku na konec (fronta) */
  if (last == true) {
    if (list->first == NULL)  /* pridavame prvni blok */
      list->first = listBlock;
    else  /* pridavame dalsi blok */
      list->last->next = listBlock;
    listBlock->prev = list->last;
    listBlock->next = NULL;
    list->last = listBlock;
  }
  /** pridani noveho bloku na zacatek (zasobnik) */
  else {
    if (list->last == NULL)  /* pridavame prvni blok */
      list->last = listBlock;
    else  /* pridavame dalsi blok */
      list->first->prev = listBlock;
    listBlock->prev = NULL;
    listBlock->next = list->first;
    list->first = listBlock;
  }

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

    listBlock = addNewListBlock(&num->list, FIRST); 
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
        /* Nacteni "zadneho cisla" neni povazovano za chybu '[]2=10' */

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
        if ((readBytes != 0 && (i + 1) <= readBytes) ||
            (readBytes == 0 && (i + 1) <= lastReadBytes)) {
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

  /** Odstraneni zbytecnych pocatecnich nul */
  listBlock = num->list.first;
  while (listBlock != NULL) {
    i = 0;
    /* dokud jsou v bloku nejake zbytecne nuly */
    while (listBlock->num[i] == 0 && listBlock->numCount != 0) {
      listBlock->numCount--;
      i++;
    }

    /* pokud je blok prazdny */
    if (listBlock->numCount == 0) {
      /* neexistuje nasledujici blok */
      if (listBlock->next == NULL) {
        /* nastavime jen jednu '0' */
        listBlock->num[0] = 0;
        listBlock->numCount = 1;
        break;
      }
      else {
        /* pokracujeme nasledujicim blokem a predchozi zrusime */
        listBlock = listBlock->next;
        destroyListBlock(listBlock->prev, &num->list);
      }
    }
    /* block neni prazdny */
    else {
      if (i != 0) {  /* musime posunovat */
        uint16_t j = 0;
        while (j < listBlock->numCount)
          listBlock->num[j++] = listBlock->num[i++];
      }
      break;
    }
  }

  /** Kontrola vstupnich cisel */
  listBlock = num->list.first;
  while (listBlock != NULL) {
    for (i = 0; i < listBlock->numCount; i++) {
      /* pokud cislo v dane ciselne soustave neexistuje */
      if (listBlock->num[i] >= num->inputNumberBase)
        return EINPUTNUMBER;
    }

    listBlock = listBlock->next;
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
      buf[i] = num2char[listBlock->num[i]];
    }
    write(STDOUT, buf, listBlock->numCount);
    listBlock = listBlock->next;

    if (listBlock != NULL)  /* zruseni zpracovaneho bloku */
      destroyListBlock(listBlock->prev, &num->list);
  }

  destroyList(&num->list);

  write(STDOUT, "]", 1);  /* konec cisla */
  if (num->outputNumberBase < 10) {  /* jednociferna soustava */
    buf[0] = num2char[num->outputNumberBase];
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
 * Funkce NEOVERUJE podminku n-te mocniny soustav!
 * @param num Ukazatel na strukuturu typu TNum.
 * @param power N-ta mocnina jedne ze soustav.
 * @return Kod z vyctu codes.
 */
uint8_t powerConvert(TNum *num, uint8_t power)
{
  TList list;  /**< vystupni seznam pro data */
  TListBlock *listBlock = NULL;  /**< ukazatel na aktualni blok */
  TListBlock *outputListBlock = NULL;  /**< ukazatel na vystupni blok */
  uint16_t i = 0;  /**< iterator cyklu for (pro vstupni seznam) */
  uint16_t j = 0;  /**< iterator cyklu for (pro vystupni seznam) */
  int8_t k;  /**< iterator cyklu for */

  /* mocniny 2 az 6 (co je '0' se nepouziva) */
  const uint8_t numPower[][6] = {
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {1, 2, 4, 8, 16, 32},  /* 2 */
    {1, 3, 9, 27, 0, 0},   /* 3 */
    {1, 4, 16, 0, 0, 0},   /* 4 */
    {1, 5, 25, 0, 0, 0},   /* 5 */
    {1, 6, 36, 0, 0, 0},   /* 6 */
  };

  inicializeList(&list);
  listBlock = num->list.first;
  j = NUM_BLOCK_SIZE;  /* nastaveni potreby alokace noveho bloku */
  power--;  /* max. uroven mocniny je o 1 mensi */

  /** vstupni soutava < vystupni soustava */
  if (num->inputNumberBase < num->outputNumberBase) {
    /* alokace a inicializace noveho bloku */
    outputListBlock = addNewListBlock(&list, FIRST);
    if (outputListBlock == NULL) {  /* chyba pri alokaci pameti */
      destroyList(&list);
      return EMEM;
    }
    j = 0;

    /* prevod neuplne prvni casti (pokud je treba) */
    k = num->list.last->numCount % (power + 1);
    if (k != 0) {
      /* prevod */
      while (k > 1) {
        k--;
        outputListBlock->num[j] += listBlock->num[i++]
                                   * numPower[num->inputNumberBase][k];
      }
      outputListBlock->num[j++] += listBlock->num[i++];
    }

    /* prevod po castech */
    if (i != listBlock->numCount) {  /* mame co prevadet */
      while (listBlock != NULL) {
        /* prevod */
        for (k = power; k >= 0; k--) {
          outputListBlock->num[j] += listBlock->num[i]
                                    * numPower[num->inputNumberBase][k];
          i++;

          /* posun na dalsi vstupni blok */
          if (i == listBlock->numCount) {
            listBlock = listBlock->next;
            i = 0;

            if (listBlock != NULL)  /* zruseni zpracovaneho bloku */
              destroyListBlock(listBlock->prev, &num->list);
            else
              break;
          }
        }
        j++;
        //outputListBlock->num[j++] += listBlock->num[i++];

        /* je treba alokovat novy blok? */
        if (j == NUM_BLOCK_SIZE) {
          outputListBlock->numCount = j;  /* pocet nactenych cisel */
          outputListBlock = addNewListBlock(&list, FIRST);
          if (outputListBlock == NULL) {  /* chyba pri alokaci pameti */
            destroyList(&list);
            return EMEM;
          }
          j = 0;
        }
      }
    }

    outputListBlock->numCount = j;  /* pocet nactenych cisel */
  }
  /** vstupni soustava > vystupni soustava */
  else {
    int8_t subtract;  /**< promenna pro ulozeni vysledku po odecteni */

    while (listBlock != NULL) {
      for (i = 0; i < listBlock->numCount; i++) {
        /* je treba alokovat novy blok? */
        if (j == NUM_BLOCK_SIZE) {
          outputListBlock = addNewListBlock(&list, FIRST);
          if (outputListBlock == NULL) {  /* chyba pri alokaci pameti */
            destroyList(&list);
            return EMEM;
          }
          j = 0;
        }

        /* prevod */
        for (k = power; k >= 1; k--) {
          subtract = listBlock->num[i];
          while ((subtract -= numPower[num->outputNumberBase][k]) >= 0) {
            listBlock->num[i] = subtract;
            outputListBlock->num[j]++;
          }
          j++;
        }
        outputListBlock->num[j++] = listBlock->num[i];
        outputListBlock->numCount = j;  /* pocet nactenych cisel */
      }
      listBlock = listBlock->next;

      if (listBlock != NULL)  /* zruseni zpracovaneho bloku */
        destroyListBlock(listBlock->prev, &num->list);
    }

    /* odstaneni '0' ze zacatku */
    outputListBlock = list.first;
    i = 0;
    /* dokud jsou v bloku nejake zbytecne nuly */
    while (outputListBlock->num[i] == 0 && outputListBlock->numCount != 0) {
      outputListBlock->numCount--;
      i++;
    }

    /* pokud je blok prazdny */
    if (outputListBlock->numCount == 0) {
      /* nastavime jen jednu '0' */
      outputListBlock->num[0] = 0;
      outputListBlock->numCount = 1;
    }
    /* blok neni prazdny */
    else {
      if (i != 0) {  /* musime posunovat */
        j = 0;
        while (j < outputListBlock->numCount) {
          outputListBlock->num[j++] = outputListBlock->num[i++];
        }
      }
    }
  }

  /** Zruseni stareho a navazani vystupniho seznamu */
  destroyList(&num->list);
  num->list.first = list.first;
  num->list.last = list.last;

  return EOK;
}


/**
 * Univerzalni prevod mezi cisel soustavami
 * @param num Ukazatel na strukturu TNum.
 * @return Kod z vyctu codes.
 */
uint8_t universalConvert(TNum *num)
{
  TList list;  /**< vystupni seznam pro data */
  TListBlock *listBlock = NULL;  /**< ukazatel na aktualni blok */
  TListBlock *outputListBlock = NULL;  /**< ukazatel na vystupni blok */
  uint16_t i = 0;  /**< iterator cyklu for */
  uint16_t j;  /**< iterator cyklu for (pro vystupni seznam) */
  uint16_t k;  /**< iterator cyklu pro prochazeni bloku */
  uint16_t borrow = 0;  /**< hodnota na preneseni do dalsiho ciselneho radu */

  /** Inicializace */
  inicializeList(&list);
  listBlock = num->list.first;

  outputListBlock = addNewListBlock(&list, LAST);
  if (outputListBlock == NULL) {  /* chyba pri alokaci pameti */
    destroyList(&list);
    return EMEM;
  }
  outputListBlock->numCount++;  /* kvuli nacteni prvniho cisla */

  /** Prevod */
  while (listBlock != NULL) {
    while (i < listBlock->numCount) {
      /* (all) list * num->outputNumberBase */
      outputListBlock = list.last;
      outputListBlock->num[NUM_BLOCK_SIZE - 1] *= num->inputNumberBase;
      /* (all) list + listBlock->num[i] */
      outputListBlock->num[NUM_BLOCK_SIZE - 1] += listBlock->num[i];
      j = NUM_BLOCK_SIZE - 2;
      k = 1;
      while (outputListBlock != NULL) {  /* cely seznam */
        /* vynasobeni cisla */
        while (k < outputListBlock->numCount) {  /* vsechny cisla */
          outputListBlock->num[j--] *= num->inputNumberBase;
          k++;
        }

        /* posun na dalsi blok a inicializace prochazeni seznamu */
        outputListBlock = outputListBlock->prev;
        j = NUM_BLOCK_SIZE - 1;
        k = 0;
      }

      /* prepocet do spravne soustavy */
      outputListBlock = list.last;
      while (outputListBlock != NULL) {  /* cely seznam */
        k = 0;
        while (k < outputListBlock->numCount) {  /* vsechny cisla */
          /* navraceni vypujcky */
          if (borrow != 0) {
            outputListBlock->num[j] += borrow;
            borrow = 0;
          }

          /* vypocet vypujcky */
          if (outputListBlock->num[j] >= num->outputNumberBase) {
            /* TODO Nejvetsi zrout vykonu => vymyslet efektivnejsi reseni */
            borrow = outputListBlock->num[j] / num->outputNumberBase;
            outputListBlock->num[j] = outputListBlock->num[j]
                                      % num->outputNumberBase;
          }

          j--;
          k++;

          /* je treba pridat dalsi prvek? */
          if (k == outputListBlock->numCount && borrow != 0) {
            /* jsme na konci bloku? */
            if (outputListBlock->numCount == NUM_BLOCK_SIZE) {
              /* jsme na konci seznamu? */
              if (outputListBlock->prev == NULL) {
                /* ano, pridame dalsi blok */
                outputListBlock = addNewListBlock(&list, LAST);
                if (outputListBlock == NULL) {  /* chyba pri alokaci pameti */
                  destroyList(&list);
                  return EMEM;
                }

                /* zapocitame vypujcku */
                outputListBlock->num[NUM_BLOCK_SIZE - 1] = borrow;
                borrow = 0;
                outputListBlock->numCount++;

                /* navrat na zpracovavany blok seznamu */
                outputListBlock = outputListBlock->next;
              }

              break;
            }
            else
              outputListBlock->numCount++;  /* pridani ciselneho mista */
          }
        }

        /* posun na dalsi blok a inicializace prochazeni seznamu */
        outputListBlock = outputListBlock->prev;
        j = NUM_BLOCK_SIZE - 1;
        k = 0;
      }

      i++;
    }
    i = 0;  /* vynulovani iteratoru */
    listBlock = listBlock->next;

    if (listBlock != NULL)  /* zruseni zpracovaneho bloku */
      destroyListBlock(listBlock->prev, &num->list);
  }

  /** Posun neuplneho seznamu */
  outputListBlock = list.first;
  if (outputListBlock->numCount != NUM_BLOCK_SIZE) {  /* seznam je neuplny */
    j = NUM_BLOCK_SIZE - outputListBlock->numCount;
    i = 0;
    while (i < outputListBlock->numCount)
      outputListBlock->num[i++] = outputListBlock->num[j++];
  }

  /** Zruseni stareho a navazani vystupniho seznamu */
  destroyList(&num->list);
  num->list.first = list.first;
  num->list.last = list.last;

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

  /* pokud jsou ciselne soustavy stejne, dojde pouze k vypisu */
  if (num.inputNumberBase != num.outputNumberBase) {
    /* jedna z ciselnych soustav je n-tou mocninou te druhe */
    uint8_t power;  /**< n-ta mocnina soutavy */
    power = isPowerOfNumberBase(num.inputNumberBase, num.outputNumberBase);
    if (power != false) {
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
