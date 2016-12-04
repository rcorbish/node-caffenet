
#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Caffelib.hpp"

using namespace std;

namespace caffenet {

  void Init() {
    ::google::InitGoogleLogging("CaffeNet") ;
  } 

  typedef std::pair<string, float> Prediction;
  bool PairCompare(const std::pair<float, int>& lhs,
  const std::pair<float, int>& rhs) {
    return lhs.first > rhs.first;
  }

  class CaffeNetImpl : public CaffeNet {

    public:
      CaffeNetImpl( const char *modelfile=NULL ) {
        net_ = NULL ;
        if( modelfile != NULL ) {
          const std::string str(modelfile);
          net_ = new caffe::Net<float>(str, caffe::TEST) ;
          caffe::Blob<float>* input_layer = net_->input_blobs()[0];
          numChannels_ = input_layer->channels();
          inputGeometry_ = cv::Size(input_layer->width(), input_layer->height());
          
        input_layer->Reshape(1, numChannels_, inputGeometry_.height, inputGeometry_.width);
        /* Forward dimension change to all layers. */
        net_->Reshape();

        } 
        isGpuMode_ = false ;
        caffe::Caffe::set_mode(caffe::Caffe::CPU);
      }
      ~CaffeNetImpl() {
        delete net_ ;
      }

      bool isGpuMode() {
        return isGpuMode_ ;
      }

      void setGpuMode( bool mode ) {
        isGpuMode_ = mode ;
        if( mode ) {
          caffe::Caffe::set_mode(caffe::Caffe::GPU);
        }
        else {
          caffe::Caffe::set_mode(caffe::Caffe::CPU);
        }
      }

      const char *getName() { return net_==NULL ? "Uninitialized Network" :"Initialized Network" ; }

      void setTrainedFile( const char *trainedfile ) {
        net_->CopyTrainedLayersFrom(trainedfile) ;
//        printf( "Trained data Set %s\n", trainedfile ) ;
      }


 	void train( const char *dataFile ) {
 		
 	}
 	
      void setLabelFile( const char *labelfile ) {
        std::ifstream labels(labelfile);
        //  CHECK(labels) << "Unable to open labels file " << label_file;
        string line;
        while (std::getline(labels, line))
          labels_.push_back(string(line));

//        printf( "Labels Set %s\n", labelfile ) ;
      }

      void setMeanFile( const char *meanfile ) {
        caffe::BlobProto blob_proto;
        ReadProtoFromBinaryFileOrDie(meanfile, &blob_proto);

        /* Convert from BlobProto to Blob<float> */
        caffe::Blob<float> mean_blob;
        mean_blob.FromProto(blob_proto);

        //          CHECK_EQ(mean_blob.channels(), numChannels_)  << "Number of channels of mean file doesn't match input layer.";

        /* The format of the mean file is planar 32-bit float BGR or grayscale. */
        std::vector<cv::Mat> channels;
        float* data = mean_blob.mutable_cpu_data();
        for (int i = 0; i < numChannels_; ++i) {
          /* Extract an individual channel. */
          cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
          channels.push_back(channel);
          data += mean_blob.height() * mean_blob.width();
        }

        /* Merge the separate channels into a single image. */
        cv::Mat mean;
        cv::merge(channels, mean);

        /* Compute the global mean pixel value and create a mean image
         * filled with this value. */
        cv::Scalar channel_mean = cv::mean(mean);
        mean_ = cv::Mat(inputGeometry_, mean.type(), channel_mean);
//        printf( "Mean set %s\n", meanfile ) ;
      }

      char **processImageFile( const char *imageFile, int N ) {
      // Read the image into a matrix
        cv::Mat img = cv::imread(imageFile, -1);

	  // 
        caffe::Blob<float>* input_layer = net_->input_blobs()[0];
        //input_layer->Reshape(1, numChannels_, inputGeometry_.height, inputGeometry_.width);
        /* Forward dimension change to all layers. */
        //net_->Reshape();

        std::vector<cv::Mat> input_channels;
        //          caffe::Blob<float>* input_layer = net_->input_blobs()[0];

        int width = input_layer->width();
        int height = input_layer->height();
        float* input_data = input_layer->mutable_cpu_data();
        for (int i = 0; i < input_layer->channels(); ++i) {
          cv::Mat channel(height, width, CV_32FC1, input_data);
          input_channels.push_back(channel);
          input_data += width * height;
        }

        cv::Mat sample;
        if (img.channels() == 3 && numChannels_ == 1)
          cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
        else if (img.channels() == 4 && numChannels_ == 1)
          cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
        else if (img.channels() == 4 && numChannels_ == 3)
          cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
        else if (img.channels() == 1 && numChannels_ == 3)
          cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
        else
          sample = img;

        cv::Mat sample_resized;
        if (sample.size() != inputGeometry_)
          cv::resize(sample, sample_resized, inputGeometry_);
        else
          sample_resized = sample;

        cv::Mat sample_float;
        if (numChannels_ == 3)
          sample_resized.convertTo(sample_float, CV_32FC3);
        else
          sample_resized.convertTo(sample_float, CV_32FC1);

        cv::Mat sample_normalized;
        cv::subtract(sample_float, mean_, sample_normalized);

        /* This operation will write the separate BGR planes directly to the
         * input layer of the network because it is wrapped by the cv::Mat
         * objects in input_channels. */
        cv::split(sample_normalized, input_channels);

        net_->Forward();

        /* Copy the output layer to a std::vector */
        caffe::Blob<float>* output_layer = net_->output_blobs()[0];
        const float* begin = output_layer->cpu_data();
        const float* end = begin + output_layer->channels();
        std::vector<float> output = std::vector<float>(begin, end);

        N = std::min<int>(labels_.size(), N);

        std::vector<std::pair<float, int> > pairs;
        for (size_t i = 0; i < output.size(); ++i)
          pairs.push_back(std::make_pair(output[i], i));
        std::partial_sort(pairs.begin(), pairs.begin() + N, pairs.end(), PairCompare);

        std::vector<int> maxN;
        for (int i = 0; i < N; ++i)
          maxN.push_back(pairs[i].second);

        std::vector<Prediction> predictions;
        char **rc = new char*[N] ;
        for (int i = 0; i < N; ++i) {
          int idx = maxN[i];
          predictions.push_back(std::make_pair(labels_[idx], output[idx]));
          rc[i] = new char[100 ] ;
          strncpy( rc[i], predictions[i].first.c_str(), 100 ) ;
        }

        return rc ;
      }

    private:
      caffe::Net<float> *net_ ;
      bool isGpuMode_ ;
      int numChannels_ ;
      cv::Mat mean_ ;
      cv::Size inputGeometry_;
      std::vector<string> labels_;
  } ;

  CaffeNet *create( const char *protofile ) {
    return new CaffeNetImpl( protofile ) ;
  }

}
