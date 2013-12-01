REGISTER proj.jar;
edges = 
	LOAD 'input/edge' 
	USING PigStorage() 
	AS (src: CHARARRAY, dest: CHARARRAY);
	
priors = 
	LOAD 'input/prior' 
	USING PigStorage() 
	AS (node: CHARARRAY, s1: DOUBLE, s2: DOUBLE, s3: DOUBLE);
	
/* Initialize the message matrix */
init_m = 
	FOREACH 
		(JOIN edges by src, priors by node) 
	GENERATE 
		FLATTEN(myUDF.MessageMatrix(*));
		
--dump init_m;
STORE init_m INTO 'output/prev';
