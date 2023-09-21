:: Batch file to run Intel Memory Latency Checker commands on a Windows machine

@echo off
:: Replace with the path to your mlc.exe
SET PATH=%PATH%;C:\Users\harrya\Documents\ECSE 6320 Adv Computer Systems\mlc_v3.10\Windows

echo "----- IDLE LATENCY TEST -----"
:: Measure idle latency with different buffer sizes 
mlc --idle_latency -b4g -U -r -x10
mlc --idle_latency -b2g -U -r -x10
mlc --idle_latency -b512m -U -r -x10
mlc --idle_latency -b128m -U -r -x10
mlc --idle_latency -b16m -U -r -x10
mlc --idle_latency -b8m -U -r -x10
mlc --idle_latency -b4m -U -r -x10
mlc --idle_latency -b2m -U -r -x10
mlc --idle_latency -b512k -U -r -x10
mlc --idle_latency -b256k -U -r -x10

:: Get histogram of memory read latencies
mlc --idle_latency -b4m -U -r -x10 -f3

echo "----- MAX BANDWIDTH UNDER DIFFERENT R/W RATIOS -----"
mlc --max_bandwidth -b8m

echo "----- BANDWIDTH UNDER DIFFERENT DATA ACCESS GRANULARITY -----"
mlc --max_bandwidth -l64 -b128m
mlc --max_bandwidth -l128 -b128m
mlc --max_bandwidth -l256 -b128m
mlc --max_bandwidth -l512 -b128m
mlc --max_bandwidth -l1024 -b128m

echo "----- BANDWIDTH vs LATENCY -----"
mlc --loaded_latency -gdelay_values.txt