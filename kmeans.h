// http://eric-yuan.me/k-means/

#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>
#include <stdlib.h>
using namespace std;

int which_is_nearest(vector<vector<float> >& centroids, vector<float> pt);

vector<vector<float> > kmeans(vector<vector<float> >&input_vectors, int K);

