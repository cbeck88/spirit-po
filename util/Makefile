BOOST_DIR ?= /usr/include

CXXFLAGS := $(CXXFLAGS) -std=c++11
INCLUDES := -isystem../include/ -isystem$(BOOST_DIR)

inspector: inspector.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f inspector

check:
	./inspector

test:
	./inspector "(n % 5 > 3) ? 1 : 0"
	./inspector "(n == 4) ? 5 : (n < 2) ? 2 : 3"
	./inspector "!(n % 6 == 2) && (n % 6 == 1) || (n % 5 == 1)"
	./inspector "(n==1?0:(((n%100>19)||((n%100==0)&&(n!=0)))?2:1))" #romanian
	./inspector "(n==0 ? 0 : n==1 ? 1 : n==2 ? 2 : n%100>=3 && n%100<=10 ? 3 : n%100>=11 ? 4 : 5)" #arabic
	./inspector "(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2)" #belarusian
	./inspector "(n==1) ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2" #kashubian
	./inspector "(n%100==1 ? 1 : n%100==2 ? 2 : n%100==3 || n%100==4 ? 3 : 0)" #slovenian
	./inspector "(n==1 || n==11) ? 0 : (n==2 || n==12) ? 1 : (n > 2 && n < 20) ? 2 : 3" #scottish gaelic
