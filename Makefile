# Makefile to build the package zip file
#
# Author:  Freja Nordsiek
# Notes:
# History: * 2013-08-15: Created 

# Basic definitisions

RM  = rm -f
ZIP = zip -r

# Package information.

PACKAGENAME=AD56X4
# Convert spaces to underscores in the version.
VERSION=`cat VERSION.txt | tr [:space:] _ `
PACKAGEFILE=$(PACKAGENAME)_$(VERSION).zip
PACKAGECONTENTS=LICENSE.txt VERSION.txt keywords.txt Makefile ChangeLog.txt \
                README.md $(PACKAGENAME).h $(PACKAGENAME).cpp examples

all: package

package: $(PACKAGECONTENTS)
	$(RM) $(PACKAGEFILE)
	$(ZIP) $(PACKAGEFILE) $(PACKAGECONTENTS)

clean:
	$(RM) $(PACKAGENAME)_*.zip
