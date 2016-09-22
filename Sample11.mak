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

#### Compiler and tool definitions shared by all build targets #####
CCC = /usr/local/gcc492/bin/g++
CXX = /usr/local/gcc492/bin/g++
BASICOPTS = -m64 -Wall -Wno-unused-parameter -Wno-deprecated-declarations -Wno-unused-local-typedefs -Wno-missing-field-initializers -fno-strict-aliasing -Wno-strict-aliasing -W -fPIC -fno-enforce-eh-specs -pthread -O3 -g -std=c++14 -std=gnu++14  -static-libstdc++ -fdiagnostics-color=always -pipe -Wpointer-arith -Wwrite-strings -Woverloaded-virtual -fmax-errors=3 -march=corei7-avx -mtune=corei7-avx -mno-avx -mno-aes

#BASICOPTS = -m64 -Wall -Wno-unused-parameter -Wno-deprecated-declarations -Wno-unused-local-typedefs -Wno-missing-field-initializers -fno-strict-aliasing -Wno-strict-aliasing -W -fPIC -fno-enforce-eh-specs -pthread -O0 -ggdb3 -std=c++14 -std=gnu++14  -static-libstdc++ -fdiagnostics-color=always -pipe -Wpointer-arith -Wwrite-strings -Woverloaded-virtual -fmax-errors=3 -mno-avx -mno-aes -DDEBUG
CCFLAGS = $(BASICOPTS)  
CXXFLAGS = $(BASICOPTS)
CCADMIN = 

# Define the target directories.
TARGETDIR_all=GNU-amd64-Linux/Sample1


all: $(TARGETDIR_all)/Sample11

## Target: all
CPPFLAGS_all = \
	-isystem/home/fxlocus/apps/linux-x86_64/boost_gcc492/boost/include \
        -I./

OBJS_all =  \
	$(TARGETDIR_all)/sample11.o \
	$(TARGETDIR_all)/socketutils.o \
	$(TARGETDIR_all)/itch_handlers.o \
	$(TARGETDIR_all)/soup_handlers.o \
	$(TARGETDIR_all)/smtp.o \
	$(TARGETDIR_all)/app.o \
	$(TARGETDIR_all)/connection.o \
	$(TARGETDIR_all)/helpers.o \
	$(TARGETDIR_all)/utils.o


SYSLIBS_all = -dynamic -lpthread -lrt
USERLIBS_all =

ifeq ($(onload),true)
USERLIBS_all +=../Onload/lib/libonload_ext.a
CPPFLAGS_all += -I../Onload/include/
CPPFLAGS_all += -DEBL_ONLOAD
SYSLIBS_all  += -ldl
endif

USERLIBS_all += $(SYSLIBS_all)

DEPLIBS_all =  
LDLIBS_all = $(USERLIBS_all)

DEPS_H = \
    src/adapter.h \
    src/app.h \
    src/buffer.h \
    src/connection.h \
    src/defs.h \
    src/dispatcher_defs.h \
    src/helpers.h \
    src/itch/book.h \
    src/itch/dispatcher.h \
    src/itch/itch.h \
    src/itch_handlers.h \
    src/ostore.h \
    src/smtp.h \
    src/soup_handlers.h \
    src/soupbin/dispatcher.h \
    src/soupbin/dispatcher_udp.h \
    src/soupbin/soupbin.h \
    src/socketutils.h \
	src/utils.h


$(OBJS_all) : | $(TARGETDIR_all)

$(TARGETDIR_all) :
	test -d $(TARGETDIR_all) || mkdir -p $(TARGETDIR_all)


# Link or archive
$(TARGETDIR_all)/Sample11: $(OBJS_all) $(DEPLIBS_all)
	$(LINK.cc) $(CCFLAGS_all) $(CPPFLAGS_all) -o $@ $(OBJS_all) $(LDLIBS_all)


# Compile source files into .o files
$(TARGETDIR_all)/sample11.o: src/sample11.cpp $(DEPS_H)
	$(COMPILE.cc) $(CCFLAGS_all) $(CPPFLAGS_all) -o $@ src/sample11.cpp

$(TARGETDIR_all)/socketutils.o: src/socketutils.cpp $(DEPS_H)
	$(COMPILE.cc) $(CCFLAGS_all) $(CPPFLAGS_all) -o $@ src/socketutils.cpp

$(TARGETDIR_all)/itch_handlers.o: src/itch_handlers.cpp $(DEPS_H)
	$(COMPILE.cc) $(CCFLAGS_all) $(CPPFLAGS_all) -o $@ src/itch_handlers.cpp

$(TARGETDIR_all)/soup_handlers.o: src/soup_handlers.cpp $(DEPS_H)
	$(COMPILE.cc) $(CCFLAGS_all) $(CPPFLAGS_all) -o $@ src/soup_handlers.cpp

$(TARGETDIR_all)/smtp.o: src/smtp.cpp $(DEPS_H)
	$(COMPILE.cc) $(CCFLAGS_all) $(CPPFLAGS_all) -o $@ src/smtp.cpp

$(TARGETDIR_all)/app.o:  src/app.cpp $(DEPS_H)
	$(COMPILE.cc) $(CCFLAGS_all) $(CPPFLAGS_all) -o $@ src/app.cpp

$(TARGETDIR_all)/connection.o: src/connection.cpp $(DEPS_H)
	$(COMPILE.cc) $(CCFLAGS_all) $(CPPFLAGS_all) -o $@ src/connection.cpp

$(TARGETDIR_all)/helpers.o: src/helpers.cpp $(DEPS_H)
	$(COMPILE.cc) $(CCFLAGS_all) $(CPPFLAGS_all) -o $@ src/helpers.cpp

$(TARGETDIR_all)/utils.o: src/utils.cpp $(DEPS_H)
	$(COMPILE.cc) $(CCFLAGS_all) $(CPPFLAGS_all) -o $@ src/utils.cpp

#### Clean target deletes all generated files ####
clean:
	rm -f \
		$(TARGETDIR_all)/Sample11 \
		$(TARGETDIR_all)/sample11.o \
		$(TARGETDIR_all)/socketutils.o \
		$(TARGETDIR_all)/itch_handlers.o \
		$(TARGETDIR_all)/soup_handlers.o \
		$(TARGETDIR_all)/smtp.o \
		$(TARGETDIR_all)/app.o \
		$(TARGETDIR_all)/connection.o \
		$(TARGETDIR_all)/helpers.o \
	    $(TARGETDIR_all)/utils.o
	$(CCADMIN)
	rm -f -r $(TARGETDIR_all)


# Create the target directory (if needed)
#$(TARGETDIR_all):
#	mkdir -p $(TARGETDIR_all)


# Enable dependency checking
.KEEP_STATE:
.KEEP_STATE_FILE:.make.state.samp11.GNU-amd64-Linux

