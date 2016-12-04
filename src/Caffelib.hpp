
namespace caffenet {

	class CaffeNet {
		public:
			virtual const char *getName() = 0 ;
			virtual void setMeanFile( const char* meanfile ) = 0 ;
			virtual void setTrainedFile( const char *trainedfile ) = 0 ;
			virtual void setLabelFile( const char *labelfile ) = 0 ;
			virtual char **processImageFile( const char *imagefile, int N ) = 0 ;
			virtual void setGpuMode( bool mode ) = 0 ;
			virtual bool isGpuMode() = 0 ;
			virtual ~CaffeNet() {} ;
	} ;

	extern CaffeNet *create( const char *protofile=NULL ) ;
	extern void Init() ;
}
