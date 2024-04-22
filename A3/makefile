PORT=52349
CFLAGS= -DPORT=\$(PORT) -g -Wall

battle: battle.c
	gcc $(CFLAGS) -o battle battle.c

clean:
	rm -f ./battle *.o *~
