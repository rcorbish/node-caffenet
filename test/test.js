
var c=require( 'caffenet' ) ;
var fs = require( 'fs' ) ;

var nn = new c.Caffe() ;
nn.train( "solver.prototxt" ) ;
nn.load( "model.prototxt", "snapshot_iter_50000.caffemodel.h5"  ) ;

const testFolder = "/home/richard/caffe/examples/images/" ;
fs.readdir(testFolder, (err, files) => {
  files.filter(function(e) { return e.endsWith('.jpg'); }).forEach(file => {
    console.log(file,  nn.processImageFile( testFolder + file ) ) ;
  });
})
