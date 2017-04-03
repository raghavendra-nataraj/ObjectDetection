all: CImg.h a3.cpp kmeans.cpp kmeans.h Classifier.h NearestNeighbor.h Baseline.h Haarlike.h Eigen.h
	g++ -g a3.cpp kmeans.cpp -o a3 -lX11 -lpthread -I. -Isiftpp -O3 siftpp/sift.cpp

clean:
	rm a3
