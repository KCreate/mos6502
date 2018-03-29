CC := clang
OPT := -O0
OPTPROD := -O3 -Ofast
SRCDIR := src
BUILDDIR := build
INCLUDEDIR := include
TARGET := bin/cpu
SRCEXT := cpp
HEADEREXT := h
HEADERS := $(shell find $(INCLUDEDIR) -type f -name *.$(HEADEREXT))
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/$(basename $(notdir %)),$(SOURCES:.$(SRCEXT)=.o))
CPPSTD := c++17
CFLAGS := -std=$(CPPSTD) \
					-g \
					-Wall \
					-Wextra \
					-Werror \
					-Wno-unused-private-field \
					-ferror-limit=50 \
					-ffast-math
CFLAGSPROD := -std=$(CPPSTD) \
							-Wall \
							-Wextra \
							-Werror \
							-Wno-unused-private-field \
							-ferror-limit=1 \
							-flto \
							-ffast-math
LFLAGS_OSX := -lm -framework sfml-window \
							-framework sfml-audio \
							-framework sfml-graphics \
							-framework sfml-system
LFLAGS_LINUX := -lX11 -lm -lpthread -lsfml-window -lsfml-audio -lsfml-graphics -lsfml-system
INC := -I libs -I $(INCLUDEDIR)
LIB := -lstdc++
OS = $(shell uname -s)

ifeq ("$(OS)","Linux")
	LFLAGS = $(LFLAGS_LINUX)
	CFLAGS += -D LINUX
	CFLAGS += -pthread
else
	LFLAGS = $(LFLAGS_OSX)
	CFLAGS += -D OSX
endif

$(TARGET): $(OBJECTS)
	$(call colorecho, " Linking...", 2)
	@$(CC) $(OBJECTS) $(CFLAGS) $(OPT) $(LIB) $(LFLAGS) -o $(TARGET)
	$(call colorecho, " Built executable $(TARGET)", 2)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(OPT) $(INC) -c -o $@ $<
	$(call colorecho, " Built $@", 2)

production:
	$(call colorecho, " Building production binary $(TARGET)", 2)
	@$(CC) $(SOURCES) $(CFLAGSPROD) $(OPTPROD) $(INC) $(LIB) $(LFLAGS) -o $(TARGET)

clean:
	$(call colorecho, " Cleaning...", 2)
	@rm -rf $(BUILDDIR) $(TARGET)

rebuild:
	@make clean
	@make -j

format:
	$(call colorecho, " Formatting...", 2)
	@clang-format -i $(SOURCES) $(HEADERS) -style=file

.PHONY: whole clean rebuild format

# Create colored output
define colorecho
      @tput setaf $2
      @echo $1
      @tput sgr0
endef
