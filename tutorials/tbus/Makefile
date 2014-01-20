include ../../common.mk

CLEANTARGET = clean
INSTALLTARGET = install
TAGSTARGET = tags

SUBMODS = tbus_client tbus_server
SUBMODSCLEAN = $(patsubst %, %.$(CLEANTARGET), $(SUBMODS)) 
SUBMODSINSTALL = $(patsubst %, %.$(INSTALLTARGET), $(SUBMODS)) 
SUBMODSTAGS = $(patsubst %, %.$(TAGSTARGET), $(SUBMODS))

.PHONY: all $(CLEANTARGET) $(INSTALLTARGET) $(SUBMODS) $(TAGSTARGET)

all: $(SUBMODS)

$(CLEANTARGET): $(SUBMODSCLEAN)

$(INSTALLTARGET): $(SUBMODSINSTALL)

$(TAGSTARGET): $(SUBMODSTAGS)
	
$(SUBMODS):
	@echo "Begin to make sub module: '$@' ......"
	cd $@ && $(MAKE);
	@echo "Finish to  make sub module: '$@'" 

$(SUBMODSTAGS): 
	@echo "Begin to tags sub module: '$(patsubst %.$(TAGSTARGET),%, $@)' ......"
	cd $(patsubst %.$(TAGSTARGET),%, $@)  && $(MAKE) $(TAGSTARGET);
	@echo "Finish to tags sub module: '$(patsubst %.$(TAGSTARGET),%, $@)'" 

$(SUBMODSCLEAN): 
	@echo "Begin to clean sub module: '$(patsubst %.$(CLEANTARGET),%, $@)' ......"
	cd $(patsubst %.$(CLEANTARGET),%, $@)  && $(MAKE) $(CLEANTARGET);
	@echo "Finish to clean sub module: '$(patsubst %.$(CLEANTARGET),%, $@)'" 

$(SUBMODSINSTALL): 
	@echo "Begin to install sub module: '$(patsubst %.$(INSTALLTARGET),%, $@)' ......"
	cd $(patsubst %.$(INSTALLTARGET),%, $@)  && $(MAKE) $(INSTALLTARGET);
	@echo "Finish to install sub module: '$(patsubst %.$(INSTALLTARGET),%, $@)'" 
	cp include $(INSTALL_PREFIX)/ -rf
	cp README.md $(INSTALL_PREFIX)/

release:
	@echo 'release'
	$(MAKE) all _RELEASE=1

