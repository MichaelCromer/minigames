########################################################################################
#
#	physics Makefeile
#
########################################################################################

#=======================================================================================
#	Setup

DIR_SRC = ./src
DIR_BLD = ./bld
DIR_OBJ = $(DIR_BLD)/obj
DIR_PRJ = ./demo

PRJ = $(wildcard $(DIR_PRJ)/*.c)
OBJ = $(PRJ:$(DIR_PRJ)/%.c=$(DIR_OBJ)/%.o)
TARGETS = $(PRJ:$(DIR_PRJ)/%.c=$(DIR_BLD)/%)

CC = gcc
FLAG_C = -Wall -Wextra -Wpedantic -Werror -MMD -MP
LIB_C = -lraylib -lm


#=======================================================================================
#	Build (compile/link)

.PHONY: all
all: $(TARGETS)


$(DIR_BLD)/%: $(DIR_OBJ)/%.o | $(DIR_BLD)
	$(CC) $(FLAG_C) $< -o $@ $(LIB_C)


$(DIR_OBJ)/%.o: $(DIR_PRJ)/%.c | $(DIR_OBJ)
	$(CC) $(FLAG_C) -c $< -o $@

-include $(OBJ:.o=.d)


#=======================================================================================
#	Directories

$(DIR_OBJ) $(DIR_BLD) : ; mkdir -p $@


.PHONY: clean
clean: ; rm -rf $(DIR_BLD)


#=======================================================================================
#	Utility

.PHONY: dev
dev : FLAG_C += -g -fsanitize=address,leak,undefined
dev : clean $(TARGETS)

.PHONY: tags
tags: ; ctags $(wildcard $(DIR_PRJ)/*.c) $(wildcard $(DIR_SRC)/*.c)
