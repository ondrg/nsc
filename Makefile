#
# Datum:    2011/05/18
# Autor:    Ondrej Gersl, ondra.gersl@gmail.com
# Projekt:  Prevod cisla [www.joineset.com]
#
# Pouziti:
#   - preklad:        make
#   - spusteni:				make run
#   - editace:       	make edit
#
#   - dokumentace:		make doc
#   - zabaleni:       make pack
#
#   - strip:          make strip
#   - vycisteni:      make clean
#


# prekladac
CC=gcc

# parametry prekladace
CFLAGS=-std=c99 -Wall -Wextra -pedantic -g

# nazev aplikace
APP=nsc

# soubory pro vytvoreni archivu
PACK=$(APP).c Makefile *.txt Doxyfile


all: $(APP)

$(APP): $(APP).c
	$(CC) $(CFLAGS) -o $(APP) $(APP).c

run:
	./$(APP)

edit:
	vim $(APP).c


doc:
	doxygen Doxyfile

pack:
	tar cvzf $(APP).tgz $(PACK) --exclude $(APP).tgz


strip:
	strip $(APP)

clean:
	rm -rf $(APP) doc/ $(APP).tgz

