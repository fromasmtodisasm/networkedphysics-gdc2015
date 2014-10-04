# GNU Make project makefile autogenerated by Premake
ifndef config
  config=debug64
endif

ifndef verbose
  SILENT = @
endif

ifndef CC
  CC = gcc
endif

ifndef CXX
  CXX = g++
endif

ifndef AR
  AR = ar
endif

ifndef RESCOMP
  ifdef WINDRES
    RESCOMP = $(WINDRES)
  else
    RESCOMP = windres
  endif
endif

ifeq ($(config),debug64)
  OBJDIR     = obj/x64/Debug/ProfileProtocol
  TARGETDIR  = ../bin
  TARGET     = $(TARGETDIR)/ProfileProtocol
  DEFINES   += 
  INCLUDES  += -I../src -I..
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -Wall -msse2 -ffast-math -m64 -std=c++11 -stdlib=libc++ -Wno-deprecated-declarations
  CXXFLAGS  += $(CFLAGS) -fno-rtti -fno-exceptions
  LDFLAGS   += -L../lib -m64 -L/usr/lib64
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LIBS      += ../lib/libcore.a ../lib/libnetwork.a ../lib/libprotocol.a
  LDDEPS    += ../lib/libcore.a ../lib/libnetwork.a ../lib/libprotocol.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(RESOURCES) $(ARCH) $(LIBS) $(LDFLAGS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),release64)
  OBJDIR     = obj/x64/Release/ProfileProtocol
  TARGETDIR  = ../bin
  TARGET     = $(TARGETDIR)/ProfileProtocol
  DEFINES   += -DNDEBUG
  INCLUDES  += -I../src -I..
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -Wall -msse2 -ffast-math -O3 -m64 -std=c++11 -stdlib=libc++ -Wno-deprecated-declarations
  CXXFLAGS  += $(CFLAGS) -fno-rtti -fno-exceptions
  LDFLAGS   += -L../lib -m64 -L/usr/lib64
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LIBS      += ../lib/libcore.a ../lib/libnetwork.a ../lib/libprotocol.a
  LDDEPS    += ../lib/libcore.a ../lib/libnetwork.a ../lib/libprotocol.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(RESOURCES) $(ARCH) $(LIBS) $(LDFLAGS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),debug32)
  OBJDIR     = obj/x32/Debug/ProfileProtocol
  TARGETDIR  = ../bin
  TARGET     = $(TARGETDIR)/ProfileProtocol
  DEFINES   += 
  INCLUDES  += -I../src -I..
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -Wall -msse2 -ffast-math -m32 -std=c++11 -stdlib=libc++ -Wno-deprecated-declarations
  CXXFLAGS  += $(CFLAGS) -fno-rtti -fno-exceptions
  LDFLAGS   += -L../lib -m32 -L/usr/lib32
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LIBS      += ../lib/libcore.a ../lib/libnetwork.a ../lib/libprotocol.a
  LDDEPS    += ../lib/libcore.a ../lib/libnetwork.a ../lib/libprotocol.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(RESOURCES) $(ARCH) $(LIBS) $(LDFLAGS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),release32)
  OBJDIR     = obj/x32/Release/ProfileProtocol
  TARGETDIR  = ../bin
  TARGET     = $(TARGETDIR)/ProfileProtocol
  DEFINES   += -DNDEBUG
  INCLUDES  += -I../src -I..
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -Wall -msse2 -ffast-math -O3 -m32 -std=c++11 -stdlib=libc++ -Wno-deprecated-declarations
  CXXFLAGS  += $(CFLAGS) -fno-rtti -fno-exceptions
  LDFLAGS   += -L../lib -m32 -L/usr/lib32
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LIBS      += ../lib/libcore.a ../lib/libnetwork.a ../lib/libprotocol.a
  LDDEPS    += ../lib/libcore.a ../lib/libnetwork.a ../lib/libprotocol.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(RESOURCES) $(ARCH) $(LIBS) $(LDFLAGS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

OBJECTS := \
	$(OBJDIR)/ProfileProtocol.o \

RESOURCES := \

SHELLTYPE := msdos
ifeq (,$(ComSpec)$(COMSPEC))
  SHELLTYPE := posix
endif
ifeq (/bin,$(findstring /bin,$(SHELL)))
  SHELLTYPE := posix
endif

.PHONY: clean prebuild prelink

all: $(TARGETDIR) $(OBJDIR) prebuild prelink $(TARGET)
	@:

$(TARGET): $(GCH) $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking ProfileProtocol
	$(SILENT) $(LINKCMD)
	$(POSTBUILDCMDS)

$(TARGETDIR):
	@echo Creating $(TARGETDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(TARGETDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(TARGETDIR))
endif

$(OBJDIR):
	@echo Creating $(OBJDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(OBJDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(OBJDIR))
endif

clean:
	@echo Cleaning ProfileProtocol
ifeq (posix,$(SHELLTYPE))
	$(SILENT) rm -f  $(TARGET)
	$(SILENT) rm -rf $(OBJDIR)
else
	$(SILENT) if exist $(subst /,\\,$(TARGET)) del $(subst /,\\,$(TARGET))
	$(SILENT) if exist $(subst /,\\,$(OBJDIR)) rmdir /s /q $(subst /,\\,$(OBJDIR))
endif

prebuild:
	$(PREBUILDCMDS)

prelink:
	$(PRELINKCMDS)

ifneq (,$(PCH))
$(GCH): $(PCH)
	@echo $(notdir $<)
ifeq (posix,$(SHELLTYPE))
	-$(SILENT) cp $< $(OBJDIR)
else
	$(SILENT) xcopy /D /Y /Q "$(subst /,\,$<)" "$(subst /,\,$(OBJDIR))" 1>nul
endif
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -MF $(@:%.o=%.d) -c "$<"
endif

$(OBJDIR)/ProfileProtocol.o: ../tests/ProfileProtocol.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -MF $(@:%.o=%.d) -c "$<"

-include $(OBJECTS:%.o=%.d)
