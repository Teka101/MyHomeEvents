NAME= MyHomeEvents
SRC=Main.cpp Domoticz.cpp
OBJ=$(SRC:.cpp=.o)

CPPFLAGS=-g3 -Wall -Wunused -Wpointer-arith -Wno-uninitialized -std=c++11 `curl-config --cflags` -DDODEBUG
LDFLAGS=`curl-config --libs` -lmicrohttpd -lsqlite3 -lboost_program_options
CPP=g++
RM=rm -f

all	: $(NAME)

$(NAME): $(OBJ)
	@echo "Compile binary	[$(NAME)]"
	@$(CPP) -o $(NAME) $(OBJ) $(LDFLAGS)

clean:
	@echo "Delete all objects"
	@$(RM) $(OBJ)

distclean: clean
	@echo "Delete all unecessary files"
	@$(RM) $(NAME)
	@$(RM) -i `$(FIND) . | grep -F '~'` *.tgz || true

.cpp.o:
	@echo "Compile		[$<]"
	@$(CPP) $(CPPFLAGS) -c -o $@ $<