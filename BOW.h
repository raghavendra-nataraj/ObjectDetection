#include <fstream>
#include <map>
#include <sstream>
#include "kmeans.h"
using namespace std;
class BOW : public Classifier
{
public:
  BOW(const vector<string> &_class_list) : Classifier(_class_list) {}
  
  // Nearest neighbor training. All this does is read in all the images, resize
  // them to a common size, convert to greyscale, and dump them as vectors to a file
  virtual void train(const Dataset &filenames) 
  {
    vector<vector<float> > points;
    for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter){
      cout << "Processing " << c_iter->first << endl;
      // convert each image to be a row of this "model" image
      for(int i=0; i<c_iter->second.size(); i++){
	cout<<i<<endl;
	vector<SiftDescriptor> features = extract_features(c_iter->second[i].c_str());
	for(int k=0;k<features.size();k++){
	  points.push_back(features[k].descriptor);
	}	
      }
    }
    centroids = kmeans(points,kmeansk);
    cout<<"Kmeans Over"<<endl;
    int imgIndex=1;
    ofstream inpFile;
    inpFile.open("Input_file.dat");
    for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter){
      // convert each image to be a row of this "model" image
      cout << "Processing " << c_iter->first << endl;
      for(int i=0; i<c_iter->second.size(); i++){
	vector<int> histogram(kmeansk,0);
	cout<<i<<endl;
	vector<SiftDescriptor> features = extract_features(c_iter->second[i].c_str());
	for(int k=0;k<features.size();k++){
	  int which = which_is_nearest(centroids,features[k].descriptor);
	  //cout<<which<<endl;
	  histogram[which]+=1;
	}
	inpFile<<imgIndex<<" ";
	for(int it=1;it<kmeansk+1;it++){
	  inpFile<<it<<":"<<histogram[it]<<" ";
	}
	inpFile<<endl;
      }
      imgIndex++;
    }
    inpFile.close();
    ofstream cntFile;
    cntFile.open("Centroids.dat");
    for(int i=0;i<centroids.size();i++){
      vector<float> cnt = centroids[i];
      for(int j=0;j<cnt.size();j++){
	cntFile<<cnt[j]<<" ";
      }
      cntFile<<endl;
    }
    cntFile.close();
    int result = system("svm_multiclass_linux64/svm_multiclass_learn -c 1.0 Input_file.dat food.model");
    cout<<result<<endl;
  }

  virtual string classify(const string &filename)
  {
    ofstream inpFile;
    inpFile.open("Temp_file.dat");	      
    // figure nearest neighbor
    vector<int> histogram(kmeansk,0);
    vector<SiftDescriptor> features = extract_features(filename.c_str());
    for(int k=0;k<features.size();k++){
      int which = which_is_nearest(centroids,features[k].descriptor);
      //cout<<which<<endl;
      histogram[which]+=1;
    }
    inpFile<<"1"<<" ";
    for(int it=1;it<kmeansk+1;it++){
      inpFile<<it<<":"<<histogram[it]<<" ";
    }
    inpFile<<endl;
    inpFile.close();
    int result = system("svm_multiclass_linux64/svm_multiclass_classify Temp_file.dat food.model predictions");
    ifstream pFile;
    pFile.open("predictions");
    int prediction;
    pFile>>prediction;
    return class_list[prediction-1];
  }

  
  virtual void load_model()
  {
    ifstream inpFile;
    inpFile.open("Centroids.dat");
    string line;
    while ( std::getline (inpFile,line) )
    {
      vector<float> temp;
      istringstream lines(line);
      for(int i=0;i<128;i++){
	float value;
	lines>>value;
	temp.push_back(value);
      }
      centroids.push_back(temp);
    }
   inpFile.close();
  }
  
protected:
  // extract features from an image, which in this case just involves resampling and 
  // rearranging into a vector of pixel data.
  vector<SiftDescriptor> extract_features(const string &filename)
    {
      // Used few lines for extracting SIFT code from our Assignemtn 2
      CImg<double> input_image(filename.c_str());
      CImg<double> gImg = input_image.get_RGBtoHSI().get_channel(2);
      vector<SiftDescriptor> img = Sift::compute_sift(gImg);
      return img;
    }
  map<string, CImg<double> > models; // trained models
  static const int kmeansk = 50;
  vector<vector<float> > centroids;
};
