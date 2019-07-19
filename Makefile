CC=g++
CFLAGS=-I/usr/include -ansi -pedantic
LDFLAGS=-L/usr/lib -lX11 -ljpeg -lm
SRC= $(wildcard *.cpp)
OBJ= $(SRC:.cpp=.o)

DESTDIR =
PREFIX = /usr
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man6

all: xpuz

xpuz : $(OBJ)
	@$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	@$(CC) -o $@ -c $< $(CFLAGS)

clean :
	@rm -rf *.o

install :
	install -Dm755 xpuz $(DESTDIR)$(BINDIR)/xpuz
	install -Dm755 xpuz.6.gz $(DESTDIR)$(MANDIR)/xpuz.6.gz
