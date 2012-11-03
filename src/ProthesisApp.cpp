#include "cinder/app/AppBasic.h"
#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "cinder/Rect.h"
#include "AntTweakBar.h"
#include "NIUser.h"
#include "Calibrate.h"
#include "PParams.h"
#include "Utils.h"
#include "StrokeManager.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ProthesisApp : public AppBasic
{
	public:
		ProthesisApp();

		void prepareSettings(Settings *settings);
		void setup();
		void shutdown();

		void resize(ResizeEvent event);

		void keyDown(KeyEvent event);

		void mouseDown(MouseEvent event);
		void mouseDrag(MouseEvent event);
		void mouseUp(MouseEvent event);

		void update();
		void draw();

		void showAllParams( bool show );

	private:
		UserManager   mUserManager;
		Calibrate     mCalibrate;

		gl::Fbo mFbo;

		enum MouseAction
		{
			MA_NONE      = 0,
			MA_STROKE    = 1,
			MA_CALIBRATE = 2,
		};

		// params
		params::PInterfaceGl mParams;
		float                mFps;
		float                mFadeOutStrength;
		MouseAction          mMouseAction;
};

void ProthesisApp::prepareSettings(Settings *settings)
{
	settings->setWindowSize(640, 480);
}

ProthesisApp::ProthesisApp()
{
}

void ProthesisApp::setup()
{
	gl::disableVerticalSync();

	// params
	fs::path paramsXml( getAssetPath( "params.xml" ));
	if ( paramsXml.empty() )
	{
#if defined( CINDER_MAC )
		fs::path assetPath( getResourcePath() / "assets" );
#else
		fs::path assetPath( getAppPath() / "../../assets" );
#endif
		createDirectories( assetPath );
		paramsXml = assetPath / "params.xml" ;
	}

	params::PInterfaceGl::load( paramsXml );

	mParams = params::PInterfaceGl( "Parameters", Vec2i( 200, 150 ));
	mParams.addPersistentSizeAndPosition();
	mParams.addText( "Debug" );
	mParams.addParam( "Fps", &mFps, "", true );
	mParams.addSeparator();
	mParams.addPersistentParam( "Fade strength", &mFadeOutStrength, 0.001f, "min=0. max=1. step=0.001" );

	vector< string > mouseActions;
	mouseActions.push_back( "None"      );
	mouseActions.push_back( "Stroke"    );
	mouseActions.push_back( "Calibrate" );
	mMouseAction = MA_NONE;
	mParams.addParam( "Mouse action", mouseActions, (int*)&mMouseAction );

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
	catch( ... ) // TODO: catching std::exception or ci::Exception does not work
	{
		console() << "Could not open Kinect" << endl;
		quit();
	}

// 	registerMouseDown( &mUserManager, &UserManager::mouseDown );
// 	registerMouseUp( &mUserManager, &UserManager::mouseUp );
// 	registerMouseDrag( &mUserManager, &UserManager::mouseDrag );

	//setFullScreen( true );
	//hideCursor();

	gl::Fbo::Format format;
	format.enableDepthBuffer( false );
	format.setSamples( 4 );
	format.setColorInternalFormat( GL_RGBA32F_ARB );
	mFbo = gl::Fbo( 2048, 1536, format );

	mUserManager.setFbo( mFbo );

	StrokeManager::setup( mFbo.getSize());
	mCalibrate.setup();

	mFbo.bindFramebuffer();
	gl::clear( Color::white() );
	mFbo.unbindFramebuffer();

	showAllParams( false );
}

void ProthesisApp::shutdown()
{
	params::PInterfaceGl::save();
}

void ProthesisApp::resize( ResizeEvent event )
{
}

void ProthesisApp::keyDown(KeyEvent event)
{
	if( event.getChar() == 'f' )
	{
		setFullScreen( ! isFullScreen());
		if( isFullScreen())
			hideCursor();
		else
			showCursor();
	}
	else if( event.getChar() == 's' )
	{
		showAllParams( ! mParams.isVisible());
		if( isFullScreen())
		{
			if( ! mParams.isVisible())
				hideCursor();
			else
				showCursor();
		}
	}

	if( event.getCode() == KeyEvent::KEY_SPACE )
	{
		mUserManager.clearStrokes();
		mFbo.bindFramebuffer();
		gl::clear( Color::white() );
		mFbo.unbindFramebuffer();
	}
	else if( event.getCode() == KeyEvent::KEY_ESCAPE )
		quit();
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

void ProthesisApp::update()
{
	mFps = getAverageFps();
	mUserManager.update();
}

void ProthesisApp::draw()
{
	mFbo.bindFramebuffer();
	gl::setMatricesWindow( mFbo.getSize(), false );
	gl::setViewport( mFbo.getBounds() );

	mUserManager.draw( mCalibrate );

	gl::enableAlphaBlending();
	gl::color( ColorA::gray( 1.f, mFadeOutStrength ) );
	gl::drawSolidRect( mFbo.getBounds() );
	gl::color( Color::white() );
	gl::disableAlphaBlending();

	mFbo.unbindFramebuffer();

	gl::setMatricesWindow( getWindowSize() );
	gl::setViewport( getWindowBounds() );

	gl::clear( Color::black() );
	gl::draw( mFbo.getTexture(), getWindowBounds() );

	gl::color( Color::black());
	gl::drawSolidRect( mCalibrate.getCoverLeft());
	gl::drawSolidRect( mCalibrate.getCoverTop());
	gl::drawSolidRect( mCalibrate.getCoverRight());
	gl::drawSolidRect( mCalibrate.getCoverBottom());

	params::InterfaceGl::draw();
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

CINDER_APP_BASIC( ProthesisApp, RendererGl() )

