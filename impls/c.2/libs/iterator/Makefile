.PHONY: clean
.PHONY: test

PATHU = ../../unity/src/
PATHS = ./
PATHT = test/
PATHB = build/
PATHO = build/objs/
PATHR = build/results/

BUILD_PATHS = $(PATHB) $(PATHO) $(PATHR)

SRCT = $(wildcard $(PATHT)*.c)
SRCS = $(wildcard $(PATHS)*.c)

COMPILE=gcc -c -g
LINK=gcc
CFLAGS=-I. -I$(PATHU) -I$(PATHS) -DTEST
LIBS = -lgc

RESULTS = $(patsubst $(PATHT)test_%.c,$(PATHR)test_%.txt,$(SRCT) )
OBJS = $(patsubst $(PATHS)%.c,$(PATHO)%.o,$(SRCS) )

PASSED = `grep -s PASS $(PATHR)*.txt`
FAIL = `grep -s FAIL $(PATHR)*.txt`
IGNORE = `grep -s IGNORE $(PATHR)*.txt`

ifeq ($(OS),Windows_NT)
  ifeq ($(shell uname -s),) # not in a bash-like shell
    CLEANUP = del /F /Q
    MKDIR = mkdir
  else # in a bash-like shell, like msys
    CLEANUP = rm -f
    MKDIR = mkdir -p
  endif
    TARGET_EXTENSION=.exe
else
    CLEANUP = rm -f
    MKDIR = mkdir -p
    TARGET_EXTENSION=out
endif


# dummy 'test' target depends on the build directories and the results file
test: $(BUILD_PATHS) $(RESULTS)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

# the results file depends on running the compiled executable
$(PATHR)test_%.txt: $(PATHB)test_%.$(TARGET_EXTENSION)
	-./$< > $@ 2>&1

# the compiled executable depends on the compiled object files
$(PATHB)%.$(TARGET_EXTENSION): $(PATHO)%.o $(OBJS) $(PATHU)unity.o
	$(LINK) -o $@ $^ $(LIBS)

# the test object files depend on the test c files
$(PATHO)test_%.o:: $(PATHT)test_%.c
	$(COMPILE) $(CFLAGS) $< -o $@

# the src object files depend on the src c files
$(PATHO)%.o:: $(PATHS)%.c
	$(COMPILE) $(CFLAGS) $< -o $@

# the unity object file depends on the unity c file
$(PATHO)%.o:: $(PATHU)%.c $(PATHU)%.h
	$(COMPILE) $(CFLAGS) $< -o $@

# the build directories are created if they don't exist
$(PATHB):
	$(MKDIR) $(PATHB)

$(PATHO):
	$(MKDIR) $(PATHO)

$(PATHR):
	$(MKDIR) $(PATHR)

# dummy target cleans up the build files
clean:
	$(CLEANUP) $(PATHO)*.o
	$(CLEANUP) $(PATHB)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(PATHR)*.txt

# retain the intermediate files until they are cleaned up
.PRECIOUS: $(PATHB)%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATHO)test_%.o
.PRECIOUS: $(PATHO)%.o
.PRECIOUS: $(PATHR)%.txt
