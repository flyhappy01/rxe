all:
	gcc -Wall -Werror main.c parse.c -o rxe -lpcap
