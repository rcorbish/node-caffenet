{
  "targets": [
{
      "target_name": "caffenet",
      "defines": [
	"GOOGLE_PROTOBUF_NO_RTTI"
       ,"BOOST_NO_EXCEPTIONS"
      ],
      "sources": [ "src/Caffe.cpp" ], 
      "libraries": [
            "-L/home/richard/workspace/caffenet/build/lib", 
            "-L/home/richard/caffe/.build_release/lib",
	    "-lcaffenet", "-lopenblas", "-lpthread", "-lgfortran", "-lboost_system", "-lcaffe"
      ],
      'include_dirs': [
      ],
      "cflags": [ "-fmax-errors=5", "-Wno-sign-compare", "-Wall" ],
      'xcode_settings': {
        'OTHER_CFLAGS': [ ],
      },
      "conditions": [
        [ 'OS=="mac"', {
            "xcode_settings": {
                'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11','-stdlib=libc++'],
                'OTHER_LDFLAGS': ['-stdlib=libc++'],
                'MACOSX_DEPLOYMENT_TARGET': '10.7' }
            }
        ]
      ]
}
  ]
}

