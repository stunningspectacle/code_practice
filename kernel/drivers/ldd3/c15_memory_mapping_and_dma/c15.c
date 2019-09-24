int simple_remap_map(struct file *filp, struct vm_area_struct *vma)
{
	if (remap_pfn_range(vma, vma->start, vma->pgoff,
				vma->end - vma_start, vma->vm_page_prot))
		return -EAGAIN;
	vma->vm_ops = &simple_remap_ops;
	simple_vm_open(vma);
	return 0;
}

void simple_vma_open(struct vm_area_struct *vma)
{
	printk("vma open virtual@%p, physical@%p, size(0x%x)\n",
			vma->vm_start, vma->vm_pgoff << PAGE_SHIFT,
			vma->end - vma_start);
}

void simple_vma_close(struct vm_area_struct *vma)
{
	printk("vma close virtual@%p, physical@%p, size(0x%x)\n",
			vma->vm_start, vma->vm_pgoff,
			vma->vm_end - vma->vm_start);
}

struct page * simple_vm_nopage(struct vm_area_struct *vma,
		unsigned long addr, int *type)
{
	struct page *page;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long phy_addr = addr - vma->vm_start + offset;
	unsigned long pfn = phy_addr >> PAGE_SHIFT;

	if (!pfn_valid(pfn))
		return -EFAULT;
	
	page = pfn_to_page(pfn);
	get_page(page);

	if (type)
		*type = VM_FAULT_NOPAGE;

	return page;
}
