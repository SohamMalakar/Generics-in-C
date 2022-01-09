
CPP=g++

CPPFLAGS=-Wall -Ofast

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $^

generics: *.o
	$(CPP) $^ -o $@

clean:
	rm *.o generics
