BUILDDIR	:= build

QUEUESRC	:= queue.c
JMPCALLSRC	:= jmp_call.asm

OBJS1		:= $(patsubst %.c,$(BUILDDIR)/%.o,$(QUEUESRC))

QUEUE		:= $(BUILDDIR)/queue
JMP_CALL	:= $(BUILDDIR)/jmp_call

all: $(QUEUE) $(JMP_CALL)

$(BUILDDIR):
	mkdir -p $@

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

$(QUEUE): $(OBJS1)
$(JMP_CALL): $(JMPCALLSRC)
	fasm $^ $@