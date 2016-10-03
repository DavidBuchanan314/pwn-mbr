all: inject payload

inject: inject.c
	gcc inject.c -Wall -std=c99 -o inject

payload: payload.s
	nasm -f bin payload.s -o payload
