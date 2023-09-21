# Project 1

For questions 1-3, we used Intel Memory Latency Checker (mlc) to gather data (see mlc_tests.bat for the commands used).

### 1.) Latency with Zero Queueing Delay

To measure memory latency with no queueing delay, we used the --idle_latency option of mlc, which measures the read latency with no contention from other threads accessing memory. We can change the buffer size to indirectly change whether it's measuring cache or main memory latency. The graph shows the measured idle read latencies for various buffer sizes.  

![alt text](images/idle_latency.png)

The computer on which we gathered data has 8MB of L3 cache, 3MB of L2 and 384 KB of L1 cache. Latency begins to fall at around 4-8MB buffer size as the buffer becomes smaller than the cache. At the smallest buffer size of 0.25MiB, the entire buffer can fit in L1 cache so we can assume that this latency is the cache latency. Similarly, the latency at large buffer sizes is approximately the latency of main memory because only a very small percentage of the memory accesses are able to be from cache.

Therefore, L1 cache read latency is about 3ns (measured with a 0.25MiB buffer) and main memory read latency is about 109.4ns (measured with a 4096MiB buffer).

TODO: estimate write latency 


### 2.) Maximum Bandwidth

The maximum bandwidth was measured with the --max_bandwidth option of mlc which measures max bandwidth with different ratios of reads/writes (1:1, 2:1, 3:1, and all reads). The bandwidth consistently decreases with a greater proportion of writes, indicating that writes are slower than reads.

TODO: why is writing slower?

![alt text](images/bandwidth_rw.png)

The maximum bandwidth was also measured with different data access granularities, which was adjusted with the -l stride size option. This data was gathered using all reads. The graph shows that the bandwidth is highest with a 64 byte stride size and drops as stride size increases

TODO: why?

![alt text](images/bandwidth_stride_size.png)


### 3.) Latency vs Throughput

The latency was measured under different load conditions by using the --loaded_latency option of mlc. This option generates lots of memory traffic with different threads and then measures read latency.

We found that the latency increases with the bandwidth. Below 10,000MiB/s, the latency remains approximately constant at around 115-120ns, which is very close to the idle latency of 109ns (measured in part 1). Between 10,000-25,000MiB/s, the latency increases only very slightly. However, as we approach maximum memory bandwidth, the latency increases sharply. The maximum latency was about 470ns at 33,681MiB/s.

This aligns with what the queueing theory predicts. With higher bandwidths the memory server is almost never idle, so the server utilization is high. However, this increases the queue length, which results in a higher latency as each memory access request needs to wait in that longer queue.

![alt text](images/latency_bandwidth.png)


### 4.)

### 5.)
