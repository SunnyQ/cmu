all:	
	javac com/thoughtworks/salestaxes/checkout/*.java com/thoughtworks/salestaxes/io/*.java com/thoughtworks/salestaxes/objects/*.java com/thoughtworks/salestaxes/tax/*.java

demo: all
	@echo "Running OrderProcessor\n"
	java com.thoughtworks.salestaxes.io.OrderProcessor

clean:
	rm com/thoughtworks/salestaxes/checkout/*.class com/thoughtworks/salestaxes/io/*.class com/thoughtworks/salestaxes/objects/*.class com/thoughtworks/salestaxes/tax/*.class
