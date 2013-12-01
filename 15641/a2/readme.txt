################################################################################
# README                                                                       #
#                                                                              #
# Description: This file serves as a README and documentation for CP2          #
#              concurrent routing daemon and flaskr web server.                #
#                                                                              #
# Author: Yang Sun<yksun@cs.cmu.edu>, Fei Xie<fxie@andrew.cmu.edu>             #
#                                                                              #
################################################################################




[TOC-1] Table of Contents
--------------------------------------------------------------------------------

        [TOC-1] Table of Contents
        [DES-2] Description of Files
        [RUN-3] How to Run
        [DESIGN-4] Design ideas




[DES-2] Description of Files
--------------------------------------------------------------------------------

Here is a listing of all files and what their purpose is:

                    .../readme.txt                  - Current document 
                    .../routed.c                    - routing daemon server
                    .../routed.h                    - routing daemon server header
                    .../ospf.c                      - ospf protocol operation
                    .../ospf.h                      - ospf protocol definition
                    .../rTable.c                    - rTable operation
                    .../rTable.h                    - rTable data structure definition         
                    .../webserver.py                - flaskr web server
                    .../Makefile                    - Contains rules for make




[RUN-3] How to Run
--------------------------------------------------------------------------------
1. You should open 3 different andrew connections. As the host addresses are specified
   in 'node.conf' file, you should open unix10; unix13; unix12 machine. If you want to 
   test the program with different configuration, you can modify the 'node.conf' file
   and open the different andrew connection accordingly.

2. 'make' on one andrew machine, as your directory is the same.

3. On each andrew machine, the command line should like: 
	*** WARNING: Please make sure node.conf contains correct parameters ***
   
   ./rd <nodeID> <configuration file> <file list file> <adv cycle time> <neighbor timeout> <retry timeout> <LSA timeout>
   For example:(Start the program one by one within several seconds.)
   ./routed 1 node.conf node1.files 10 50 3 50 
   ./routed 2 node.conf node2.files 10 50 3 50
   ./routed 3 node.conf node3.files 10 50 3 50 
  
4. On each andrew machine, start the flask web server, like:(Do the same thing on each machine)
   ./python webserver.py 5001
   ./python webserver.py 5001
   ./python webserver.py 5001
   
5. If the above steps are all successful, you can open the browser and open the flaskr page, and use it.
 
If there is anything wrong during these steps, here are the possible causes:
1. Someone else is using the same machine on the same port.
2. Make sure the parameters are corresponding to the 'node.conf' file.
3. Permission issues... Please make sure all files have 755 permission.

If there is still problem, please email us. We have tested it on the andrew machines.
                    
[DESIGN-4] Design ideas
--------------------------------------------------------------------------------
Router Table (Longest prefix match implemented)
Maintain a matrix which describes the network topology known by the router.
1. Update
Every time a new LSA entered, router read the information based on the OSPF Packet Format.
If the LSA has some new information (Like new nodes, new objects etc.), update the matrix and the related 
data structure. Then recompute the shortest path to the node which is known by this router based on the 
maintained matrix. Save the according next hop (according neighbor node in the shortest path).

2. Get certain object and longest prefix match
Check if the object is saved at local.
If not, router begins to check from the nearest node. If there is no full match, record the 
longest prefix match. Return the next hop along the shortest path with the best match result.

Routing Daemon (pipeline implemented)
Routing daemon uses select IO to enable the timeout control and pipeline mechanism. It can completely 
fulfill all the project requirements. 
The regular operations that the routing daemon does are:
1. broadcast to neighbors (probably excludes some neighbor accordingly)
	condition: cycle time reaches; file list update; neighbor is down; forward LSA
2. send ACK to some neighbor
	condition: receive valid LSA from neighbor
3. send packet back to origin
	condition: sequence number is smaller than that is stored locally
4. retransmit the packet
	condition: ACK not received within retransmission timeout
	

