
# makefile is something like this

DEVICE_LIB=bunch of .o files

# first pass linking used to build mpu table
$(BINDIR)/device_first_pass: $(DEVICE_LIB)
	@echo " g++ linking device $@"
	@mkdir -p $(BINDIR)
	@$(LINKER) -o $@ $(filter %.o,$^)

# memory map (in the BIN directory!) is calculated after linking the firmware the first time.
$(BINDIR)/memory_map.h: $(BINDIR)/device_first_pass memory_map.yaml
	@echo ""
	@echo "--> extracting symbols"
	$(OBJDUMP) -t $(BINDIR)/device_first_pass > $(BINDIR)/device_first_pass.syms
	# note, the script sets the mpu table size to 15. This has to match the value of MPU_TABLE_SIZE
	# in mpu_table.h
	sh ../scripts/run_update_memory_map.sh \
	     $(BINDIR)/device_first_pass.syms \
	     memory_map.yaml $(BINDIR)/memory_map.h 15

# recompile the memory map with the results of the first firmware link.
# note the very very important -I$(BINDIR) at the beginning of the compile line to prefer our new memory_map.h file.
$(DEVICE_OBJDIR)/new_mpu_table.o: ../device/core/m7/mpu_table.cpp $(BINDIR)/memory_map.h
	@echo " compiling device $(notdir $<) (note with -I$(BINDIR) to use new memory_map.h)"
	@echo " if compiling new_mpu_table fails see run_update_memory_map.sh or contact Byron or Vas"
	@$(CC) -I$(BINDIR) $(CFLAGS) -o $@ -c $<

# create libdevice_final.a by removing the first pass mpu table from libdevice.a and substituting the new mpu_table
$(DEVICE_LIB_FINAL): $(DEVICE_LIB) $(DEVICE_OBJDIR)/new_mpu_table.o
	cp $(DEVICE_LIB) $(DEVICE_LIB_FINAL)
	$(ARCHIVE) d $(DEVICE_LIB_FINAL) mpu_table.o
	$(ARCHIVE) r $(DEVICE_LIB_FINAL) $(DEVICE_OBJDIR)/new_mpu_table.o

$(BINDIR)/device: $(DEVICE_OBJDIR)/device_main.o $(DEVICE_LIB_FINAL)
	@echo " linking device $@"
	@mkdir -p $(BINDIR)
	@$(LINK) $(LFLAGS) -o $@ \
		$(filter %.o,$^) \
		$(patsubst lib%.a,-l%,$(notdir $(filter %.a,$^)))
