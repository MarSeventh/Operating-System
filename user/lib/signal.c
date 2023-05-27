#include <signal.h>
#include <string.h>
#include <lib.h>

void sigemptyset(sigset_t *set) {
	memset(set, 0, sizeof(sigset_t));
}

void sigfillset(sigset_t *set) {
	memset(set, 0xff, sizeof(sigset_t));
}

void sigaddset(sigset_t *set, int signum) {
	if(signum >= 1 && signum <= 64) {
		int index = (signum - 1) / 32;
		int offset = (signum - 1) % 32;
		set->sig[index] |= (1 << offset);
	}
}

void sigdelset(sigset_t *set, int signum) {
	if(signum >= 1 && signum <= 64){
		int index = (signum - 1) / 32;
                int offset = (signum - 1) % 32;
                set->sig[index] &= ~(1 << offset);
        }
}

int sigismember(const sigset_t *set, int signum) {
        if(signum >= 1 && signum <= 64){
                int index = (signum - 1) / 32;
                int offset = (signum - 1) % 32;
                return (set->sig[index] & (1 << offset)) != 0;
        }
        return 0;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {

