REGISTER proj.jar;
prev = 
	LOAD 'output/prev' 
	USING PigStorage()
	AS (src: CHARARRAY, dest: CHARARRAY, s1: DOUBLE, s2: DOUBLE, s3: DOUBLE);
	
priors = 
	LOAD 'input/prior' 
	USING PigStorage() 
	AS (node: CHARARRAY, s1: DOUBLE, s2: DOUBLE, s3: DOUBLE);
	
/* for now, just do once... the dataset might be wrong */
m_update = 
	FOREACH 
		(JOIN (GROUP prev by $0) by group, priors by node) 
	GENERATE 
		FLATTEN(myUDF.MessageUpdate(*));
	
m_update = 
	FOREACH 
		(JOIN (GROUP m_update by $0) by group, priors by node) 
	GENERATE 
		FLATTEN(myUDF.MessageUpdate(*));
STORE m_update INTO 'output/updated';

result = 
	FOREACH
		(JOIN prev BY ($0, $1), m_update BY ($0, $1))
	GENERATE
		FLATTEN(myUDF.MessageConverge(*));

--dump m_update;
STORE result INTO 'output/finishFlag';
