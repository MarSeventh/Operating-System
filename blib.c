#include <blib.h>

size_t strlen(const char *s) {
	size_t len=0;
	while(s[len]!='\0'){
               len++;
	}
	return len;
}

char *strcpy(char *dst, const char *src) {
	char *res = dst;
	while(*src){
		*dst++=*src++;
	}
	*dst='\0';
	return res;
}

char *strncpy(char *dst, const char *src, size_t n) {
	char *res = dst;
	while (*src && n--) {
		*dst++ = *src++;
	}
	*dst = '\0';
	return res;
}

char *strcat(char *dst, const char *src) {
	char *res = dst;
	while(*dst){
		dst++;
	}
        while(*src){
                *dst++ = *src++;
	}
	*dst = '\0';
	return res;
}

int strcmp(const char *s1, const char *s2) {
	while(*s1 && *s2){
		if(*s1<*s2){
			return -1;
		}
		else if (*s1>*s2){
			return 1;
		}
		s1++;
		s2++;
	}
	if(*s1) return 1;
	else if(*s2) return -1;
	else return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	while (n--) {
		if (*s1 != *s2) {
			return *s1 - *s2;
		}
		if (*s1 == 0) {
			break;
		}
		s1++;
		s2++;
	}
	return 0;
}

void *memset(void *s, int c, size_t n) {
	char *cdest = (char *)s;
	while(n--){
		*cdest++ = c;
	}
	return s;
}

void *memcpy(void *out, const void *in, size_t n) {
	char *csrc = (char *)in;
	char *cdest = (char *)out;
	for (int i = 0; i < n; i++) {
		cdest[i] = csrc[i];
	}
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	char *c1 = (char *)s1;
	char *c2 = (char *)s2;
	while(n--){
		if(*c1 < *c2) return -1;
		else if(*c1 > *c2) return 1;
		c1++;
		c2++;
	}
	return 0;
}
