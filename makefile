APPNAME=wadcli
LOCALBIN=/usr/local/bin/

BINDIR=./bin
OBJDIR=./obj
SRCDIR=./src
DEPDIR=./src/headers

_LDLIBS=-l:liblzf.so
LDFLAGS=
CPPFLAGS=-Wall -fexceptions -pedantic-errors -Wextra\
	-std=c++17
DIRLOC=

ifeq ($(DEBUG), 1)
	CPPFLAGS += -DDEBUG=1
else
	CPPFLAGS += -DDEBUG=0 -O3
endif

ifeq ($(STATIC), 1)
	LDLIBS = $(patsubst %.so, %.a, $(_LDLIBS))
	LDFLAGS += -static -static-libgcc -static-libstdc++
else
	LDLIBS = $(_LDLIBS)
endif

ifeq ($(WINDOWS), 1)
	CXX = x86_64-w64-mingw32-g++
	DIRAFTER = -idirafter /usr/include/
endif

_DEPS=wadformat.h palette.h helpers.h
DEPS=$(patsubst %, $(DEPDIR)/%, $(_DEPS))

_OBJ=main.o wadformat.o palette.o
OBJ=$(patsubst %, $(OBJDIR)/%, $(_OBJ))

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	@mkdir -p $(OBJDIR)
	@$(CXX) -g $(CPPFLAGS) -c -o $@ $< $(DIRAFTER)

$(APPNAME): $(OBJ)
	$(CXX) -g $(CPPFLAGS) -o $@ $^ $(DIRAFTER) $(LDFLAGS) $(LDLIBS) $(DIRLOC)

.PHONY: clean

clean	:
	rm -f $(OBJDIR)/*.o

install : 
	/bin/bash installscript.sh $(APPNAME) $(LOCALBIN)