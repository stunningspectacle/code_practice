struct student;
struct class {
	struct student *stud;
	struct class *next;
	struct dlass *prev;
};

struct student {
	struct class *class;
	struct student *next;
	struct student *prev;
};

void main(void) {

}
