REGISTER proj.jar;
m_update = 
	LOAD 'output/updated' 
	USING PigStorage()
	AS (src: CHARARRAY, dest: CHARARRAY, s1: DOUBLE, s2: DOUBLE, s3: DOUBLE);

priors = 
	LOAD 'input/prior' 
	USING PigStorage() 
	AS (node: CHARARRAY, s1: DOUBLE, s2: DOUBLE, s3: DOUBLE);

/* calculate the last step - belief propagation */
belief_compute = 
	FOREACH 
		(JOIN (GROUP m_update by $0) by group, priors by node) 
	GENERATE 
		FLATTEN(myUDF.BeliefComputation(*));

--dump belief_compute;
STORE belief_compute INTO 'output/bp';
