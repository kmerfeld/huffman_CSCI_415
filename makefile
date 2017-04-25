all: CreateFile HuffmanCoding

CreateFile: CreateFile.cpp
	g++ -o CreateFile CreateFile.cpp 

HuffmanCoding: HuffmanCoding.cpp
	g++ -o HuffmanCoding HuffmanCoding.cpp -lpthread


