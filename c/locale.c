#include <stdio.h>
#include <locale.h>
#include <libintl.h>

#define PACKAGE "starter"
#define LOCALEDIR "/usr/share/locale"

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	printf("%s\n", gettext("ok"));
}

