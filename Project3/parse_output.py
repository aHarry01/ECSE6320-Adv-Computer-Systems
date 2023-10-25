# Simple parser for output data
# makes a lot of assumptions about the way the output is formatted
# but it should work for our use here

import os

folder = 'test_data'
output_csv = "parsed_test_data.csv"

f_out = open(output_csv, "w")

f_out.write("block size,percent reads,iodepth,read bandwidth(MiB/s),avg read latency(us),write bandwidth(MiB/s), avg write latency(us)\n")
 
# iterate over all files in test_data folder
for filename in os.listdir(folder):
    f = os.path.join(folder, filename)
    if os.path.isfile(f) and filename[0:4] == "Test":
        # get input parameters from filename
        name_split = (filename.split('/')[-1]).strip('.txt').split('_')
        block_size = name_split[1]
        percent_reads = name_split[2]
        iodepth = name_split[3]
        f_out.write(f"{block_size},{percent_reads},{iodepth},")

        f_data = open(f, 'r')
        data = f_data.readlines()
        read_bw = "-"
        write_bw = "-"
        read_latency = "-"
        write_latency = "-"

        for (i,line) in enumerate(data):
            if "read:" in line:
                bw_loc = line.find("BW=")
                end_bw_loc = line.find("MiB/s")
                read_bw = line[bw_loc+3:end_bw_loc]

                latency_line = data[i+3]
                latency_stats = latency_line.split(',')
                for x in latency_stats:
                    if "avg" in x:
                        read_latency = x.split("=")[1]

            elif "write:" in line:
                bw_loc = line.find("BW=")
                end_bw_loc = line.find("MiB/s")
                write_bw = line[bw_loc+3:end_bw_loc]

                latency_line = data[i+3]
                latency_stats = latency_line.split(',')
                for x in latency_stats:
                    if "avg" in x:
                        write_latency = x.split("=")[1]
        f_out.write(f"{read_bw},{read_latency},{write_bw},{write_latency}\n")
        f_data.close()

f_out.close()
