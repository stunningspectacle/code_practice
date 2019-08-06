#
# Pattern rules
#
$(OUT_FOLDER)/%.o:			%.S 
	@echo compiling (ASM) $(notdir $<) ...
	@$(CC) $(CFLAGS) $(addprefix -Xassembler ,$(ARCH_FLAGS))	-D__ASSEMBLY__	\
		-Wa,-mindex-reg	-Wa,--divide $(DEP_FLAGS) $(OUT_FOLDER)\.$(notdir $<).d -c $< -o $@
$(OUT_FOLDER)/%.o:			%.c 
ifeq ($(TRACE_CATALOG),1)
	@echo compiling with trace catalog $(notdir $<) ...
	@echo $(subst \,/,-DTRACE_CATALOG $(CFLAGS)) > $(subst /,\,$@.flags)
	@${CATALOG2_DICT_GEN} -operation compile -compiler $(CC) -cflags-file $@.flags -src $< -out $@ -ult-build
	@$(CC) -MM -MP -MT $@ $< @$(subst /,\,$@.flags) > $(OUT_FOLDER)\.$(notdir $<).d
else
	@echo compiling $(notdir $<) ...
	@$(CC) $(CFLAGS) $(DEP_FLAGS) $(OUT_FOLDER)\.$(notdir $<).d -c $< -o $@
endif

$(OUT_FOLDER)/%.o:			%.s
	@echo assembling $(notdir $<) ...
	@$(CC) $(CFLAGS) -xassembler-with-cpp -c $< -o $@
	