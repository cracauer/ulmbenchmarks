# ulmbenchmarks
Microbenchmarks that wrap around system calls and functions

Here are results for FreeBSD-15-PRERELEASE and Linux Mint 22:

       1.5 nsec/call        1.6 user        0.0 sys: atoi
      76.5 nsec/call       76.5 user        0.0 sys: snprintf
     153.5 nsec/call      153.5 user        0.0 sys: snprintf_float
     138.0 nsec/call      138.0 user        0.0 sys: fnmatch
      21.1 nsec/call       21.1 user        0.0 sys: condvar_signal
      15.8 nsec/call       15.8 user        0.0 sys: mutex_lock_unlock
       6.1 nsec/call        6.1 user        0.0 sys: pthread_mutex_trylock
      27.7 nsec/call       27.7 user        0.0 sys: gettimeofday
     197.0 nsec/call      197.0 user        0.0 sys: strncpy
       5.6 nsec/call        5.6 user        0.0 sys: strchr
     139.7 nsec/call       13.3 user      125.4 sys: getrusage
      75.9 nsec/call       18.3 user       57.7 sys: read
     116.1 nsec/call       33.3 user       82.7 sys: read1bdevzero
     214.8 nsec/call       30.5 user      184.3 sys: read8kdevzero
      55.8 usec/call      298.6 user    55527.9 sys: read2mdevzero
       4.5 nsec/call        4.5 user        0.0 sys: rand
       2.8 nsec/call        2.8 user        0.0 sys: random
       5.2 nsec/call        5.2 user        0.0 sys: floatrand
    5864.5 nsec/call     5874.8 user        0.0 sys: cpp_testhrow_throw_48
    5469.4 nsec/call     5466.5 user        0.0 sys: cpp_testhrow_throw_24
    5297.6 nsec/call     5296.7 user        0.0 sys: cpp_testhrow_throw_12
    5180.9 nsec/call     5172.3 user        2.7 sys: cpp_testhrow_throw_4
    5130.7 nsec/call     5138.3 user        0.0 sys: cpp_testhrow_throw
       5.9 nsec/call        5.9 user        0.0 sys: cpp_testhrow_no_throw
       4.9 nsec/call        4.9 user        0.0 sys: cpp_testhrow_no_possible_throw
       2.2 nsec/call        2.2 user        0.0 sys: cpp_testhrow_no_cleanup_no_throw
       1.2 nsec/call        1.2 user        0.0 sys: cpp_testhrow_no_cleanup_no_possible_throw
    9060.3 usec/call  9062237.3 user        0.0 sys: prepare_qsort_1m
   15571.7 usec/call 15564085.7 user        0.0 sys: qsort_1m
   15593.5 usec/call 15581454.5 user        0.0 sys: qsort_1m2
   15815.9 usec/call 15806735.3 user        0.0 sys: qsort_1m3
       2.5 nsec/call        2.5 user        0.0 sys: hash
Hash 3EFA4189 -> B04FD36C
Size of mapped area for protect/segv tests: 8192 MB, 262144 cards
no touch by default
    1104.6 nsec/call       15.4 user     1088.1 sys: mprotect_randloc
     161.8 nsec/call       26.4 user      135.4 sys: mprotect_sameloc
     699.8 nsec/call       55.8 user      643.9 sys: mmap_no_touch
  315830.5 usec/call 20351500.0 user 295478500.0 sys: mmap_and_touch
    4956.8 nsec/call      756.6 user     4200.2 sys: trigger_segfaults
  (98307 segfaults, 105050 writes, watch collision ratio)
     164.9 nsec/call       22.8 user      142.1 sys: madvise_randloc
    1091.9 nsec/call        0.0 user     1100.2 sys: mprotect_randloc
with touch by default
      13.4 usec/call     1535.4 user    11826.9 sys: mprotect_randloc
     163.2 nsec/call       27.2 user      136.0 sys: mprotect_sameloc
     780.6 nsec/call       72.9 user      707.7 sys: mmap_no_touch
  316058.5 usec/call 25682500.0 user 290375500.0 sys: mmap_and_touch
    9086.7 nsec/call     1075.5 user     8008.4 sys: trigger_segfaults
  (54822 segfaults, 57978 writes, watch collision ratio)
     781.9 nsec/call       46.2 user      735.7 sys: madvise_randloc
      15.9 usec/call     1041.1 user    14817.0 sys: mprotect_randloc
All mprotect calls double:
      11.3 usec/call     1211.6 user    10099.8 sys: mprotect_randloc
     270.1 nsec/call       17.4 user      252.7 sys: mprotect_sameloc
     785.6 nsec/call       59.5 user      726.1 sys: mmap_no_touch
  316539.5 usec/call 34157000.0 user 282382000.0 sys: mmap_and_touch
    9741.1 nsec/call     1524.7 user     8216.5 sys: trigger_segfaults
  (50060 segfaults, 52931 writes, watch collision ratio)
     783.7 nsec/call       49.7 user      733.9 sys: madvise_randloc
      14.6 usec/call     1506.1 user    13096.8 sys: mprotect_randloc
All mprotect calls 10x:
      12.1 usec/call     1192.5 user    10926.6 sys: mprotect_randloc
    1020.3 nsec/call      192.7 user      827.6 sys: mprotect_sameloc
     791.7 nsec/call       32.3 user      759.4 sys: mmap_no_touch
  316685.0 usec/call 12573500.0 user 304111000.0 sys: mmap_and_touch
      12.5 usec/call     1440.2 user    11036.8 sys: trigger_segfaults
  (38056 segfaults, 40289 writes, watch collision ratio)
     781.3 nsec/call       89.8 user      691.5 sys: madvise_randloc
      12.9 usec/call     1148.7 user    11731.1 sys: mprotect_randloc
All mprotect calls single:
      11.2 usec/call     1100.8 user    10123.3 sys: mprotect_randloc
     163.5 nsec/call       17.7 user      145.8 sys: mprotect_sameloc
     792.9 nsec/call       58.4 user      734.5 sys: mmap_no_touch
  316458.0 usec/call 35992500.0 user 280464500.0 sys: mmap_and_touch
    9939.3 nsec/call     1540.0 user     8399.3 sys: trigger_segfaults
  (47776 segfaults, 50452 writes, watch collision ratio)
     783.6 nsec/call        0.0 user      784.8 sys: madvise_randloc
      13.9 usec/call     1273.5 user    12668.9 sys: mprotect_randloc


Linux Mint 22:

       1.2 nsec/call        1.2 user        0.0 sys: atoi
      49.4 nsec/call       49.4 user        0.0 sys: snprintf
     195.2 nsec/call      195.2 user        0.0 sys: snprintf_float
      24.2 nsec/call       24.2 user        0.0 sys: fnmatch
       6.2 nsec/call        6.2 user        0.0 sys: condvar_signal
       4.6 nsec/call        4.6 user        0.0 sys: mutex_lock_unlock
       5.4 nsec/call        5.4 user        0.0 sys: pthread_mutex_trylock
      21.5 nsec/call       21.5 user        0.0 sys: gettimeofday
      67.3 nsec/call       67.3 user        0.0 sys: strncpy
       3.6 nsec/call        3.6 user        0.0 sys: strchr
     567.8 nsec/call       80.4 user      487.4 sys: getrusage
     522.3 nsec/call       58.9 user      463.4 sys: read
     313.5 nsec/call       79.0 user      234.5 sys: read1bdevzero
     576.3 nsec/call       95.6 user      480.7 sys: read8kdevzero
      86.1 usec/call     5334.1 user    80733.4 sys: read2mdevzero
      10.0 nsec/call       10.0 user        0.0 sys: rand
       9.6 nsec/call        9.6 user        0.0 sys: random
      10.9 nsec/call       10.9 user        0.0 sys: floatrand
    1686.5 nsec/call     1686.5 user        0.0 sys: cpp_testhrow_throw_48
    1373.5 nsec/call     1373.4 user        0.0 sys: cpp_testhrow_throw_24
    1219.1 nsec/call     1219.1 user        0.0 sys: cpp_testhrow_throw_12
    1121.2 nsec/call     1121.1 user        0.0 sys: cpp_testhrow_throw_4
    1078.4 nsec/call     1078.5 user        0.0 sys: cpp_testhrow_throw
       5.3 nsec/call        5.3 user        0.0 sys: cpp_testhrow_no_throw
       4.4 nsec/call        4.4 user        0.0 sys: cpp_testhrow_no_possible_throw
       2.2 nsec/call        2.2 user        0.0 sys: cpp_testhrow_no_cleanup_no_throw
       1.2 nsec/call        1.2 user        0.0 sys: cpp_testhrow_no_cleanup_no_possible_throw
   31320.1 usec/call 30949937.5 user   368875.0 sys: prepare_qsort_1m
   54682.5 usec/call 54384500.0 user   296400.0 sys: qsort_1m
   57482.9 usec/call 57253000.0 user   226555.6 sys: qsort_1m2
   52062.5 usec/call 51868400.0 user   194100.0 sys: qsort_1m3
       2.4 nsec/call        2.4 user        0.0 sys: hash
Hash AE22D42 -> 9E235462
Size of mapped area for protect/segv tests: 8192 MB, 262144 cards
no touch by default
    5118.9 nsec/call        0.0 user     4954.3 sys: mprotect_randloc
    1538.9 nsec/call       88.4 user     1450.6 sys: mprotect_sameloc
      10.8 usec/call      199.8 user    10571.4 sys: mmap_no_touch
  571765.9 usec/call 22855000.0 user 548756000.0 sys: mmap_and_touch
cpu  8523793 322766 3892210 28179108082 662240 0 420825 0 0 0
      12.8 usec/call      194.6 user    12130.3 sys: trigger_segfaults
  (37052 segfaults, 39210 writes, watch collision ratio)
cpu  8523795 322766 3892258 28179108833 662240 0 420829 0 0 0
     476.9 nsec/call       94.6 user      381.0 sys: madvise_randloc
    5760.9 nsec/call        0.0 user     5584.8 sys: mprotect_randloc
with touch by default
      26.3 usec/call     1100.9 user    24832.2 sys: mprotect_randloc
    1901.6 nsec/call       34.1 user     1826.9 sys: mprotect_sameloc
      13.5 usec/call        0.0 user    13086.8 sys: mmap_no_touch
  573307.0 usec/call 18374000.0 user 554773000.0 sys: mmap_and_touch
cpu  8523827 322766 3892783 28179117223 662241 0 420840 0 0 0
      28.0 usec/call      862.3 user    26784.7 sys: trigger_segfaults
  (32769 segfaults, 34991 writes, watch collision ratio)
cpu  8523831 322766 3892877 28179118696 662241 0 420843 0 0 0
    1274.0 nsec/call      101.6 user     1172.5 sys: madvise_randloc
      26.5 usec/call     1128.1 user    25068.1 sys: mprotect_randloc
All mprotect calls double:
      28.1 usec/call      822.4 user    26878.9 sys: mprotect_randloc
    4374.5 nsec/call       73.1 user     3984.5 sys: mprotect_sameloc
      13.5 usec/call      150.5 user    12961.7 sys: mmap_no_touch
  572773.9 usec/call 27866000.0 user 544265000.0 sys: mmap_and_touch
cpu  8523864 322766 3893559 28179129439 662243 0 420849 0 0 0
      29.5 usec/call     1172.9 user    27899.0 sys: trigger_segfaults
  (32769 segfaults, 34951 writes, watch collision ratio)
cpu  8523869 322766 3893657 28179130985 662243 0 420853 0 0 0
    1272.2 nsec/call      109.2 user     1162.9 sys: madvise_randloc
      28.1 usec/call     1088.3 user    26567.1 sys: mprotect_randloc
All mprotect calls 10x:
      16.5 usec/call     1013.4 user    15169.7 sys: mprotect_randloc
      12.5 usec/call      853.9 user    11271.2 sys: mprotect_sameloc
      13.5 usec/call        0.0 user    13098.8 sys: mmap_no_touch
  575168.1 usec/call 28247000.0 user 546157000.0 sys: mmap_and_touch
cpu  8523905 322766 3894329 28179141577 662245 0 420858 0 0 0
     253.0 usec/call    12168.8 user   240209.8 sys: trigger_segfaults
  (2523 segfaults, 2536 writes, watch collision ratio)
cpu  8523908 322766 3894390 28179142542 662245 0 420859 0 0 0
    1267.8 nsec/call       75.8 user     1192.1 sys: madvise_randloc
      16.5 usec/call     1250.2 user    14886.8 sys: mprotect_randloc
All mprotect calls single:
     243.2 usec/call    12166.8 user   230083.2 sys: mprotect_randloc
    1903.0 nsec/call      103.0 user     1735.5 sys: mprotect_sameloc
      13.5 usec/call      140.6 user    13006.6 sys: mmap_no_touch
  571947.1 usec/call 31660000.0 user 539459000.0 sys: mmap_and_touch
cpu  8523944 322766 3895010 28179152378 662246 0 420863 0 0 0
      28.0 usec/call     1200.6 user    26470.6 sys: trigger_segfaults
  (32769 segfaults, 35012 writes, watch collision ratio)
cpu  8523949 322766 3895103 28179153850 662246 0 420867 0 0 0
    1270.3 nsec/call      108.3 user     1161.6 sys: madvise_randloc
      26.7 usec/call     1082.3 user    25271.0 sys: mprotect_randloc
