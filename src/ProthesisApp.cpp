#include <boost/logic/tribool.hpp>
#include <boost/assign/std/vector.hpp>

#include "cinder/app/AppBasic.h"
#include "cinder/Cinder.h"
#include "cinder/Display.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Rect.h"
#include "AntTweakBar.h"
#include "Calibrate.h"
#include "Kaleidoscope.h"
#include "NIUser.h"
#include "PParams.h"
#include "Resources.h"
#include "StrokeManager.h"
#include "Utils.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace boost::assign;

class ProthesisApp : public AppBasic
{
	public:
		ProthesisApp();

		void prepareSettings(Settings *settings);
		void setup();
		void setupDisplays();
		void shutdown();

		void resize();

		void keyDown(KeyEvent event);
		void keyUp(KeyEvent event );

		void mouseDown(MouseEvent event);
		void mouseDrag(MouseEvent event);
		void mouseUp(MouseEvent event);

		void update();
		void draw();

		void showAllParams( bool show );
		void makeScreenshot();

	private:
		UserManager   mUserManager;
		Calibrate     mCalibrate;

		gl::Fbo mFbo;
		gl::GlslProg mBlendShader;
		int mFboPingPongId; // 1 or 2

		void clearFbo();

		enum MouseAction
		{
			MA_NONE      = 0,
			MA_STROKE    = 1,
			MA_CALIBRATE = 2,
		};

		enum BlendModes
		{
			BLENDMODE_DARKEN = 0,
			BLENDMODE_ERASE
		};

		// params
		mndl::params::PInterfaceGl mParams;
		float                mFps;
		float                mFadeOutStrength;
		int                  mBlendmode;
		MouseAction          mMouseAction;

		// multidisplay
		Vec2i mMultiSize; // size of the spanning window
		Area mOutputArea; // projected output
		Area mOutputAreaWindowed, mOutputAreaSpanning;
		Vec2i mFullScreenPos; // fullscreen window position
		boost::logic::tribool mSpanning;

		void setSpanningWindow( bool spanning );
		bool isSpanningWindow() const;

		KaleidoscopeRef mKaleidoscope;
};

void ProthesisApp::prepareSettings(Settings *settings)
{
	settings->setResizable( false );
}

ProthesisApp::ProthesisApp() :
	mSpanning( boost::logic::indeterminate )
{
}

void ProthesisApp::setup()
{
	setupDisplays();

	gl::disableVerticalSync();

	// params
	mndl::params::PInterfaceGl::load( std::string( "params.xml" ) );

	mParams = mndl::params::PInterfaceGl( "Parameters", Vec2i( 200, 150 ), Vec2i( 16, 16 ) );
	mParams.addPersistentSizeAndPosition();
	mParams.addText( "Debug" );
	mParams.addParam( "Fps", &mFps, "", true );
	mParams.addSeparator();
	mParams.addPersistentParam( "Fade strength", &mFadeOutStrength, 0.995f, "min=0. max=1. step=0.001" );

	vector< string > blendNames;
	blendNames += "Darken", "Erase";
	mBlendmode = BLENDMODE_DARKEN;
	mParams.addParam( "Blendmode", blendNames, &mBlendmode );

	vector< string > mouseActions;
	mouseActions.push_back( "None"      );
	mouseActions.push_back( "Stroke"    );
	mouseActions.push_back( "Calibrate" );
	mMouseAction = MA_NONE;
	mParams.addParam( "Mouse action", mouseActions, (int*)&mMouseAction );
	mParams.setOptions( "", "refresh=.5" );

	try
	{
#if USE_KINECT_RECORD == 0
		mUserManager.setup();
#else
		// use openni recording
		fs::path recordingPath = getAssetPath( "prothesis-capture.oni" );
		mUserManager.setup( recordingPath );
#endif /* USE_KINECT_RECORD */
	}
	catch ( const exception &exc )
	{
		console() << exc.what() << endl;
		quit();
	}

// 	registerMouseDown( &mUserManager, &UserManager::mouseDown );
// 	registerMouseUp( &mUserManager, &UserManager::mouseUp );
// 	registerMouseDrag( &mUserManager, &UserManager::mouseDrag );

	gl::Fbo::Format format;
	format.enableDepthBuffer( false );
	// FIXME: enabling MSAA results in white stripes between stroke triangles
// 	format.setSamples( 4 );
	format.setColorInternalFormat( GL_RGBA32F_ARB );
	format.enableColorBuffer( true, 4 );
	mFbo = gl::Fbo( 1024, 768, format );
	clearFbo();

	mUserManager.setFbo( mFbo );

	StrokeManager::setup( mFbo.getSize());
	mCalibrate.setup();

	mKaleidoscope = Kaleidoscope::create( mFbo.getWidth(), mFbo.getHeight() );

	try
	{
		mBlendShader = gl::GlslProg( loadResource( RES_STROKE_VERT ),
									 loadResource( RES_STROKE_FRAG ) );
	}
	catch( const std::exception &e )
	{
		app::console() << e.what() << std::endl;
	}
	mBlendShader.bind();
	mBlendShader.uniform( "background", 0 );
	mBlendShader.uniform( "brush", 1 );
	mBlendShader.unbind();

	setSpanningWindow( true );
	showAllParams( false );
}

void ProthesisApp::setupDisplays()
{
	const vector< DisplayRef > displays = Display::getDisplays();

	// main display is the first one
	for ( vector< DisplayRef >::const_iterator it = displays.begin();
			it != displays.end(); ++it )
	{
		console() << (*it)->getBounds() << " " <<
			(*it)->getWidth() << "x" << (*it)->getHeight() << endl;
	}

	if ( displays.size() == 1 )
	{
		mOutputAreaSpanning = displays[ 0 ]->getBounds();
		mOutputAreaWindowed = Area( 0, 0,
				mOutputAreaSpanning.getWidth() / 2,
				mOutputAreaSpanning.getHeight() / 2 );
		mMultiSize = Vec2i( displays[ 0 ]->getWidth(), displays[ 0 ]->getHeight() );
		mFullScreenPos = Vec2i::zero();
	}
	else
	{
		//Area primaryArea = displays[ 0 ]->getBounds();
		Area secondaryArea = displays[ 1 ]->getBounds();
		mMultiSize = Vec2i( displays[ 0 ]->getWidth() + displays[ 1 ]->getWidth(),
							math< int >::max( displays[ 0 ]->getHeight(),
											  displays[ 1 ]->getHeight() ) );
		int xS = displays[ 1 ]->getBounds().getX1();
		// secondary display on the left
		if ( xS < 0 )
		{
			mFullScreenPos = Vec2i( xS, 0 );
			mOutputAreaSpanning = secondaryArea - mFullScreenPos;
			mOutputAreaSpanning.setY1( 0 ); // top bar
			mOutputAreaWindowed = Area( 0, 0,
										mOutputAreaSpanning.getWidth() / 2,
										mOutputAreaSpanning.getHeight() / 2 );
		}
		else // secondary display on the right
		{
			mFullScreenPos = Vec2i::zero();
			mOutputAreaSpanning = secondaryArea;
			int w = secondaryArea.getWidth() / 2;
			int h = secondaryArea.getHeight() / 2;
			mOutputAreaSpanning.setY1( 0 ); // top bar
			mOutputAreaWindowed = Area( mMultiSize.x / 2 - w, 0,
										mMultiSize.x / 2, h );
		}
	}
}

void ProthesisApp::shutdown()
{
	mndl::params::PInterfaceGl::save();
}

void ProthesisApp::resize()
{
}

void ProthesisApp::setSpanningWindow( bool spanning )
{
	static Vec2i lastWindowPos = getWindowPos();

	if ( spanning == mSpanning )
		return;

	WindowRef w = getWindow();
	if ( spanning )
	{
		lastWindowPos = w->getPos();
		w->setBorderless();
		w->setSize( mMultiSize.x, mMultiSize.y );
		w->setPos( mFullScreenPos );
		w->setAlwaysOnTop();
		mSpanning = true;
		mOutputArea = mOutputAreaSpanning;
	}
	else
	{
		w->setBorderless( false );
		w->setSize( mMultiSize.x / 2, mMultiSize.y / 2 );
		w->setPos( lastWindowPos );
		w->setAlwaysOnTop( false );
		mSpanning = false;
		mOutputArea = mOutputAreaWindowed;
	}
	mUserManager.setSourceBounds( mOutputArea );
}

bool ProthesisApp::isSpanningWindow() const
{
	return mSpanning;
}

void ProthesisApp::keyDown( KeyEvent event )
{
	switch ( event.getCode() )
	{
		case KeyEvent::KEY_f:
			if ( !isSpanningWindow() )
			{
				setSpanningWindow( true );

				if ( mParams.isVisible() )
					showCursor();
				else
					hideCursor();
			}
			else
			{
				setSpanningWindow( false );
				showCursor();
			}
			break;

		case KeyEvent::KEY_s:
			showAllParams( !mParams.isVisible() );
			if ( isSpanningWindow() )
			{
				if ( mParams.isVisible() )
					showCursor();
				else
					hideCursor();
			}
			break;

		case KeyEvent::KEY_e:
			mBlendmode = BLENDMODE_ERASE;
			break;

		case KeyEvent::KEY_d:
			mBlendmode = BLENDMODE_DARKEN;
			break;

		case KeyEvent::KEY_SPACE:
			mUserManager.clearStrokes();
			clearFbo();
			break;

		case KeyEvent::KEY_ESCAPE:
			quit();
			break;

		case KeyEvent::KEY_RETURN:
			makeScreenshot();
			break;

		default:
			break;
	}
}

void ProthesisApp::keyUp( KeyEvent event )
{
	mUserManager.keyUp( event );
}

void ProthesisApp::mouseDown(MouseEvent event)
{
	switch( mMouseAction )
	{
	case MA_NONE      : /* do nothing */                 break;
	case MA_STROKE    : mUserManager.mouseDown( event ); break;
	case MA_CALIBRATE : mCalibrate  .mouseDown( event ); break;
	}
}

void ProthesisApp::mouseDrag(MouseEvent event)
{
	switch( mMouseAction )
	{
	case MA_NONE      : /* do nothing */                 break;
	case MA_STROKE    : mUserManager.mouseDrag( event ); break;
	case MA_CALIBRATE : mCalibrate  .mouseDrag( event ); break;
	}
}

void ProthesisApp::mouseUp(MouseEvent event)
{
	switch( mMouseAction )
	{
	case MA_NONE      : /* do nothing */               break;
	case MA_STROKE    : mUserManager.mouseUp( event ); break;
	case MA_CALIBRATE : mCalibrate  .mouseUp( event ); break;
	}
}

void ProthesisApp::clearFbo()
{
	mFboPingPongId = 1;
	mFbo.bindFramebuffer();
	glDrawBuffer( GL_COLOR_ATTACHMENT1_EXT );
	gl::clear( Color::white() );
	glDrawBuffer( GL_COLOR_ATTACHMENT2_EXT );
	gl::clear( Color::white() );
	mFbo.unbindFramebuffer();
}

void ProthesisApp::update()
{
	mFps = getAverageFps();
	mUserManager.update();
}

void ProthesisApp::draw()
{
	// draw and blend strokes in fbo
	mFbo.bindFramebuffer();
	// draw strokes to attachment 0
	glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT );
	if ( mBlendmode == BLENDMODE_DARKEN )
		gl::clear( ColorA::white() );
	else // BLENDMODE_ERASE
		gl::clear( ColorA( 0, 0, 0, 0 ) );

	gl::setMatricesWindow( mFbo.getSize(), false );
	gl::setViewport( mFbo.getBounds() );

	mUserManager.drawStroke( mCalibrate );

	// blend it with previous frame to attachment pingpongid
	glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT + mFboPingPongId );
	gl::color( Color::white() );
	int otherId = ( mFboPingPongId == 1 ) ? 2 : 1;
	mBlendShader.bind();
	mBlendShader.uniform( "fadeout", mFadeOutStrength );
	mBlendShader.uniform( "mode", mBlendmode );
	mFbo.getTexture( otherId ).bind( 0 ); // bind previous frame to sampler 0
	mFbo.getTexture( 0 ).bind( 1 ); // bind strokes to sampler 1
	gl::drawSolidRect( mFbo.getBounds() );
	mFbo.getTexture( otherId ).unbind();
	mFbo.getTexture( 0 ).unbind( 1 );
	mBlendShader.unbind();

	mFbo.unbindFramebuffer();

	// kaleidoscope

	if ( mKaleidoscope->isEnabled() )
	{
		gl::Texture processed = mKaleidoscope->process( mFbo.getTexture( mFboPingPongId ) );
		mFbo.bindFramebuffer();
		glDrawBuffer( GL_COLOR_ATTACHMENT3 );
		gl::clear();
		gl::setViewport( mFbo.getBounds() );
		gl::setMatricesWindow( mFbo.getSize(), false );
		gl::color( Color::white() );
		gl::draw( processed, mFbo.getBounds() );
		mFbo.unbindFramebuffer();
	}

	// draw fbo in window
	gl::setMatricesWindow( getWindowSize() );
	gl::setViewport( getWindowBounds() );

	gl::clear( Color::black() );
	if ( mKaleidoscope->isEnabled() )
	{
		gl::draw( mFbo.getTexture( 3 ), mOutputArea );
	}
	else
	{
		gl::draw( mFbo.getTexture( mFboPingPongId ), mOutputArea );
	}
	mFboPingPongId = otherId;

	mUserManager.drawBody( mCalibrate );

	gl::color( Color::black());

	RectMapping normCoverMap( Rectf( 0.f, 0.f, 1.f, 1.f ), mOutputArea );
	gl::drawSolidRect( normCoverMap.map( mCalibrate.getCoverLeft() ) );
	gl::drawSolidRect( normCoverMap.map( mCalibrate.getCoverRight() ) );
	gl::drawSolidRect( normCoverMap.map( mCalibrate.getCoverTop() ) );
	gl::drawSolidRect( normCoverMap.map( mCalibrate.getCoverBottom() ) );

	mParams.draw();
}

void ProthesisApp::showAllParams( bool show )
{
	int barCount = TwGetBarCount();

	int32_t visibleInt = show ? 1 : 0;
	for ( int i = 0; i < barCount; ++i )
	{
		TwBar *bar = TwGetBarByIndex( i );
		TwSetParam( bar, NULL, "visible", TW_PARAM_INT32, 1, &visibleInt );
	}

	TwDefine( "TW_HELP visible=false" );
}

void ProthesisApp::makeScreenshot()
{
	Surface snapshot;
	if ( mKaleidoscope->isEnabled() )
	{
		snapshot = Surface( mFbo.getTexture( 3 ) );
	}
	else
	{
		snapshot = Surface( mFbo.getTexture( mFboPingPongId ) );
	}

	fs::path screenshotFolder = getAppPath();
#ifdef CINDER_MAC
	screenshotFolder /= "..";
#endif
	screenshotFolder /= "screenshots/";
	fs::create_directory( screenshotFolder );

	string filename = "snap-" + timeStamp() + ".png";
	fs::path pngPath( screenshotFolder / fs::path( filename ) );

	try
	{
		if( ! pngPath.empty())
			writeImage( pngPath, snapshot );
	}
	catch ( ... )
	{
		console() << "unable to save image file " << pngPath << endl;
	}
}

CINDER_APP_BASIC( ProthesisApp, RendererGl() )

