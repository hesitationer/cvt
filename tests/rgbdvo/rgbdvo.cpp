#include <cvt/util/String.h>
#include <cvt/util/EigenBridge.h>
#include <cvt/math/Matrix.h>
#include <cvt/gui/Application.h>

#include <RGBDVOApp.h>
#include <ConfigFile.h>

#include <Eigen/Geometry>

using namespace cvt;

void writePoseToFile( std::ofstream& file, const Matrix4f& pose, double stamp )
{
    Quaternionf q( pose.toMatrix3() );

    file.precision( 15 );
    file << std::fixed << stamp << " "
             << pose[ 0 ][ 3 ] << " "
             << pose[ 1 ][ 3 ] << " "
             << pose[ 2 ][ 3 ] << " "
             << q.x << " "
             << q.y << " "
             << q.z << " "
             << q.w << std::endl;
}

void testQuaternionPrecision( const Matrix4f& pose )
{
    Matrix3f R = pose.toMatrix3();
    Quaternionf q( R );
    Matrix3f tmp = q.toMatrix3();

    std::cout << tmp-R << std::endl;

}

void runBatch( const VOParams& params, const Matrix3f& K, const String& folder, ConfigFile& cfg )
{
    RGBDParser parser( folder, 0.05f );

    //typedef VOKeyframe KFType;
    //typedef ESMKeyframe KFType;
    typedef MultiscaleKeyframe<VOKeyframe> KFType;
    //typedef MultiscaleKeyframe<ESMKeyframe> KFType;
    RGBDVisualOdometry<KFType> vo( K, params );

    vo.setMaxRotationDistance( cfg.valueForName( "maxRotationDist", 3.0f ) );
    vo.setMaxTranslationDistance( cfg.valueForName( "maxTranslationDist", 3.0f ) );
    vo.setMaxSSD( cfg.valueForName( "maxSSD", 0.2f ) );

    parser.loadNext();
    const RGBDParser::RGBDSample& sample = parser.data();

    Image gray( sample.rgb.width(), sample.rgb.height(), IFormat::GRAY_FLOAT );
    //Image smoothed( sample.rgb.width(), sample.rgb.height(), IFormat::GRAY_FLOAT );
    sample.rgb.convert( gray );
    //gray.convolve( smoothed, IKernel::GAUSS_HORIZONTAL_3, IKernel::GAUSS_VERTICAL_3 );
    vo.addNewKeyframe( gray, sample.depth, sample.pose ); // add initial

    std::ofstream file;
    file.open( "trajectory.txt" );
    Matrix4f pose; pose.setIdentity();

    size_t stepIter = parser.size() / 20;
    while( parser.hasNext() ){
        parser.loadNext();
        const RGBDParser::RGBDSample& d = parser.data();

        d.rgb.convert( gray );
        //gray.convolve( smoothed, IKernel::GAUSS_HORIZONTAL_3, IKernel::GAUSS_VERTICAL_3 );

        vo.updatePose( gray, d.depth );
        vo.pose( pose );
        writePoseToFile( file, pose, d.stamp );

       //testQuaternionPrecision( d.pose );

        if( parser.iter() % stepIter == 0 )
            std::cout << "#"; std::flush( std::cout );
    }
    std::cout << std::endl;
    file.close();
}

int main( int argc, char* argv[] )
{
    ConfigFile cfg( "rgbdvo.cfg" );

    if( argc < 2 ){
        std::cout << "Usage: " << argv[ 0 ] << " <rgbd_dataset_folder>" << std::endl;
        return 1;
    }

    String folder( argv[ 1 ] );
    VOParams params;
    params.maxIters = cfg.valueForName( "maxIterations", 10 );
    params.gradientThreshold = cfg.valueForName( "gradientThreshold", 0.2f );
    params.depthScale = cfg.valueForName( "depthFactor", 5000.0f ) * cfg.valueForName( "depthScale", 1.0f );
    params.minParameterUpdate = cfg.valueForName( "minDeltaP", 0.0f );

    Matrix3f K;
    K.setIdentity();
    K[ 0 ][ 0 ] = 520.9f; K[ 0 ][ 2 ] = 325.1f;
    K[ 1 ][ 1 ] = 521.0f; K[ 1 ][ 2 ] = 249.7f;

    runBatch( params, K, folder, cfg );
    return 0;

    RGBDVOApp app( folder, K, params );
    app.setMaxRotationDistance( cfg.valueForName( "maxRotationDist", 3.0f ) );
    app.setMaxTranslationDistance( cfg.valueForName( "maxTranslationDist", 0.3f ) );
    app.setMaxSSD( cfg.valueForName( "maxSSD", 0.2f ) );
    Application::run();

    return 0;
}
