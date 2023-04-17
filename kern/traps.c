#include <env.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>

extern void handle_int(void);
extern void handle_tlb(void);
extern void handle_sys(void);
extern void handle_mod(void);
extern void handle_reserved(void);
extern void handle_ov(void);

void (*exception_handlers[32])(void) = {
    [0 ... 31] = handle_reserved,
    [0] = handle_int,
    [2 ... 3] = handle_tlb,
    [12] = handle_ov,
#if !defined(LAB) || LAB >= 4
    [1] = handle_mod,
    [8] = handle_sys,
#endif
};

/* Overview:
 *   The fallback handler when an unknown exception code is encountered.
 *   'genex.S' wraps this function in 'handle_reserved'.
 */
void do_reserved(struct Trapframe *tf) {
	print_tf(tf);
	panic("Unknown ExcCode %2d", (tf->cp0_cause >> 2) & 0x1f);
}

void do_ov(struct Trapframe *tf) {
	curenv->env_ov_cnt++;
	u_long va = tf->cp0_epc;
	struct Page *pp;
        pp = page_lookup(curenv->env_pgdir, va, NULL);
	panic_on(pp == NULL);
//	u_long paddress = page2pa(pp) | (va & 0xfff);
//	u_long new_va = KADDR(paddress);
        u_long new_va = page2kva(pp) | (va & 0xfff);
        u_int *pt = (u_int *)new_va;
	u_int name = *pt;
	if((name >> 26) == 0 &&((name << 21)>>21) == 0x20){
		*pt = *pt + 1;
		 printk("add ov handled\n");
	}else if((name >> 26) == 0 && ((name << 21)>>21) == 0x22){
		*pt = *pt + 1;
		printk("sub ov handled\n");
	}else if((name >> 26) == 0x08){
		u_long s, t, imm;
		s = ((name << 6)>>6)>>21;
		t = ((name << 11)>>11)>>16;
		imm = (name << 16) >> 16;
		tf->regs[t] = (u_long)((u_long)(tf->regs[s] / (u_long)(2)) + (u_long)(imm / (u_long)(2)));
		tf->cp0_epc += 4;
		printk("addi ov handled\n");
	}
}
