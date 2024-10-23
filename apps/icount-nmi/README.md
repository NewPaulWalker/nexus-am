
## icount

该test由U模式下执行5条指令触发BP，且最后一条指令执行后有一个NMI中断

其会先去到MN模式处理NMI，返回到U模式后，然后再trap到M模式处理BP
