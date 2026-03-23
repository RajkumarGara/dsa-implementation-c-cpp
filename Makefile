CC ?= clang
CPPFLAGS ?= -MMD -MP
CFLAGS ?= -Wall -Wextra -Wpedantic -g

TARGET := arrays/array_adt
PROGRAM ?= $(TARGET)
SRCS := arrays/array_adt.c utils/utils.c
OBJS := $(SRCS:.c=.o)
DEPS := $(OBJS:.o=.d)
STALE_ARTIFACTS := arrays/Array_ADT arrays/Array_ADT.dSYM arrays/array_adt.dSYM
STALE_FILES := $(filter-out %.dSYM,$(STALE_ARTIFACTS))
STALE_DIRS := $(filter %.dSYM,$(STALE_ARTIFACTS))

.PHONY: all clean run run-one

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

run-one: $(PROGRAM)
	./$(PROGRAM)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET) $(STALE_FILES)
	rm -rf $(STALE_DIRS)

-include $(DEPS)