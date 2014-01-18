include common.mk

CLEANTARGET = clean
INSTALLTARGET = install
SUBMODS = source tutorials
SUBMODSCLEAN = $(patsubst %, %.$(CLEANTARGET), $(SUBMODS)) 
SUBMODSINSTALL = $(patsubst %, %.$(INSTALLTARGET), $(SUBMODS)) 
MAKE = make -I $(ROOT_DIR)

.PHONY: all $(CLEANTARGET) $(INSTALLTARGET) $(SUBMODS) 

all: $(SUBMODS)
$(CLEANTARGET): $(SUBMODSCLEAN)
$(INSTALLTARGET): $(SUBMODSINSTALL)
	
$(SUBMODS):
	@echo "Begin to make sub module: '$@' ......"
	cd $@ && $(MAKE);
	@echo "Finish to  make sub module: '$@'" 

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

