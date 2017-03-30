//Used base code from  http://eric-yuan.me/k-means/ for kmean. However adapted to our need and removed dependency to libraries.

#include "kmeans.h"
float getDistance(vector<float> a, vector<float> b){
  float sum=0.0;
  for(int i=0;i<a.size();i++){
    //cout<<a[i]<<" "<<b[i]<<endl;
    sum+=pow((a[i]-b[i]),2);
  }
  return sqrt(sum);
}

int which_is_nearest(vector<vector<float> >& centroids, vector<float> pt){
    float minDistance = 0;
    int minLabel = 0;
    for(int i=0; i<centroids.size(); i++){
      float tempDistance = getDistance(centroids[i], pt);
      if(i == 0|| tempDistance < minDistance){
	minDistance = tempDistance;
	minLabel = i;
      }
    }
    return minLabel;
}

/*float getDistortionFunction(mat data, vector<vector<int> >& cluster, vector<rowvec>& centroids){

    int nSamples = data.n_rows;
    int nDim = data.n_cols;
    float SumDistortion = 0.0;
    for(int i = 0; i < cluster.size(); i++){
        for(int j = 0; j < cluster[i].size(); j++){
            float temp = getDistance(data.row(cluster[i][j]), centroids[i]);
            SumDistortion += temp;
        }
    }
    return SumDistortion;
    }*/

vector<vector<float> > kmeans(vector<vector<float> >&input_vectors, int K){
  int sampleSize = input_vectors[0].size();
  int nSamples = input_vectors.size();
  //randomly select k samples to initialize k centroid
  vector<vector<float> > centroids;
  vector<int> indexes;
  for(int i=0; indexes.size() < K;i++){
    int randInt = rand() % nSamples;
    if(find(indexes.begin(),indexes.end(),randInt)==indexes.end()){
      indexes.push_back(randInt);
      centroids.push_back(input_vectors[randInt]);
    }
  }
  //iteratively find better centroids
  vector<vector<int> > cluster;
  for(int k = 0; k < K; k ++){
    vector<int > vectemp;
    cluster.push_back(vectemp);
  }
  int counter = 0;
  while(1){
    for(int k = 0; k < K; k ++){
      cluster[k].clear();
    }
    bool converge = true;
    //for every sample, find which cluster it belongs to, 
    //by comparing the distance between it and each clusters' centroid.
    for(int m = 0; m < nSamples; m++){
      int which = which_is_nearest(centroids, input_vectors[m]);
      cluster[which].push_back(m);
    }
    //for every cluster, re-calculate its centroid.
    vector<vector<float> > centroids_temp;//(k,vector<float>(k));
    for(int k = 0; k < K; k++){
      int cluster_size = cluster[k].size();
      vector<float> tempCen(sampleSize);
      for(int i = 0; i < cluster_size; i++){
	vector<float> tempCl = input_vectors[cluster[k][i]];
	for(int j=0;j<tempCl.size();j++){
	  //centroids_temp[k][j] = (centroids_temp[k][j] + tempCl[j]) / (float)cluster_size;
	  tempCen[j] = tempCen[j] + tempCl[j]/cluster_size;
	}
	//cout<<k<<" "<<tempCen[0]<<" "<<tempCen[1]<<endl; 
      }
      //centroids_temp.push_back(tempCen);
      if(getDistance(centroids[k], tempCen) >= 1e-6) converge = false;
      centroids[k] = tempCen;
    }
    if(converge) break;
    ++counter;
    cout<<counter<<endl;
  }
  //  float distortion = getDistortionFunction(data, cluster, centroids);
  return centroids;
}

