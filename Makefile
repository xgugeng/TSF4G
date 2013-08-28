CLEANTARGET = clean
INSTALLTARGET = install
SUBMODS = tbus tbusmgr tconnd tdatad tprotocol scened worldd
SUBMODSCLEAN = $(patsubst %, %.$(CLEANTARGET), $(SUBMODS)) 
SUBMODSINSTALL = $(patsubst %, %.$(INSTALLTARGET), $(SUBMODS)) 
TDSHOME=$(shell pwd)
MAKE = make -I $(TDSHOME)

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
	@echo 'hello'
	$(MAKE) all _RELEASE=1

