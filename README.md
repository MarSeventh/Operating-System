# Lab3实验报告

## 一、思考题

### 1、请结合 MOS 中的页目录自映射应用解释代码中 e->env_pgdir[PDX(UVPT)] = PADDR(e->env_pgdir) | PTE_V 的含义。

```text
要求将e->env_pgdir自身的物理地址映射到UVPT的位置，由于UVPT这个地址在页目录的第(UVPT >> 10)项，即第PDX(UVPT)项，而e->env_pgdir这个页表自身的物理地址可以通过PADDR(e->env_pgdir)来获得，同时我们要使页目录的这一项有效，因此将PADDR(e->env_pgdir)|PTE_V存入e->env_pgdir[PDX(UVPT)]中即可。
```

### 2、elf_load_seg 以函数指针的形式，接受外部自定义的回调函数 map_page。 请你找到与之相关的 data 这一参数在此处的来源，并思考它的作用。没有这个参数可不可 以？为什么？

```text
data在此处的来源为load_icode,data即传入load_icode的参数struct Env* e;
它的作用是在回调函数中使用page_insert时传入对应Env的页目录地址pgdir以及进程标志asid。
没有这个参数不可以，如果没有这个参数就无法满足后续回调函数所需要的参数，page_insert函数无法正确分配物理页面。
```

### 3、结合 elf_load_seg 的参数和实现，考虑该函数需要处理哪些页面加载的情 况。

```
1.需加载到的虚拟地址va不是页对齐的
2.拷贝数据长度bin_size不是页对齐的
3.拷贝文件大小bin_size小于占据内存长度sg_size
4.占据内存长度sgsize不是页对齐的
```

### 4、思考上面这一段话，并根据自己在 Lab2 中的理解，回答：

###  • 你认为这里的 env_tf.cp0_epc 存储的是物理地址还是虚拟地址?

```
上面的话是指：这里的 env_tf.cp0_epc 字段指示了进程恢复运行时 PC 应恢复到的位置。我们要运行的进
程的代码段预先被载入到了内存中，且程序入口为 e_entry，当我们运行进程时，CPU 将自动
从 PC 所指的位置开始执行二进制码。

我认为，应该存储的是虚拟地址，在重新执行时依然会重新寻址再执行。
```

### 5、试找出 0、1、2、3 号异常处理函数的具体实现位置。8 号异常（系统调用） 涉及的 do_syscall() 函数将在 Lab4 中实现。 

```
0、1、2、3号异常都是在kern/genex.S中实现的
```

### 6、阅读 init.c、kclock.S、env_asm.S 和 genex.S 这几个文件，并尝试说出 enable_irq 和 timer_irq 中每行汇编代码的作用。

```asm
enable_irq:
        li      t0, (STATUS_CU0 | STATUS_IM4 | STATUS_IEc) //CU0置为1表示允许协处理器0执行任务，IM4置为1表示4号位可中断，IEc置为1表示开启中断
        mtc0    t0, CP0_STATUS  //将上述置位写入CP0_STATUS寄存器中
        jr      ra  //返回原来位置
timer_irq:         
        sw      zero, (KSEG1 | DEV_RTC_ADDRESS | DEV_RTC_INTERRUPT_ACK) //将GXemul的DEV_RTC_INTERRUPT_ACK寄存器位置0，清除RTC中断标志位，禁止未来的中断   
        li      a0, 0 //将参数a0置为0
        j       schedule      //跳转执行schedule调度函数 
```

### 7、阅读相关代码，思考操作系统是怎么根据时钟中断切换进程的。

```
每次发生时钟中断时都会执行一次schedule函数，使得进程的时间片计数count减去1，当count等于0时便进行进程的切换，从而实现了根据时钟中断切换进程。
```

## 二、实验难点

在Lab3中，我们实现了进程的创建、时钟中断的产生以及根据时钟中断进行进程调度三大任务，本次实验在MOS中的概况如下图所示：

![image-20230413101109248](C:\Users\King.xx\AppData\Roaming\Typora\typora-user-images\image-20230413101109248.png)

回顾整个Lab3实验，可以说每一部分都有一些不易理解的点。

> 在**进程创建**部分，第一个难点就是如何构造进程控制块PCB，因为PCB中需要包含一个进程运行的所有信息，如进程标识，上下文状态、页目录地址等内容，所以PCB的结构直接决定了进程的相关功能能否正常运作。课程组对该问题的实现是构造了一个Env结构体，其结构如下：
>
> ```C
> 1 struct Env {
> 2 struct Trapframe env_tf; // saved context (registers) before switching
> 3 LIST_ENTRY(Env) env_link; // intrusive entry in 'env_free_list'
> 4 u_int env_id; // unique environment identifier
> 5 u_int env_parent_id; // env_id of this env's parent
> 6 u_int env_status; // status of this env
> 7 Pde *env_pgdir; // page directory
> 8 TAILQ_ENTRY(Env) env_sched_link; // intrusive entry in 'env_sched_list'
> 9 u_int env_pri; // schedule priority
> 10 };
> ```
>
> 

> 第二个难点在于使用env_setup_vm函数初始化新进程的地址空间。这个函数的作用是为进程的页目录分配地址空间，然后将UTOP到UVPT这一部分内核数据对应的内核页表base_pgdir拷贝到进程页表中，最后再把自己的页表放在UVPT以上的位置。实现该函数要求我们对页表的映射规则以及MOS的内存布局有一个较为透彻的理解。

> 第三个难点在于如何将进程的程序段加载到对应的虚拟地址，也即load_icode函数的实现，该函数通过调用elf_from函数完成ELF文件头的解析，再调用elf_load_seg函数将 ELF 文件的一个 segment加载到内存，其中需要注意elf_load_seg函数需要判断将要加载到的虚拟地址va、该段占据的内存长度sg_size和需要拷贝的数据长度bin_size等是否是页对齐的，如果不是页对齐的需要将其处理后再调用回调函数load_icode_mapper完成加载。



> 在**时钟中断产生**部分，难点在于如何响应时钟中断，这一共分为三个步骤：
>
> 1 通过异常分发，判断出当前异常为中断异常，随后进入相应的中断处理程序。在 MOS 中 即对应 handle_int 函数。
>
> 2 在中断处理程序中进一步判断 Cause 寄存器中是由几号中断位引发的中断，然后进入不 同中断对应的中断服务函数。
>
> 3 中断处理完成，通过 ret_from_exception 函数恢复现场，继续执行。
>
> 简单来说就是首先要判断中断类型，然后保存现场，跳转执行中断处理程序，最终返回现场继续执行与原程序。



> 在**进程调度**部分，难点则在于调度函数schedule的实现。该函数是进程调度的核心，需要控制在什么时候进行进程的切换。需要切换的情况主要有以下几种：
>
> ```
> 1、进程的可执行次数count减到0
> 2、yield参数被置为1
> 3、没有正在执行的进程
> 4、当前进程被阻塞或退出
> ```
>
> 在这些情况下，我们需要从调度队列里取出一个新的进程执行，并且如果原来的进程仍然可执行，将其重新插入调度队列尾部。具体函数实现如下：
>
> ```C
> 17 void schedule(int yield) {
> 18         static int count = 0; // remaining time slices of current env
> 19         struct Env *e = curenv;
> 20         /* We always decrease the 'count' by 1.
> 21          *
> 22          * If 'yield' is set, or 'count' has been decreased to 0, or 'e' (previous 'curenv') is
> 23          * 'NULL', or 'e' is not runnable, then we pick up a new env from 'env_sched_list' (list of
> 24          * all runnable envs), set 'count' to its priority, and schedule it with 'env_run'. **Panic
> 25          * if that list is empty**.
> 26          *
> 27          * (Note that if 'e' is still a runnable env, we should move it to the tail of
> 28          * 'env_sched_list' before picking up another env from its head, or we will schedule the
> 29          * head env repeatedly.)
> 30          *
> 31          * Otherwise, we simply schedule 'e' again.
> 32          *
> 33          * You may want to use macros below:
> 34          *   'TAILQ_FIRST', 'TAILQ_REMOVE', 'TAILQ_INSERT_TAIL'
> 35          */
> 36         /* Exercise 3.12: Your code here. */
> 37         if(yield || count == 0 || e == NULL || e->env_status != ENV_RUNNABLE){
> 38                 if(e!=NULL && e->env_status == ENV_RUNNABLE){
> 39                         TAILQ_REMOVE(&env_sched_list, e, env_sched_link);
> 40                         TAILQ_INSERT_TAIL(&env_sched_list, e, env_sched_link);
> 41                 }
> 42                 if(TAILQ_EMPTY(&env_sched_list)){
> 43                         panic("schedule: no runnable envs\n");
> 44                 }
> 45                 struct Env* new_e = TAILQ_FIRST(&env_sched_list);
> 46                 count = new_e->env_pri;
> 47                 count--;
> 48                 env_run(new_e);
> 49         }else {
> 50                 count--;
> 51                 env_run(e);
> 52         }
> 53 }               
> ```



## 三、心得体会

Lab3的实验中，我们首次实现了进程的创建、运行以及调度等功能，可以说是从之前奠基的阶段跨越到了真正开始运行操作系统的阶段。这次实验相对于前几次实验还是有着不小的难度，无论是进程控制块的构造、新进程的初始化、程序镜像的加载、进程的运行，还是中断的产生和处理、中断处理函数的设计、上下文环境的保存，抑或是进程调度算法的实现，都有着较大的挑战，需要仔细的分析才能做到真正地理解与实现。

通过这次实验，我也收获了不少东西。例如，我对页表映射等知识的掌握更进了一步，使用也更加熟练。除此之外，还对进程的执行和调度等概念有了更加深刻的理解，知道了进程的所有信息都可以使用PCB块进行保存。

但是，在这次实验中也暴露出了不少的问题，比如随着系统内容越来越多，结构越来越复杂，很多时候难以找到函数之间对应的调用关系，导致对某些功能的实现方式理解的不甚透彻，~~基本处于按照hint写代码的状态~~。除此之外，我觉得自己对于各种地址的转换还没有理解清楚，在某些需要进行物理地址和虚拟地址的转换时经常会不知所措（例如page2pa,pa2page,page2kva,PADDR,KADDR等），接下来需要对这一部分再进行一些仔细的研究。

总而言之，随着实验时间越来越长，可以明显地感觉到实验难度的增长，在接下来的实验中还需要更加仔细、更加努力才行。
