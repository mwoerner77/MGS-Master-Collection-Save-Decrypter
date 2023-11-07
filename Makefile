all: decrypter

decrypter:
	gcc -D _GNU_SOURCE -o mgs-mc-hd-decrypter main.c

clean:
	-rm -f mgs-mc-hd-decrypter
