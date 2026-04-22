#
#           Makefile   :     This is the compilation file
#                            
#   
#
# --------------------------------------------------------
#       @Copyright 2026 - APACHE-2.0 
#
#

CC 		  = gcc
CFLAGS 	  = -Wall -Wextra -Iinclude -g

Q 		  = @

SRC 	  = src/main.c src/lexer.c
TARGET    = bin/bullz

DISTFILES = src/*.o .git

all: $(TARGET)


# ....
$(TARGET): $(SRC)
	$(Q)echo "  CC    $(SRC) \n  .."
	$(Q)$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
	$(Q)echo "  BIN   $(TARGET)\n"

# clean
clean:
	$(Q)rm -f $(TARGET) bullz.tab.c bullz.tab.h

distclean: clean
	$(Q)echo "  CLEAN  $(DISTFILES)"
	$(Q)rm -rf $(DISTFILES)