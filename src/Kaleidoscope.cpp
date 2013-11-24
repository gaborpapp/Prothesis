#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#include "Kaleidoscope.h"
#include "Resources.h"

using namespace ci;

Kaleidoscope::Kaleidoscope( int w, int h )
{
	mParams = mndl::params::PInterfaceGl( "Kaleidoscope", Vec2i( 200, 150 ), Vec2i( 690, 16 ) );
	mParams.addPersistentSizeAndPosition();
	mParams.addPersistentParam( "Kaleidoscope enable", &mEnabled, false );
	mParams.addPersistentParam( "Reflection lines", &mNumReflectionLines, 3, "min=0 max=32" );
	mParams.addPersistentParam( "Reflection rotation", &mRotation, 0.f, "step=.01" );

	mParams.addPersistentParam( "Kaleidoscope X", &mCenter.x, .5f,
			"min=0 max=1 step=.005 group='Kaleidoscope Center'" );
	mParams.addPersistentParam( "Kaleidoscope Y", &mCenter.y, .5f,
			"min=0 max=1 step=.005 group='Kaleidoscope Center'" );

	gl::Fbo::Format fboFormat;
	fboFormat.enableDepthBuffer( false );
	fboFormat.setSamples( 4 );
	mFbo = gl::Fbo( w, h, fboFormat );

	try
	{
		mShader = gl::GlslProg( app::loadResource( RES_KALEIDOSCOPE_VERT ),
								app::loadResource( RES_KALEIDOSCOPE_FRAG ) );
	}
	catch ( gl::GlslProgCompileExc &exc )
	{
		app::console() << exc.what() << std::endl;
	}
}

gl::Texture Kaleidoscope::process( const ci::gl::Texture &source )
{
	if ( !mEnabled )
		return source;

	gl::SaveFramebufferBinding fboSaver;
	gl::pushMatrices();

	mFbo.bindFramebuffer();
	glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT );
	gl::setViewport( mFbo.getBounds() );
	gl::setMatricesWindow( mFbo.getSize(), false );

	if ( mShader )
	{
		float addA = float( M_PI ) / float( mNumReflectionLines );
		float a = mRotation;
		Vec3f lines[ 32 ];
		for ( int i = 0; i < mNumReflectionLines; i++ )
		{
			Vec2f v( math< float >::cos( a ), math< float >::sin( a ) );
			Vec2f p( mCenter + 0.3f * v );
			Vec2f n( -v.y, v.x );

			lines[ i ] = Vec3f( n, -p.dot( n ) ); // normalized line equation
			a += addA;
		}
		mShader.bind();
		mShader.uniform( "numReflectionLines", mNumReflectionLines );
		mShader.uniform( "lines", lines, 32 );
		mShader.uniform( "txt", 0 );
	}

	gl::color( Color::white() );
	gl::draw( source, mFbo.getBounds() );

	if ( mShader )
		mShader.unbind();

	gl::popMatrices();
	return mFbo.getTexture();
}

