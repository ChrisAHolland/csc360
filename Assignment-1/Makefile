.phony all:
all: shellInterpreter

shellInterpreter: shellInterpreter.c
		gcc shellInterpreter.c -lreadline -o ssi

.PHONY clean:
clean:
		-rm -rf *.o *.exe