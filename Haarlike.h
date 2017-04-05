#include <fstream>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
struct haar{
  int width;
  int height;
  string pattern;
  map<pair<int,int>,char> region;
};

class Haarlike : public Classifier
{
public:
  Haarlike(const vector<string> &_class_list) : Classifier(_class_list) {}
  
  // Nearest neighbor training. All this does is read in all the images, resize
  // them to a common size, convert to greyscale, and dump them as vectors to a file
  virtual void train(const Dataset &filenames) 
  {
    loadStrides();
    ofstream inpFile;
    int imgIndex=1;
    inpFile.open("Input_file.dat");
    for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
      {
	cout << "Processing " << c_iter->first << endl;
	// convert each image to be a row of this "model" image
	for(int i=0; i<c_iter->second.size(); i++){
	  cout<<i<<endl;
	  map<long,double> temp = extract_features(c_iter->second[i].c_str());
	  map<long,double>::iterator it;
	  inpFile<<imgIndex<<" ";
	  int k=1;
	  for(it=temp.begin();it!=temp.end();it++){
	    inpFile<<k<<":"<<it->second<<" ";
	    k++;
	  }
	  inpFile<<endl;
	}
	imgIndex++;
      }
    inpFile.close();
    int result = system("./svm_multiclass_learn -c 1.0 Input_file.dat food.model");
    cout<<result<<endl;
  }

  virtual string classify(const string &filename)
  {
    loadStrides();
    ofstream inpFile;
    inpFile.open("Temp_file.dat");	      
    // figure nearest neighbor
    map<long,double> temp = extract_features(filename.c_str());
    inpFile<<"1 ";
    map<long,double>::iterator it;
    int k=1;
    for(it=temp.begin();it!=temp.end();it++){
      inpFile<<k<<":"<<it->second<<" ";
      k++;
    }
    inpFile<<endl;
    inpFile.close();
    int result = system("./svm_multiclass_classify Temp_file.dat food.model predictions");
    ifstream pFile;
    pFile.open("predictions");
    int prediction;
    pFile>>prediction;
    return class_list[prediction-1];
  }

  void loadStrides(){
    int num=10;
    int fType = 12;
    //haar * strides =(haar*) malloc(sizeof(haar)*num*fType);
    vector<haar> strides;
    for(int w=60;w<=100;w+=50){
      for(int h=60;h<=100;h+=50){
	haar t1 = {w,h,"b,w,w,b"};
	strides.push_back(t1);
	haar t2 = {w,h,"b,w;w,b"};
	strides.push_back(t2);
	haar t3 = {w,h,"w,b;b,w"};
	strides.push_back(t3);
	haar t4 = {w,h,"w,b,b,w"};
	strides.push_back(t4);
	haar t5 = {w,h,"b,w"};
	strides.push_back(t5);
	haar t6 = {w,h,"w,b"};
	strides.push_back(t6);
	/*haar t7 = {w,h,"b;w"};
	strides.push_back(t7);
	haar t8 = {w,h,"w;b"};
	strides.push_back(t8);
	haar t9 = {w,h,"b,w,b"};
	strides.push_back(t9);
	haar t10 = {w,h,"w,b,w"};
	strides.push_back(t10);
	haar t11 = {w,h,"b;w;b"};
	strides.push_back(t11);
	haar t12 = {w,h,"w;b;w"};
	strides.push_back(t12);*/
      }
    }
    /*haar strides[] = {
      {50,20,"b,w,w,b"},
      {50,20,"b,w;w,b"},
      {50,20,"w,b;b,w"},
      {50,20,"w,b,b,w"},
      {50,20,"b,w"},
      {50,20,"w,b"},
      {50,20,"b;w"},
      {50,20,"w;b"},
      {50,20,"b,w,b"},
      {50,20,"w,b,w"},
      {50,20,"b;w;b"},
      {50,20,"w;b;w"} 
      };*/
    for(int i=0;i<strides.size();i++){
      int x=1,y=1;
      for(int j=0;j<strides[i].pattern.length();j++){
	if(strides[i].pattern[j]==','){ y++;continue;}
	if(strides[i].pattern[j]==';'){ x++;y=1;continue;}
	pair<int,int> key;
	key = make_pair(x,y);
	strides[i].region[key] = strides[i].pattern[j];
      }
      regions.push_back(strides[i]);
    }
  }
  
  virtual void load_model()
  {
    
  }
  
protected:
  // extract features from an image, which in this case just involves resampling and 
  // rearranging into a vector of pixel data.
  map<long,double> extract_features(const string &filename)
    {
      CImg<double> temp(filename.c_str());
      temp = temp.get_RGBtoHSI().get_channel(2).resize(size,size);
      for(int i=0;i<temp.width();i++){
	for(int j =1;j<temp.height();j++){
	  temp(i,j) = temp(i,j-1)+temp(i,j);
	}
      }
      for(int j=0;j<temp.height();j++){
	for(int i =1;i<temp.width();i++){
	  temp(i,j) = temp(i-1,j)+temp(i,j);
	}
      }
      map<long,double> rVector;
      for(int i=0;i<regions.size();i++){
	haar tempr = regions[i];
	for(int w=0;w<temp.width();w+=tempr.width){
	  for(int h=0;h<temp.height();h+=tempr.height){
	    double sum=0.0;
	    map<pair<int,int>,char>::iterator it;
	    for(it=regions[i].region.begin();it!=regions[i].region.end();it++){
	      pair<int,int> key = it->first;
	      int offWidth = tempr.width*key.second;
	      int offHeight = tempr.height*key.first;
	      if((w+offWidth)<temp.width() && (h+offHeight)<temp.height()){
		int nWidth = w+offWidth;
		int nHeight = h+offHeight;
		int bWidth = nWidth-tempr.width;
		int bHeight = nHeight-tempr.height;
		double value = temp(nWidth,nHeight)-temp(bWidth,nHeight)-temp(nWidth,bHeight)+temp(bWidth,bHeight);
		if(it->second == 'b')
		  sum+=value;
		if(it->second == 'w')
		  sum-=value;
	      }
	    }
	    ostringstream ss;
	    ss<<i+1<<h<<w;
	    long mkey = atol(ss.str().c_str());
	    if(sum!=0)
	      rVector[mkey] = sum;
	  }
	}
      }
      return rVector;
    }
  map<string, CImg<double> > models; // trained models
  vector<haar> regions;
  static const int size = 200;
  
};
