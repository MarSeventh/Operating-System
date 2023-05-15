/*
 * operations on IDE disk.
 */

#include "serv.h"
#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>
int map[32];
int ssd[32];
int erase[32];
void ssd_init() {
	for(int i = 0; i < 32; i++) {
		map[i] = -1;
	}
	for(int i = 0; i < 32; i++) {
		ssd[i] = 1;
	}
	for(int i = 0; i < 32; i++) {
		erase[i] = 0;
	}
}

int ssd_read(u_int logic_no, void *dst) {
	if(map[logic_no] == -1) {
		return -1;
	} else {
		ide_read(0, map[logic_no], dst, 1);
		return 0;
	}
}

void py_erase(u_int py_no) {
	char eraser[512];
	memset(eraser, 0, 512);
	ide_write(0, py_no, eraser, 1);
	erase[py_no]++;
	ssd[py_no] = 1;
}

u_int alloc_ssd() {
	int min = -1;
	int minid = -1;
	for(int i = 0; i < 32; i++) {
		if(ssd[i] == 1 && (min == -1 || erase[i] < min)) {
			min = erase[i];
			minid = i;
		}
	}
	if(min >= 5) {
		int no_min = -1;
		int no_minid = -1;
		for(int i = 0; i < 32; i++) {
			if(ssd[i] == 0 && (no_min == -1 || erase[i] < no_min)) {
				no_min = erase[i];
				no_minid = i;
			}
		}
		char temp[512];
		ide_read(0, no_minid, temp, 1);
		ide_write(0, minid, temp, 1);
		ssd[minid] = 0;
		int pre_logic = 0;
		for(int j = 0; j < 32; j++) {
			if(map[j] == no_minid) {
				pre_logic = j;
				break;
			}
		}
		map[pre_logic] = minid;
		py_erase(no_minid);
		minid = no_minid;
	}
	ssd[minid] = 0;
	return minid;
}
void ssd_write(u_int logic_no, void *src) {
	if(map[logic_no] == -1) {
		u_int py_no = alloc_ssd();
		map[logic_no] = py_no;
		ide_write(0, py_no, src, 1);
	} else {
		u_int py_no = map[logic_no];
		py_erase(py_no);
		py_no = alloc_ssd();
		map[logic_no] = py_no;
		ide_write(0, py_no, src, 1);
	}
}

void ssd_erase(u_int logic_no) {
	if(map[logic_no] != -1) {
		py_erase(map[logic_no]);
		map[logic_no] = -1;
	}
}
// Overview:
//  read data from IDE disk. First issue a read request through
//  disk register and then copy data from disk buffer
//  (512 bytes, a sector) to destination array.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  dst: destination for data read from IDE disk.
//  nsecs: the number of sectors to read.
//
// Post-Condition:
//  Panic if any error occurs. (you may want to use 'panic_on')
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_OPERATION_READ',
//  'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS', 'DEV_DISK_BUFFER'
void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (1/2) */
		uint32_t disoff = begin + off;
		uint32_t op = DEV_DISK_OPERATION_READ;
                panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_ID, 4));
		panic_on(syscall_write_dev(&disoff, DEV_DISK_ADDRESS + DEV_DISK_OFFSET, 4));
		panic_on(syscall_write_dev(&op, DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION, 4));
		uint32_t result;
		panic_on(syscall_read_dev(&result, DEV_DISK_ADDRESS + DEV_DISK_STATUS, 4));
		panic_on(result == 0);
		void *newdst = dst + off;
		panic_on(syscall_read_dev(newdst, DEV_DISK_ADDRESS + DEV_DISK_BUFFER, DEV_DISK_BUFFER_LEN));
	}
}

// Overview:
//  write data to IDE disk.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  src: the source data to write into IDE disk.
//  nsecs: the number of sectors to write.
//
// Post-Condition:
//  Panic if any error occurs.
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (2/2) */
                uint32_t disoff = begin + off;
                uint32_t op = DEV_DISK_OPERATION_WRITE;
		void *newsrc = src + off;
                panic_on(syscall_write_dev(newsrc, DEV_DISK_ADDRESS + DEV_DISK_BUFFER, DEV_DISK_BUFFER_LEN));
                panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_ID, 4));
                panic_on(syscall_write_dev(&disoff, DEV_DISK_ADDRESS + DEV_DISK_OFFSET, 4));
                panic_on(syscall_write_dev(&op, DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION, 4));
                uint32_t result;
                panic_on(syscall_read_dev(&result, DEV_DISK_ADDRESS + DEV_DISK_STATUS, 4));
                panic_on(result == 0);
	}
}
