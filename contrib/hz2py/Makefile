BIN=$(DESTDIR)/usr/bin
build:
	gcc -Wall -std=c99 linereader.c pinyin.c utf8vector.c hz2py.c -o hz2py	
install:
	install -d  $(BIN)
	install ./hz2py $(BIN)
clean:
	-rm hz2py
