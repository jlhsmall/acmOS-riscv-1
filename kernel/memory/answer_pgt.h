void enable_paging() {
    // TODO: Homework 2: Enable paging
    // After initializing the page table, write to register SATP register for kernel registers.
    // Flush the TLB to invalidate the existing TLB Entries
    // Suggested: 2 LoCs
    w_satp(MAKE_SATP(kernel_pagetable));
    flush_tlb();
}

// Return the address of the PTE in page table *pagetable*
// The Risc-v Sv48 scheme has four levels of page table.
// For VA:
//   47...63 zero
//   39...47 -- 9  bits of level-3 index
//   30...38 -- 9  bits of level-2 index
//   21...29 -- 9  bits of level-1 index
//   12...20 -- 9  bits of level-0 index
//   0...11  -- 12 bits of byte offset within the page
// Return the last-level page table entry.
static pte_t* pt_query(pagetable_t pagetable, vaddr_t va, int alloc){
    if(va >= MAXVA) BUG_FMT("get va[0x%lx] >= MAXVA[0x%lx]", va, MAXVA);
    //printk("%lx %lx\n",pagetable,va);
    // Suggested: 18 LoCs
    pagetable_t p3,p2,p1;
    pte_t* e3=(pte_t*)(pagetable+PX(3,va));
    if(PTE_FLAGS(*e3)&PTE_V)
        p3=(pagetable_t)PTE2PA(*e3);
    else if(!alloc)
        return NULL;
    else {
        p3 = (pagetable_t) mm_kalloc();
        memset(p3, 0, BD_LEAF_SIZE);
        *e3= PA2PTE(p3)|PTE_V;
    }
    //printk("%lx %lx %lx\n",p3,e3,*e3);
    pte_t* e2=(pte_t*)(p3+PX(2,va));
    if(PTE_FLAGS(*e2)&PTE_V)
        p2=(pagetable_t)PTE2PA(*e2);
    else if(!alloc)
        return NULL;
    else {
        p2 = (pagetable_t) mm_kalloc();
        memset(p2, 0, BD_LEAF_SIZE);
        *e2= PA2PTE(p2)|PTE_V;
    }

    //printk("%lx %lx %lx %lx\n",p2,e2,*e2,PX(1,va));
    pte_t* e1=(pte_t*)(p2+PX(1,va));
    if(PTE_FLAGS(*e1)&PTE_V)
        p1=(pagetable_t)PTE2PA(*e1);
    else if(!alloc)
        return NULL;
    else {
        p1 = (pagetable_t) mm_kalloc();
        memset(p1, 0, BD_LEAF_SIZE);
        *e1= PA2PTE(p1)|PTE_V;
    }

    //printk("%lx %lx %lx\n",p1,e1,*e1);
    pte_t* e0=(pte_t*)(p1+PX(0,va));
    /*if(PTE_FLAGS(*e0)&PTE_V)
        p0=(pagetable_t)PTE2PA(*e0);
    else if(!alloc)
        return NULL;
    else {
        p0 = (pagetable_t) mm_kalloc();
        memset(p0, 0, BD_LEAF_SIZE);
        *e0= PA2PTE(p0)|PTE_V;
    }*/

    //printk("%lx %lx %lx\n",p0,e0,*e0);
    //return (pte_t*)((uint64)p0+(va&0xFFF));
    return e0;
}
int pt_map_pages(pagetable_t pagetable, vaddr_t va, paddr_t pa, uint64 size, int perm){
    // Suggested: 11 LoCs
    perm|=PTE_V;
    uint64 cnt= PGROUNDUP(size)/PGSIZE;
    va= PGROUNDDOWN(va);
    while(cnt--){
        pte_t *e=pt_query(pagetable,va,1);
        *e= PA2PTE(pa)|perm;
        va+=PGSIZE;
        pa+=PGSIZE;
    }
    return 0; // Do not modify
}

paddr_t pt_query_address(pagetable_t pagetable, vaddr_t va){
    // Suggested: 3 LoCs

    pte_t *e=pt_query(pagetable, va, 1);
    if(e==NULL||(!((*e)&PTE_V))) return NULL;
    return PTE2PA(*e)+(va&0xFFF);
}

int pt_unmap_addrs(pagetable_t pagetable, vaddr_t va){
    // Suggested: 2 LoCs
    pte_t *e=pt_query(pagetable, va, 0);
    if(e!=NULL){
        *e^=PTE_V;
        mm_kfree(PTE2PA(*e));
    }
    return 0; // Do not modify
}

int pt_map_addrs(pagetable_t pagetable, vaddr_t va, paddr_t pa, int perm){
    // Suggested: 2 LoCs
    pte_t *e=pt_query(pagetable, va, 1);
    *e= PA2PTE(pa)|perm;
    return 0; // Do not modify
}
