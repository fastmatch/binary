##
##    This file is part of Fastmatch binary market data and order flow examples.
##
##    Fastmatch binary market data and order flow examples are free software: you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation, either version 3 of the License, or
##    (at your option) any later version.
##
##    Fastmatch binary market data and order flow examples are distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with Fastmatch binary market data examples.  If not, see <http:#www.gnu.org/licenses/>.
##

# Main makefile

PROG = BinaryTests
TRGTS = ItchMon Sniper Samples

$(PROG): $(TRGTS)

all: $(TRGTS)

ItchMon:
	make -j 6 -f ItchMon.mak

Sniper:
	make -j 6 -f Sniper.mak

Samples:
	make -j 6 -f Sample1.mak  
	make -j 6 -f Sample11.mak
	make -j 6 -f Sample12.mak  
	make -j 6 -f Sample12x.mak  
	make -j 6 -f Sample13.mak  
	make -j 6 -f Sample14.mak  
	make -j 6 -f Sample15.mak

clean:
	rm -f *.o *~
	make -f ItchMon.mak clean
	make -f Sniper.mak clean
	make -j 6 -f Sample11.mak clean
	make -j 6 -f Sample1.mak clean
	make -j 6 -f Sample12.mak clean
	make -j 6 -f Sample12x.mak clean
	make -j 6 -f Sample13.mak clean
	make -j 6 -f Sample14.mak clean
	make -j 6 -f Sample15.mak clean

# End of the main math makefile
