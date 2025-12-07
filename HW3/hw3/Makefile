CXX = g++ 
CXXFLAGS = -lpthread -std=c++11

TARGET1 = study_test2
TARGET2 = study_test

SOURCE1 = study_test2.cpp Study.cpp
SOURCE2 = study_test.cpp Study.cpp

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(SOURCE1)
	$(CXX) $(SOURCE1) -o $(TARGET1) $(CXXFLAGS)

$(TARGET2): $(SOURCE2)
	$(CXX) $(SOURCE2) -o $(TARGET2) $(CXXFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGET1) $(TARGET2)