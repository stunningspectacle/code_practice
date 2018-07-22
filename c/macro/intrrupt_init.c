#define irq(x, y) IRQ_##x##y##handler

#define irq16(x) \
	irq(x, 0), irq(x, 1), irq(x, 2), irq(x, 3) \
	irq(x, 4), irq(x, 5), irq(x, 6), irq(x, 7) \
	irq(x, 8), irq(x, 9), irq(x, a), irq(x, b) \
	irq(x, c), irq(x, d), irq(x, e), irq(x, f)

void (*interrupt[])(void) = {
	irq16(0), irq16(1), irq16(2), irq16(3),
	irq16(4), irq16(5), irq16(6), irq16(7),
	irq16(8), irq16(9), irq16(a), irq16(b),
	irq16(c), irq16(d), irq16(e), irq16(f),
};


void main(void)
{
}
