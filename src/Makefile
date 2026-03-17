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
WFLAGS	= -Wstrict-prototypes -Wold-style-definition -Wall -Werror -Wno-error=unused-result -Wno-error=format-overflow -Wno-error=format -Wno-error=unused-but-set-variable  -Wno-error=builtin-declaration-mismatch -Wno-error=stringop-overflow -Wno-error=restrict -Wno-error=unused-variable
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
acct.o: acct.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/locations.h ../include/objects.h ../include/mobiles.h \
 ../include/pflags.h ../include/oflags.h ../include/lflags.h \
 ../include/cflags.h ../include/sflags.h ../include/bprintf.h \
 ../include/parse.h ../include/acct.h ../include/zones.h \
 ../include/mobile.h ../include/objsys.h ../include/rooms.h \
 ../include/timing.h ../include/uaf.h ../include/flags.h
actions.o: actions.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/locations.h sendsys.h ../include/bprintf.h \
 ../include/objects.h ../include/mobiles.h ../include/actions.h \
 ../include/mobile.h ../include/sflags.h ../include/mflags.h \
 ../include/objsys.h ../include/zones.h ../include/parse.h \
 ../include/rooms.h ../include/fight.h ../include/log.h
bootstrap.o: bootstrap.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/bootstrap.h ../include/wizlist.h ../include/actions.h \
 ../include/timing.h ../include/oflags.h ../include/nflags.h \
 ../include/eflags.h ../include/log.h
bprintf.o: bprintf.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/sflags.h ../include/pflags.h ../include/lflags.h \
 ../include/bprintf.h ../include/mud.h ../include/mobile.h \
 ../include/exit.h ../include/log.h ../include/rooms.h ../include/parse.h
change.o: change.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/pflags.h ../include/sflags.h ../include/uaf.h \
 ../include/mobile.h ../include/flags.h sendsys.h ../include/writer.h \
 ../include/change.h ../include/parse.h ../include/zones.h \
 ../include/bprintf.h ../include/log.h ../include/wizlist.h \
 ../include/rooms.h ../include/objsys.h ../include/mud.h
climate.o: climate.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h sendsys.h \
 ../include/climate.h ../include/pflags.h ../include/sflags.h \
 ../include/lflags.h ../include/bprintf.h ../include/parse.h
clone.o: clone.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/bootstrap.h ../include/mobile.h ../include/pflags.h \
 ../include/cflags.h sendsys.h ../include/objsys.h ../include/oflags.h \
 ../include/zones.h ../include/parse.h ../include/fight.h \
 ../include/clone.h ../include/eflags.h ../include/commands.h \
 ../include/rooms.h ../include/bprintf.h ../include/uaf.h \
 ../include/log.h
comm.o: comm.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/mobiles.h sendsys.h ../include/bprintf.h ../include/mobile.h \
 ../include/pflags.h ../include/sflags.h ../include/eflags.h \
 ../include/nflags.h ../include/lflags.h ../include/parse.h \
 ../include/flags.h ../include/comm.h ../include/uaf.h
commands.o: commands.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/locations.h ../include/objects.h ../include/mobiles.h \
 ../include/sflags.h ../include/pflags.h ../include/oflags.h \
 ../include/lflags.h ../include/cflags.h ../include/eflags.h \
 ../include/mflags.h ../include/quests.h sendsys.h ../include/commands.h \
 ../include/rooms.h ../include/objsys.h ../include/mobile.h \
 ../include/flags.h ../include/bprintf.h ../include/parse.h \
 ../include/uaf.h ../include/clone.h ../include/acct.h ../include/mud.h \
 ../include/nflags.h ../include/fight.h ../include/zones.h \
 ../include/log.h ../include/reboot.h ../include/wizlist.h \
 ../include/climate.h ../include/mailer.h ../include/timing.h
crypt.o: crypt.c
exit.o: exit.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/mobile.h sendsys.h ../include/timing.h ../include/exit.h \
 ../include/pflags.h ../include/log.h ../include/reboot.h \
 ../include/commands.h ../include/bprintf.h ../include/mud.h \
 ../include/uaf.h ../include/parse.h
fight.o: fight.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/cflags.h ../include/oflags.h ../include/lflags.h \
 ../include/mflags.h ../include/sflags.h ../include/pflags.h \
 ../include/quests.h sendsys.h ../include/verbs.h ../include/bprintf.h \
 ../include/commands.h ../include/move.h ../include/mud.h \
 ../include/rooms.h ../include/log.h ../include/clone.h \
 ../include/mobiles.h ../include/objects.h ../include/locations.h \
 ../include/spell.h ../include/fight.h ../include/mobile.h \
 ../include/objsys.h
flags.o: flags.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/pflags.h ../include/lflags.h ../include/mflagnames.h \
 ../include/pflagnames.h ../include/sflagnames.h ../include/lflagnames.h \
 ../include/nflagnames.h ../include/eflagnames.h ../include/levelnames.h \
 ../include/questnames.h ../include/sflags.h ../include/nflags.h \
 ../include/eflags.h sendsys.h ../include/bprintf.h ../include/parse.h \
 ../include/rooms.h ../include/mobile.h ../include/uaf.h ../include/log.h
frob.o: frob.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h sendsys.h \
 ../include/pflags.h ../include/sflags.h ../include/frob.h \
 ../include/uaf.h ../include/mobile.h ../include/log.h ../include/parse.h \
 ../include/bprintf.h ../include/mud.h ../include/flags.h \
 ../include/wizlist.h
generate.o: generate.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h
god.o: god.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/locations.h sendsys.h ../include/wizlist.h \
 ../include/bprintf.h ../include/actions.h ../include/pflags.h \
 ../include/sflags.h ../include/mobile.h ../include/timing.h \
 ../include/fight.h ../include/rooms.h ../include/verbs.h \
 ../include/parse.h ../include/log.h ../include/god.h ../include/uaf.h \
 ../include/mud.h
log.o: log.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/log.h
magic.o: magic.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h sendsys.h \
 ../include/magic.h ../include/pflags.h ../include/sflags.h \
 ../include/cflags.h ../include/lflags.h ../include/oflags.h \
 ../include/objects.h ../include/bprintf.h ../include/parse.h \
 ../include/mobile.h ../include/mud.h ../include/uaf.h \
 ../include/objsys.h ../include/rooms.h
mailer.o: mailer.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h sendsys.h \
 ../include/bprintf.h ../include/mobile.h ../include/mailer.h \
 ../include/parse.h ../include/log.h ../include/mud.h ../include/uaf.h
main.o: main.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/jmp.h ../include/mud.h ../include/log.h \
 ../include/bootstrap.h sendsys.h ../include/mobile.h \
 ../include/commands.h ../include/timing.h ../include/exit.h \
 ../include/s_socket.h ../include/locations.h ../include/pflags.h \
 ../include/update.h ../include/climate.h ../include/reboot.h \
 ../include/objsys.h ../include/bprintf.h ../include/zones.h \
 ../include/uaf.h ../include/fight.h ../include/rooms.h ../include/ver.h \
 ../include/main.h
mkdata.o: mkdata.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/cflags.h ../include/mflagnames.h ../include/pflagnames.h \
 ../include/sflagnames.h ../include/lflagnames.h ../include/oflagnames.h \
 ../include/nflagnames.h ../include/eflagnames.h ../include/exitnames.h \
 ../include/lflags.h ../include/nflags.h
mobile.o: mobile.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/locations.h ../include/commands.h ../include/wizlist.h \
 sendsys.h ../include/objects.h ../include/mobiles.h ../include/bprintf.h \
 ../include/objsys.h ../include/pflags.h ../include/mflags.h \
 ../include/sflags.h ../include/oflags.h ../include/lflags.h \
 ../include/nflags.h ../include/eflags.h ../include/quests.h \
 ../include/cflags.h ../include/timing.h ../include/spell.h \
 ../include/verbs.h ../include/rooms.h ../include/fight.h \
 ../include/parse.h ../include/zones.h ../include/flags.h \
 ../include/puff.h ../include/move.h ../include/uaf.h ../include/mud.h \
 ../include/log.h ../include/jmp.h ../include/mobile.h
move.o: move.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/parse.h ../include/objects.h ../include/mobiles.h \
 ../include/locations.h ../include/lflags.h ../include/mflags.h sendsys.h \
 ../include/move.h ../include/bprintf.h ../include/objsys.h \
 ../include/mobile.h ../include/rooms.h ../include/fight.h \
 ../include/uaf.h
mud.o: mud.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h sendsys.h \
 ../include/pflags.h ../include/sflags.h ../include/mud.h \
 ../include/uaf.h ../include/mobile.h ../include/timing.h \
 ../include/locations.h ../include/bprintf.h ../include/fight.h \
 ../include/zones.h ../include/rooms.h ../include/mailer.h \
 ../include/log.h ../include/parse.h ../include/acct.h
objsys.o: objsys.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/objects.h ../include/mobiles.h ../include/mobile.h \
 ../include/locations.h ../include/sflags.h ../include/pflags.h \
 ../include/mflags.h ../include/oflags.h ../include/lflags.h \
 ../include/cflags.h ../include/eflags.h ../include/quests.h sendsys.h \
 ../include/parse.h ../include/objsys.h ../include/oflagnames.h \
 ../include/rooms.h ../include/bprintf.h ../include/fight.h \
 ../include/commands.h ../include/log.h ../include/clone.h
parse.o: parse.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/condact.h ../include/verbs.h ../include/sflags.h sendsys.h \
 ../include/locations.h ../include/objects.h ../include/parse.h \
 ../include/mobile.h ../include/move.h ../include/commands.h \
 ../include/climate.h ../include/wizard.h ../include/change.h \
 ../include/clone.h ../include/pflags.h ../include/objsys.h \
 ../include/bprintf.h ../include/uaf.h ../include/actions.h \
 ../include/spell.h ../include/flags.h ../include/viewcom.h \
 ../include/log.h ../include/exit.h ../include/wizlist.h \
 ../include/timing.h ../include/frob.h ../include/acct.h \
 ../include/zones.h ../include/magic.h ../include/reboot.h \
 ../include/mailer.h ../include/fight.h ../include/rooms.h \
 ../include/comm.h ../include/main.h ../include/ver.h ../include/god.h
puff.o: puff.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h sendsys.h \
 ../include/mobiles.h ../include/puff.h ../include/sflags.h \
 ../include/mobile.h
rooms.o: rooms.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/locations.h ../include/objects.h ../include/pflags.h \
 ../include/oflags.h ../include/lflags.h ../include/cflags.h \
 ../include/sflags.h sendsys.h ../include/rooms.h ../include/climate.h \
 ../include/parse.h ../include/mobile.h ../include/objsys.h \
 ../include/zones.h ../include/exitnames.h ../include/bprintf.h \
 ../include/fight.h ../include/uaf.h ../include/log.h
s_socket.o: s_socket.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/s_socket.h
sendsys.o: sendsys.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/sflags.h ../include/pflags.h ../include/lflags.h sendsys.h \
 ../include/mobile.h ../include/nflags.h ../include/bprintf.h \
 ../include/mud.h ../include/commands.h ../include/log.h
spell.o: spell.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/oflags.h ../include/eflags.h ../include/pflags.h \
 ../include/lflags.h sendsys.h ../include/verbs.h ../include/mobiles.h \
 ../include/objects.h ../include/locations.h ../include/objsys.h \
 ../include/spell.h ../include/spells.h ../include/eflags.h \
 ../include/oflags.h ../include/bprintf.h ../include/parse.h \
 ../include/fight.h ../include/mobile.h ../include/sflags.h
timing.o: timing.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/locations.h ../include/climate.h ../include/bprintf.h \
 sendsys.h ../include/timing.h ../include/mobile.h ../include/cflags.h \
 ../include/objsys.h ../include/spell.h ../include/rooms.h \
 ../include/log.h ../include/mud.h
uaf.o: uaf.c ../include/pflags.h kernel.h VERSION.H MACHINE.H \
 ../include/config.h ../include/logconfig.h ../include/levels.h \
 ../include/exits.h ../include/types.h ../include/utils.h mudtypes.h \
 mudmacros.h ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/sflags.h ../include/mflags.h ../include/nflags.h \
 ../include/eflags.h ../include/uaf.h ../include/mobile.h \
 ../include/flags.h ../include/exit.h sendsys.h ../include/mud.h \
 ../include/bprintf.h ../include/wizlist.h ../include/bootstrap.h \
 ../include/log.h ../include/parse.h
update.o: update.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/objsys.h ../include/mobile.h ../include/update.h \
 ../include/log.h
utils.o: utils.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/global.h ../include/crypt.h
ver.o: ver.c LINKCOUNT.H kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/bprintf.h ../include/ver.h
viewcom.o: viewcom.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/viewcom.h ../include/commands.h ../include/wizard.h \
 ../include/mud.h ../include/uaf.h ../include/writer.h \
 ../include/mailer.h ../include/change.h ../include/frob.h \
 ../include/bprintf.h ../include/timing.h ../include/pflags.h \
 ../include/mobile.h
wizard.o: wizard.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/timing.h ../include/log.h ../include/uaf.h \
 ../include/bprintf.h ../include/verbs.h ../include/wizard.h \
 ../include/parse.h sendsys.h ../include/mobile.h ../include/objsys.h \
 ../include/rooms.h ../include/flags.h ../include/pflags.h \
 ../include/lflags.h ../include/sflags.h ../include/cflags.h \
 ../include/locations.h ../include/fight.h ../include/wizlist.h \
 ../include/commands.h ../include/clone.h ../include/mud.h \
 ../include/logcolors.h
wizlist.o: wizlist.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/fluid.h ../include/log.h ../include/bprintf.h \
 ../include/parse.h
writer.o: writer.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/writer.h ../include/bprintf.h ../include/parse.h \
 ../include/mud.h
zones.o: zones.c kernel.h VERSION.H MACHINE.H ../include/config.h \
 ../include/logconfig.h ../include/levels.h ../include/exits.h \
 ../include/types.h ../include/utils.h mudtypes.h mudmacros.h \
 ../include/macros.h ../include/extern.h ../include/files.h \
 ../include/zones.h ../include/mobile.h ../include/rooms.h \
 ../include/objsys.h ../include/clone.h ../include/bootstrap.h \
 ../include/bprintf.h ../include/log.h ../include/parse.h \
 ../include/uaf.h
