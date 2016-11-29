
var c=require( 'caffenet' ) ;
var fs = require( 'fs' ) ;

var N = new c.Caffe( "/home/richard/caffe/models/bvlc_reference_caffenet/deploy.prototxt" ) ;
N.mode = 'GPU' ;
N.setTrainedFile( "/home/richard/caffe/models/bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel" ) ;
N.setMeanFile( "/home/richard/caffe/data/ilsvrc12/imagenet_mean.binaryproto" ) ;
N.setLabelFile( "/home/richard/caffe/data/ilsvrc12/synset_words.txt" ) ;

const testFolder = "/home/richard/caffe/examples/images/" ;
fs.readdir(testFolder, (err, files) => {
  files.filter(function(e) { return e.endsWith('.jpg'); }).forEach(file => {
    console.log(file);
    console.log( N.processImageFile( testFolder + file ) ) ;
  });
})
/*
console.log( "Castle") ;
console.log( N.processImageFile( "/home/richard/caffe/examples/images/castle.jpg" ) ) ;
console.log( "Cat") ;
console.log( N.processImageFile( "/home/richard/caffe/examples/images/cat.jpg" ) ) ;
console.log( "Me") ;
console.log( N.processImageFile( "/home/richard/caffe/examples/images/me.jpg" ) ) ;
console.log( "Taylor") ;
console.log( N.processImageFile( "/home/richard/caffe/examples/images/taylor.jpg" ) ) ;
console.log( "Jesus") ;
console.log( N.processImageFile( "/home/richard/caffe/examples/images/jesus.jpg" ) ) ;
console.log( "Cindy") ;
console.log( N.processImageFile( "/home/richard/caffe/examples/images/cindy.jpg" ) ) ;

*/
