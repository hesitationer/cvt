#include <RefinedFASTTracking.h>

namespace cvt
{
    RefinedFASTTracking::RefinedFASTTracking() :
        _detector( SEGMENT_9 ),
        _pyramid( 4, 0.5f ),
        _klt( 3 ),
        _fastMatchingWindowSqr( Math::sqr( 50 ) ),
        _fastMaxSADThreshold( 50 ),
        _kltSSDThreshold( Math::sqr( 50 ) )
    {
        _detector.setBorder( PatchSize );
        _detector.setNonMaxSuppress( true );
        _detector.setThreshold( 30 );

        _simd = SIMD::instance();
    }

    RefinedFASTTracking::~RefinedFASTTracking()
    {
    }

    void RefinedFASTTracking::trackFeatures( PointSet2d&                     trackedPositions,
                                             std::vector<size_t>&            trackedFeatureIds,
                                             const std::vector<Vector2f>&	 predictedPositions,
                                             const std::vector<size_t>&      predictedIds,
                                             const Image&                    img )
    {
        // detect current fast features
        _associatedIndexes.clear();
        _currentFeatures.clear();
        detectCurrentFeatures( img );

        const float maxSSD = Math::sqr( PatchSize ) * _kltSSDThreshold;
        const float maxSAD = Math::sqr( PatchSize ) * _fastMaxSADThreshold;
        static const size_t nPixels = Math::sqr( PatchSize );

        Vector2f tmpVec;
        for( size_t i = 0; i < predictedIds.size(); i++ ){
            PatchType* patch = _patchForId[ predictedIds[ i ] ];
            if( patch == 0 )
                continue;

            patch->initPose( predictedPositions[ i ] );

            // find best fast feature within a certain window
            int idx = bestFASTFeatureInRadius( *patch );

            if( idx != -1 ){
                _associatedIndexes.insert( ( size_t )idx );

                // update the current position offset to the one of the fast feature
                /*Eigen::Matrix3f& m = patch->pose().transformation();
                m( 0, 2 ) = _currentFeatures[ idx ].pt.x;
                m( 1, 2 ) = _currentFeatures[ idx ].pt.y;*/
                patch->initPose( _currentFeatures[ idx ].pt );

                // now try to track with klt in multiscale fashion
                if( _klt.trackPatchMultiscale( *patch, _pyramid ) ){
                    // check similarity measures before assuming it's correctly tracked
                    float ssd = _simd->SSD( patch->pixels(), patch->transformed(), nPixels );
                    float sad = _simd->SAD( patch->pixels(), patch->transformed(), nPixels );

                    if( sad < maxSAD && ssd < maxSSD ){
                        patch->currentCenter( tmpVec );
                        trackedPositions.add( Vector2d( tmpVec.x, tmpVec.y ) );
                        trackedFeatureIds.push_back( predictedIds[ i ] );
                    } else {
                        std::cout << "TrackPatch failed -> sad/ssd check:" << std::endl;
                        std::cout << " SAD: " << sad / Math::sqr(PatchSize) << std::endl;
                        std::cout << " SSD: " << ssd / Math::sqr(PatchSize) << std::endl;
                    }
                }
            }
        }
    }

    /**
     * \brief add a new feature to the database! (e.g. after triangulation)
     */
    void RefinedFASTTracking::addFeatureToDatabase( const Vector2f & f, size_t id )
    {
        std::vector<PatchType*> patches;
        std::vector<Vector2f>  feature;
        feature.push_back( f );

        PatchType::extractPatches( patches, feature, _pyramid );

        if( id != _patchForId.size() ){
            throw CVTException( "Patch IDs out of sync" );
        }

        // FIXME: shall we handle this differently?
        // Problem: Map has already added feature with id at this point
        if( patches.size() )
            _patchForId.push_back( patches[ 0 ] );
        else
            _patchForId.push_back( 0 );
    }

    void RefinedFASTTracking::clear()
    {
        for( size_t i = 0; i < _patchForId.size(); i++ )
            delete _patchForId[ i ];
        _patchForId.clear();
    }

    void RefinedFASTTracking::detectCurrentFeatures( const Image& img )
    {
        // create the scale space
        _pyramid.update( img );

        float scale = 1.0f;
        // detect for each scale
        for( size_t i = 0; i < _pyramid.octaves(); i++ ){
            std::vector<Feature2Df> scaleFeatures;
            VectorFeature2DInserter<float> inserter( scaleFeatures );
            _detector.extract( _pyramid[ i ], inserter );

            size_t start = _currentFeatures.size();

            // insert the features
            _currentFeatures.insert( _currentFeatures.end(), scaleFeatures.begin(), scaleFeatures.end() );
            if( i != 0 ){
                scale /= _pyramid.scaleFactor();
                for( size_t f = start; f < _currentFeatures.size(); f++ ){
                    _currentFeatures[ f ].pt *= scale;
                }
            }
        }

    }

    int RefinedFASTTracking::bestFASTFeatureInRadius( const PatchType& patch )
    {
        Vector2f patchPos;
        patch.currentCenter( patchPos );

        size_t octave = _pyramid.octaves() - 1;

        IMapScoped<const uint8_t> map( _pyramid[ octave ] );
        float downscale = Math::pow( _pyramid.scaleFactor(), octave );

        std::set<size_t>::const_iterator assocEnd = _associatedIndexes.end();

        static const float patchHalfOffset = PatchSize / 2.0f;


        float best = Math::sqr( PatchSize ) * _fastMaxSADThreshold;
        int idx = -1;

        for( size_t i = 0; i < _currentFeatures.size(); i++ ){
            // avoid double associations
            if( _associatedIndexes.find( i ) != assocEnd )
                continue;

            const Vector2f& fpos = _currentFeatures[ i ].pt;

            if( ( fpos - patchPos ).lengthSqr() < _fastMatchingWindowSqr ){
                // within radius: check the SAD value
                const uint8_t* p1 = patch.pixels( octave );
                const uint8_t* p2 = map.ptr() + ( int )( downscale * fpos.y - patchHalfOffset ) * map.stride() + ( int ) ( downscale * fpos.x - patchHalfOffset );

                size_t rows = PatchSize;
                size_t sadSum = 0;
                while( rows-- ){
                    sadSum += _simd->SAD( p1, p2, PatchSize );
                    p1 += PatchSize;
                    p2 += map.stride();
                }

                if( sadSum < best ){
                    best = sadSum;
                    idx = i;
                }
            }
        }

        return idx;
    }
}