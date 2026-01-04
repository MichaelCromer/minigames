########################################################################################
#
#	breakout Makefeile
#
########################################################################################

NAME = breakout

#=======================================================================================
#	Setup

DIR_SRC = ./src
DIR_BLD = ./bld
DIR_OBJ = $(DIR_BLD)/obj
DIRS = $(DIR_SRC) $(DIR_BLD) $(DIR_OBJ)

SRC = $(DIR_SRC)/main.c
OBJ = $(SRC:$(DIR_SRC)/%.c=$(DIR_OBJ)/%.o)
EXE = $(DIR_BLD)/$(NAME)


CC = gcc
FLAG_C = -Wall -Wextra -Wpedantic -Werror
LIB_C = -lraylib -lm


#=======================================================================================
#	Build (compile/link)

$(EXE): $(OBJ) | $(DIR_BLD)
	$(CC) $(FLAG_C) $(OBJ) -o $@ $(LIB_C)


$(OBJ) : $(wildcard $(DIR_SRC)/*.c) | $(DIR_OBJ)
	$(CC) $(FLAG_C) -c $(SRC) -o $@


$(DIR_SRC)/%.c:


#=======================================================================================
#	Directories

$(DIRS) : ; mkdir -p $@


.PHONY: clean
clean: ; rm -f $(EXE) $(OBJ)


#=======================================================================================
#	Utility

.PHONY: dev
dev : FLAG_C += -g -fsanitize=address,leak,undefined
dev : clean $(EXE)

.PHONY: tags
tags: ; ctags $(wildcard $(DIR_SRC)/*.c)
