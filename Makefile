# Makefile for emulator compiler

EXEC=compile # compile

HEADS=$(wildcard ./*.h)
SRC=$(wildcard ./*.cpp)
# FILTEROUT=$(wildcard ./test/*.cpp)
# SRC_FILES=$(filter-out $(FILTEROUT), $(SRC))
OBJ=$(SRC:%.cpp=%.o)

CPP=g++
OPTS=-std=c++11 -O2 -Wall -g
LIBS=-lmetis -lGKlib

all: ${EXEC}

${EXEC}: ${OBJ}
	${CPP} ${OPTS} -o $@ ${OBJ} ${LIBS}

%.o: %.cpp ${HEADS}
	${CPP} ${OPTS} -c -o $@ $<

clean:
	rm -f ${OBJ} ${EXEC}
