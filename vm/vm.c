/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "threads/mmu.h"

bool page_less (const struct hash_elem *a_, const struct hash_elem *b_ , void *aux UNUSED);
unsigned page_hash (const struct hash_elem *p_ , void *aux UNUSED);
struct page *page_lookup (const void *va, struct hash *pages_);

struct list frame_table;
/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	// 위 코드를 변경하지 마시오.
	/* TODO: Your code goes here. */
	// 당신의 코드는 여기에 적으시오.
	list_init(&frame_table);
}



// 페이지의 타입을 얻습니다. 이 함수는 페이지가 initialized 이후 어떠한 타입을 갖는지 알아내는데 유용합니다.
// 이 함수는 완전하게 구현되었습니다.

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}
// 도움을 주는 함수들
/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);


// initializer로 페이지를 만드십시오. 직접 페이지를 만들지 마시고 
// 이 함수나 vm_alloc_page를 통해서 만드십시오.

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */

		/* TODO: Insert the page into the spt. */
	}
err:
	return false;
}
// spt에서 VA를 찾고 page를 return 하십시오. error가 발생하면 NULL을 return 하십시오.

/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	struct page *page = page_lookup(va, &spt->pages);
	return page;
}

// PAGE를 확인하고 spt에 삽입하십시오.

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	int succ = false;
	/* TODO: Fill this function. */
	struct page *result = hash_insert(&spt->pages, &page->hash_elem);
	if(result != page)
		succ = true;
	return succ;
}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

// struct frame을 확보하십시오. 그것은 evicted 될 것입니다.

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */

	return victim;
}

// 한 페이지를 evict 하고 일치하는 frame을 return 합니다.
// error 발생시 NULL을 return 합니다.

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */

	return NULL;
}

// palloc() 하고 frame을 얻습니다. 만약 이용가능한 페이지가 없다면 페이지를 evict하고 그 페이지를 return합니다.
// 이 함수는 항상 정확한 주소를 return 합니다. 그럼으로 , 만약 user pool memory가 가득 차게되면
// 이 함수는 frame을 이용가능한 메모리 공간을 얻기위해  evict합니다.

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame (void) {
	struct frame *frame = malloc(sizeof frame);
	frame->kva = palloc_get_page(PAL_USER | PAL_ZERO);
	// NOTE: 만약 페이지가 2개 이상 필요한 작업이면? -> 한번 pop해서는 부족하다. while 문으로 될 때까지 해야되는 것이 아닌가?
	if (frame == NULL || frame->kva == NULL)
		list_pop_front(&frame_table);
	else
		list_push_back(&frame_table, &frame->frame_elem);

	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);
	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
}

// write_protected 페이지의 fault를 핸들링합니다.

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */

	return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

// VA에 할당된 페이지를 요청합니다.
/* Claim the page that allocate on VA. */
bool
vm_claim_page (void *va) {
	// spt에서 va를 통해 물리 페이지랑 연결한 페이지를 찾는다.
	// 해당 페이지를 vm_do_claim_page에 전달한다.
	struct page *page = spt_find_page(&thread_current()->spt, va);
	return vm_do_claim_page (page);
}

// PAGE를 요청하고 MMU에 set up 합니다.
/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	ASSERT (page != NULL);
	if (page->frame != NULL)
		return false;

	// 전달받은 페이지가 이미 프레임을 할당받았는지 확인
	struct frame *frame = vm_get_frame ();
	frame->page = page;
	page->frame = frame;

	// 유저 페이지가 페이지 테이블에 정상적으로 들어갔는지 확인(= 커널 가상 메모리가 반환되는지)
	if (!pml4_set_page(thread_current()->pml4, page->va, frame->kva, true))
		return false;
	return swap_in (page, frame->kva);
}

// 새로운 보조 페이지 테이블을 초기화합니다.

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	hash_init(&spt->pages, page_hash, page_less, NULL);
}

// src의 보조 페이지 테이블을  dst로 복사합니다.

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) {
} 
// 보조 페이지 테이블에 의해 hold된 자원을 Free합니다.

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
}


unsigned
page_hash (const struct hash_elem *p_ , void *aux UNUSED) {
	const struct page *p = hash_entry(p_, struct page, hash_elem);
	return hash_bytes (&p->va, sizeof p->va);
}

bool
page_less (const struct hash_elem *a_, const struct hash_elem *b_ , void *aux UNUSED) {
	const struct page *a = hash_entry (a_, struct page, hash_elem);
	const struct page *b = hash_entry (b_, struct page, hash_elem);

	return a->va < b->va;
}

struct page *
page_lookup (const void *va, struct hash *pages_) {
	struct page p;
	struct hash_elem *e;
	struct hash *pages = pages_;

	p.va = va;
	e = hash_find (pages, &p.hash_elem);
	return e != NULL ? hash_entry (e, struct page, hash_elem) : NULL;
}