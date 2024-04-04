/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
#include "threads/vaddr.h"
#include "threads/mmu.h"
#include "filesys/file.h"
#include "threads/malloc.h"
#include "userprog/syscall.h"
#include "filesys/filesys.h"

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);

/* DO NOT MODIFY this struct */
static const struct page_operations file_ops = {
	.swap_in = file_backed_swap_in,
	.swap_out = file_backed_swap_out,
	.destroy = file_backed_destroy,
	.type = VM_FILE,
};

/* The initializer of file vm */
void
vm_file_init (void) {
}

/* Initialize the file backed page */
bool
file_backed_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	page->operations = &file_ops;
	return true;
}

/* Swap in the page by read contents from the file. */
static bool
file_backed_swap_in (struct page *page, void *kva) {
	struct file_page *file_page UNUSED = &page->file;
	return true;
}

/* Swap out the page by writeback contents to the file. */
static bool
file_backed_swap_out (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void
file_backed_destroy (struct page *page) {
	struct file_page *file_page = &page->file;
	uint64_t *pml4 = thread_current()->pml4;
	struct file_page *file_page_aux = (struct file_page *) page->uninit.aux;

	if (pml4_is_dirty(pml4, page->va)) {
		file_write_at(file_page->pending_file, page->va, file_page->length, file_page->offset);
		pml4_set_dirty(pml4, page->va, false);
	}
	pml4_clear_page(pml4, page->va);
}

bool lazy_load_file(struct page *page, void *aux) {
	struct file_page *file_page_aux = (struct file_page *) aux;
	struct file_page *file_page = &page->file;

	file_page->length = file_page_aux->length;
	file_page->offset = file_page_aux->offset;
	file_page->pending_file = file_page_aux->pending_file;
	file_page->has_next = file_page_aux->has_next;
	void *kpage = page->frame->kva;
	file_seek(file_page->pending_file, file_page->offset);
	if (file_read (file_page->pending_file, kpage, file_page->length) != (int) file_page->length) {
		return false;
	}
	free(file_page_aux);
	return true;
}

/* Do the mmap */
void *
do_mmap (void *addr, size_t length, int writable, struct file *file, off_t offset) {
	struct supplemental_page_table *spt = &thread_current ()->spt;
	struct page *page = spt_find_page(spt, addr);
	if (page) {
		return NULL;
	}

	void *cur_addr = addr;
	file_seek(file, offset);
	while (length > 0) {
		size_t read_bytes = length > PGSIZE ? PGSIZE : length;
		struct file_page *fp = malloc(sizeof (struct file_page));
		fp->pending_file = file;
		fp->offset = offset;
		fp->length = read_bytes;
		fp->has_next = length > PGSIZE;
		if (!vm_alloc_page_with_initializer(VM_FILE, cur_addr, writable, lazy_load_file, fp)) {
			return NULL;
		}
		cur_addr += PGSIZE;
		length -= read_bytes;
		offset += read_bytes;
	}
	return addr;
}

/* Do the munmap */
void
do_munmap (void *addr) {
	struct thread *curr = thread_current();
	struct page *page = spt_find_page(&curr->spt, addr);
	if (!page) {
		return;
	}
	bool has_next;
	do {
		has_next = page->file.has_next;
		spt_remove_page(&curr->spt, page);
		addr += PGSIZE;
	} while (has_next && (page = spt_find_page(&curr->spt, addr)));
}
