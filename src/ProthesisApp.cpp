#include "cinder/app/AppBasic.h"
#include "cinder/Cinder.h"
#include "AntTweakBar.h"
#include "NIUser.h"
#include "PParams.h"
#include "Utils.h"

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

	private:
		UserManager mUserManager;

		gl::Fbo mFbo;

		// params
		params::PInterfaceGl mParams;
		float                mFps;
		float                mFadeOutStrength;
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

	TwDefine( "TW_HELP visible=false" );
	mParams = params::PInterfaceGl("Parameters", Vec2i(300, 200));
	mParams.addPersistentSizeAndPosition();
	mParams.addText( "Debug" );
	mParams.addParam( "Fps", &mFps, "", true );
	mParams.addSeparator();
	mParams.addPersistentParam( "Fade strength", &mFadeOutStrength, 0.001f,
			"min=0. max=1. step=0.001" );

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
		console() << "Could not open Kinect "  << endl;
		quit();
	}

	registerMouseDown( &mUserManager, &UserManager::mouseDown );
	registerMouseUp( &mUserManager, &UserManager::mouseUp );
	registerMouseDrag( &mUserManager, &UserManager::mouseDrag );

	//setFullScreen( true );
	//hideCursor();

	mParams.hide();
	mUserManager.showParams( false );

	gl::Fbo::Format format;
	format.enableDepthBuffer( false );
	//format.setSamples( 4 );
	format.setColorInternalFormat( GL_RGBA32F_ARB );
	mFbo = gl::Fbo( 2048, 1536, format );

	mUserManager.setFbo( mFbo );

	mFbo.bindFramebuffer();
	gl::clear( Color::white() );
	mFbo.unbindFramebuffer();
}

void ProthesisApp::shutdown()
{
	params::PInterfaceGl::save();
}

void ProthesisApp::resize(ResizeEvent event)
{
	Area area = getWindowBounds();
	mUserManager.setBounds( area );
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
		bool visible = mParams.isVisible();
		mParams.show( ! visible );
		mUserManager.showParams( ! visible );
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
}

void ProthesisApp::mouseDrag(MouseEvent event)
{
}

void ProthesisApp::mouseUp(MouseEvent event)
{
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

	mUserManager.draw();

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

	params::InterfaceGl::draw();
}

CINDER_APP_BASIC( ProthesisApp, RendererGl() )

