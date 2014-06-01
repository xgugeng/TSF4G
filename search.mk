#parameter list
SUBMODS ?=
#parameter list

CLEANTARGET = clean
INSTALLTARGET = install
TAGSTARGET = tags

SUBMODSCLEAN = $(patsubst %, %.$(CLEANTARGET), $(SUBMODS)) 
SUBMODSINSTALL = $(patsubst %, %.$(INSTALLTARGET), $(SUBMODS)) 
SUBMODSTAGS = $(patsubst %, %.$(TAGSTARGET), $(SUBMODS))

.PHONY: all $(CLEANTARGET) $(INSTALLTARGET) $(SUBMODS) $(TAGSTARGET)

all: $(SUBMODS)

$(CLEANTARGET): $(SUBMODSCLEAN)

$(INSTALLTARGET): $(SUBMODSINSTALL)

$(TAGSTARGET): $(SUBMODSTAGS)

$(SUBMODS):
	cd $@ && $(MAKE);

$(SUBMODSTAGS): 
	cd $(patsubst %.$(TAGSTARGET),%, $@)  && $(MAKE) $(TAGSTARGET);

$(SUBMODSCLEAN): 
	cd $(patsubst %.$(CLEANTARGET),%, $@)  && $(MAKE) $(CLEANTARGET);

$(SUBMODSINSTALL): 
	cd $(patsubst %.$(INSTALLTARGET),%, $@)  && $(MAKE) $(INSTALLTARGET);

