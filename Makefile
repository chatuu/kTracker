ROOTCONFIG   := root-config
ROOTCINT     := rootcint
ARCH         := $(shell $(ROOTCONFIG) --arch)
ROOTCFLAGS   := $(shell $(ROOTCONFIG) --cflags)
ROOTLDFLAGS  := $(shell $(ROOTCONFIG) --ldflags)
ROOTGLIBS    := $(shell $(ROOTCONFIG) --libs)

CXX           = g++
CXXFLAGS      = -O3 -Wall -fPIC
LD            = g++
LDFLAGS       = -O3
SOFLAGS       = -shared

CXXFLAGS     += $(ROOTCFLAGS)
LDFLAGS      += $(ROOTLDFLAGS) $(ROOTGLIBS)

CXXFLAGS     += -I$(MYSQL_INCLUDE)
LDFLAGS      += -lz -L$(MYSQL_LIB) -lmysqlclient

SRAWEVENTO    = SRawEvent.o SRawEventDict.o
SRAWEVENTS    = SRawEvent.cxx SRawEventDict.cxx

GEOMSVCO      = GeomSvc.o
GEOMSVCS      = GeomSvc.cxx

MYSQLSVCO     = MySQLSvc.o
MYSQLSVCS     = MySQLSvc.cxx

FASTTRACKLETO = FastTracklet.o FastTrackletDict.o
FASTTRACKLETS = FastTracklet.cxx FastTrackletDict.cxx

KALMANFASTO   = KalmanFastTracking.o
KALMANFASTS   = KalmanFastTracking.cxx

KFASTTRACKO   = kFastTracking.o
KFASTTRACKS   = kFastTracking.cxx
KFASTTRACK    = kFastTracking

KONLINETRACKO   = kOnlineTracking.o
KONLINETRACKS   = kOnlineTracking.cxx
KONLINETRACK    = kOnlineTracking

KTRACKERSO    = libkTracker.so

CLASSOBJS     = $(GEOMSVCO) $(SRAWEVENTO) $(KALMANFASTO) $(FASTTRACKLETO) $(MYSQLSVCO)
OBJS          = $(CLASSOBJS) $(KFASTTRACKO) $(KONLINETRACKO)
SLIBS         = $(KTRACKERSO)
PROGRAMS      = $(KFASTTRACK) $(KONLINETRACK)

all:            $(PROGRAMS) $(SLIBS)

.SUFFIXES: .cxx .o

$(KTRACKERSO):  $(CLASSOBJS)
	$(LD) $^ -o $@  $(SOFLAGS) $(LDFLAGS) 
	@echo "$@ done."

$(KFASTTRACK):   $(KFASTTRACKO) $(CLASSOBJS) 
	$(LD) $^ -o $@ $(LDFLAGS) 
	@echo "$@ done."

$(KONLINETRACK):   $(KONLINETRACKO) $(CLASSOBJS)
	$(LD) $^ -o $@ $(LDFLAGS) 
	@echo "$@ done."

.SUFFIXES: .cxx

SRawEventDict.cxx: SRawEvent.h SRawEventLinkDef.h
	@echo "Generating dictionary for $@ ..."
	$(ROOTCINT) -f $@ -c $^

FastTrackletDict.cxx: FastTracklet.h FastTrackletLinkDef.h
	@echo "Generating dictionary for $@ ..."
	$(ROOTCINT) -f $@ -c $^

.cxx.o:
	$(CXX) $(CXXFLAGS) -c $<

.PHONY: clean

clean:
	@echo "Cleanning everything ... "
	@rm $(PROGRAMS) $(OBJS) $(SLIBS) *Dict.cxx *Dict.h
