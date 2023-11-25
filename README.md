# Lab1实验报告

## 思考题

### 1、请阅读附录中的编译链接详解，尝试分别使用实验环境中的原生 x86 工具 链（gcc、ld、readelf、objdump 等）和 MIPS 交叉编译工具链（带有 mips-linux-gnu前缀），重复其中的编译和解析过程，观察相应的结果，并解释其中向 objdump 传入的参 数的含义。

```C
  1 #include<stdio.h>    //测试用例hello.c                                                                                                          
  2 
  3 int main(){
  4         printf("Hello World!");
  5         return 0;
  6 }
```

* 首先使用gcc仅执行预处理指令：

  ```bash
  ~/test $ gcc -E hello.c > hello_pre.txt
  ```

​       观察到预处理器将头文件添加到了我们的源文件当中。

* 然后我们使用gcc执行仅编译而不链接指令，将产生的目标文件命名为hello.o：

  ```bash
  ~/test $ gcc -c hello.c -o hello.o
  ```

* 再使用objdump进行反汇编，将反汇编结果导入o_reverse.txt中

  ```bash
  ~/test $ objdump -DS hello.o > o_reverse.txt
  ```

  观察到main函数部分如下：

  ```
    1 
    2 hello.o：     文件格式 elf64-x86-64
    3 
    4 
    5 Disassembly of section .text:
    6 
    7 0000000000000000 <main>:
    8    0:   f3 0f 1e fa             endbr64 
    9    4:   55                      push   %rbp
   10    5:   48 89 e5                mov    %rsp,%rbp
   11    8:   48 8d 05 00 00 00 00    lea    0x0(%rip),%rax        # f <main+0xf>
   12    f:   48 89 c7                mov    %rax,%rdi
   13   12:   b8 00 00 00 00          mov    $0x0,%eax
   14   17:   e8 00 00 00 00          call   1c <main+0x1c>
   15   1c:   b8 00 00 00 00          mov    $0x0,%eax
   16   21:   5d                      pop    %rbp
   17   22:   c3                      ret
  ```

  观察call的部分，发现此时printf函数的位置仍然没有被填入我们的程序中。

* 接下来，我们允许gcc进行链接，生成可执行文件，然后再使用objdump进行反汇编。

  ```bash
  ~/test $ gcc -o hello hello.c
  ~/test $ objdump -DS hello > hello_reverse.txt
  ```

  我们可以看到反汇编结果中main函数的部分：

  ```
  448 0000000000001149 <main>:
  449     1149:       f3 0f 1e fa             endbr64 
  450     114d:       55                      push   %rbp
  451     114e:       48 89 e5                mov    %rsp,%rbp
  452     1151:       48 8d 05 ac 0e 00 00    lea    0xeac(%rip),%rax        # 2004  <_IO_stdin_used+0x4>
  453     1158:       48 89 c7                mov    %rax,%rdi
  454     115b:       b8 00 00 00 00          mov    $0x0,%eax
  455     1160:       e8 eb fe ff ff          call   1050 <printf@plt>
  456     1165:       b8 00 00 00 00          mov    $0x0,%eax
  457     116a:       5d                      pop    %rbp
  458     116b:       c3                      ret    
  ```

  其中printf处已经不再是0，而是填入了标记为printf@plt的位置，说明printf函数是在链接这一步才被插入到最终的可执行文件中的。

* 采用mips-linux-gnu交叉编译工具链重复上述过程

  得到目标文件中main函数部分如下：

  ```
    1                                                                                                                                
    2 hello1.o：     文件格式 elf32-tradbigmips
    3 
    4 
    5 Disassembly of section .text:
    6 
    7 00000000 <main>:
    8    0:   27bdffe0        addiu   sp,sp,-32
    9    4:   afbf001c        sw      ra,28(sp)
   10    8:   afbe0018        sw      s8,24(sp)
   11    c:   03a0f025        move    s8,sp
   12   10:   3c1c0000        lui     gp,0x0
   13   14:   279c0000        addiu   gp,gp,0
   14   18:   afbc0010        sw      gp,16(sp)
   15   1c:   3c020000        lui     v0,0x0
   16   20:   24440000        addiu   a0,v0,0
   17   24:   8f820000        lw      v0,0(gp)
   18   28:   0040c825        move    t9,v0
   19   2c:   0320f809        jalr    t9
   20   30:   00000000        nop
   21   34:   8fdc0010        lw      gp,16(s8)
   22   38:   00001025        move    v0,zero
   23   3c:   03c0e825        move    sp,s8
   24   40:   8fbf001c        lw      ra,28(sp)
   25   44:   8fbe0018        lw      s8,24(sp)
   26   48:   27bd0020        addiu   sp,sp,32
   27   4c:   03e00008        jr      ra
   28   50:   00000000        nop
   29         ...
  ```

  得到可执行文件中main函数的部分如下：

  ```
  380 004006e0 <main>:
  381   4006e0:       27bdffe0        addiu   sp,sp,-32
  382   4006e4:       afbf001c        sw      ra,28(sp)
  383   4006e8:       afbe0018        sw      s8,24(sp)
  384   4006ec:       03a0f025        move    s8,sp
  385   4006f0:       3c1c0042        lui     gp,0x42
  386   4006f4:       279c9010        addiu   gp,gp,-28656
  387   4006f8:       afbc0010        sw      gp,16(sp)
  388   4006fc:       3c020040        lui     v0,0x40
  389   400700:       24440830        addiu   a0,v0,2096
  390   400704:       8f828030        lw      v0,-32720(gp)
  391   400708:       0040c825        move    t9,v0
  392   40070c:       0320f809        jalr    t9
  393   400710:       00000000        nop
  394   400714:       8fdc0010        lw      gp,16(s8)
  395   400718:       00001025        move    v0,zero
  396   40071c:       03c0e825        move    sp,s8
  397   400720:       8fbf001c        lw      ra,28(sp)
  398   400724:       8fbe0018        lw      s8,24(sp)
  399   400728:       27bd0020        addiu   sp,sp,32
  400   40072c:       03e00008        jr      ra
  401   400730:       00000000        nop                                                                                            
  402         ...
  ```

  可以看到其中 `390   400704:       8f828030        lw      v0,-32720(gp)` 这一句也出现了填入printf函数地址的操作，与之间的分析一致。

* **向objdump**传入参数的含义：

  > * `objdump -d <file(s)>`: 将代码段反汇编；
  >
  > - `objdump -S <file(s)>`: 将代码段反汇编的同时，将反汇编代码与源代码交替显示，编译时需要使用`-g`参数，即需要调试信息；
  > - `objdump -C <file(s)>`: 将C++符号名逆向解析
  > - `objdump -l <file(s)>`: 反汇编代码中插入文件名和行号
  > - `objdump -j section <file(s)>`: 仅反汇编指定的section

### 2、思考以下问题：

### •  尝试使用我们编写的 readelf 程序，解析之前在 target 目录下生成的内核 ELF 文 件。 

### • 也许你会发现我们编写的 readelf 程序是不能解析 readelf 文件本身的，而我们刚 才介绍的系统工具 readelf 则可以解析，这是为什么呢？（提示：尝试使用 readelf -h，并阅读 tools/readelf 目录下的 Makefile，观察 readelf 与 hello 的不同）

* 使用我们编写的readelf程序，解析mos文件得到以下内容：

  ```
  0:0x0
  1:0x80010000
  2:0x80015000
  3:0x80015018
  4:0x80015030
  5:0x80015ca0
  6:0x80015cb0
  7:0x0
  8:0x0
  9:0x0
  10:0x0
  11:0x0
  12:0x0
  13:0x0
  14:0x0
  15:0x0
  16:0x0
  17:0x0
  18:0x0
  ```

* 对readelf和hello使用readelf -h的结果如下：

  ```
  #readelf
  ELF 头：
    Magic：   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
    类别:                              ELF64
    数据:                              2 补码，小端序 (little endian)
    Version:                           1 (current)
    OS/ABI:                            UNIX - System V
    ABI 版本:                          0
    类型:                              DYN (Position-Independent Executable file)
    系统架构:                          Advanced Micro Devices X86-64
    版本:                              0x1
    入口点地址：               0x1180
    程序头起点：          64 (bytes into file)
    Start of section headers:          14488 (bytes into file)
    标志：             0x0
    Size of this header:               64 (bytes)
    Size of program headers:           56 (bytes)
    Number of program headers:         13
    Size of section headers:           64 (bytes)
    Number of section headers:         31
    Section header string table index: 30
  ```

  ```
  #hello
  ELF 头：
    Magic：   7f 45 4c 46 01 01 01 03 00 00 00 00 00 00 00 00 
    类别:                              ELF32
    数据:                              2 补码，小端序 (little endian)
    Version:                           1 (current)
    OS/ABI:                            UNIX - GNU
    ABI 版本:                          0
    类型:                              EXEC (可执行文件)
    系统架构:                          Intel 80386
    版本:                              0x1
    入口点地址：               0x8049600
    程序头起点：          52 (bytes into file)
    Start of section headers:          746252 (bytes into file)
    标志：             0x0
    Size of this header:               52 (bytes)
    Size of program headers:           32 (bytes)
    Number of program headers:         8
    Size of section headers:           40 (bytes)
    Number of section headers:         35
    Section header string table index: 34
  ```

  对比两者可以看到其类别、类型、系统架构等均有所不同，所以我们所写的readelf能解析hello但不能解析自身。除此之外，在makefile中我们还可以看到readelf的编译依赖于readelf.o和main.o，而hello的编译仅依赖于main.c，而且采用了静态链接，这也是造成两者不同的原因。

### 3、在理论课上我们了解到，MIPS 体系结构上电时，启动入口地址为 0xBFC00000 （其实启动入口地址是根据具体型号而定的，由硬件逻辑确定，也有可能不是这个地址，但 一定是一个确定的地址），但实验操作系统的内核入口并没有放在上电启动地址，而是按照 内存布局图放置。思考为什么这样放置内核还能保证内核入口被正确跳转到？ （提示：思考实验中启动过程的两阶段分别由谁执行。） 

```
因为启动过程的第一阶段由硬件执行，因此必须要有一个上电启动地址，而我们的MOS操作系统是在Gxemul仿真器上运行的，他已经提供了bootloader的引导功能，直接支持加载ELF格式的内核，因此我们只需要按照内存布局图来放置内核即可。
```

## 难点分析

本次实验主要目的是完成内核的启动以及printk函数的实现，难度并不是很大。

难点主要在于掌握ELF的文件结构，了解段头表和节头表的相关概念。

* ELF的文件结构如下图：

  ![image-20230318125949301](C:\Users\King.xx\AppData\Roaming\Typora\typora-user-images\image-20230318125949301.png)

* ELF头的结构如下：

  ```C
  1
  2 /* 文件的前面是各种变量类型定义，在此省略 */
  3 /* The ELF file header. This appears at the start of every ELF file. */
  4 /* ELF 文件的文件头。所有的 ELF 文件均以此为起始 */
  5 #define EI_NIDENT (16)
  6
  7 typedef struct {
  8 unsigned char e_ident[EI_NIDENT]; /* Magic number and other info */
  9 // 存放魔数以及其他信息
  10 Elf32_Half e_type; /* Object file type */
  11 // 文件类型
  12 Elf32_Half e_machine; /* Architecture */
  13 // 机器架构
  14 Elf32_Word e_version; /* Object file version */
  15 // 文件版本
  16 Elf32_Addr e_entry; /* Entry point virtual address */
  17 // 入口点的虚拟地址
  18 Elf32_Off e_phoff; /* Program header table file offset */
  19 // 程序头表所在处与此文件头的偏移
  20 Elf32_Off e_shoff; /* Section header table file offset */
  21 // 节头表所在处与此文件头的偏移
  22 Elf32_Word e_flags; /* Processor-specific flags */
  23 // 针对处理器的标记
  24 Elf32_Half e_ehsize; /* ELF header size in bytes */
  25 // ELF 文件头的大小（单位为字节）
  26 Elf32_Half e_phentsize; /* Program header table entry size */
  27 // 程序头表表项大小
  28 Elf32_Half e_phnum; /* Program header table entry count */
  29 // 程序头表表项数
  30 Elf32_Half e_shentsize; /* Section header table entry size */
  31 // 节头表表项大小
  32 Elf32_Half e_shnum; /* Section header table entry count */
  33 // 节头表表项数
  34 Elf32_Half e_shstrndx; /* Section header string table index */
  35 // 节头字符串编号
  36 } Elf32_Ehdr;
  ```

* 节头表和段头表的结构如下：

  ```C
  38 typedef struct {
  39 // section name
  40 Elf32_Word sh_name;
  41 // section type
  42 Elf32_Word sh_type;
  43 // section flags
  44 Elf32_Word sh_flags;
  45 // section addr
  46 Elf32_Addr sh_addr;
  47 // offset from elf head of this entry
  48 Elf32_Off sh_offset;
  49 // byte size of this section
  50 Elf32_Word sh_size;
  51 // link
  52 Elf32_Word sh_link;
  53 // extra info
  54 Elf32_Word sh_info;
  55 // alignment
  56 Elf32_Word sh_addralign;
  57 // entry size
  58 Elf32_Word sh_entsize;
  59 } Elf32_Shdr;
  60
  61 typedef struct {
  62 // segment type
  63 Elf32_Word p_type;
  64 // offset from elf file head of this entry
  65 Elf32_Off p_offset;
  66 // virtual addr of this segment
  67 Elf32_Addr p_vaddr;
  68 // physical addr, in linux, this value is meanless and has same value of p_vaddr
  69 Elf32_Addr p_paddr;
  70 // file size of this segment
  71 Elf32_Word p_filesz;
  72 // memory size of this segment
  73 Elf32_Word p_memsz;
  74 // segment flag
  75 Elf32_Word p_flags;
  76 // alignment
  77 Elf32_Word p_align;
  78 } Elf32_Phdr;
  ```

  本单元中我们需要掌握从ELF头中根据偏移量寻找段头表和节头表的地址，进而寻找每个段和每一节的方法。

第二个难点在于根据内存布局图寻找相应的地址空间：

* 内存布局图如下：

  ![image-20230318130926328](C:\Users\King.xx\AppData\Roaming\Typora\typora-user-images\image-20230318130926328.png)

## 心得体会

本次实验，在指导书的引导下，我成功完成了MOS系统的启动过程以及printk函数的设计。其中，系统启动的过程完成较为顺利，但是printk函数完成过程中却遇到了一些小小的问题。主要的问题是没有考虑局部变量的初始化，导致好几个格式化输出的flag都没能正确复制，并且在修复了一个类似问题之后没有吃一堑长一智，在其他flag的地方再次出现了该问题，严重影响了整体进度。这个过程给我的教训便是在找bug的过程中一定要多汲取以前的教训，不能修完bug之后就抛之脑后，下次该怎样还是怎样。

总体来说，这次实验难度并不算大，但是确实一个关键的开头。在接下来的实验中，更应该认真地去领悟操作系统中每一部分的精髓，争取在每次实验中都对操作系统的理解更深一步。

