
    SHELL = /bin/sh

VERSION = 4.03

# Default machine ... override on command line
  MACHINE = SUN

# Target subdirectories
  SUBDIRS = ipcv4.0 examples

all:	machdep everything

# Compilation assuming that makefiles etc. have been made
everything:
	for dir in $(SUBDIRS); do \
		echo Making $@ in $$dir ; \
		(cd $$dir; make) ; \
	done

# Convert machine dependent files that don't enjoy the
# C preprocessor
machdep:
	for dir in . $(SUBDIRS) ;\
		do \
		for file in $$dir/*.proto ; \
			do \
			./port "$(MACHINE)" $$file ; \
			done \
		done
	chmod a+x examples/demo
	chmod a+x makep

clean:
	for dir in $(SUBDIRS); do \
		echo Making $@ in $$dir ; \
		(cd $$dir; make clean) ; \
	done

realclean:
	/bin/rm -f *~ makep
	for dir in $(SUBDIRS); do \
		echo Making $@ in $$dir ; \
		(cd $$dir; make realclean) ; \
	done

# This rule seems to only work on a SUN due to the use
# of the X option to exclude the RCS directory.
tcgmsg.tar:	
	(cd ..; \
	echo tcgmsg/ipcv4.0/RCS  > exclude.$$$$ ; \
	echo tcgmsg/ipcv4.1     >> exclude.$$$$ ; \
	tar cvfX tcgmsg.$(VERSION).tar exclude.$$$$ tcgmsg; \
	compress -c tcgmsg.$(VERSION).tar > tcgmsg.$(VERSION).tar.Z; \
	uuencode tcgmsg.$(VERSION).tar.Z < tcgmsg.$(VERSION).tar.Z \
		> tcgmsg.$(VERSION).tar.Z.uu; \
	/bin/rm -f exclude.$$$$)

tcgmsg.shar:	
	(cd ..; \
	 shar tcgmsg > tcgmsg.$(VERSION).shar ; \
	 /bin/rm -f tcgmsg.$(VERSION).shar.??; \
	 split -4000 tcgmsg.$(VERSION).shar tcgmsg.$(VERSION).shar. ; \
	)
