struct vm_area_struct *find_vma(struct mm_struct *mm, unsigned long addr)
{
	struct vm_area_struct *vma = NULL;

	if (mm) {
		vma = mm->mmap_cache;
		if (!(vma && vma->vm_end > addr && vma->vm_start <= addr)) {
			struct rb_node *rb_node;
			
			rb_node = mm->mm_rb.rb_node;
			vma = NULL;

			while (rb_node) {
				struct vm_area_struct *vma_tmp;
				vma_tmp = rb_entry(rb_node,
						struct vm_area_struct, vm_rb);
				if (vma_tmp->vm_end > addr) {
					vma = vma_tmp;
					if (vma_tmp->vm_start <= addr)
						break;
					rb_node = rb_node->left;
				} else 
					rb_node = rb_node->right;
			}
			if (vma)
				mm->mmap_cache = vma;
		}
	}

	return vma;
}

/* vma->vm_pgoff - regarding do_brk():
 * if vma is a anon one, the vma->vm_pgoff is address >> PAGE_SHIFT
 * if vma has a backing file, vma->vm_pgoff is the offset in pages in the file
 */


// Get the virtual address of a page
static inline unsigned long
vma_address(struct page *page, struct vm_area_struct *vma)
{
	pgoff_t pgoff = page->index << (PAGE_CACHE_SHIFT - PAGE_SHIFT);
	unsigned long address;

	// TODO: Why need to subtract vma->vm_pgoff?
	address = vma->vm_start + ((pgoff - vma->vm_pgoff) << PAGE_SHIFT);
	if (unlikely(address < vma->vm_start || address >= vma->vm_end))
			return -EFAULT;
	return address;
}

struct mutex {
	atomic_t count;
	spinlock_t wait_lock;
	list_head_t wait_list;
};

struct mcs_spinlock {
	struct mcs_spinlock *next;
	int locked;
};

inline void spin_lock(struct mcs_spinlock **lock, struct mcs_spinlock *node)
{
	struct mcs_spinlock *prev;

	node->next = NULL;
	node->locked = 0;

	prev = xchg(lock, node);
	if (likely(prev == NULL))
		return; 

	prev->next = node;

	while (!(node->locked));
}

inline void spin_unlock(struct mcs_spinlock **lock, struct mcs_spinlock *node)
{
	struct mcs_spinlock *next = node->next;

	if (likely(!next)) {
		if (likely(cmpxchg(lock, node, NULL) == node))
			return;

		while (!(next = node->next));
	}

	next->locked = 1;
}

