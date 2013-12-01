################################################################################
# README                                                                       #
#                                                                              #
# Description: This file serves as a README and documentation for CP1          #
#              WHOHAS flooding and IHAVE responses and simple chunk            #
#              download with stop and wait.                                    #
#                                                                              #
# Author: Yang Sun<yksun@cs.cmu.edu>, Fei Xie<fxie@andrew.cmu.edu>             #
#                                                                              #
################################################################################




[TOC-1] Table of Contents
--------------------------------------------------------------------------------

        [TOC-1] Table of Contents
        [DES-2] Description of Files
        [RUN-3] How to Run



[DES-2] Description of Files
--------------------------------------------------------------------------------

Here is a listing of all files and what their purpose is:

                    .../readme.txt                  - Current document 
                    .../peer.c                      - bt server implementation
                    .../bt_parse.h                  - definition of data and package
              					      structures used in implementation
                    .../bt_parse.c                  - tool functions
                    .../Makefile                    - Contains rules for make


[RUN-3] How to Run
--------------------------------------------------------------------------------
1. 'make' on one andrew machine.

2. Set up the simulator router.

3. Open three peer program in the same andrew machine.

3. On each andrew machine, the command line should like: 
       ./peer -p ./nodes.map -c ./example/A.haschunks -f ./example/C.masterchunks -m 4 -i 1
       ./peer -p ./nodes.map -c ./example/B.haschunks -f ./example/C.masterchunks -m 4 -i 2  
       ./peer -p ./nodes.map -c ./example/B.haschunks -f ./example/C.masterchunks -m 4 -i 3  
 
4. Then, enter the following command in 1
     GET example/B.chunks example/newB.tar

5. The 2 and 3 will send according chunks to 1.

6. Diff newB.tar B.tar

[DESING-4] Design
--------------------------------------------------------------------------------
Reliable transmit
We implemented duplicate ACK protection and timeout mechanism to ensure the reliable data transfer.
If more than one peer can provide the data, we will split the tasks fairly so that the download will
be concurrent. The timeout mechanism works for both GET and DATA type of transactions. If one data
provider is down while transferring, the requester will broadcast WHOHAS again.

Congestion control
The congestion control is based on the current network condition, which is indicated by
 the ack packages. The implementation of congestion control separate the whole process 
into two phases -- Slow Start and Congestion Avoidance. So the congestion control is integrated with the transmit part. In ACK processing part, the window size will increase if the current condition is slow start. If the current condition is congestion avoidance, the window size will changed in RTT time. A rtt recorder is applied here to record  the RTT time. The rtt recorder is to record the last package number when several packages were sent. When the ack for the package in record is received, it means it's time of a  rtt.
Then the window size will increase one accordingly.