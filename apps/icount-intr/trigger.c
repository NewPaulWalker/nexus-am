/*
** It may be useless because it may be a wrong test invloved with the function call and asm inlined.
*/

#include <klib.h>
#include <stdint.h>
#define __riscv_xlen 64
#define PMP_NAPOT 0x18
#define PMP_R     0x01
#define PMP_W     0x02
#define PMP_X     0x04
#define TDATA1_TYPE 3ULL // icount
#define ICOUNT_U (1<<6)
#define ICOUNT_COUNT_OFFSET 10

void setTrigger(){
    asm volatile(
        "csrw tselect, 1\n\t"
        "csrw tdata1, %0\n\t"
        :
        :"r"((TDATA1_TYPE << (__riscv_xlen - 4)) | ICOUNT_U | (5 << ICOUNT_COUNT_OFFSET))
        :
    );
}

void init_pmp(){
    asm volatile(
        "csrw pmpaddr1, %0\n\t"
        "csrw pmpcfg0, %1\n\t"
        :
        :"r"((1ULL << (31 + (__riscv_xlen / 64) * (53 - 31))) - 1), "r"((PMP_NAPOT | PMP_R | PMP_W | PMP_X) << 8)
    );
}

/*
** The value in the BASE field of mtvec 
** must always be aligned on a 4-byte boundary
** aligned(4) means aligned on a 4-byte boundary
** not aligned on a 2^4 byte boundary
*/
__attribute__((aligned(4))) void __am_asm_mtrap(void){
    uint64_t mcause, mepc, scause, sepc;
    asm volatile( 
                    "csrr %0, mcause\n\t"
                    "csrr %1, mepc\n\t"
                    "csrr %2, scause\n\t"
                    "csrr %3, sepc\n\t"
                    :"=r"(mcause), "=r"(mepc), "=r"(scause), "=r"(sepc)
                );
    printf("mcause: %lx, mepc: 0x%lx\n", mcause, mepc);
    printf("scause: %lx, sepc: 0x%lx\n", scause, sepc);

    asm volatile(
    "csrc mip, 2\n\t"
    "csrr a0, mepc\n\t"
    "add a0, a0, 4\n\t"
    "csrw mepc, a0\n\t"
    "nop\n\t"
    "mret"
    );
}

__attribute__((aligned(4))) void __am_asm_strap(void){
    asm volatile(
    "nop\n\t"
    "ecall\n\t"
    "nop\n\t"
    "sret"
    );
}

__attribute__((aligned(4))) void __am_asm_ucode(void){
asm volatile(
    "li t1, 0\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "add a0, a1, a2\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t" 
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    );
    asm volatile("mv a0, %0; .word 0x0000006b" : :"r"(0));
}

int main() {
    setTrigger();
    init_pmp();
    asm volatile(
        "csrw mtvec, %0\n\t"
        "csrw stvec, %1\n\t"
        "csrw mepc, %2\n\t"
        "csrc mstatus, %3\n\t"
        "csrs mideleg, 2\n\t"
        "csrs mie, 2\n\t"
        "mret"
        :
        : "r"(__am_asm_mtrap), "r"(__am_asm_strap), "r"(__am_asm_ucode), "r"(0x1800)
        );    

    return 0;
}
