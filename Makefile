#----------------------------------------------------------------------------
# Makefile for building a sample debugger
#----------------------------------------------------------------------------

# Run 'make VERBOSE=1' to output executed commands
ifdef VERBOSE
	VERB := 
else
	VERB := @
endif


# compiler that is used to compile the debugger
CC := gcc
# additional compiler options
CCFLAGS := 


# linker that is used to link the debugger modules
LD := gcc
# additional linker options
LDFLAGS :=

# directories for the source and object files, as well as for the dependency files
DIR_SRC := src
DIR_OBJ := obj
DIR_DEP := dep

# target file (executable)
DIR_BIN := bin
TARGET := $(DIR_BIN)/debugger

# Get all source and object files
SRC_FILES := $(shell find $(DIR_SRC) -name "*.c")
OBJ_FILES := $(patsubst %.c, $(DIR_OBJ)/%.o, $(notdir $(SRC_FILES)))


#----------------------------------------------------------------------------
#----------------------------------------------------------------------------


# Print 'header' whenever make is issued
$(info --------------------------------)
$(info a sample debugger implementation)
$(info --------------------------------)
$(info )


#----------------------------------------------------------------------------
#----------------------------------------------------------------------------


.PHONY: all
all: $(TARGET)


$(TARGET): $(OBJ_FILES)
	$(VERB)mkdir -p $(DIR_BIN)
	@echo -e 'LD\t$@'
	$(VERB)$(LD) $(LDFLAGS) $^ -o $@


-include $(patsubst %.c, $(DIR_DEP)/%.md, $(notdir $(SRC_FILES)))


$(DIR_OBJ)/%.o: $(DIR_SRC)/%.c
	$(VERB)mkdir -p $(DIR_OBJ)
	$(VERB)mkdir -p $(DIR_DEP)
	@echo -e 'DEP\t$<'
	$(VERB)$(CC) $(CCFLAGS) -MM -MP -MT $(DIR_OBJ)/$(*F).o -MT $(DIR_DEP)/$(*F).md $< > $(DIR_DEP)/$(*F).md
	@echo -e 'CMPL\t$<'
	$(VERB)$(CC) $(CCFLAGS) -c $< -o $@


.PHONY: clean
clean:
	@echo -e 'RM\t $(DIR_OBJ)'
	$(VERB)rm -rf $(DIR_OBJ)
	@echo -e 'RM\t $(DIR_BIN)'
	$(VERB)rm -rf $(DIR_BIN)
	@echo -e 'RM\t $(DIR_DEP)'
	$(VERB)rm -rf $(DIR_DEP)
