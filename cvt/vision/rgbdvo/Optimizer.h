/*
            CVT - Computer Vision Tools Library

     Copyright (c) 2012, Philipp Heise, Sebastian Klose

    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.
*/

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <cvt/vision/rgbdvo/RGBDKeyframe.h>

namespace cvt {


    template <class WarpFunc, class Weighter>
    class Optimizer
    {
        public:
            typedef typename RGBDKeyframe<WarpFunc>::Result   ResultType;

            Optimizer();

            void setMaxIterations( size_t iter ) { _maxIter = iter; }
            void setMinUpdate( float v )         { _minUpdate = v; }
            void setRobustThreshold( float v )   { _robustThreshold = v; }

            void optimize( ResultType& result,
                           const Matrix4f& posePrediction,
                           RGBDKeyframe<WarpFunc>& reference,
                           const ImagePyramid& grayPyramid,
                           const Image& depthImage ) const;


        private:
            typedef typename WarpFunc::JacobianType     JacobianType;
            typedef typename WarpFunc::HessianType      HessianType;
            typedef typename WarpFunc::DeltaVectorType  DeltaType;
            typedef typename RGBDKeyframe<WarpFunc>::AlignDataType AlignDataType;
            size_t  _maxIter;
            float   _minUpdate;
            float   _robustThreshold;

            void optimizeNonRobust( ResultType& result,
                                    const Matrix4f& posePrediction,
                                    RGBDKeyframe<WarpFunc>& reference,
                                    const ImagePyramid& grayPyramid,
                                    const Image& depthImage ) const;

            void optimizeRobust( ResultType& result,
                                 const Matrix4f& posePrediction,
                                 RGBDKeyframe<WarpFunc>& reference,
                                 const ImagePyramid& grayPyramid,
                                 const Image& depthImage ) const;
    };

    template <class WarpFunc, class LossFunc>
    inline Optimizer<WarpFunc, LossFunc>::Optimizer() :
        _maxIter( 10 ),
        _minUpdate( 1e-6 ),
        _robustThreshold( 0.1f )
    {
    }

    template <class WarpFunc, class LossFunc>
    inline void Optimizer<WarpFunc, LossFunc>::optimize( ResultType& result,
                                                         const Matrix4f& posePrediction,
                                                         RGBDKeyframe<WarpFunc> &reference,
                                                         const ImagePyramid& grayPyramid,
                                                         const Image& depthImage ) const
    {
        if( IsRobustWeighting<LossFunc>::Value )
            optimizeRobust( result, posePrediction, reference, grayPyramid, depthImage );
        else
            optimizeNonRobust( result, posePrediction, reference, grayPyramid, depthImage );
    }

    template <class WarpFunc, class LossFunc>
    inline void Optimizer<WarpFunc, LossFunc>::optimizeNonRobust( ResultType& result,
                                                                  const Matrix4f& posePrediction,
                                                                  RGBDKeyframe<WarpFunc>& reference,
                                                                  const ImagePyramid& grayPyramid,
                                                                  const Image& /*depthImage*/ ) const
    {
        SIMD* simd = SIMD::instance();
        Matrix4f tmp4;
        tmp4 = posePrediction.inverse() * reference.pose();

        result.warp.setPose( tmp4 );
        result.costs = 0.0f;
        result.iterations = 0;
        result.numPixels = 0;
        result.pixelPercentage = 0.0f;

        Matrix4f projMat;
        std::vector<Vector2f> warpedPts;
        std::vector<float> interpolatedPixels;
        // sum of jacobians * delta
        JacobianType deltaSum, jtmp;

        for( int o = grayPyramid.octaves() - 1; o >= 0; o-- ){
            ResultType scaleResult;
            scaleResult = result;

            const size_t width = grayPyramid[ o ].width();
            const size_t height = grayPyramid[ o ].height();

            const AlignDataType& data = reference.dataForScale( o );

            const size_t num = data.points3d.size();
            Matrix4f K4( data.intrinsics );
            const Vector3f* p3dPtr = &data.points3d[ 0 ];
            const float* referencePixVals = &data.pixelValues[ 0 ];
            const JacobianType* referenceJ = &data.jacobians[ 0 ];

            warpedPts.resize( num );
            interpolatedPixels.resize( num );

            IMapScoped<const float> grayMap( grayPyramid[ o ] );

            scaleResult.iterations = 0;
            scaleResult.numPixels = 0;
            scaleResult.pixelPercentage = 0.0f;

            while( scaleResult.iterations < _maxIter ){
                // build the updated projection Matrix
                projMat = K4 * scaleResult.warp.poseMatrix();

                // project the points:
                simd->projectPoints( &warpedPts[ 0 ], projMat, p3dPtr, num );

                // interpolate the pixel values
                simd->warpBilinear1f( &interpolatedPixels[ 0 ], &warpedPts[ 0 ].x, grayMap.ptr(), grayMap.stride(), width, height, -1.0f, num );

                deltaSum.setZero();
                scaleResult.numPixels = 0;
                scaleResult.costs = 0.0f;
                for( size_t i = 0; i < num; i++ ){
                    // compute the delta
                    if( interpolatedPixels[ i ] >= 0.0f ){
                        float delta = result.warp.computeResidual( referencePixVals[ i ], interpolatedPixels[ i ] );
                        scaleResult.costs += Math::sqr( delta );
                        scaleResult.numPixels++;

                        jtmp = delta * referenceJ[ i ];
                        deltaSum += jtmp;
                    }
                }

                if( !scaleResult.numPixels ){
                    break;
                }

                // evaluate the delta parameters
                DeltaType deltaP = -data.inverseHessian * deltaSum.transpose();
                scaleResult.warp.updateParameters( deltaP );

                scaleResult.iterations++;
                if( deltaP.norm() < _minUpdate )
                    break;
            }

            if( scaleResult.numPixels )
                scaleResult.pixelPercentage = ( float )scaleResult.numPixels / ( float )num;

            // TODO: ensure the result on this scale is good enough (pixel percentage & error )
            result = scaleResult;
        }

        tmp4 = result.warp.poseMatrix();
        tmp4 = reference.pose() * tmp4.inverse();
        result.warp.setPose( tmp4 );
    }

    template <class WarpFunc, class LossFunc>
    inline void Optimizer<WarpFunc, LossFunc>::optimizeRobust( ResultType& result,
                                                               const Matrix4f& posePrediction,
                                                               RGBDKeyframe<WarpFunc>& reference,
                                                               const ImagePyramid& grayPyramid,
                                                               const Image& /*depthImage*/ ) const
    {
        SIMD* simd = SIMD::instance();
        Matrix4f tmp4;
        tmp4 = posePrediction.inverse() * reference.pose();

        result.warp.setPose( tmp4 );
        result.costs = 0.0f;
        result.iterations = 0;
        result.numPixels = 0;
        result.pixelPercentage = 0.0f;

        /* TODO: robust statistics should use median of residuals for threshold */
        LossFunc weighter( _robustThreshold );

        Matrix4f projMat;
        std::vector<Vector2f> warpedPts;
        std::vector<float> interpolatedPixels;
        // sum of jacobians * delta
        JacobianType deltaSum, jtmp;
        HessianType  hessian;

        for( int o = grayPyramid.octaves() - 1; o >= 0; o-- ){
            ResultType scaleResult;
            scaleResult = result;

            const size_t width = grayPyramid[ o ].width();
            const size_t height = grayPyramid[ o ].height();

            const AlignDataType& data = reference.dataForScale( o );

            const size_t num = data.points3d.size();
            Matrix4f K4( data.intrinsics );
            const Vector3f* p3dPtr = &data.points3d[ 0 ];
            const float* referencePixVals = &data.pixelValues[ 0 ];
            const JacobianType* referenceJ = &data.jacobians[ 0 ];

            warpedPts.resize( num );
            interpolatedPixels.resize( num );

            IMapScoped<const float> grayMap( grayPyramid[ o ] );

            scaleResult.iterations = 0;
            scaleResult.numPixels = 0;
            scaleResult.pixelPercentage = 0.0f;

            hessian.setZero();

            while( scaleResult.iterations < _maxIter ){
                // build the updated projection Matrix
                projMat = K4 * scaleResult.warp.poseMatrix();

                // project the points:
                simd->projectPoints( &warpedPts[ 0 ], projMat, p3dPtr, num );

                // interpolate the pixel values
                simd->warpBilinear1f( &interpolatedPixels[ 0 ], &warpedPts[ 0 ].x, grayMap.ptr(), grayMap.stride(), width, height, -1.0f, num );

                deltaSum.setZero();
                scaleResult.numPixels = 0;
                scaleResult.costs = 0.0f;
                for( size_t i = 0; i < num; i++ ){
                    // compute the delta
                    if( interpolatedPixels[ i ] >= 0.0f ){
                        float delta = result.warp.computeResidual( referencePixVals[ i ], interpolatedPixels[ i ] );
                        scaleResult.costs += Math::sqr( delta );
                        scaleResult.numPixels++;

                        float weight = weighter.weight( delta );
                        jtmp = weight * referenceJ[ i ];

                        hessian.noalias() += jtmp.transpose() * referenceJ[ i ];
                        deltaSum.noalias() += jtmp * delta;
                    }
                }

                if( !scaleResult.numPixels ){
                    break;
                }

                // evaluate the delta parameters
                DeltaType deltaP = -hessian.inverse() * deltaSum.transpose();
                scaleResult.warp.updateParameters( deltaP );

                scaleResult.iterations++;
                if( deltaP.norm() < _minUpdate )
                    break;
            }

            if( scaleResult.numPixels )
                scaleResult.pixelPercentage = ( float )scaleResult.numPixels / ( float )num;

            // TODO: ensure the result on this scale is good enough (pixel percentage & error )
            result = scaleResult;
        }

        tmp4 = result.warp.poseMatrix();
        tmp4 = reference.pose() * tmp4.inverse();
        result.warp.setPose( tmp4 );
    }

}

#endif // OPTIMIZER_H