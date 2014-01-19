include ../common.mk

CLEANTARGET = clean
INSTALLTARGET = install
SUBMODS = tlog tbus tbusmgr tconnd utils/start-stop-daemon
SUBMODSCLEAN = $(patsubst %, %.$(CLEANTARGET), $(SUBMODS)) 
SUBMODSINSTALL = $(patsubst %, %.$(INSTALLTARGET), $(SUBMODS)) 

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

release:
	@echo 'release'
	$(MAKE) all _RELEASE=1

