CC=g++
CFLAGS=$(shell llvm-config-4.0 --cxxflags)
LDFLAGS=$(shell llvm-config-4.0 --ldflags --system-libs --libs)

executable: lex.yy.o parser.tab.o expression.o statements.o function.o
	$(CC) -o $@ $^ $(LDFLAGS)
parser.tab.o: parser.tab.cpp parser.tab.hpp 
	$(CC) -Wno-unknown-warning-option $(CFLAGS) -c -o $@ $<
parser.tab.cpp parser.tab.hpp: parser.ypp
	bison -d -v $<
lex.yy.o: lex.yy.c parser.tab.hpp
	$(CC) -Wno-unknown-warning-option $(CFLAGS) -Wno-sign-compare -c -o $@ $<
lex.yy.c: lexer.lex
	flex $<
expression.o: expression.cpp expression.hpp
	$(CC) -Wno-unknown-warning-option $(CFLAGS) -c -o $@ $<
statements.o: statements.cpp statements.hpp expression.hpp
	$(CC) -Wno-unknown-warning-option $(CFLAGS) -c -o $@ $<
function.o: function.cpp function.hpp expression.hpp
	$(CC) -Wno-unknown-warning-option $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -rf *~ *tab* lex.yy.* *.o kaleidoscope *.output *.dwo

