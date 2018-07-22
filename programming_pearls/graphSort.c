struct adjacent {
	struct vertex *from;
	struct vertex *to;
	int distance;
};

struct vertex {
	int indegree;
	struct adjacent *ad;
};

struct graph {
	struct vertex *v;
	int size;
};

int getShort(struct vertex* s, struct graph *graph)
{
	struct queue q;
	struct vertex* v;

	enqueue(q, s);
	while (!empty(q)) {
		v = dequeue(q);
		for adjacent(v, t) in v {
			if (adjacent + v.value < t.value)
				t.value = adjacent + v.value;
			t.adjacent--;
			if (t.adjacent == 0)
				enqueue(t);
		}
	}
}



