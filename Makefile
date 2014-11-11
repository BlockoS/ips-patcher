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

SRC      := src/log.cpp src/ips.cpp src/io.cpp src/utils.cpp
OBJS     := $(SRC:.cpp=.o)
OBJ_BASE := $(addprefix $(OBJDIR)/, $(OBJS))

SRC_CLI  := src/cli.cpp
OBJS_CLI := $(SRC_CLI:.cpp=.o)
OBJ_CLI  := $(addprefix $(OBJDIR)/, $(OBJS_CLI))
EXE_CLI  := $(OUTDIR)/$(BIN_CLI)

LIBS_GUI = `pkg-config --libs gtk+-3.0`
CXXFLAGS_GUI = `pkg-config --cflags gtk+-3.0`

SRC_GUI  := src/gui.cpp
OBJS_GUI := $(SRC_GUI:.cpp=.o)
OBJ_GUI  := $(addprefix $(OBJDIR)/, $(OBJS_GUI))
EXE_GUI  := $(OUTDIR)/$(BIN_GUI)

all: $(EXE_CLI) $(EXE_GUI)

$(EXE_CLI): $(OBJ_BASE) $(OBJ_CLI)
	@$(ECHO) "	LD	$@"
	@$(CXX) $(CXXFLAGS) -o $(EXE_CLI) $^ $(LIBS)

$(EXE_GUI): $(OBJ_BASE) $(OBJ_GUI)
	@$(ECHO) "	LD	$@"
	@$(CXX) $(CXXFLAGS) $(CXXFLAGS_GUI) -o $(EXE_GUI) $^ $(LIBS) $(LIBS_GUI)

$(OBJDIR)/%.o: %.cpp
	@$(ECHO) "	C++	$<"
	@$(shell mkdir -p `dirname $@`)
	@$(CXX) $(CXXFLAGS) $(CXXFLAGS_GUI) -c -o $@ $<

$(OBJ_BASE): | $(OBJDIR) $(OUTDIR)

$(OBJ_CLI): | $(OBJDIR) $(OUTDIR)

$(OBJ_GUI): | $(OBJDIR) $(OUTDIR)

$(OUTDIR):
	@mkdir -p $(OUTDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	@$(ECHO) "	CLEAN	obj"
	@find $(BUILD_DIR) -name "*.o" -exec rm -f {} \;
