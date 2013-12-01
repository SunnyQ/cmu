all:	
	javac cmu/edu/ds/a2/rmi/*.java cmu/edu/ds/a2/proxy/*.java cmu/edu/ds/a2/test/*.java

demo: all
	@echo "Running Registry Server on port 4000...\n"
	java cmu.edu.ds.a2.rmi.RMIRegistry 4000 &
	sleep 1
	@echo "Running Proxy Dispatcher on port 4001...\n"
	@echo "Connecting Registry Server on localhost, port 4000...\n"
	java cmu.edu.ds.a2.rmi.RMIProxyDispatcher -p 4001 -c localhost -rp 4000 -classes cmu.edu.ds.a2.test.HelloWorldImpl:HelloWorld\;cmu.edu.ds.a2.test.HelloInconsistencyImpl:HelloInconsistency\;cmu.edu.ds.a2.test.HelloTimeoutImpl:HelloTimeout\;cmu.edu.ds.a2.test.HelloWorldNoArgsImpl:HelloWorldNoArgs &
	sleep 1
	@echo "Test program running...Looking for Registry Server on localhost, port 4000\n"
	java cmu.edu.ds.a2.test.HelloWorldClient 

clean:
	rm cmu/edu/ds/a2/proxy/*.class cmu/edu/ds/a2/rmi/*.class cmu/edu/ds/a2/test/*.class
