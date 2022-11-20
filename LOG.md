# Log of runs

| Implementation | short1-bal | short2-bal | All traces (Number) | All traces (util) | All traces (thru) |
| -------------- | ---------- | ---------- | ------------------- | ----------------- | ----------------- |
| Naive          | 70         | 100        | NA                  | NA                | NA                |
| The book       | 80         | 94         | 
| New realloc    | 80         | 94         | 71                  | 48                | 23                | 

## Naive
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

## The book
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

## New realloc
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
