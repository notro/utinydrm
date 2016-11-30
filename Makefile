

XFLAGS    = -Wall -Wshadow -Wstrict-prototypes -Wmissing-prototypes \
            -DDEBUG -DVERBOSE_DEBUG -Wredundant-decls

INCDIRS   = -Iinclude -I/home/pi/work/tinydrm/usr/include

CFLAGS    = ${INCDIRS} ${OFLAGS} ${XFLAGS} ${PFLAGS} ${UFLAGS}
LDFLAGS   = -Wl,--no-as-needed -lrt


DEPDIR = .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

OBJS = utinydrm.o tinydrm-core.o tinydrm-helpers.o tinydrm-regmap.o mipi-dbi.o
MAIN = utinydrm

all: $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.o : %.c
%.o : %.c $(DEPDIR)/%.d
	$(COMPILE.c) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))
