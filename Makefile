CXX      = g++
CXXFLAGS = -std=c++11 -W -Wall

ECHO = echo

BIN_CLI = ips-patcher-cli
BIN_GUI = ips-patcher

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

SRC      := src/log.cpp src/ips.cpp src/io.cpp
OBJS     := $(SRC:.cpp=.o)
OBJ_BASE := $(addprefix $(OBJDIR)/, $(OBJS))

SRC_CLI  := src/cli.cpp
OBJS_CLI := $(SRC_CLI:.cpp=.o)
OBJ_CLI  := $(addprefix $(OBJDIR)/, $(OBJS_CLI))
EXE_CLI  := $(OUTDIR)/$(BIN_CLI)

all: $(EXE_CLI) 

$(EXE_CLI): $(OBJ_BASE) $(OBJ_CLI)
	@$(ECHO) "	LD	$@"
	@$(CXX) $(CXXFLAGS) -o $(EXE_CLI) $^ $(LIBS)

$(OBJDIR)/%.o: %.cpp
	@$(ECHO) "	C++	$<"
	@$(shell mkdir -p `dirname $@`)
	@$(CXX) $(CXXFLAGS) $(CXXFLAGS_GUI) -c -o $@ $<

$(OBJ_BASE): | $(OBJDIR) $(OUTDIR)

$(OBJ_CLI): | $(OBJDIR) $(OUTDIR)

$(OUTDIR):
	@mkdir -p $(OUTDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	@$(ECHO) "	CLEAN	obj"
	@find $(BUILD_DIR) -name "*.o" -exec rm -f {} \;
