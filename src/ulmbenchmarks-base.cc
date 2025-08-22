#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <pthread.h>
#include <fnmatch.h>

#include <string>

#include "stuff.h"

using namespace std;

int global_blah = 0;
int global_blah2 = 0;

volatile int stopit = 0;

const long usec_per_test = 500000;

int do_hugepages = 0;

void onsig(int sig)
{
  stopit = 1;
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condvar = PTHREAD_COND_INITIALIZER;
pthread_mutex_t condvar_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condvar2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t condvar_mutex2 = PTHREAD_MUTEX_INITIALIZER;

class gbuf {
  // these always fly in parallel.  If in == NULL there is nothing, no
  // need to check more
  size_t last_allocated_size;
public:
  void **test_compression_on_this; // for algorithms to say where the result is
  void *buf;
  void *buf2; // in case an algorithm needs separate out from in
  void *buf3; // for compression test later
  gbuf() {
    buf = NULL;
    test_compression_on_this = &buf;
  }
  ~gbuf() {
    if (buf) {
      free(buf3);
      free(buf2);
      free(buf);
    }
  }
  void ensure_size(size_t size) {
    if (buf) {
      if (size != last_allocated_size) {
	free(buf3);
	free(buf2);
	free(buf);
	buf = NULL;
      }
    }
    if (!buf) {
      if (! ((buf = malloc(size)) && (buf2 = malloc(size)) && (buf3 = malloc(size * 2)))) {
	perror("malloc");
	exit(2);
      }
      // printf("# allocated gbuf with %lld k\n", (long long)size / 1024);
      last_allocated_size = size;
    }
    test_compression_on_this = &buf;
  }
} global_buf;

volatile int subtract_real_time = 0;
void need_sleep(int usecs)
{
  usleep(usecs);
  subtract_real_time += usecs;
}

// Verbosity controls
//
// states: 0=nuffing, 1=need to print, 2=did print
int pf_msg_control = 0;

void runtest(void (*func)(void), void (*funcs)(size_t size)
	     , const char *funcname, long long size)
{
  struct itimerval it;
  int n;
  struct timeval beginning;
  struct timeval end;
  struct rusage ru1;
  struct rusage ru2;

  timerclear(&it.it_interval);
  it.it_value.tv_sec = usec_per_test / 1000000;
  it.it_value.tv_usec = usec_per_test % 1000000;

  setitimer(ITIMER_REAL, &it, NULL);

  subtract_real_time = 0;
  if (getrusage(RUSAGE_SELF, &ru1) == -1) {
    perror("getrusage");
    exit(2);
  }
  if (gettimeofday(&beginning, NULL) == -1) {
    perror("gettimeofday1");
    exit(2);
  }
  if (func)
    for (n = 0; !stopit; n++)
      (*func)();
  else 
    for (n = 0; !stopit; n++)
      (*funcs)(size);
  it.it_value.tv_sec = 0;
  it.it_value.tv_usec = 0;
  setitimer(ITIMER_REAL, &it, NULL);
  stopit = 0;
  if (gettimeofday(&end, NULL) == -1) {
    perror("gettimeofday2");
    exit(2);
  }
  if (getrusage(RUSAGE_SELF, &ru2) == -1) {
    perror("getrusage");
    exit(2);
  }
  
  double seconds_per_call = 
    (double)end.tv_sec + (double)end.tv_usec / 1000000.0
    - (double)beginning.tv_sec - (double)beginning.tv_usec / 1000000.0
    //- (double)subtract_real_time / 1000000.0
    ;
  seconds_per_call /= (double) n;

  if (seconds_per_call > 1.0 / 100000.0 ) {
    printf("%10.1f usec/call", seconds_per_call * 1000000.0);
  } else {
    printf("%10.1f nsec/call", seconds_per_call * 1000000000.0);
  }

  printf(" %10.1f user %10.1f sys"
	 , (((double)ru2.ru_utime.tv_sec + (double)ru2.ru_utime.tv_usec / 1000000.0) -
	    ((double)ru1.ru_utime.tv_sec + (double)ru1.ru_utime.tv_usec / 1000000.0)) / (double)n * 1000000000.0
	 , (((double)ru2.ru_stime.tv_sec + (double)ru2.ru_stime.tv_usec / 1000000.0) -
	    ((double)ru1.ru_stime.tv_sec + (double)ru1.ru_stime.tv_usec / 1000000.0)) / (double)n * 1000000000.0
	 );

#if 0
  // activate this to print page fault stats
  int spfs = ru2.ru_minflt - ru1.ru_minflt;
  double spfspc = (double)spfs / (double)n;
  int hpfs = ru2.ru_majflt - ru1.ru_majflt;
  double hpfspc = (double)hpfs / (double)n;
  // only print if there was at least 1% of calls causing a pagefault
  if (spfspc > 0.01 || hpfspc > 0.01) {
    if (pf_msg_control == 0)
      pf_msg_control = 1;
    printf(" %.3f hpf %.3f spf"
	   , (double)(ru2.ru_minflt - ru1.ru_minflt) / (double)n
	   , (double)(ru2.ru_majflt - ru1.ru_majflt) / (double)n
	   );
  }
#if 0
  printf("  %10.1f signals", (double)(ru2.ru_nsignals - ru1.ru_nsignals) / (double)n);
#endif
#endif

  if (size) {
    double bytes_second = size / seconds_per_call;
    if (bytes_second > (1024 * 1024 * 1024))
      printf(" %7.1f G/sec ", bytes_second / 1024 / 1024 / 1024.0);
    else
      printf(" %7.1f M/sec ", bytes_second / 1024 / 1024.0);
  }
  printf(": %s", funcname);
  if (size) {
    if (size % (1024 * 1024) == 0) {
      printf(" size %lld M", (long long)size / 1024 / 1024);
    } else {
      if (size % 1024 == 0)
	printf(" size %lld k", (long long)size / 1024);
      else
	printf(" size %lld", (long long)size);
    }
  }
  printf("\n");
  if (pf_msg_control == 1) {
    printf("started printing pagefaults. spf=minor pagefaults, hpf=major pagefault. Numbers are per bench call\n");
    pf_msg_control = 2;
  }
  fflush(stdout);
}
void runtest_no_size(void (*func)(void), const char *funcname)
{
  runtest(func, NULL, funcname, 0);
}
void runtest_with_size(void (*funcs)(size_t), const char *funcname, size_t size)
{
  runtest(NULL, funcs, funcname, size);
}

#define call_stuff(_FUNCNAME_) \
  runtest_no_size(test_ ## _FUNCNAME_, #_FUNCNAME_)

#define call_stuff_with_size(_FUNCNAME_, size)	\
  runtest_with_size(test_ ## _FUNCNAME_, #_FUNCNAME_, size)

void do_dev(const char *const filename, void *const buffer, const size_t size, int *cached_fd)
{
  int fd;

  if (*cached_fd != -1)
    fd = *cached_fd;
  else {
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
      fprintf(stderr, "open %s: ", filename);
      perror(NULL);
      exit(2);
    }
    *cached_fd = fd;
  }
  if (read(fd, buffer, size) == -1) {
    perror("read random");
    exit(2);
  }
}

int cached_fd_urandom = -1;
int cached_fd_random = -1;
int cached_fd_zero = -1;

void test_gettimeofday()
{
  struct timeval tv;
  if (gettimeofday(&tv, NULL) == -1) {
    perror("gettimofday");
    exit(2);
  }
}

const char *buf1 = "lkaskdjkasdjkajsdkjasddlasdjlaskdjklajsdljaszdjlasdjlkajsldjlajsdl";
void test_strncpy()
{
  char buf2[8192];
  strncpy(buf2, buf1, sizeof(buf2)-1);
  if (buf2[1] == 'a')
    exit(1);
}

void test_strchr()
{
  const char *res;
  res = strchr(buf1, 'z');
  if (*res == 'a')
    exit(1);
}

int fd_to_dev_zero  = -1;
void test_read2mdevzero()
{
  char buf[2 * 1024 * 1024];
  if (read(fd_to_dev_zero, buf, sizeof(buf)) == -1) {
    perror("read devzero");
    exit(1);
  }
}
void test_read8kdevzero()
{
  char buf[8192];
  if (read(fd_to_dev_zero, buf, sizeof(buf)) == -1) {
    perror("read devzero");
    exit(1);
  }
}
void test_read1bdevzero()
{
  char buf[1];
  if (read(fd_to_dev_zero, buf, sizeof(buf)) == -1) {
    perror("read devzero");
    exit(1);
  }
}

void test_getrusage()
{
  struct rusage ru;
  if (getrusage(RUSAGE_SELF, &ru) == -1) {
    perror("getrusage");
    exit(2);
  }
}
#define empty_test(_SYSCALLNAME_, _ARGS_) \
void test_ ## _SYSCALLNAME_() { _SYSCALLNAME_ _ARGS_;}

#define empty_test_checking(_SYSCALLNAME_, _ARGS_) \
void test_ ## _SYSCALLNAME_() { \
  if (_SYSCALLNAME_ _ARGS_ == -1) { \
    perror(#_SYSCALLNAME_ #_ARGS_); \
    exit(2); \
  } \
}

char buf[1];

empty_test(rand, ());
empty_test(random, ());
empty_test_checking(read, (0, buf, 0));
empty_test_checking(pthread_mutex_trylock, (&mutex));

void test_mutex_lock_unlock()
{
  pthread_mutex_lock(&mutex);
  pthread_mutex_unlock(&mutex);
}

void *setup_background_condvarbouncer(void *nothing)
{
  for (;;) {
    // Wait for foreground to be ready
    pthread_mutex_lock(&condvar_mutex2);
    pthread_cond_wait(&condvar2, &condvar_mutex2);
    write(1, "BGI2\n", 5);
    pthread_mutex_unlock(&condvar_mutex2);

    write(1, "BGL3\n", 5);
    pthread_mutex_lock(&condvar_mutex);
    pthread_cond_signal(&condvar);
    write(1, "BGL4\n", 5);
    pthread_mutex_unlock(&condvar_mutex);

  }
  return NULL;
}

void test_condvar_round_trip()
{
  pthread_mutex_lock(&condvar_mutex2);
  write(1, "FG01\n", 5);
  pthread_cond_signal(&condvar2);
  pthread_mutex_unlock(&condvar_mutex2);

  pthread_mutex_lock(&condvar_mutex);
  write(1, "FG02\n", 5);
  pthread_cond_wait(&condvar, &condvar_mutex);
  pthread_mutex_unlock(&condvar_mutex);
}

void test_condvar_signal()
{
  pthread_mutex_lock(&condvar_mutex);
  pthread_cond_signal(&condvar);
  pthread_mutex_unlock(&condvar_mutex);
}

void test_snprintf()
{
  char foo[8192];
  snprintf(foo, sizeof(foo), "Blah: %d\n", 42);
}

void test_snprintf_float()
{
  char foo[8192];
  snprintf(foo, sizeof(foo), "Blah: %10.4f\n", 42.0);
}

void test_fnmatch()
{
  fnmatch("*.iso", "ubuntu.iso", 0);
}

static int meh;
void test_atoi()
{
  meh = atoi("42");
}

double globby1 = 0.0;
void test_floatrand()
{
  globby1 = (double)rand() / (double) RAND_MAX;
}

Foo::Foo(int printme_p) {
  printme = printme_p;
}
void Foo::testme(void) {
  if (printme)
    fprintf(stderr, "testme\n");
}
Foo::~Foo() {
  global_blah2 = 0;
  if (printme)
    fprintf(stderr, "Destructor invoked\n");
}

void test_cpp_testhrow_throw(void)
{
  Foo foo(0);

  try {
    cpp_thrower(1);
    foo.testme();
  } catch(...) {
    if (global_blah)
      fprintf(stderr, "Caught exception\n");
  }
}

void test_cpp_testhrow_no_throw(void)
{
  Foo foo(0);

  try {
    cpp_thrower(0);
    foo.testme();
  } catch(...) {
    if (global_blah)
      fprintf(stderr, "Caught exception\n");
  }
}

void test_cpp_testhrow_no_possible_throw(void)
{
  Foo foo(0);

  try {
    foo.testme();
  } catch(...) {
    if (global_blah)
      fprintf(stderr, "Caught exception\n");
  }
}

void test_cpp_testhrow_no_cleanup_no_throw(void)
{
  try {
    cpp_thrower(0);
  } catch(...) {
    if (global_blah)
      fprintf(stderr, "Caught exception\n");
  }
}

void test_cpp_testhrow_no_cleanup_no_possible_throw(void)
{
  try {
    global_blah2 = 0;
  } catch(...) {
    if (global_blah)
      fprintf(stderr, "Caught exception\n");
  }
}

const int qsort_size = 1024 * 1024;
const int n_sort_tests = 3;

int qsort_data[n_sort_tests][qsort_size];
int qsort_data_source[n_sort_tests][qsort_size];

void test_prepare_qsort_1m(void)
{
  int i, j;
  for (j = 0; j < n_sort_tests; j++) {
    for (i = 0 ; i < qsort_size; i++) {
      qsort_data_source[j][i] = random();
    }
  }
}

int qsort_1m_compare(const void *p1, const void *p2)
{
  int *i1 = (int *)p1;
  int *i2 = (int *)p2;

  if (*i1 < *i2)
    return -1;
  if (*i1 == *i2)
    return 0;
  return 1;
}

int qsort_1m2_compare(const void *p1, const void *p2)
{
  int i1 = *(int *)p1;
  int i2 = *(int *)p2;

  if (i1 < i2)
    return -1;
  if (i1 == i2)
    return 0;
  return 1;
}

int qsort_1m3_compare(const void *p1, const void *p2)
{
  int i1 = *(int *)p1;
  int i2 = *(int *)p2;

  if (i1 > i2)
    return 1;
  if (i1 < i2)
    return -1;
  return 0;
}

void test_qsort_1m(void)
{
  test_prepare_qsort_1m();

  qsort(qsort_data[0], qsort_size, sizeof(**qsort_data), qsort_1m_compare);
}

void test_qsort_1m2(void)
{
  test_prepare_qsort_1m();

  qsort(qsort_data[1], qsort_size, sizeof(**qsort_data), qsort_1m2_compare);
}

void test_qsort_1m3(void)
{
  test_prepare_qsort_1m();

  qsort(qsort_data[2], qsort_size, sizeof(**qsort_data), qsort_1m3_compare);
}

unsigned int shake_bits(unsigned int key) {
  key = ~key + (key << 15);
  key = key ^ (key >> 12);
  key = key + (key << 2);
  key = key ^ (key >> 4);
  key = key * 2057;
  key = key ^ (key >> 16);
  return key;
}

int stuff = 98349823;
int stuff2;

void test_hash(void)
{
  stuff2 = shake_bits(stuff);
}

void *raw_mapped_area = NULL;
char *mapped_area = NULL;
volatile long long mapped_area_size = 0;
volatile long long mprotect_size = 32 * 1024;
volatile long pagesize = 4 * 1024;
volatile char* cheating = 0; // backchannel to tell true address
volatile int n_mappings = 0;
const int max_mappings = 32768;
volatile int mprotect_do_n = 1;
volatile int touch_p = 1;

// we only want to use a part of those pages
// "8" here means every 8th mprotect_size
// Keep in mind at the time of this writing we
// only touch the first page in each region
volatile int every_nth = 1;

char *random_location_in_parts_of_mapped_region(int do_check_p)
{
  // gives a location inside the mapped area, but takes into
  // account the restriction about how much and what to touch

  int n_regions_in_play = mapped_area_size / mprotect_size / every_nth;

  double ran = (double)rand() / (double) RAND_MAX;
  if (ran >= 1.0) ran /= 0.42; // count be one
  long long picked_area = (double)n_regions_in_play * ran;
  long long physical_area = picked_area * every_nth;

  char *ret = mapped_area;

  ret += physical_area * mprotect_size;

  if (do_check_p)
    if (touch_p && *ret != 'a' && *ret != 'y') {
      fprintf(stderr, "touched area not in sync %p %lld %lld: '%d', regions: %d rnd %f area @ %p\n"
	      , ret
	      , physical_area
	      , picked_area
	      , *ret
	      , n_regions_in_play
	      , ran
	      , mapped_area
	      );

    for (char *meh = ret - 32768 * 8;
	 meh < ret + 32768 * 8 && meh < mapped_area + mapped_area_size;
	 meh++) {
      if (*meh != 0) {
	fprintf(stderr, "'%c'(%d) found at %p\n", *meh, *meh > 32 ? *meh : '?', meh);
      }
    }
    exit(1);
  }

  return ret;
}

int my_mprotect(void *addr, size_t len, int prot, const char *msg)
{
  int i;
  int tmp_prot = prot | PROT_EXEC;
  int ret = -1;
  for (i = 0; i < mprotect_do_n - 1; i++) {
    if (tmp_prot == (PROT_READ | PROT_WRITE | PROT_EXEC)) {
      tmp_prot = PROT_READ | PROT_EXEC;
    } else {
      tmp_prot = PROT_READ | PROT_WRITE | PROT_EXEC;
    }
    if ((ret = mprotect(addr, len, tmp_prot)) == -1) {
      perror("meh mprotect");
    }
  }
  ret = mprotect(addr, len, prot);
  if (ret == -1) {
    perror("mprotect");
    fprintf(stderr, "mprotect did: %p size %lld\n", addr, (long long)len);
    if (msg) {
      fprintf(stderr, "At '%s'\n", msg);
      fflush(stderr);
      exit(1);
    }
  }
  return ret;
}

void my_munmap(void)
{
  if (munmap(raw_mapped_area, mapped_area_size + mprotect_size) == -1) {
    perror("cannot munmap");
  }
  raw_mapped_area = NULL;
  mapped_area = NULL;
}

void reestablish_mapping(int prot)
{
  if (raw_mapped_area) {
    if (munmap(raw_mapped_area, mapped_area_size + mprotect_size) == -1) {
      perror("munmap");
    }
    raw_mapped_area = 0;
  }
  // we need this aligned by mprotect_size, not page size
  int attributes;
  if (do_hugepages) {
#ifdef MAP_HUGETLB
    attributes = MAP_PRIVATE | MAP_ANON | MAP_HUGETLB;
#else
    fprintf(stderr, "dunno how to do hugepages here\n");
    exit(1);
#endif
  } else
    attributes = MAP_PRIVATE | MAP_ANON;
  raw_mapped_area = mmap(NULL, mapped_area_size + mprotect_size, prot|PROT_WRITE
			 , attributes, -1, 0);
  if (raw_mapped_area == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }
  mapped_area = (char *)(((long long)raw_mapped_area / (long long)mprotect_size)
			 * (long long)mprotect_size + (long long)mprotect_size);
  if ((long long) mapped_area % mprotect_size != 0) {
    fprintf(stderr, "map not aligned correctly\n");
    exit(1);
  }
  if (touch_p) {
    // touch every nth card, but only the 1st page for now (RAM savings)
    int n_to_touch = mapped_area_size / mprotect_size / every_nth;
    for (int i = 0; i < n_to_touch; i++) {
    char *addr;
    addr = mapped_area + i * mprotect_size * every_nth;
    // printf("touching region %d at %p\n", i * every_nth, addr);
    *addr = 'a';
    }
  }
    
  //fprintf(stderr, "touch finished\n");

  if (prot != (prot|PROT_WRITE)) {
    if (mprotect(raw_mapped_area, mapped_area_size + mprotect_size, prot) == -1) {
      perror("mprotect(initial re-perm)");
      exit(1);
    }
  }
}

void test_mmap_no_touch(void)
{
  int old_touch_p = touch_p;
  touch_p = 0;
  reestablish_mapping(PROT_READ|PROT_WRITE);
  my_munmap();
  touch_p = old_touch_p;
}
void test_mmap_and_touch(void)
{
  int old_touch_p = touch_p;
  touch_p = 1;
  reestablish_mapping(PROT_READ|PROT_WRITE);
  my_munmap();
  touch_p = old_touch_p;
}

void test_madvise_randloc(void)
{
  char *ptr = random_location_in_parts_of_mapped_region(0);
  if (ptr + mprotect_size > mapped_area + mapped_area_size) {
    fprintf(stderr, "size error ma %lld %lld %lld %lld\n"
	    , (long long)ptr
	    , (long long)mprotect_size
	    , (long long)mapped_area
	    , (long long)mapped_area_size);
    exit(1);
  }

  if (madvise((void *)ptr, mprotect_size, MADV_DONTNEED) == -1) {
    fprintf(stderr, "madvice was doing %p %lld\n", (void *)ptr, mprotect_size);
    perror("madvise(DONTNEED)");
    exit(1);
  }
}

// BEGIN section taken from sbcl
void test_mprotect_randloc()
{
  char *ptr = random_location_in_parts_of_mapped_region(1);
  if (ptr + mprotect_size > mapped_area + mapped_area_size) {
    fprintf(stderr, "size error mp %lld %lld %lld %lld\n", (long long)ptr, (long long)mprotect_size, (long long)mapped_area, (long long)mapped_area_size);
    exit(1);
  }

  n_mappings++;
  if (n_mappings > max_mappings) {
#if 0
    stopit = 1;
#else
    reestablish_mapping(PROT_READ|PROT_WRITE);
    n_mappings = 0;
#endif
  }

  my_mprotect((void *)ptr, mprotect_size, PROT_READ, "randloc");
}

typedef struct ucontext os_context_t;
typedef void *os_vm_address_t;
os_vm_address_t
arch_get_bad_addr(int sig, siginfo_t *code, os_context_t *context)
{
    return (os_vm_address_t)code->si_addr;
}
// END section taken from sbcl

volatile long n_segfaults = 0;
volatile long n_writes = 0;

void count_vmmap(int printmap)
{
#if 0
  char cmd[8192];

  fflush(stdout);

  snprintf(cmd, sizeof(cmd), "echo `wc -l /proc/%d/maps` '(number of mappings)'", getpid());
  system(cmd);
  if (printmap)
    snprintf(cmd, sizeof(cmd), "cat /proc/%d/maps | tee maps", getpid());
  else
    snprintf(cmd, sizeof(cmd), "cat /proc/%d/maps > maps", getpid());
  system(cmd);
#endif
}

static void
sigsegv_handler(int signal, siginfo_t *info, void *ctx)
{
  os_context_t *context = (os_context_t *)ctx;
  os_vm_address_t addr = arch_get_bad_addr(signal, info, context);

  n_segfaults++;
  n_mappings++;
  if (n_mappings > max_mappings) {
#if 0
    stopit = 1;
#else
    reestablish_mapping(PROT_READ);
    n_mappings = 0;
#endif
  }

#if 0
  // note: printf is not legal to use from a signal handler
  fprintf(stderr, "fault addr: %p\n", addr);
#endif

  if (addr < mapped_area) {
    char msg[] = "got wrong segfault, too low\n";
    write(2, msg, sizeof(msg));
  }
  if (addr > mapped_area + mapped_area_size) {
    char msg[] = "got wrong segfault, too high\n";
    write(2, msg, sizeof(msg));
  }

  void *mprotect_addr = (void *)(((long long)addr / (long long)mprotect_size) * (long long)mprotect_size);
  if (my_mprotect(mprotect_addr, mprotect_size, PROT_READ|PROT_WRITE, NULL) == -1) {
    // note: printf is not legal to use from a signal handler
    fprintf(stderr, "mprotect_size: %lld\n", mprotect_size);
    fprintf(stderr, "cheating was %p, n_mappings %d/%d\n", cheating, n_mappings, max_mappings);
    fprintf(stderr, "step back was %ld\n", (long)addr % (long)mprotect_size);
    fprintf(stderr, "addr %p -> %p\n", addr, mprotect_addr);
    count_vmmap(0);
    fflush(stderr);
    exit(1);
  }

#if 0
  const char msg[] = "sigsegv\n";
  write(2, msg, sizeof(msg));
#endif
  return;
}

// write character to random location in mapped areas
void test_trigger_segfaults()
{
  char *write_to = random_location_in_parts_of_mapped_region(1);
  char mychar = 'y';

#if 0
  printf("doing segv, char is '%c', addr was %p, card is %lld of %lld\n", mychar, write_to, (write_to - mapped_area) / (long long)mprotect_size, mapped_area_size / mprotect_size);
  fflush(stdout);
#endif

  cheating = write_to;
  *write_to = mychar;
  n_writes++;

#if 0
  count_vmmap(0);
#endif
#if 0
  printf("survived segv, char is '%c', addr was %p, card is %lld of %lld\n", *write_to, write_to, (write_to - mapped_area) / (long long)mprotect_size, mapped_area_size / mprotect_size);
  fflush(stdout);
#endif
}

void test_mprotect_sameloc(void)
{
  my_mprotect(mapped_area + mprotect_size, mprotect_size, PROT_READ|((rand() % 2) * PROT_WRITE), "sameloc");
}

void pagefaults(long long size)
{
  mapped_area_size = size;
  n_segfaults = 0;
  n_writes = 0;

#if 1
  reestablish_mapping(PROT_READ|PROT_WRITE);
  call_stuff(mprotect_randloc);
  count_vmmap(0);
  reestablish_mapping(PROT_READ|PROT_WRITE);
  call_stuff(mprotect_sameloc);
  count_vmmap(0);
  reestablish_mapping(PROT_READ|PROT_WRITE);
  call_stuff(mmap_no_touch);
  call_stuff(mmap_and_touch);
  count_vmmap(0);
#endif

  reestablish_mapping(PROT_READ|PROT_WRITE);
  my_mprotect(mapped_area, mapped_area_size, PROT_READ, "protect all RO");
  struct sigaction sa, oldsa;
  sigset_t blockable_sigset;
  bzero(&sa, sizeof(sa));

  bzero(&blockable_sigset, sizeof(blockable_sigset));
  sa.sa_sigaction = sigsegv_handler;
  memcpy(&sa.sa_mask, &blockable_sigset, sizeof(sa.sa_mask));
  sa.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;

  if (sigaction(SIGSEGV, &sa, &oldsa) == -1) {
    perror("sigaction");
    exit(1);
  }
  // OSX
  if (sigaction(SIGBUS, &sa, &oldsa) == -1) {
    perror("sigaction");
    exit(1);
  }
  char *write_to = (char *)mapped_area + 3888;

#if 0
  printf("Should have readonly at %p - %p, size %lld\n", mapped_area, mapped_area + mapped_area_size, mapped_area_size);
  count_vmmap(1);
#endif

  *write_to = 'c';

  //printf("survived segv, char is '%c'\n", *write_to);

  //count_vmmap(1);

  fflush(stdout);
  system("test -f /proc/stat && head -1 /proc/stat");
  call_stuff(trigger_segfaults);
  printf("  (%ld segfaults, %ld writes, watch collision ratio)\n", n_segfaults, n_writes);
  fflush(stdout);
  system("test -f /proc/stat && head -1 /proc/stat");
  count_vmmap(0);

  if (!do_hugepages) {
    reestablish_mapping(PROT_READ|PROT_WRITE);
    call_stuff(madvise_randloc);
    count_vmmap(0);
  }
  reestablish_mapping(PROT_READ|PROT_WRITE);
  call_stuff(mprotect_randloc);
  count_vmmap(0);
  my_munmap();
}

int main_once(void)
{
  pthread_attr_t attr;
  pthread_t background_condvarbouncer;
  
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  if (0)
  pthread_create(&background_condvarbouncer, &attr
		 , setup_background_condvarbouncer, NULL);

  if (signal(SIGALRM, onsig) == SIG_ERR) {
    perror("signal");
    exit(2);
  }

  printf("FIXME: back to be changed pages with physical memory first\n");

  printf("sizeof char/short/int/long/long_long/void*/size_t/off_t:\n"
	 "%lu/%lu/%lu/%lu/%lu/%lu/%lu/%lu\n"
	 , (long unsigned)sizeof(char)
	 , (long unsigned)sizeof(short)
	 , (long unsigned)sizeof(int)
	 , (long unsigned)sizeof(long)
	 , (long unsigned)sizeof(long long)
	 , (long unsigned)sizeof(void*)
	 , (long unsigned)sizeof(size_t)
	 , (long unsigned)sizeof(off_t)
	 );
  int clk = sysconf(_SC_CLK_TCK);
  printf("clk is %d\n", clk);

#if 1
  call_stuff(atoi);
  call_stuff(snprintf);
  call_stuff(snprintf_float);
  call_stuff(fnmatch);
  //call_stuff(condvar_round_trip);
  call_stuff(condvar_signal);
  call_stuff(mutex_lock_unlock);
  call_stuff(pthread_mutex_trylock);
  call_stuff(gettimeofday);
  call_stuff(strncpy);
  call_stuff(strchr);
  call_stuff(getrusage);
  call_stuff(read);

  if ((fd_to_dev_zero = open("/dev/zero", O_RDONLY)) == -1) {
    perror("open /dev/zero");
    exit(1);
  }  
  call_stuff(read1bdevzero);
  call_stuff(read8kdevzero);
  call_stuff(read2mdevzero);
  close(fd_to_dev_zero);

  call_stuff(rand);
  call_stuff(random);
  call_stuff(floatrand);
#endif

#if 1
  call_stuff(cpp_testhrow_throw_48);
  call_stuff(cpp_testhrow_throw_24);
  call_stuff(cpp_testhrow_throw_12);
  call_stuff(cpp_testhrow_throw_4);
  call_stuff(cpp_testhrow_throw);
  call_stuff(cpp_testhrow_no_throw);
  call_stuff(cpp_testhrow_no_possible_throw);
  call_stuff(cpp_testhrow_no_cleanup_no_throw);
  call_stuff(cpp_testhrow_no_cleanup_no_possible_throw);
  call_stuff(prepare_qsort_1m);
  call_stuff(qsort_1m);
  call_stuff(qsort_1m2);
  call_stuff(qsort_1m3);

  stuff = rand();
  call_stuff(hash);
  printf("Hash %X -> %X\n", stuff, stuff2);
#endif

  if (getenv("GT_DO_HUGEPAGES")) {
    printf("doing hugepages\n");
    do_hugepages = 1;
    pagesize = 2 * 1024 * 1024; // fixme, should be determined based on a mapped area
    mprotect_size = pagesize;
  }

  long long size_for_mprotect_tests;
  if (getenv("GT_MPROTECT_SIZE_IN_MB"))
    size_for_mprotect_tests = atoll(getenv("GT_MPROTECT_SIZE_IN_MB"))
      * 1024ll * 1024ll;
  else {
    if (sizeof(long) == 4)
      size_for_mprotect_tests = 512ll * 1024ll * 1024ll;
    else
      size_for_mprotect_tests = 8ll * 1024ll * 1024ll * 1024ll;
  }
  printf("Size of mapped area for protect/segv tests: %lld MB, %lld cards\n"
	 , size_for_mprotect_tests / 1024ll / 1024ll
	 , size_for_mprotect_tests / mprotect_size);

  mprotect_do_n = 1;
#if 1
  printf("no touch by default\n");
  touch_p = 0;
  pagefaults(size_for_mprotect_tests);
#endif
  printf("with touch by default\n");
  touch_p = 1;
  pagefaults(size_for_mprotect_tests);

#if 1
  printf("All mprotect calls double:\n");
  mprotect_do_n = 2;
  pagefaults(size_for_mprotect_tests);
  printf("All mprotect calls 10x:\n");
  mprotect_do_n = 10;
  pagefaults(size_for_mprotect_tests);
  printf("All mprotect calls single:\n");
  mprotect_do_n = 1;
  pagefaults(size_for_mprotect_tests);
  mprotect_do_n = 1;
#endif

  return 0;
}

int main(void)
{
  main_once();
  //for (;;)
  //  main_once();
}