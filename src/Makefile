#########################################################
# iDiRT Makefile					#
# 1995, 1996 Illusion					#
#########################################################

# Directory Definitions
#
BIN	= /mud/bin/
DATA	= /mud/data/
ZONE	= $(DATA)ZONES
UTIL	= /mud/utils/

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
OPFLAGS	= -pipe -m64 -O3
WFLAGS	= -Wall
CFLAGS	= $(DEFINES) $(DBFLAGS) $(OPFLAGS) $(WFLAGS) $(HDIR)

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
# Last Updated: Sat Jan 5 10:37:29 STD 2019                 #
#############################################################

# Dependencies for the MUD source code
#
acct.o: acct.c kernel.h VERSION.H MACHINE.H config.h logconfig.h levels.h \
 exits.h types.h utils.h mudtypes.h mudmacros.h macros.h extern.h files.h \
 locations.h objects.h mobiles.h pflags.h oflags.h lflags.h cflags.h \
 sflags.h bprintf.h parse.h acct.h zones.h mobile.h objsys.h rooms.h \
 timing.h uaf.h flags.h
actions.o: actions.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h locations.h sendsys.h bprintf.h objects.h mobiles.h \
 actions.h mobile.h sflags.h mflags.h objsys.h zones.h parse.h rooms.h \
 fight.h log.h
bootstrap.o: bootstrap.c kernel.h VERSION.H MACHINE.H config.h \
 logconfig.h levels.h exits.h types.h utils.h mudtypes.h mudmacros.h \
 macros.h extern.h files.h bootstrap.h wizlist.h actions.h timing.h \
 oflags.h nflags.h eflags.h log.h
bprintf.o: bprintf.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h sflags.h pflags.h lflags.h bprintf.h mud.h mobile.h \
 exit.h log.h rooms.h parse.h
change.o: change.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h pflags.h sflags.h uaf.h mobile.h flags.h sendsys.h \
 writer.h change.h parse.h zones.h bprintf.h log.h wizlist.h rooms.h \
 objsys.h mud.h
climate.o: climate.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h sendsys.h climate.h pflags.h sflags.h lflags.h \
 bprintf.h parse.h
clone.o: clone.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h bootstrap.h mobile.h pflags.h cflags.h sendsys.h \
 objsys.h oflags.h zones.h parse.h fight.h clone.h eflags.h commands.h \
 rooms.h bprintf.h uaf.h log.h
comm.o: comm.c kernel.h VERSION.H MACHINE.H config.h logconfig.h levels.h \
 exits.h types.h utils.h mudtypes.h mudmacros.h macros.h extern.h files.h \
 mobiles.h sendsys.h bprintf.h mobile.h pflags.h sflags.h eflags.h \
 nflags.h lflags.h parse.h flags.h comm.h uaf.h
commands.o: commands.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h locations.h objects.h mobiles.h sflags.h pflags.h \
 oflags.h lflags.h cflags.h eflags.h mflags.h quests.h sendsys.h \
 commands.h rooms.h objsys.h mobile.h flags.h bprintf.h parse.h uaf.h \
 clone.h acct.h mud.h nflags.h fight.h zones.h log.h reboot.h wizlist.h \
 climate.h mailer.h timing.h
crypt.o: crypt.c
exit.o: exit.c kernel.h VERSION.H MACHINE.H config.h logconfig.h levels.h \
 exits.h types.h utils.h mudtypes.h mudmacros.h macros.h extern.h files.h \
 mobile.h sendsys.h timing.h exit.h pflags.h log.h reboot.h commands.h \
 bprintf.h mud.h uaf.h parse.h
fight.o: fight.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h cflags.h oflags.h lflags.h mflags.h sflags.h pflags.h \
 quests.h sendsys.h verbs.h bprintf.h commands.h move.h mud.h rooms.h \
 log.h clone.h mobiles.h objects.h locations.h spell.h fight.h mobile.h \
 objsys.h
flags.o: flags.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h pflags.h lflags.h mflagnames.h pflagnames.h \
 sflagnames.h lflagnames.h nflagnames.h eflagnames.h levelnames.h \
 questnames.h sflags.h nflags.h eflags.h sendsys.h bprintf.h parse.h \
 rooms.h mobile.h uaf.h log.h
frob.o: frob.c kernel.h VERSION.H MACHINE.H config.h logconfig.h levels.h \
 exits.h types.h utils.h mudtypes.h mudmacros.h macros.h extern.h files.h \
 sendsys.h pflags.h sflags.h frob.h uaf.h mobile.h log.h parse.h \
 bprintf.h mud.h flags.h wizlist.h
generate.o: generate.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h
god.o: god.c kernel.h VERSION.H MACHINE.H config.h logconfig.h levels.h \
 exits.h types.h utils.h mudtypes.h mudmacros.h macros.h extern.h files.h \
 locations.h sendsys.h wizlist.h bprintf.h actions.h pflags.h sflags.h \
 mobile.h timing.h fight.h rooms.h verbs.h parse.h log.h god.h uaf.h \
 mud.h
log.o: log.c kernel.h VERSION.H MACHINE.H config.h logconfig.h levels.h \
 exits.h types.h utils.h mudtypes.h mudmacros.h macros.h extern.h files.h \
 log.h
magic.o: magic.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h sendsys.h magic.h pflags.h sflags.h cflags.h lflags.h \
 oflags.h objects.h bprintf.h parse.h mobile.h mud.h uaf.h objsys.h \
 rooms.h
mailer.o: mailer.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h sendsys.h bprintf.h mobile.h mailer.h parse.h log.h \
 mud.h uaf.h
main.o: main.c kernel.h VERSION.H MACHINE.H config.h logconfig.h levels.h \
 exits.h types.h utils.h mudtypes.h mudmacros.h macros.h extern.h files.h \
 jmp.h mud.h log.h bootstrap.h sendsys.h mobile.h commands.h timing.h \
 exit.h s_socket.h locations.h pflags.h update.h climate.h reboot.h \
 objsys.h bprintf.h zones.h uaf.h fight.h rooms.h ver.h main.h
mkdata.o: mkdata.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h cflags.h mflagnames.h pflagnames.h sflagnames.h \
 lflagnames.h oflagnames.h nflagnames.h eflagnames.h exitnames.h lflags.h \
 nflags.h
mobile.o: mobile.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h locations.h commands.h wizlist.h sendsys.h objects.h \
 mobiles.h bprintf.h objsys.h pflags.h mflags.h sflags.h oflags.h \
 lflags.h nflags.h eflags.h quests.h cflags.h timing.h spell.h verbs.h \
 rooms.h fight.h parse.h zones.h flags.h puff.h move.h uaf.h mud.h log.h \
 jmp.h mobile.h
move.o: move.c kernel.h VERSION.H MACHINE.H config.h logconfig.h levels.h \
 exits.h types.h utils.h mudtypes.h mudmacros.h macros.h extern.h files.h \
 parse.h objects.h mobiles.h locations.h lflags.h mflags.h sendsys.h \
 move.h bprintf.h objsys.h mobile.h rooms.h fight.h uaf.h
mud.o: mud.c kernel.h VERSION.H MACHINE.H config.h logconfig.h levels.h \
 exits.h types.h utils.h mudtypes.h mudmacros.h macros.h extern.h files.h \
 sendsys.h pflags.h sflags.h mud.h uaf.h mobile.h timing.h locations.h \
 bprintf.h fight.h zones.h rooms.h mailer.h log.h parse.h acct.h
objsys.o: objsys.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h objects.h mobiles.h mobile.h locations.h sflags.h \
 pflags.h mflags.h oflags.h lflags.h cflags.h eflags.h quests.h sendsys.h \
 parse.h objsys.h oflagnames.h rooms.h bprintf.h fight.h commands.h log.h \
 clone.h
parse.o: parse.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h condact.h verbs.h sflags.h sendsys.h locations.h \
 objects.h parse.h mobile.h move.h commands.h climate.h wizard.h change.h \
 clone.h pflags.h objsys.h bprintf.h uaf.h actions.h spell.h flags.h \
 viewcom.h log.h exit.h wizlist.h timing.h frob.h acct.h zones.h magic.h \
 reboot.h mailer.h fight.h rooms.h comm.h main.h ver.h god.h
puff.o: puff.c kernel.h VERSION.H MACHINE.H config.h logconfig.h levels.h \
 exits.h types.h utils.h mudtypes.h mudmacros.h macros.h extern.h files.h \
 sendsys.h mobiles.h puff.h sflags.h mobile.h
rooms.o: rooms.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h locations.h objects.h pflags.h oflags.h lflags.h \
 cflags.h sflags.h sendsys.h rooms.h climate.h parse.h mobile.h objsys.h \
 zones.h exitnames.h bprintf.h fight.h uaf.h log.h
s_socket.o: s_socket.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h s_socket.h
sendsys.o: sendsys.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h sflags.h pflags.h lflags.h sendsys.h mobile.h nflags.h \
 bprintf.h mud.h commands.h log.h
spell.o: spell.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h oflags.h eflags.h pflags.h lflags.h sendsys.h verbs.h \
 mobiles.h objects.h locations.h objsys.h spell.h spells.h bprintf.h \
 parse.h fight.h mobile.h sflags.h
timing.o: timing.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h locations.h climate.h bprintf.h sendsys.h timing.h \
 mobile.h cflags.h objsys.h spell.h rooms.h log.h mud.h
uaf.o: uaf.c pflags.h kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h sflags.h mflags.h nflags.h eflags.h uaf.h mobile.h \
 flags.h exit.h sendsys.h mud.h bprintf.h wizlist.h bootstrap.h log.h \
 parse.h
update.o: update.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h objsys.h mobile.h update.h log.h
utils.o: utils.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h global.h crypt.h
ver.o: ver.c LINKCOUNT.H kernel.h VERSION.H MACHINE.H config.h \
 logconfig.h levels.h exits.h types.h utils.h mudtypes.h mudmacros.h \
 macros.h extern.h files.h bprintf.h ver.h
viewcom.o: viewcom.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h viewcom.h commands.h wizard.h mud.h uaf.h writer.h \
 mailer.h change.h frob.h bprintf.h timing.h pflags.h mobile.h
wizard.o: wizard.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h timing.h log.h uaf.h bprintf.h verbs.h wizard.h parse.h \
 sendsys.h mobile.h objsys.h rooms.h flags.h pflags.h lflags.h sflags.h \
 cflags.h locations.h fight.h wizlist.h commands.h clone.h mud.h \
 logcolors.h
wizlist.o: wizlist.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h fluid.h log.h bprintf.h parse.h
writer.o: writer.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h writer.h bprintf.h parse.h mud.h
zones.o: zones.c kernel.h VERSION.H MACHINE.H config.h logconfig.h \
 levels.h exits.h types.h utils.h mudtypes.h mudmacros.h macros.h \
 extern.h files.h zones.h mobile.h rooms.h objsys.h clone.h bootstrap.h \
 bprintf.h log.h parse.h uaf.h
