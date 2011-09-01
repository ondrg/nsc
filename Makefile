#
# Datum:    2011/05/18
# Autor:    Ondrej Gersl, ondra.gersl@gmail.com
# Projekt:  Prevod cisla [www.joineset.com]
#
# Pouziti:
#   - preklad:        make
#   - spusteni:				make run
#   - editace:       	make edit
#		- debugovani: 		make debug
#
#   - dokumentace:		make doxygen
#   - zabaleni:       make pack
#
#   - strip:          make strip
#   - vycisteni:      make clean
#
#		- predklad aplikace pro grenerovani testu:		make test
#		- generovani testovych prikladu:							make generate
#		- generovani obrovskych testovych prikladu:   make generatehuge
#


# prekladac
CC=gcc

# parametry prekladace
#CFLAGS=-std=c99 -Wall -Wextra -pedantic -g
CFLAGS=-std=c99 -Wall -Wextra -pedantic -O3

# nazvy aplikaci
APP=nsc
TEST=test

# nazvy slozek
TESTS=tests

# soubory pro vytvoreni archivu
PACK=$(APP).c Makefile *.txt Doxyfile documentation.pdf


all: $(APP)

$(APP): $(APP).c
	$(CC) $(CFLAGS) -o $(APP) $(APP).c

run:
	./$(APP)

edit:
	vim $(APP).c

debug:
	ddd ./$(APP)


doxygen:
	doxygen Doxyfile

pack:
	tar cvzf $(APP).tgz $(PACK) --exclude $(APP).tgz


strip:
	strip $(APP)

clean:
	rm -rf $(APP) doc/ $(APP).tgz $(TEST) $(TESTS)


$(TEST): $(TEST).c
	$(CC) $(CFLAGS) -o $(TEST) $(TEST).c

generate:
	mkdir -p $(TESTS)
	./$(TEST) 10 10 10 > $(TESTS)/10to10.txt
	./$(TEST) 10 2 10 > $(TESTS)/10to2.txt
	./$(TEST) 2 10 10 > $(TESTS)/2to10.txt
	./$(TEST) 2 2 10 > $(TESTS)/2to2.txt
	./$(TEST) 2 16 10 > $(TESTS)/2to16.txt
	./$(TEST) 16 16 10 > $(TESTS)/16to16.txt
	./$(TEST) 16 2 10 > $(TESTS)/16to2.txt
	./$(TEST) 2 8 10 > $(TESTS)/2to8.txt
	./$(TEST) 8 8 10 > $(TESTS)/8to8.txt
	./$(TEST) 8 2 10 > $(TESTS)/8to2.txt

generatehuge:
	mkdir -p $(TESTS)
	./$(TEST) 10 2 1047546 > $(TESTS)/1M.txt  # 1 MB
	./$(TEST) 10 2 10475460 > $(TESTS)/10M.txt  # 10 MB
	./$(TEST) 10 2 20950920 > $(TESTS)/20M.txt  # 20 MB
	./$(TEST) 10 2 52377300 > $(TESTS)/50M.txt  # 50 MB
#	./$(TEST) 10 2 104754600 > $(TESTS)/100M.txt  # 100 MB
#	./$(TEST) 10 2 209509200 > $(TESTS)/200M.txt  # 200 MB
#	./$(TEST) 10 2 523773000 > $(TESTS)/500M.txt  # 500 MB
#	./$(TEST) 10 2 1047546000 > $(TESTS)/1G.txt  # 1 GB

