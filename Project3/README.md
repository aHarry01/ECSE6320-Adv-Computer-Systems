# Project 3

We tested combinations of different block sizes (4KB, 16KB, 32KB and 128KB), read-write ratios (0%, 50%, 70%, and 100% reads), and I/O depths (1, 32, 256, and 1024). For each combination, fio provides the read latency, write latency, read throughput and write throughput. All of the output data is in parsed_test_data.xls. 

## Data Access Size

We used the -bs option to change the data access size. The graphs show how latency and throughput change with data access size. These graphs have iodepth fixed at 1 and % reads fixed at 50%.

The latency increases approximately linearly with block size. The latency is measured per-I/O access so it makes sense that a larger block size takes longer to read from memory. 

<p align="center"> <img src="graphs/latency_blocksize.PNG" alt="drawing" width="75%"/> </p>

The throughput graph shows that throughput increases with block size and eventually levels off. This shows that there is a performance improvement associated with using larger I/O accesses. This could be due to less overhead in making requests. For example, with the 4KiB block size it has to make 32 times as many read requests as it would with the 128KiB block size to read the same amount of data. 

<p align="center"> <img src="graphs/throughput_blocksize.PNG" alt="drawing" width="75%"/> </p>

## Read vs Write Intensity Ratio

Next we used the -rw and the -rwmixread options to change the percentage of memory accesses that were reads. The graphs show the throughput and latency vs percentage reads while holding block size fixed at 16KiB and iodepth fixed at 1.

The read and write latency are fairly constant compared to the percentage reads and are fairly close to each other.

<p align="center"> <img src="graphs/latency_percentreads.PNG" alt="drawing" width="75%"/> </p>

Looking at the throughput, it looks like fio maintains a constant total throughput despite the mixture of reads and writes. This leads to a lower read throughput and a lower write throughput when the accesses are a mix between reads and writes, but when added together they still amount to a similar overall total throughput. Overall performance was pretty constant for all cases.

<p align="center"> <img src="graphs/throughput_percentreads.PNG" alt="drawing" width="75%"/> </p>

## I/O Queue Depth

Last we used the -iodepth option with 1, 32, 256 or 1024 to see how the performance changes with different queue lengths. The graphs are for a 4KiB block size.

The latency increased with a larger I/O depth, as the queueing theory predicts. When there's a larger queue, each individual read request has to wait longer to be carried out.

<p align="center"> <img src="graphs/latency_iodepth.PNG" alt="drawing" width="75%"/> </p>

However, the bandwidth doesn't align with what queueing theory would predict. We would expect that with a larger queue, the bandwidth would increase. However, our results show the largest bandwidth at iodepth = 1 and a fairly constant bandwidth for other iodepths. It's possible that the I/O depth wasn't high enough to be show significant results or that fio was implemented in a way that offsets the performance gains that should be seen by allowing more I/O requests to queue up.

<p align="center"> <img src="graphs/throughput_iodepth.PNG" alt="drawing" width="75%"/> </p>

## Conclusion

To summarize our results, we noticed that the latency increases with a larger I/O depth and was mostly unaffected by read/write ratio. For bandwidth, we found that it increases with a larger data access size and also seems unaffected by read/write ratio. We had some unexpected behavior for throughput vs I/O queue depth, but theoretically it should increase as I/O depth increases.

Our random write-only performance with a 4KB block size was 29k IOPS, which isn't as fast as the Intel Data Center NVMe SSD D7-P5600. But if it was faster, then it would probably be because the enterprise-grade SSD is optimized for reliability, endurance, or latency rather than focusing purely on throughput. Also, one thing to notice here is that we used 10 threads for testing, so the result is about 10 times larger than using only 1 thread. We assume that IOPS can be larger if we use more threads.
