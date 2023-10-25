fio --name=Test_4_0_1 --filename=F: --direct=1 --rw=write --bs=4k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_0_1.txt
fio --name=Test_4_0_32 --filename=F: --direct=1 --rw=write --bs=4k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_0_32.txt
fio --name=Test_4_0_256 --filename=F: --direct=1 --rw=write --bs=4k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_0_256.txt
fio --name=Test_4_0_1024 --filename=F: --direct=1 --rw=write --bs=4k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_0_1024.txt
fio --name=Test_4_50_1 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=4k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_50_1.txt
fio --name=Test_4_50_32 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=4k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_50_32.txt
fio --name=Test_4_50_256 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=4k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_50_256.txt
fio --name=Test_4_50_1024 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=4k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_50_1024.txt
fio --name=Test_4_70_1 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=4k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_70_1.txt
fio --name=Test_4_70_32 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=4k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_70_32.txt
fio --name=Test_4_70_256 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=4k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_70_256.txt
fio --name=Test_4_70_1024 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=4k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_70_1024.txt
fio --name=Test_4_100_1 --filename=F: --direct=1 --rw=read --bs=4k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_100_1.txt
fio --name=Test_4_100_32 --filename=F: --direct=1 --rw=read --bs=4k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_100_32.txt
fio --name=Test_4_100_256 --filename=F: --direct=1 --rw=read --bs=4k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_100_256.txt
fio --name=Test_4_100_1024 --filename=F: --direct=1 --rw=read --bs=4k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_4_100_1024.txt
fio --name=Test_16_0_1 --filename=F: --direct=1 --rw=write --bs=16k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_0_1.txt
fio --name=Test_16_0_32 --filename=F: --direct=1 --rw=write --bs=16k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_0_32.txt
fio --name=Test_16_0_256 --filename=F: --direct=1 --rw=write --bs=16k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_0_256.txt
fio --name=Test_16_0_1024 --filename=F: --direct=1 --rw=write --bs=16k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_0_1024.txt
fio --name=Test_16_50_1 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=16k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_50_1.txt
fio --name=Test_16_50_32 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=16k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_50_32.txt
fio --name=Test_16_50_256 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=16k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_50_256.txt
fio --name=Test_16_50_1024 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=16k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_50_1024.txt
fio --name=Test_16_70_1 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=16k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_70_1.txt
fio --name=Test_16_70_32 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=16k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_70_32.txt
fio --name=Test_16_70_256 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=16k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_70_256.txt
fio --name=Test_16_70_1024 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=16k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_70_1024.txt
fio --name=Test_16_100_1 --filename=F: --direct=1 --rw=read --bs=16k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_100_1.txt
fio --name=Test_16_100_32 --filename=F: --direct=1 --rw=read --bs=16k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_100_32.txt
fio --name=Test_16_100_256 --filename=F: --direct=1 --rw=read --bs=16k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_100_256.txt
fio --name=Test_16_100_1024 --filename=F: --direct=1 --rw=read --bs=16k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_16_100_1024.txt
fio --name=Test_32_0_1 --filename=F: --direct=1 --rw=write --bs=32k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_0_1.txt
fio --name=Test_32_0_32 --filename=F: --direct=1 --rw=write --bs=32k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_0_32.txt
fio --name=Test_32_0_256 --filename=F: --direct=1 --rw=write --bs=32k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_0_256.txt
fio --name=Test_32_0_1024 --filename=F: --direct=1 --rw=write --bs=32k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_0_1024.txt
fio --name=Test_32_50_1 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=32k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_50_1.txt
fio --name=Test_32_50_32 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=32k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_50_32.txt
fio --name=Test_32_50_256 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=32k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_50_256.txt
fio --name=Test_32_50_1024 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=32k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_50_1024.txt
fio --name=Test_32_70_1 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=32k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_70_1.txt
fio --name=Test_32_70_32 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=32k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_70_32.txt
fio --name=Test_32_70_256 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=32k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_70_256.txt
fio --name=Test_32_70_1024 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=32k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_70_1024.txt
fio --name=Test_32_100_1 --filename=F: --direct=1 --rw=read --bs=32k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_100_1.txt
fio --name=Test_32_100_32 --filename=F: --direct=1 --rw=read --bs=32k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_100_32.txt
fio --name=Test_32_100_256 --filename=F: --direct=1 --rw=read --bs=32k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_100_256.txt
fio --name=Test_32_100_1024 --filename=F: --direct=1 --rw=read --bs=32k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_32_100_1024.txt
fio --name=Test_128_0_1 --filename=F: --direct=1 --rw=write --bs=128k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_0_1.txt
fio --name=Test_128_0_32 --filename=F: --direct=1 --rw=write --bs=128k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_0_32.txt
fio --name=Test_128_0_256 --filename=F: --direct=1 --rw=write --bs=128k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_0_256.txt
fio --name=Test_128_0_1024 --filename=F: --direct=1 --rw=write --bs=128k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_0_1024.txt
fio --name=Test_128_50_1 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=128k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_50_1.txt
fio --name=Test_128_50_32 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=128k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_50_32.txt
fio --name=Test_128_50_256 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=128k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_50_256.txt
fio --name=Test_128_50_1024 --filename=F: --direct=1 --rw=randrw --rwmixread=50 --bs=128k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_50_1024.txt
fio --name=Test_128_70_1 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=128k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_70_1.txt
fio --name=Test_128_70_32 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=128k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_70_32.txt
fio --name=Test_128_70_256 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=128k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_70_256.txt
fio --name=Test_128_70_1024 --filename=F: --direct=1 --rw=randrw --rwmixread=70 --bs=128k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_70_1024.txt
fio --name=Test_128_100_1 --filename=F: --direct=1 --rw=read --bs=128k --iodepth=1 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_100_1.txt
fio --name=Test_128_100_32 --filename=F: --direct=1 --rw=read --bs=128k --iodepth=32 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_100_32.txt
fio --name=Test_128_100_256 --filename=F: --direct=1 --rw=read --bs=128k --iodepth=256 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_100_256.txt
fio --name=Test_128_100_1024 --filename=F: --direct=1 --rw=read --bs=128k --iodepth=1024 --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_128_100_1024.txt
