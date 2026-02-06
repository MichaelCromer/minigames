########################################################################################
#
#	physics Makefeile
#
########################################################################################

PROJECT = physics

#=======================================================================================
#	Setup

DIR_SRC = ./src
DIR_BLD = ./bld
DIR_OBJ = $(DIR_BLD)/obj

TARGET = $(DIR_BLD)/$(PROJECT)

SRC = $(DIR_SRC)/main.c
OBJ = $(SRC:$(DIR_SRC)/%.c=$(DIR_OBJ)/%.o)

CC = gcc
FLAG_C = -Wall -Wextra -Wpedantic -Werror
LIB_C = -lraylib -lm


#=======================================================================================
#	Build (compile/link)

$(TARGET): $(OBJ) | $(DIR_BLD)
	$(CC) $(FLAG_C) $(OBJ) -o $@ $(LIB_C)


$(OBJ) : $(wildcard $(DIR_SRC)/*.c) | $(DIR_OBJ)
	$(CC) $(FLAG_C) -c $(SRC) -o $@


$(DIR_SRC)/%.c:


#=======================================================================================
#	Directories

$(DIR_OBJ) $(DIR_BLD) : ; mkdir -p $@


.PHONY: clean
clean: ; rm -f $(TARGET) $(OBJ)


#=======================================================================================
#	Utility

.PHONY: dev
dev : FLAG_C += -g -fsanitize=address,leak,undefined
dev : clean $(TARGET)

.PHONY: tags
tags: ; ctags $(wildcard $(DIR_SRC)/*.c)
