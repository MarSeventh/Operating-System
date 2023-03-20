void sprintf_1_check() {
	char str[100];
	printk("%d\n",sprintf(str, "%s\n", "abd"));
	printk("%s", str);
	sprintf(str, "%c\n", 97);
	printk("%s", str);
	sprintf(str, "%d\n",1114514);
	printk("%s", str);
}

void mips_init() {
	sprintf_1_check();
	halt();
}
