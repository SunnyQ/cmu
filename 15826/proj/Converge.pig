REGISTER proj.jar;
prev = 
	LOAD 'output/prev' 
	USING PigStorage()
	AS (src: CHARARRAY, dest: CHARARRAY, s1: DOUBLE, s2: DOUBLE, s3: DOUBLE);
	
updated = 
	LOAD 'output/updated' 
	USING PigStorage()
	AS (src: CHARARRAY, dest: CHARARRAY, s1: DOUBLE, s2: DOUBLE, s3: DOUBLE);
	
result = 
	FOREACH
		(JOIN prev BY (src, dest), updated BY (src, dest))
	GENERATE
		myUDF.MessageConverge(*);
		
STORE result INTO 'output/compare';
