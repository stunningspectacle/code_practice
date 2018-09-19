SOURCE = fread  static  test  va_arg hello mmap

$(SOURCE): fread.c static.c test.c va_arg.c hello.c mmap.c
	@echo $<
	@echo $>
	@echo $^
	@echo $@
	for i in $(SOURCE); do \
		echo $$i;	\
		gcc -o $$i $$i.c ;\
	done
