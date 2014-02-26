include common.mk

SUBMODS = source/tcommon source/tlog source/tbus source/tbusmgr source/tconnd source/utils/start-stop-daemon tutorials/tbus/tbus_client 
SUBMODS += tutorials/tlog/ tutorials/tbus/tbus_server 
SUBMODS += tutorials/tconnd/tconnd_client tutorials/tconnd/tconnd_server tutorials/tconnd/socket_client tutorials/tconnd/socket_server
SUBMODS += source/tsqld_protocol source/tsqld tutorials/tsqld/tsqld_client

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
	mkdir -p $(INSTALL_PREFIX)
	cp README.md $(INSTALL_PREFIX)/
	cp $(PACKAGE_PATH)/* $(INSTALL_PREFIX)/ -rf

release:
	@echo 'release'
	$(MAKE) all _RELEASE=1

