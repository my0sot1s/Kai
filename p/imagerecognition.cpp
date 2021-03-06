#include "imagerecognition.h"

//#define TOP 3/5
//#define BOT 2/5


// hold labels -> training filenames


std::vector<int> labels;

std::vector<std::string> trainingFilenames;


ImageRecognition::ImageRecognition()
{
    // Load SVM classifier
    svm = cv::ml::StatModel::load<cv::ml::SVM>(CLASSIFIER);
    svm_start = cv::ml::StatModel::load<cv::ml::SVM>(START);//svm2
    svm_symbol=cv::ml::StatModel::load<cv::ml::SVM>(SYMBOL);
    svm_number=cv::ml::StatModel::load<cv::ml::SVM>(NUMBER);
}

/// Lấy dữ liệu cho việc train
void ImageRecognition::getTrain(QString dir,int type)
{
    labels.clear();
    trainingFilenames.clear();

    QDir recoredDir(dir);
    QStringList allFiles = recoredDir.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);//(QDir::Filter::Files,QDir::SortFlag::NoSort)
    int key=0,lbl_size=0;
    vector<string> labels2;
    if(allFiles[0]==".DS_Store") allFiles.removeFirst();
    for(int i=0;i<allFiles.size();i++) {
      trainingFilenames.push_back(allFiles[i].toStdString());
      switch(type){
          case 1:// classifier
          case 4:// start
              labels2.push_back(allFiles[i].left(2).toStdString()); break;
          case 2:// values
              labels2.push_back(allFiles[i].left(1).toStdString()); break;
          case 3:// symbols
              {
                labels2.push_back(allFiles[i].mid(1,1).toStdString());
                cout<<allFiles[i].mid(1,1).toStdString();
                break;
                }
           default: return;
      }

    }
    lbl_size = labels2.size();

    if(type==3){
        vector<string> vtemp,vtong,vtlabel,vt_tong_label;
        string mang[]={"b","r","c","t"};
        for(int i=0;i<4;i++){
            for(int j=0;j<trainingFilenames.size();j++){
                if(QString::fromStdString(trainingFilenames[j]).mid(1,1).toStdString()==mang[i])
                {
                    vtemp.push_back(trainingFilenames[j]);
                    vtlabel.push_back(labels2[j]);
                }
            }
            vtong.insert(vtong.end(),vtemp.begin(),vtemp.end());
            vt_tong_label.insert(vt_tong_label.end(),vtlabel.begin(),vtlabel.end());
            vtemp.clear();vtlabel.clear();
        }
        trainingFilenames.clear();trainingFilenames=vtong;
        labels2.clear();labels2=vt_tong_label;
    }

    for(int index=0; index<lbl_size; index++)
    {
        labels.push_back(key);
        if(labels2[index+1]!=labels2[index])
            key++;
    }

}

/// tiến hành train classifier +start
///
int ImageRecognition::train(QString dir,string fileSave,int imgArea,int h,int type)
{
    getTrain(QString(PATH)+dir,type);

    cv::Mat trainingMat(labels.size(), imgArea, CV_32FC1);
    int len_size=trainingFilenames.size(),lbl_size=labels.size();
    // loop over training files
    for(int index=0; index<len_size; index++)
    {
        // output on which file we are training
        std::cout << "Analyzing label -> file: " <<  labels[index] << "|" <<  trainingFilenames[index] << std::endl;

        // read image file (grayscale)
        // khái quát:
        // cứ ứng với 1 file name sẽ có 1 dạng string đi kèm. và đước mã hóa thành 1 số có giới hạn từ 0-51
        cv::Mat imgMat = cv::imread(string(PATH)+dir.toStdString() +"/"+trainingFilenames[index], 0);

        if(type==TYPE_NUMBER){
            imgMat=imgMat.rowRange(0,3*h/5);
        }else if(type==TYPE_SYMBOL){
            imgMat=imgMat.rowRange(3*h/5,h);
        }


        int ii = 0; // Current column in training_mat

        // process individual pixels and add to our training material
        for (int i = 0; i<imgMat.rows; i++) {
           for (int j = 0; j < imgMat.cols; j++) {
                trainingMat.at<float>(index, ii++) = imgMat.at<uchar>(i,j);
            }
        }
    }


    int labelsArray[lbl_size];

    // loop over labels
    for(int index=0; index<lbl_size; index++)
    {
//        cout<<labels[index]<<endl;
        labelsArray[index] = labels[index];
    }


    cv::Mat labelsMat(labels.size(), 1, CV_32S, labelsArray);

    //
    // train SVM

    // Set up SVM's parameters
    cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();
    svm->setType(cv::ml::SVM::C_SVC);
    svm->setKernel(cv::ml::SVM::POLY);
    svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 100, 1e-6));
    svm->setGamma(3);
    svm->setDegree(3);

    // train svm classifier

    svm->train( trainingMat , cv::ml::ROW_SAMPLE , labelsMat );

    // store trained classifier
    std::cout << "Saving SVM data" << std::endl;
    svm->save(fileSave);// lưu tại chương trình.
//    Lưu tai lõi dev
    svm->save(string(PATH)+fileSave);
    return labels.size();
}

/// load dữ liệu train

string ImageRecognition::loadTrain(string name,int w,int h)
{
    try{

        // read image file (grayscale)
        string t=QDir::currentPath().toStdString()+name;

        cv::Mat imgMat = cv::imread(QDir::currentPath().toStdString()+"/"+name,0);
        cv::Mat demo;

//        threshold(imgMat,demo,110,255,CV_THRESH_BINARY);
//        imshow("",imgMat);
//        imshow("",demo);
//        waitKey(0);

        Mat value=imgMat.rowRange(0,3*h/5)
                ,symbol=imgMat.rowRange(3*h/5,h);
//        cv::Mat testMat = imgMat.clone().reshape(1,1);
//        testMat.convertTo(testMat, CV_32F);

        // try to predict which number has been drawn

//        return loadTrainPer(value,w,3*h/5,TYPE_NUMBER)+loadTrainPer(symbol,w,2*h/5,TYPE_SYMBOL);
          return loadTrainPer(imgMat,w,h,TYPE_CLASSIFIER);
        }catch(cv::Exception ex){
            std::cout<<"error at loadTrain"<<endl;
            return "~~";
    }
}

string ImageRecognition::loadTrain(QImage src,int w,int h)
{
      try{
        cv::Mat imgMat=QImageToCvMat(src);
        cv::cvtColor(imgMat, imgMat, CV_BGR2GRAY);
           Mat value=imgMat.rowRange(0,3*h/5)
                        ,symbol=imgMat.rowRange(3*h/5,h);

//        return loadTrainPer(value,w,3*h/5,TYPE_NUMBER)+loadTrainPer(symbol,w,2*h/5,TYPE_SYMBOL);
        return loadTrainPer(imgMat,w,h,TYPE_CLASSIFIER);
      }catch(cv::Exception ex){
            std::cout<<"error at loadTrain2"<<endl;
            return "~~";
    }
}

string ImageRecognition::loadTrainPer(Mat imgMat, int w, int h,int type)
{
    try{
        resize(imgMat,imgMat,Size(w,h));
        cv::Mat testMat = imgMat.clone().reshape(1,1);


//        Mat morpho,demo;
//        threshold(imgMat,demo,135,255,CV_THRESH_BINARY);


//        int z_size=0.7;

//        Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2*z_size+1,2*z_size+1),Point(z_size,z_size));

//        erode(demo, morpho, element);
//        imshow("Anh sau khi thuc hien phep gian no", morpho);
//        waitKey(0);

        testMat.convertTo(testMat, CV_32F);


        // try to predict which number has been drawn
        if(type==TYPE_NUMBER)
            num = svm_number->predict(testMat);
        else if(type==TYPE_SYMBOL)
            num = svm_symbol->predict(testMat);
        else if(type==TYPE_CLASSIFIER)
            num = svm->predict(testMat);
        else return "~~";

            return getResult(type);
        }catch(cv::Exception ex){
            std::cout<<"error at trainper "<<type<<endl;
            return "~~";
    }
}

void ImageRecognition::trainAll(int w, int h)
{
    // train values symbol and classifier start
    try {
        // trainclassifier
        std::cout << "Start training SVM classifier" << std::endl;
        std::cout<<"Train thành công Classifier"<<train("Raw",CLASSIFIER,w*h,h,TYPE_CLASSIFIER)<<endl;

        int trainStarter=70*20;
        std::cout << "Start training SVM start" << train("start",START,trainStarter,h,TYPE_START)<<std::endl;


        std::cout<<"Train thành công Start"<<endl;

        std::cout << "Start training SVM number & symbol" <<train("Raw",NUMBER,3*w*h/5,h,TYPE_NUMBER)<<
                     train("Raw",SYMBOL,2*w*h/5,h,TYPE_SYMBOL)<<std::endl;
        std::cout<<"Train thành công NUMBER & SYMBOL"<<endl;

    } catch (...) {
        cout<<"error at trainAll"<<endl;
    }


}

int ImageRecognition::loadTrainStart(QImage src)
{
    try{
      cv::Mat imgMat=QImageToCvMat(src);
      cv::cvtColor(imgMat, imgMat, CV_BGR2GRAY);
      resize(imgMat,imgMat,Size(70,20));
      // convert 2d to 1d
//      imshow("",imgMat);//5t4r3r9b8t6r5c5rarqbjc9c9t
      cv::Mat testMat = imgMat.clone().reshape(1,1);
      testMat.convertTo(testMat, CV_32F);
      // try to predict which number has been drawn
//          return 1;
          return num = svm_start->predict(testMat);
  //            std::cout << std::endl  << "Recognizing following number -> " << getResult() << std::endl << std::endl;

    }catch(cv::Exception ex){
        cout<<"error at StartTrain"<<endl;
          return -1;
  }
}

string ImageRecognition::getResult(int type)
{
    if(type==TYPE_CLASSIFIER)
        switch(num){
            case 0:return "0b";case 1:return "0c";case 2:return "0r";case 3:return "0t";
            case 4:return "2b";case 5:return "2c";case 6:return "2r";case 7:return "2t";
            case 8:return "3b";case 9:return "3c";case 10:return "3r";case 11:return "3t";
            case 12:return "4b";case 13:return "4c";case 14:return "4r";case 15:return "4t";
            case 16:return "5b";case 17:return "5c";case 18:return "5r";case 19:return "5t";
            case 20:return "6b";  case 21:return "6c";  case 22:return "6r";  case 23:return "6t";
            case 24:return "7b";  case 25:return "7c";  case 26:return "7r";  case 27:return "7t";
            case 28:return "8b";  case 29:return "8c";  case 30:return "8r";  case 31:return "8t";
            case 32:return "9b";  case 33:return "9c";  case 34:return "9r";  case 35:return "9t";
            case 36:return "ab";  case 37:return "ac";  case 38:return "ar";  case 39:return "at";
            case 40:return "jb";  case 41:return "jc";  case 42:return "jr";  case 43:return "jt";
            case 44:return "kb";  case 45:return "kc";  case 46:return "kr";  case 47:return "kt";
            case 48:return "qb";  case 49:return "qc";  case 50:return "qr";  case 51:return "qt";
        default: return "undefined";
        }
    else if(type==TYPE_SYMBOL){
        switch(num){
            case 0: return "b";
            case 1: return "r";
            case 2: return "c";
            case 3: return "t";
            default: return "~~";
        }
    }
    else if(type==TYPE_NUMBER){
        switch(num){
            case 0: return "0";
            case 1: return "2";
            case 2: return "3";
            case 3: return "4";
            case 4: return "5";
            case 5: return "6";
            case 6: return "7";
            case 7: return "8";
            case 8: return "9";
            case 9: return "a";
            case 10: return "j";
            case 11: return "k";
            case 12: return "q";
            default: return "~~";
        }
    }else return "undefined";
}

Mat ImageRecognition::QImageToCvMat(QImage inImage)
{
    switch ( inImage.format() )
          {
             // 8-bit, 4 channel
             case QImage::Format_ARGB32:
             case QImage::Format_ARGB32_Premultiplied://is
            case QImage::Format_RGB32:
            case QImage::Format_RGBX8888:
           case QImage::Format_RGBA8888:
            case QImage::Format_RGBA8888_Premultiplied:
             {
                cv::Mat  mat( inImage.height(), inImage.width(),
                              CV_8UC4,
                              const_cast<uchar*>(inImage.bits()),
                              static_cast<size_t>(inImage.bytesPerLine())
                              );
                return mat;
             }

             // 8-bit, 3 channel

             case QImage::Format_RGB888:
             {

                QImage   swapped = inImage;
                swapped = swapped.rgbSwapped();
                qDebug()<<2;
                return cv::Mat( swapped.height(), swapped.width(),
                                CV_8UC3,
                                const_cast<uchar*>(swapped.bits()),
                                static_cast<size_t>(swapped.bytesPerLine())
                                ).clone();
                }

             // 8-bit, 1 channel
             case QImage::Format_Indexed8:
            case QImage::Format_Alpha8:
            case QImage::Format_Grayscale8:
             {
                cv::Mat  mat( inImage.height(), inImage.width(),
                              CV_8UC1,
                              const_cast<uchar*>(inImage.bits()),
                              static_cast<size_t>(inImage.bytesPerLine())
                              );
                return mat;
             }
             default:
                qWarning() << "ASM::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
                break;
          }

    return cv::Mat();
}


