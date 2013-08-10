CLEANTARGET = clean
SUBMODS = tbus tbusmgr tconnd tdatad scened
SUBMODSCLEAN =$(patsubst %, %.$(CLEANTARGET), $(SUBMODS)) 

.PHONY: all $(CLEANTARGET) $(SUBMODS) 

release:
	make all _RELEASE=1

all: $(SUBMODS)
$(CLEANTARGET): $(SUBMODSCLEAN)
	
$(SUBMODS):
	@echo "Begin to make sub module: '$@' ......"
	cd $@ && $(MAKE);
	@echo "Finish to  make sub module: '$@'" 

$(SUBMODSCLEAN): 
	@echo "Begin to clean sub module: '$(patsubst %.$(CLEANTARGET),%, $@)' ......"
	cd $(patsubst %.$(CLEANTARGET),%, $@)  && make $(CLEANTARGET);
	@echo "Finish to  clean sub module: '$(patsubst %.$(CLEANTARGET),%, $@)'" 

