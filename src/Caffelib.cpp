
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <caffe/caffe.hpp>
#include <caffe/sgd_solvers.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Caffelib.hpp"

//using namespace std;

using caffe::Blob;
using caffe::Caffe;
using caffe::Net;
using caffe::Layer;
using caffe::Solver;
using caffe::shared_ptr;
using caffe::string;
using caffe::Timer;
using caffe::vector;

  template class caffe::SolverRegistry<float>  ;

  void caffenet::Init() {
    ::google::InitGoogleLogging("CaffeNet") ;
  } 

  typedef std::pair<string, float> Prediction;
  bool PairCompare(const std::pair<float, int>& lhs,
  const std::pair<float, int>& rhs) {
    return lhs.first > rhs.first;
  }

namespace caffenet {

  class CaffeNetImpl : public CaffeNet {

    public:
      CaffeNetImpl( const char *modelfile=NULL ) {
        net_ = NULL ;
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

	  caffe::Net<float> *createNet( const char *modelFile ) {
        const std::string str(modelFile);
        caffe::Net<float> *net = new caffe::Net<float>(str, caffe::TRAIN) ;
		
        return net ;
	  }
	  
      const char *getName() { return net_==NULL ? "Uninitialized Network" :"Initialized Network" ; }

      void setTrainedFile( const char *trainedfile ) {
        net_->CopyTrainedLayersFrom(trainedfile) ;
//        printf( "Trained data Set %s\n", trainedfile ) ;        
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


      cv::Mat readImage( const char *imageFile ) {
      	      // Read the image into a matrix
        cv::Mat img = cv::imread(imageFile, -1);

        LOG(INFO) << "Image read - has " << img.channels() << " channels" ;

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

        LOG(INFO) << "Image colored now has " << sample.channels() << " channels" ;
        LOG(INFO) << "Image size " <<  sample.rows << "x" << sample.cols ;
        LOG(INFO) << "Net size " <<  inputGeometry_.height << "x" << inputGeometry_.width ;

        cv::Mat sample_resized;
        if (sample.size() != inputGeometry_)
          cv::resize(sample, sample_resized, inputGeometry_);
        else
          sample_resized = sample;

        LOG(INFO) << "Image resized to " <<  sample_resized.rows << "x" << sample_resized.cols ;

        cv::Mat sample_float;
        if (numChannels_ == 3)
          sample_resized.convertTo(sample_float, CV_32FC3);
        else
          sample_resized.convertTo(sample_float, CV_32FC1);

        LOG(INFO) << "Image set to float" ;

        //cv::Mat sample_normalized;
        //cv::subtract(sample_float, mean_, sample_normalized);
		
		// return sample_normalized ;
		return sample_float ;
      }
  

 	void load( const char *modelFile, const char *weightsFile ) {
  
  		cudaDeviceProp device_prop;
		cudaGetDeviceProperties(&device_prop, 0);
		
		LOG(INFO) << "GPU " << device_prop.name;
        caffe::Caffe::SetDevice(0);
        caffe::Caffe::set_mode(caffe::Caffe::GPU);
        
        if( net_ != NULL ) {
        	delete net_ ;
        	net_ = NULL ;
        }
        const vector<string> stages {  "deploy" } ; 
  		net_ = new caffe::Net<float>( modelFile, caffe::TEST, 0, &stages );
       	LOG(INFO) << "Loaded model" ;
  		
  		if( weightsFile != NULL ) {
  			net_->CopyTrainedLayersFrom(weightsFile);
        	LOG(INFO) << "Loaded weights params" ;
        }

        net_->Reshape();

        LOG(INFO) << "Got the net #inputs=" << net_->num_inputs() ;
        LOG(INFO) << "Net name: " << net_->name() ;
        
        const vector< string > &layerNames = net_->layer_names() ;
        const vector< shared_ptr< Layer<float> > > &layers = net_->layers() ;
        for( int i=0 ; i<layerNames.size() ; i++ ) {
	        LOG(INFO) << "Layer " << i << " : " << layerNames[i] << " " << layers[i]->type() ;   	
	        vector< shared_ptr< Blob<float> > > & blobs = layers[i]->blobs() ;
	        for( int j=0 ; j<blobs.size() ; j++ ) {
	        	if(  blobs[j]->num_axes() == 4 )
		        	LOG(INFO) << "Blob " << j << " : " << blobs[j]->shape(0) << ", " << blobs[j]->shape(1) << ", " << blobs[j]->shape(2) << ", " << blobs[j]->shape(3) ;
	        	if(  blobs[j]->num_axes() == 3 )
		        	LOG(INFO) << "Blob " << j << " : " << blobs[j]->shape(0) << ", " << blobs[j]->shape(1) << ", " << blobs[j]->shape(2) ;    	
	        	if(  blobs[j]->num_axes() == 2 )
		        	LOG(INFO) << "Blob " << j << " : " << blobs[j]->shape(0) << ", " << blobs[j]->shape(1) ;    	
	        	if(  blobs[j]->num_axes() == 1 )
		        	LOG(INFO) << "Blob " << j << " : " << blobs[j]->shape(0) ;    	
	        }
        }

        caffe::Blob<float>* input_layer = net_->input_blobs()[0];

        numChannels_ = input_layer->channels();
        inputGeometry_ = cv::Size(input_layer->width(), input_layer->height());
          
        input_layer->Reshape(1, numChannels_, inputGeometry_.height, inputGeometry_.width);
        net_->Reshape();

        int width = input_layer->width();
        int height = input_layer->height();
        LOG(INFO) << "Network input width " << width ;
        LOG(INFO) << "Network input height " << height ;
        LOG(INFO) << "Number of input channels " << numChannels_ ;        
 	}

 	void train( const char *solverFile ) {
 	/*
 		caffe::Net<float> *net = createNet( "model.prototxt" ) ;

        caffe::Blob<float>* input_layer = net->input_blobs()[0];
        numChannels_ = input_layer->channels();
        inputGeometry_ = cv::Size(input_layer->width(), input_layer->height());
          
        input_layer->Reshape(1, numChannels_, inputGeometry_.height, inputGeometry_.width);
        net->Reshape();

        int width = input_layer->width();
        int height = input_layer->height();
        LOG(INFO) << "Network input width " << width ;
        LOG(INFO) << "Network input height " << height ;
        LOG(INFO) << "Number of input channels " << numChannels_ ;
        
        
        float* input_data = input_layer->mutable_cpu_data();

		// prepare the network inputs
        std::vector<cv::Mat> input_channels ;

        LOG(INFO) << "Prepared input channel" ;
        cv::Mat channel(height, width, CV_32FC1, input_data);
        LOG(INFO) << "Created input channel" ;
        input_channels.push_back(channel);
        LOG(INFO) << "Linked input channel" ;
        input_data += width * height;
        LOG(INFO) << "Input channel done" ;

 		      // Read the image into a matrix
        const cv::Mat img = readImage(imageFile);

		LOG(INFO) << "Image width " << img.rows ;
        LOG(INFO) << "Image height " << img.cols ;
        LOG(INFO) << "Image input channels " << img.channels() ;
        

        cv::split(img, input_channels);
        LOG(INFO) << "Image saved to network input";        
*/
		caffe::SolverParameter solver_param;
  		caffe::ReadSolverParamsFromTextFileOrDie( solverFile, &solver_param);

        LOG(INFO) << "Loaded solver param " ;

        solver_param.mutable_train_state()->set_level(0);
  
  		cudaDeviceProp device_prop;
		cudaGetDeviceProperties(&device_prop, 0);
		
		LOG(INFO) << "GPU " << device_prop.name;
        solver_param.set_device_id(0);
        caffe::Caffe::SetDevice(0);
        caffe::Caffe::set_mode(caffe::Caffe::GPU);
        caffe::Caffe::set_solver_count(1);
    
  		caffe::SolverRegistry<float>::CreatorRegistry& registry = caffe::SolverRegistry<float>::Registry() ;
 
        shared_ptr<caffe::Solver<float> > solver(caffe::SolverRegistry<float>::CreateSolver(solver_param)) ;
        LOG(INFO) << "Created solver";

        solver->Solve();

		const shared_ptr<caffe::Net<float>> &net = solver->net() ;
        LOG(INFO) << "Got the net " << net->num_inputs() ;
        LOG(INFO) << "Net name: " << net->name() ;
        
        const vector< string > &layerNames = net->layer_names() ;
        const vector< shared_ptr< Layer<float> > > &layers = net->layers() ;
        for( int i=0 ; i<layerNames.size() ; i++ ) {
	        LOG(INFO) << "Layer " << i << " : " << layerNames[i] << " " << layers[i]->type() ;   	
	        vector< shared_ptr< Blob<float> > > & blobs = layers[i]->blobs() ;
	        for( int j=0 ; j<blobs.size() ; j++ ) {
	        	if(  blobs[j]->num_axes() == 4 )
		        	LOG(INFO) << "Blob " << j << " : " << blobs[j]->shape(0) << ", " << blobs[j]->shape(1) << ", " << blobs[j]->shape(2) << ", " << blobs[j]->shape(3) ;
	        	if(  blobs[j]->num_axes() == 3 )
		        	LOG(INFO) << "Blob " << j << " : " << blobs[j]->shape(0) << ", " << blobs[j]->shape(1) << ", " << blobs[j]->shape(2) ;    	
	        	if(  blobs[j]->num_axes() == 2 )
		        	LOG(INFO) << "Blob " << j << " : " << blobs[j]->shape(0) << ", " << blobs[j]->shape(1) ;    	
	        	if(  blobs[j]->num_axes() == 1 )
		        	LOG(INFO) << "Blob " << j << " : " << blobs[j]->shape(0) ;    	
	        }
        }
/*        
        caffe::Blob<float>* input_layer = net->input_blobs()[0];
        LOG(INFO) << "Got the input layer";

        int width = input_layer->width();
        int height = input_layer->height();
        float* input_data = input_layer->mutable_cpu_data();

        LOG(INFO) << "Got the input layer data";
        

		// prepare the network inputs
        std::vector<cv::Mat> input_channels;
        for (int i = 0; i < input_layer->channels(); ++i) {
          cv::Mat channel(height, width, CV_32FC1, input_data);
          input_channels.push_back(channel);
          input_data += width * height;
        }

  */  
        LOG(INFO) << "Solved";
        
        
 	}


      int processImageFile( const char *imageFile, int N ) {
      // Read the image into a matrix
        const cv::Mat img = readImage(imageFile);
        caffe::Blob<float>* input_layer = net_->input_blobs()[0];


        int width = input_layer->width();
        int height = input_layer->height();
        float* input_data = input_layer->mutable_cpu_data();
        

		// prepare the network inputs
        std::vector<cv::Mat> input_channels;
        for (int i = 0; i < input_layer->channels(); ++i) {
          cv::Mat channel(height, width, CV_32FC1, input_data);
          input_channels.push_back(channel);
          input_data += width * height;
        }

        cv::split(img, input_channels);

        net_->Forward();

        caffe::Blob<float>* output_layer = net_->output_blobs()[0];
        const float* begin = output_layer->cpu_data();
        const float* end = begin + output_layer->channels();
        std::vector<float> output = std::vector<float>(begin, end);


        //N = std::min<int>(labels_.size(), N);
        int rc = 0 ;
        const float *f = begin ;
        float mx = 0.f ;
        
        for( int i=0 ; i<output_layer->channels() ; i++, f++ ) {
   	        LOG(INFO) << "Output: " << i << " = " << *f ;
   	        if( *f>mx) {
   	        	mx = *f ;
   	        	rc = i ;
   	        }
        }
	    LOG(INFO) << "Done rc=" << mx << "@" << rc ;
/*
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
*/

        return rc ;
      }

    private:
      caffe::Net<float> *net_ ;
      bool isGpuMode_ ;
      int numChannels_ ;
      cv::Mat mean_ ;
      cv::Size inputGeometry_;
      std::vector<string> labels_;
      caffe::Solver<float> *solver_ ;
  } ;

  CaffeNet *create( const char *protofile ) {
    return new CaffeNetImpl( protofile ) ;
  }

  typedef caffe::SolverRegistry<float> reg;
}
