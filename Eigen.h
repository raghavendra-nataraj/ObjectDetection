#include <fstream>
#include <map>
#include <sstream>

class Eigen : public Classifier
{
public:
  Eigen(const vector<string> &_class_list) : Classifier(_class_list) {}
  
  // Nearest neighbor training. All this does is read in all the images, resize
  // them to a common size, convert to greyscale, and dump them as vectors to a file
  virtual void train(const Dataset &filenames) 
  {
    CImg<double> T;
    map<int,int> imageMapping;
    vector<CImg<double> > vecOfImg;
    CImg<double> mean(1,size*size);
    mean.fill(0);
    int M=0;
    int imageIndex=1;
    for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
      {
	cout << "Processing " << c_iter->first << endl;
	// convert each image to be a row of this "model" image
	for(int i=0; i<c_iter->second.size(); i++){
	  CImg<double> temp = extract_features(c_iter->second[i].c_str());
	  vecOfImg.push_back(temp);
	  mean=mean+temp;
	  imageMapping[M] = imageIndex;
	  M++;
	}
	imageIndex++;
      }
    mean/=M;
    int res = system("mkdir Eigen");
    mean.save_cimg("Eigen/mean.cimg");
    for(int i=0;i<vecOfImg.size();i++)
      {
	CImg<double> t1 = vecOfImg[i];	
	t1-=mean;
	T.append(t1);
      }
    
    CImg<double> TTran;
    TTran = T;
    TTran.transpose();
    CImg<double> A = TTran*T;
    CImg<double> val;
    CImg<double> vec;
    cout<<"Mean"<<mean.height()<<" "<<mean.width()<<endl;
    cout<<T.height()<<" "<<T.width()<<endl;
    cout<<TTran.height()<<" "<<TTran.width()<<endl;
    cout<<A.height()<<" "<<A.width()<<endl;
    A.symmetric_eigen(val,vec);
    //val.display();
    CImg<double> U;
    for(int i=0;i<vec.width();i++){
      CImg<double> temp = T*vec.get_column(i);
      cout<<temp.width()<<" "<<temp.height()<<endl;
      U.append(temp);
      temp.assign(size,size,1,1);
      //temp.display();
      ostringstream ss;
      string imgName = "face"+i;
      ss<<"Eigen/face"<<i<<".png";
      cout<<ss.str()<<endl;
      temp.save_png(ss.str().c_str());
    }
    U.save_cimg("Eigen/U.cimg");
    ofstream inpFile;
    inpFile.open("Input_file.dat");
    for(int i=0;i<T.width();i++){
      int k=1;
      inpFile<<imageMapping[i]<<" ";
      cout<<i<<endl;
      for(int j=0;j<vec.width();j++){
	CImg<double> temp = T*vec.get_column(j);
	CImg<double> tVl = temp.transpose() * T.get_column(i);
	inpFile<<k<<":"<<tVl(0,0)<<" ";
	k++;	
	//cout<<tVl.width()<<" "<<tVl.height()<<endl;
      }
      inpFile<<endl;
    }
    inpFile.close();
    int result = system("svm_multiclass_linux64/svm_multiclass_learn -c 1.0 Input_file.dat food.model");
    cout<<result<<endl;
  }

  virtual string classify(const string &filename)
  {
    CImg<double> test_image = extract_features(filename);
    test_image-=meanClassify;
    ofstream inpFile;
    inpFile.open("Temp_file.dat");	      
    // figure nearest neighbor
    inpFile<<"1 ";
    int k=1;
    for(int j=0;j<uClassify.width();j++){
      CImg<double> temp = uClassify.get_column(j);
      CImg<double> tVl = temp.transpose() * test_image;
      inpFile<<k<<":"<<tVl(0,0)<<" ";
      k++;	
      //cout<<tVl.width()<<" "<<tVl.height()<<endl;
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
    CImg<double> temp("Eigen/U.cimg");
    uClassify = temp;
    CImg<double> temp1("Eigen/mean.cimg");
    meanClassify=temp1;
  }
protected:
  // extract features from an image, which in this case just involves resampling and 
  // rearranging into a vector of pixel data.
  CImg<double> extract_features(const string &filename)
    {
      CImg<double> input_image(filename.c_str());
      CImg<double> gImg = input_image.get_RGBtoHSI().get_channel(2);
      gImg.resize(size,size);
      return gImg.unroll('y');
    }
  static const int size=300;  // subsampled image resolution
  map<string, CImg<double> > models; // trained model
  CImg<double> meanClassify;
  CImg<double> uClassify;
};
