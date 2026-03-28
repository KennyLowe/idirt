#########################################################
# iDiRT Makefile					#
# 1995, 1996 Illusion					#
#########################################################

# Directory Definitions
#
BIN	= ../bin/
DATA	= ../data/
ZONE	= $(DATA)ZONES
UTIL	= ../utils/
HDIR    = ../include/

# Target Executables
#
MUDD	= aberd
GEN	= generate
DUMP	= Dump
PFILTER	= pfilter
CONVERT	= convert

# Compiler and Compiler Flags
#
# -DNOCATCH in DEFINES to prevent the AutoUpdater from running.
# -lcrypt in LDFLAGS for most BSD systems.
#
CC	= gcc
LDFLAGS	= -lcrypt
DEFINES	= 
DBFLAGS	= -g
OPFLAGS	= -pipe -m64 -O3 -fcommon
WFLAGS	= -Wwrite-strings -Wstrict-prototypes -Wold-style-definition -Wall -Werror -Wno-error=unused-result -Wno-error=format-overflow -Wno-error=unused-but-set-variable  -Wno-error=builtin-declaration-mismatch -Wno-error=stringop-overflow -Wno-error=restrict -Wno-error=unused-variable
CFLAGS	= $(DEFINES) $(DBFLAGS) $(OPFLAGS) $(WFLAGS) -I$(HDIR)

# Extra Defines
#
ECHO	= /bin/echo		# Echo Utility
#STRIPD	= STRIPD		# See Below (for MUDD)
STRIP	= STRIP			# Strip Binaries After Compilation
                                # If you don't define this and you
                                # run make strip, you will have to
                                # recompile most of the MUD the next
                                # time you compile.

# Zone File Dependencies
#
ZONES = $(ZONE)/ancient.zone		$(ZONE)/blizzard.zone \
	$(ZONE)/camelot.zone		$(ZONE)/castle.zone \
	$(ZONE)/catacomb.zone		$(ZONE)/cave.zone \
	$(ZONE)/church.zone		$(ZONE)/eforest.zone \
	$(ZONE)/fantasy.zone		$(ZONE)/forest.zone \
	$(ZONE)/frobozz.zone		$(ZONE)/hick.zone \
	$(ZONE)/home.zone		$(ZONE)/icecave.zone \
	$(ZONE)/island.zone		$(ZONE)/labyrinth.zone \
	$(ZONE)/ledge.zone		$(ZONE)/moapt.zone \
	$(ZONE)/moor.zone		$(ZONE)/mountain.zone \
	$(ZONE)/oaktree.zone		$(ZONE)/orchold.zone \
	$(ZONE)/quarry.zone		$(ZONE)/playhouse.zone \
	$(ZONE)/ruins.zone		$(ZONE)/sea.zone \
	$(ZONE)/sherwood.zone		$(ZONE)/tower.zone \
	$(ZONE)/treehouse.zone		$(ZONE)/valley.zone \
	$(ZONE)/village.zone		$(ZONE)/waste.zone \
	$(ZONE)/xlimbo.zone 		$(ZONE)/zodiac.zone \
	$(ZONE)/talon.zone		$(ZONE)/mrealms.zone \
	$(ZONE)/mithdan.zone		$(ZONE)/Zonelist

# MUD Daemon Dependencies
#
MUDDOBJS = acct.o actions.o bootstrap.o bprintf.o change.o climate.o \
	   clone.o comm.o commands.o exit.o fight.o flags.o frob.o \
	   god.o log.o magic.o mailer.o main.o mobile.o move.o mud.o \
	   objsys.o parse.o puff.o rooms.o sendsys.o spell.o s_socket.o \
	   timing.o uaf.o update.o utils.o ver.o viewcom.o wizard.o \
	   wizlist.o writer.o zones.o

# Generator Dependencies
#
GENOBS = generate.o mkdata.o utils.o

# Dump Dependencies
#
DUMPD = utils/Dump.c $(HDIR)kernel.h $(HDIR)macros.h $(HDIR)sflags.h \
	$(HDIR)pflags.h $(HDIR)pflagnames.h

# PFilter Dependencies
#
PFILTD = utils/pfilter.c $(HDIR)kernel.h $(HDIR)macros.h $(HDIR)pflags.h \
         $(HDIR)pflagnames.h $(HDIR)levelnames.h

# Make Targets
#
all:	gen mudd dump pfilt
gen:	$(BIN)$(GEN)
mudd:	$(BIN)$(MUDD)
dump:	$(BIN)$(DUMP)
pfilt:	$(BIN)$(PFILTER)

# Make Dependencies
#
depend:
	@$(UTIL)makedep

# Version/Link Tracker
#
MakeLinkCount:
	@rm -f $(HDIR)LINKCOUNT.H; \
	if [ ! -f .LINKCOUNT ]; then \
		echo 1 > .LINKCOUNT; \
	fi \

	@if [ -f .LINKED ]; then \
		rm -f .LINKED; \
		expr `cat .LINKCOUNT` + 1 > .LINKCOUNT; \
	fi

LinkCount:	MakeLinkCount
	@$(ECHO) "#ifndef _LINKCOUNT_H" > $(HDIR)LINKCOUNT.H
	@$(ECHO) "#define _LINKCOUNT_H" >> $(HDIR)LINKCOUNT.H
	@$(ECHO) >> $(HDIR)LINKCOUNT.H
	@$(ECHO) "#define LINKCOUNT" >> $(HDIR)LINKCOUNT.H
	@$(ECHO) >> $(HDIR)LINKCOUNT.H
	@$(ECHO) "#endif"               >> $(HDIR)LINKCOUNT.H

# Generator (gen) Target
#
$(BIN)$(GEN): $(GENOBS)
	@$(ECHO) -n 'Linking Generator...'
	@$(CC) -o $(BIN)$(GEN) $(CFLAGS) $(GENOBS) $(LDFLAGS)
	
ifdef STRIP
	@$(ECHO) -n 'Stripping Binary...'
	@strip $(BIN)$(GEN)
endif
	@$(ECHO) 'Done'

	@$(ECHO) 'Generating World Files...'
	@$(BIN)$(GEN) data $(DATA)generate.conf
	@$(ECHO) -n 'Generating Verb Files...'
	@$(BIN)$(GEN) verbs $(DATA)verbs.src $(DATA)verbs $(HDIR)verbs.h
	@$(ECHO) 'Done'

# MUD Daemon (mud) Target
#
$(BIN)$(MUDD): LinkCount $(MUDDOBJS)
	@$(ECHO) -n 'Linking MUD Daemon...'
	@$(CC) -o $(BIN)$(MUDD) $(CFLAGS) $(MUDDOBJS) $(LDFLAGS)
	@touch .LINKED

ifdef STRIPD
	@$(ECHO) -n 'Stripping Binary...'
	@strip $(BIN)$(MUDD)
endif

	@$(ECHO) 'Done'

# C source code to object data directive.
#
.c.o:
	@$(ECHO) 'Compiling $*.c...'
	@$(CC) -c $(CFLAGS) $*.c

# Dump Target
#
$(BIN)$(DUMP): $(DUMPD)
	@$(ECHO) -n 'Making Dump Utility...'
	@$(CC) $(CFLAGS) -o $(BIN)$(DUMP) utils/$(DUMP).c

ifdef STRIP
	@$(ECHO) -n 'Stripping Binary...'
	@strip $(BIN)$(DUMP)
endif

	@$(ECHO) 'Done'

# PFilter Target
#
$(BIN)$(PFILTER): $(PFILTD)
	@$(ECHO) -n 'Making PFilter Utility...'
	@$(CC) $(CFLAGS) -o $(BIN)$(PFILTER) utils/$(PFILTER).c

ifdef STRIP
	@$(ECHO) -n 'Stripping Binary...'
	@strip $(BIN)$(PFILTER)
endif

	@$(ECHO) 'Done'
	
# Convert Utility Target
#
convert:
	$(CC) $(CFLAGS) -o $(UTIL)$(CONVERT) utils/$(CONVERT).c utils.c

ifdef STRIP
	@$(ECHO) -n 'Stripping Binary...'
	@strip $(UTIL)$(CONVERT)
	@$(ECHO) 'Done'
endif

# Machine Type Dependency
#
$(HDIR)MACHINE.H:
	@$(ECHO) "Running 'make depend'..."
	@make depend

# Verbs Dependencies
#
$(HDIR)verbs.h $(DATA)verbs: $(BIN)$(GEN) $(DATA)verbs.src
	@$(ECHO) -n 'Generating Verb Files...'
	@$(BIN)$(GEN) verbs $(DATA)/verbs.src $(DATA)/verbs $(HDIR)verbs.h
	@$(ECHO) 'Done'

# MUD World Dependencies
#
$(HDIR)locations.h $(HDIR)mobiles.h $(HDIR)objects.h \
$(DATA)/locations $(DATA)/mobiles $(DATA)/objects: $(BIN)$(GEN) $(ZONES)
	@$(ECHO) 'Generating World Files...'
	@$(BIN)$(GEN) data $(DATA)generate.conf

# Strip Binaries Target
#
strip:
	@$(ECHO) -n 'Stripping Binaries...'
	@cd $(BIN); strip $(MUDD) $(GEN) $(DUMP) $(PFILTER)
	@$(ECHO) 'Done'

#############################################################
# Do not delete this line, the makedep program requires it. #
# Last Updated: Tue 14 Apr 19:11:31 BST 2026                 #
#############################################################

# Dependencies for the MUD source code
#
