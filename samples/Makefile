INCLUDE_PATH=$(shell pwd)/../include

utils: utils/sample_smart_ifstream.cpp
	g++ -Wall -I$(INCLUDE_PATH) utils/sample_smart_ifstream.cpp -o sample_smart_ifstream

labs: labs/sample_lexer.cpp
	g++ -Wall -I$(INCLUDE_PATH) labs/sample_lexer.cpp -o sample_lexer

clean:
	rm sample*
