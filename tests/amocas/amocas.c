/*
** It may be useless because it may be a wrong test invloved with the function call and asm inlined.
*/

#include <klib.h>
#include <stdint.h>
#define __riscv_xlen 64
#define PMP_NAPOT 0x18
#define PMP_NA4   0x10
#define PMP_TOR   0x08
#define PMP_R     0x01
#define PMP_W     0x02
#define PMP_X     0x04

void init_pmp(){
    asm volatile(
        "csrw pmpaddr1, %0\n\t"
        "csrw pmpaddr2, %1\n\t"
        "csrw pmpcfg0, %2\n\t"
        :
        :"r"(0x81000008ULL >> 2), "r"(0x81000010ULL >> 2), "r"((PMP_TOR | PMP_R | PMP_X) << 16)
    );

    asm volatile(
        "csrw pmpaddr8, %0\n\t"
        "csrw pmpcfg2, %1\n\t"
        :
        :"r"((1ULL << (53)) - 1), "r"(PMP_NAPOT | PMP_R | PMP_W | PMP_X)
    );

    asm volatile(
        "li t0, 0x81000000\n\t"
        "li t1, 0\n\t"
        "sd t1, 0(t0)\n\t"
        "sd t1, 8(t0)\n\t"
    );
}

/*
** The value in the BASE field of mtvec 
** must always be aligned on a 4-byte boundary
** aligned(4) means aligned on a 4-byte boundary
** not aligned on a 2^4 byte boundary
*/
__attribute__((aligned(4))) void __am_asm_mtrap(void){

    uint64_t mcause, mepc;
    asm volatile( 
                    "csrr %0, mcause\n\t"
                    "csrr %1, mepc\n\t"
                    :"=r"(mcause), "=r"(mepc)
                );
    printf("mcause: %ld, mepc: 0x%lx\n", mcause, mepc);

    asm volatile(
    "nop\n\t"
    "csrw mcause, 0\n\t"
    "csrr a0, mepc\n\t"
    "addi a0, a0, 4\n\t"
    "csrw mepc, a0\n\t"
    "mret"
    );
}

__attribute__((aligned(4))) void __am_asm_ucode(void){
    asm volatile(
        "li t0, 0x81000008\n\t"
        "li a0, 0\n\t"
        "li a1, 5\n\t"
        ".insn r 0x2f, 0x3, 0x14, a0, t0, a1\n\t"
    );

    asm volatile(
        "li t0, 0x81000008\n\t"
        "li a0, 1\n\t"
        "li a1, 5\n\t"
        ".insn r 0x2f, 0x3, 0x14, a0, t0, a1\n\t"
    );

    asm volatile(
        "li t0, 0x81000000\n\t"
        "li a2, 0\n\t"
        "li a3, 0\n\t"
        ".insn r 0x2f, 0x4, 0x14, a2, t0, a0\n\t"
    );
    asm volatile("mv a0, %0; .word 0x0000006b" : :"r"(0));
}

int main() {
    init_pmp();
    asm volatile(
        "csrw mtvec, %0\n\t"
        "csrw mepc, %1\n\t"
        "csrc mstatus, %2\n\t"
        "mret"
        :
        : "r"(__am_asm_mtrap), "r"(__am_asm_ucode), "r"(0x1800)
        );    

    return 0;
}
