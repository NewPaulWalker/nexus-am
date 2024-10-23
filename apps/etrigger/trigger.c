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
#define TDATA1_TYPE 5ULL // etrigger
#define ETRIGGER_U (1<<6)

void setTrigger(){
    asm volatile(
        "csrw tselect, 1\n\t"
        "csrw tdata1, %0\n\t"
        "csrw tdata2, %1\n\t"
        "csrs medeleg, 4\n\t"
        :
        :"r"((TDATA1_TYPE << (__riscv_xlen - 4)) | ETRIGGER_U), "r"(4)
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
                    "csrr %1, scause\n\t"
                    "csrr %2, mepc\n\t"
                    "csrr %3, sepc\n\t"
                    :"=r"(mcause), "=r"(scause), "=r"(mepc), "=r"(sepc)
                );
    printf("mcause: %ld, mepc: 0x%lx\n", mcause, mepc);
    printf("scause: %ld, sepc: 0x%lx\n", scause, sepc);

    asm volatile(
    "mret"
    );
}

__attribute__((aligned(4))) void __am_asm_strap(void){
    asm volatile(
    "csrr t0, sepc\n\t"
    "addi t0, t0, 4\n\t"
    "csrw sepc, t0\n\t"
    "sret"
    );
}

__attribute__((aligned(4))) void __am_asm_ucode(void){
asm volatile(
    "li t1, 0\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "csrw mstatus,0\n\t"
    "li t1, 0\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
    "addi t1, t1, 1\n\t"
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
        "mret"
        :
        : "r"(__am_asm_mtrap), "r"(__am_asm_strap), "r"(__am_asm_ucode), "r"(0x1800)
        );    

    return 0;
}