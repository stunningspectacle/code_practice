int func(int);

int bar(int i) {
	if (i > 3)
		return i;
	else
		return func(i);
}
