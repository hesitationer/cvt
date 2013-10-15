
#include <cvt/util/CVTTest.h>
#include <cvt/util/Time.h>
#include <cvt/math/Vector.h>
#include <cvt/geom/KDTree.h>

namespace cvt {

    template <size_t dim>
    static void generateVectors( std::vector<typename Vector<dim, float>::TYPE >& pts, size_t n )
    {
        typename Vector<dim, float>::TYPE vec;
        while( n-- ){
            for( size_t i = 0; i < dim; i++ ){
                vec[ i ] = Math::rand( -4000.0f, 4000.0f );
            }
            pts.push_back( vec );
        }
    }

    template <size_t dim>
    static bool checkRangeResult( const std::vector< typename Vector<dim, float>::TYPE > & rangedPts,
                                  const std::vector< typename Vector<dim, float>::TYPE > & all,
                                  const typename Vector<dim, float>::TYPE & pt,
                                  float range )
    {
        size_t num = 0;
        for( size_t i = 0; i < all.size(); i++ ){
            if( ( all[ i ] - pt ).length() < range ){
                num++;
                bool inRange = false;
                for( size_t k = 0; k < rangedPts.size(); k++ ){
                    if( rangedPts[ k ] == all[ i ] ){
                        inRange = true;
                        break;
                    }
                }
                if( !inRange ){
                    std::cout << "RangeTest failed: center: " << pt << " Range: " << range << " Point not captured: " << all[ i ] << std::endl;
                    return false;
                }
            }
        }

        if( num != rangedPts.size() ){
            std::cout << "KDTree found " << rangedPts.size() << " points in range, true: " << num << std::endl;
            return false;
        }

        return true;
    }

    template <size_t dim>
    static bool rangeTest()
    {
        typedef typename Vector<dim, float >::TYPE VecType;
        std::vector<VecType> data;
        generateVectors<dim>( data, 100000 );

        // create the KD Tree
        //Time t;
        KDTree<VecType> kdtree( data );
        //std::cout << "KDTree Construction: " << t.elapsedMilliSeconds() << "ms" << std::endl;

        std::vector<VecType> kresult;
        VecType  pt;
        for( size_t i = 0; i < dim; i++ )
            pt[ dim ] = Math::rand( -50.0f, 50.0f );

        float range = Math::rand( 0.0f, 50.0f );
        kdtree.rangeSearch( kresult, pt, range );
        bool b = checkRangeResult<dim>( kresult, data, pt, range );

        return b;
    }

}

BEGIN_CVTTEST( KDTree )
    bool ret = true;

    ret &= cvt::rangeTest<2>();
    ret &= cvt::rangeTest<2>();
    ret &= cvt::rangeTest<2>();
    CVTTEST_PRINT( "range test Vector 2", ret );
    ret &= cvt::rangeTest<3>();
    ret &= cvt::rangeTest<3>();
    ret &= cvt::rangeTest<3>();
    CVTTEST_PRINT( "range test Vector 3", ret );
    ret &= cvt::rangeTest<4>();
    ret &= cvt::rangeTest<4>();
    ret &= cvt::rangeTest<4>();
    CVTTEST_PRINT( "range test Vector 4", ret );

    return ret;
END_CVTTEST
