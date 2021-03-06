# xHarbour project
# Demo makefile for GNU MAKE in tests/ subdir
#
# $Id: Makefile 9279 2011-02-14 18:06:32Z druzus $
#
# Copyright 2003 Giancarlo Niccolai
#
# To create a binary file from a single .prg, just
# launch 
#    make <target_file>
#
# where <target_file> is the name of the binary file
# to be generated. Examples:
# 
#    make test  --->to compile and link test from test.prg
#    make test.exe ---> under windows
#
# Remember to set HB_MT=MT if you want to compile
# programs using Multithreading aware libs
# and HB_MULTI_GT=yes if you use multi GT libs
#

ifeq ($(HB_MT),MT)
   MTLIBS= -lvmmt -lrtlmt -lrddmt -lrtlmt -lvmmt -ldbfntxmt -ldbfcdx -ldbfdbt -lmacromt
   ifeq ($(HB_ARCHITECTURE),bsd)
      C_USR+=-pthread
   endif
   ifeq ($(HB_ARCHITECTURE),linux)
      MTLIBS+=-lpthread
   endif
else
   MTLIBS= -lvm -lrtl -lrdd -lrtl -lvm -ldbfntx -ldbfcdx -ldbfdbt -lmacro 
endif

ifeq ($(HB_ARCHITECTURE),bsd)
   HB_UNIX=yes
endif
ifeq ($(HB_ARCHITECTURE),darwin)
   HB_UNIX=yes
endif
ifeq ($(HB_ARCHITECTURE),linux)
   HB_UNIX=yes
endif

ifeq ($(HB_UNIX),yes)
   ifeq ($(HB_MULTI_GT),yes)
      GT_LIBS=-lgtxvt -lX11 -L/usr/X11R6/lib -lslang -lncurses -lgpm
   else
      ifeq ($(HB_GT_LIB),gtxvt)
         GT_LIBS=-lgtxvt -lX11 -L/usr/X11R6/lib
      else
         ifeq ($(HB_GT_LIB),gtsln)
            GT_LIBS=-lgtsln -lslang
         else
            ifeq ($(HB_GT_LIB),gtcgi)
               GT_LIBS=-lgtcgi
            else
               ifeq ($(HB_GT_LIB),gtnul)
                  GT_LIBS=-lgtnul
               else
	          ifeq ($(HB_GT_LIB),gtstd)
	             GT_LIBS=-lgtstd
	         else
	             HB_GT_LIB=gtcrs
	             GT_LIBS=-lgtcrs -lncurses
	         endif
	       endif
            endif
         endif
      endif
   endif
   EXE_EXT=
   
   ifeq ($(HB_ARCHITECTURE),linux)
      ifeq ($(HB_GT_LIB),gtsln)
         GT_LIBS+=-lgpm
      endif
      ifeq ($(HB_GT_LIB),gtcrs)
         GT_LIBS+=-lgpm
      endif
   endif
   ifeq ($(HB_ARCHITECTURE),bsd)
      ifeq ($(HB_GT_LIB),gtsln)
         GT_LIBS:=-L/usr/local/lib $(GT_LIBS) -lncurses -lgtstd
      endif
      ifeq ($(HB_GT_LIB),gtcrs)
         GT_LIBS:=-L/usr/local/lib $(GT_LIBS)
      endif
   endif
else
   ifeq ($(HB_ARCHITECTURE),dos)
      ifeq ($(HB_GT_LIB),)
         HB_GT_LIB=gtstd
      endif
      GT_LIBS=-l$(HB_GT_LIB)
   else
      ifeq ($(HB_ARCHITECTURE),OS2)
         ifeq ($(HB_GT_LIB),)
            HB_GT_LIB=gtos2
         endif
         GT_LIBS=-l$(HB_GT_LIB) -lsocket 
      else  
         GT_LIBS=-lgtwin -lwsock32 -lwinspool
      endif
   endif
   EXE_EXT=.exe
endif

ifeq ($(HB_MULTI_GT),yes)
   ifeq ($(HB_MT),MT)
      ALL_LIBS = -lxharbourmt $(GT_LIBS) -lm $(EXTRA_LIBS) -lpthread
   else
      ALL_LIBS = -lxharbour $(GT_LIBS) -lm $(EXTRA_LIBS)
   endif
else
   ALL_LIBS = -ldebug $(MTLIBS) -lpp -llang -lcommon $(GT_LIBS) -lm $(EXTRA_LIBS)
endif

ifeq ($(HB_ARCHITECTURE),darwin)
   LD_LIBS = $(ALL_LIBS) $(ALL_LIBS)
else
   ifeq ($(HB_ARCHITECTURE),OS2)	
       LD_LIBS = $(ALL_LIBS)
   else	
       LD_LIBS = -Wl,--start-group $(ALL_LIBS) -Wl,--end-group
   endif
endif

%.c: %.prg
	$(HB_BIN_INSTALL)/harbour -I$(HB_INC_INSTALL) $(PRG_USR) -n -w2 -es2 $<
%.o: %.c
	gcc $(C_USR) -I$(HB_INC_INSTALL) -c -o $@ $<


%$(EXE_EXT): %.o
	gcc -o $@ $< $(C_USR) -L$(HB_LIB_INSTALL) $(LD_LIBS)

