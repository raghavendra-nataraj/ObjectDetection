#include <fstream>
class Baseline : public Classifier
{
public:
  Baseline(const vector<string> &_class_list) : Classifier(_class_list) {}
  
  // Nearest neighbor training. All this does is read in all the images, resize
  // them to a common size, convert to greyscale, and dump them as vectors to a file
  virtual void train(const Dataset &filenames) 
  {
    ofstream inpFile;
    int imgIndex=1;
    inpFile.open("Input_file.dat");
    for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
      {
	cout << "Processing " << c_iter->first << endl;
	// convert each image to be a row of this "model" image
	for(int i=0; i<c_iter->second.size(); i++){
	  CImg<double> temp = extract_features(c_iter->second[i].c_str());
	  inpFile<<imgIndex<<" ";
	  for(int j=0;j<temp.height();j++){
	    inpFile<<j+1<<":"<<temp(j)<<" ";
	  }
	  inpFile<<endl;
	}
	imgIndex++;
      }
    inpFile.close();
    int result = system("./svm_multiclass_learn -c 1.0 Input_file.dat food.model");
    //cout<<result<<endl;
  }

  virtual string classify(const string &filename)
  {
    CImg<double> test_image = extract_features(filename);
    ofstream inpFile;
    inpFile.open("Temp_file.dat");	      
    // figure nearest neighbor
    CImg<double> temp = extract_features(filename.c_str());
    inpFile<<"1 ";
    for(int j=0;j<temp.height();j++){
      inpFile<<j+1<<":"<<temp(j)<<" ";
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

  virtual void load_model()
  {
    
  }
protected:
  // extract features from an image, which in this case just involves resampling and 
  // rearranging into a vector of pixel data.
  CImg<double> extract_features(const string &filename)
    {
      CImg<double> temp(filename.c_str());
      //temp = temp.get_RGBtoHSI().get_channel(2).resize(size,size);
      temp = temp.resize(size,size);
      return temp.unroll('y');
    }

  static const int size=100;  // subsampled image resolution
  map<string, CImg<double> > models; // trained models
};
