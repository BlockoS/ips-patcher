CXX      = g++
CXXFLAGS = -std=c++11 -W -Wall

ECHO = echo

BIN     = ips-patcher
BIN_CLI = ips-patcher-cli

BUILD_DIR = build

OUTDIR = $(BUILD_DIR)

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	OUTDIR   := $(OUTDIR)/Debug
	CXXFLAGS += -g -DDEBUG
else
	OUTDIR   := $(OUTDIR)/Release
	CXXFLAGS += -O2
endif
OBJDIR = $(OUTDIR)/obj

LIBS = -lm

EXE_SRC := src/ips.cpp
OBJS    := $(EXE_SRC:.cpp=.o)
EXE_OBJ := $(addprefix $(OBJDIR)/, $(OBJS))
EXE     := $(OUTDIR)/$(BIN)
EXE_CLI := $(OUTDIR)/$(BIN_CLI)

all: $(EXE_CLI)

$(EXE_CLI): $(EXE_OBJ)
	@$(ECHO) "	LD	$@"
	@$(CXX) $(CXXFLAGS) -o $(EXE_CLI) src/cli.cpp $^ $(LIBS)

$(OBJDIR)/%.o: %.cpp
	@$(ECHO) "	C++	$<"
	@$(shell mkdir -p `dirname $@`)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(EXE_OBJ): | $(OBJDIR) $(OUTDIR)

$(OUTDIR):
	@mkdir -p $(OUTDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	@$(ECHO) "	CLEAN	obj"
	@find $(BUILD_DIR) -name "*.o" -exec rm -f {} \;
