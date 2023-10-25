

import os
import time
if __name__ == '__main__':
    DataAccessSize = [4, 16, 32, 128]
    RWRatio = [0, 50, 70, 100]
    IODepth = [1, 32, 256, 1024]
    with open("cmdLines.bat", "w") as f:
        for size in DataAccessSize:
            for ratio in RWRatio:
                for depth in IODepth:
                    if ratio == 0:
                        rwcmd = "write"
                    elif ratio == 100:
                        rwcmd = "read"
                    else:
                        rwcmd = "randrw --rwmixread={0}".format(ratio)
                    
                    cmd = "fio --name=Test_{0}_{1}_{2} --filename=F: --direct=1 --rw={3} --bs={4}k --iodepth={5} --size=1g --ioengine=windowsaio --thread --numjobs=10 --group_reporting > Test_{0}_{1}_{2}.txt\n" \
                            .format(size, ratio, depth, rwcmd, size, depth)
                    f.writelines(cmd)