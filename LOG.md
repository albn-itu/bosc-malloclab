# Log of runs

| Implementation | short1-bal | short2-bal | All traces (Number) | All traces (util) | All traces (thru) |
| -------------- | ---------- | ---------- | ------------------- | ----------------- | ----------------- |
| Naive          | 70         | 100        | NA                  | NA                | NA                |
| The book       | 80         | 94         | 64                  | 44                | 19                |
| New realloc    | 80         | 94         | 71                  | 48                | 23                | 

## Naive
### Standard
```
> ./mdriver -Vf short1-bal.rep
Team Name:ateam
Member 1 :Harry Bovik:bovik@cs.cmu.edu
Measuring performance with gettimeofday().

Testing mm malloc
Reading tracefile: short1-bal.rep
Checking mm_malloc for correctness, efficiency, and performance.

Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   50%      12  0.000000   inf
Total          50%      12  0.000000   inf

Perf index = 30 (util) + 40 (thru) = 70/100

> ./mdriver -Vf short2-bal.rep
Team Name:ateam
Member 1 :Harry Bovik:bovik@cs.cmu.edu
Measuring performance with gettimeofday().

Testing mm malloc
Reading tracefile: short2-bal.rep
Checking mm_malloc for correctness, efficiency, and performance.

Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes  100%      12  0.000000   inf
Total         100%      12  0.000000   inf

Perf index = 60 (util) + 40 (thru) = 100/100
```

### All
NA

## The book
### Standard
```
> ./mdriver -Vf short1-bal.rep
Team Name:the goose
Member 1 :Albert Rise Nielsen:albn@itu.dk
Measuring performance with gettimeofday().

Testing mm malloc
Reading tracefile: short1-bal.rep
Checking mm_malloc for correctness, efficiency, and performance.

Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   66%      12  0.000000 60000
Total          66%      12  0.000000 60000

Perf index = 40 (util) + 40 (thru) = 80/100

> ./mdriver -Vf short2-bal.rep
Team Name:the goose
Member 1 :Albert Rise Nielsen:albn@itu.dk
Measuring performance with gettimeofday().

Testing mm malloc
Reading tracefile: short2-bal.rep
Checking mm_malloc for correctness, efficiency, and performance.

Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   89%      12  0.000000 60000
Total          89%      12  0.000000 60000

Perf index = 54 (util) + 40 (thru) = 94/100
```

### All
```
trace  valid  util     ops      secs  Kops
 0       yes   99%    5694  0.005890   967
 1       yes   99%    5848  0.005470  1069
 2       yes   99%    6648  0.009113   730
 3       yes  100%    5380  0.006733   799
 4       yes   66%   14400  0.000081176904
 5       yes   91%    4800  0.005095   942
 6       yes   92%    4800  0.004854   989
 7       yes   55%   12000  0.120649    99
 8       yes   51%   24000  0.234658   102
 9       yes   27%   14401  0.066810   216
10       yes   34%   14401  0.001936  7437
Total          74%  112372  0.461290   244

Perf index = 44 (util) + 19 (thru) = 64/100
```

## New realloc
### Standard
```
> ./mdriver -Vf short1-bal.rep
Team Name:the goose
Member 1 :Albert Rise Nielsen:albn@itu.dk
Measuring performance with gettimeofday().

Testing mm malloc
Reading tracefile: short1-bal.rep
Checking mm_malloc for correctness, efficiency, and performance.

Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   66%      12  0.000001  8571
Total          66%      12  0.000001  8571

Perf index = 40 (util) + 40 (thru) = 80/100

> ./mdriver -Vf short2-bal.rep
Team Name:the goose
Member 1 :Albert Rise Nielsen:albn@itu.dk
Measuring performance with gettimeofday().

Testing mm malloc
Reading tracefile: short2-bal.rep
Checking mm_malloc for correctness, efficiency, and performance.

Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   89%      12  0.000001 12000
Total          89%      12  0.000001 12000

Perf index = 54 (util) + 40 (thru) = 94/100
```

### All
```
trace  valid  util     ops      secs  Kops
 0       yes   99%    5694  0.006011   947
 1       yes   99%    5848  0.005391  1085
 2       yes   99%    6648  0.009291   716
 3       yes  100%    5380  0.006770   795
 4       yes   66%   14400  0.000082176471
 5       yes   91%    4800  0.005150   932
 6       yes   92%    4800  0.004926   974
 7       yes   55%   12000  0.122449    98
 8       yes   51%   24000  0.235002   102
 9       yes   80%   14401  0.000131109597
10       yes   46%   14401  0.000077187269
Total          80%  112372  0.395281   284

Perf index = 48 (util) + 23 (thru) = 71/100
```
