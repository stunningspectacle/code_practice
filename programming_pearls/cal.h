struct ops;

void parseBuf(char *buf, struct ops *head);
int isSymbol(char c);
void insertSymbol(char symbol, struct ops *head);
void insertNum(int num, struct ops *head);
