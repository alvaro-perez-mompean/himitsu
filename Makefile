OWNER=bin
GROUP=bin
BINDIR=/usr/bin

himitsu: src/vocab.o src/learning.o src/search.o src/edict.o src/kanji.o src/curses.o src/main.o
	$(CC) $(LDFLAGS) $^ -o $@

CFLAGS = -O2 -Wall -Wextra
LDFLAGS= -O2 -lncursesw

clean:
	-rm -f src/*.o himitsu

install: himitsu
	install -c -o $(OWNER) -g $(GROUP) -m 755 himitsu $(DESTDIR)$(BINDIR)

