
class CNN : public Classifier
{
public:
  CNN(const vector<string> &_class_list) : Classifier(_class_list) {}
  
  // Nearest neighbor training. All this does is read in all the images, resize
  // them to a common size, convert to greyscale, and dump them as vectors to a file
  virtual void train(const Dataset &filenames) 
  {
    ofstream inpFile;
    inpFile.open("Input_file.dat");
    int imgIndex=1;
    for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
      {
	cout << "Processing " << c_iter->first << endl;
	// convert each image to be a row of this "model" image
	for(int i=0; i<c_iter->second.size(); i++){
	  cout<<i<<endl;
	  inpFile<<imgIndex<<" ";
	  map<int,double> temp = extract_features(c_iter->second[i].c_str());
	  map<int,double>::iterator it;
	  for(it=temp.begin();it!=temp.end();it++){
	    inpFile<<it->first<<":"<<it->second<<" ";
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
    ofstream inpFile;
    inpFile.open("Temp_file.dat");
    map<int,double> temp = extract_features(filename);
    map<int,double>::iterator it;
    inpFile<<"1 ";
    for(it=temp.begin();it!=temp.end();it++){
      inpFile<<it->first<<":"<<it->second<<" ";
    }
    inpFile<<endl;
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
  map<int,double> extract_features(const string &filename)
    {
      CImg<double> tempFile(filename.c_str());
      tempFile.resize(size,size);
      tempFile.save("temp_img.png");
      map<int,double> temp;
      string cmd = "./overfeat/bin/linux_64/overfeat -f temp_img.png >temp.out";
      system(cmd.c_str());
      ifstream inpFile;
      inpFile.open("temp.out");
      double tempValue;
      int k=1;
      int valueC,row,col;
      inpFile>>valueC>>row>>col;
      int count = valueC*row*col;
      for(int i=0;i<count;i++){
	inpFile>>tempValue;
	temp[k]=tempValue;
	k++;
      }
      inpFile.close();
      return temp;
    }

  static const int size=231;  // subsampled image resolution
  map<string, CImg<double> > models; // trained models
};
