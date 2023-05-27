#define SIGKILL 9
#define SIGSEGV 11
#define SIGTERM 15

typedef struct sigset_t{
    int sig[2]; //最多 32*2=64 种信号
}sigest_t;

struct sigaction{
    void (*sa_handler)(int);
    sigset_t sa_mask;
};


#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

void sigemptyset(sigset_t *set); // 清空信号集，将所有位都设置为 0
void sigfillset(sigset_t *set); // 设置信号集，即将所有位都设置为 1
void sigaddset(sigset_t *set, int signum); // 向信号集中添加一个信号，即将指定信号的位设置为 1
void sigdelset(sigset_t *set, int signum); // 从信号集中删除一个信号，即将指定信号的位设置为 0
int sigismember(const sigset_t *set, int signum); // 检查一个信号是否在信号集中，如果在则返回 1，否则返回 0

