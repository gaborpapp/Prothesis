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

		// params
		params::PInterfaceGl mParams;
		float                mFps;
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
	GLint n;
	glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS_EXT, &n );
	console() << "max fbo color attachments " << n << endl;

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
	catch( ... )
	{
		console() << "Could not open Kinect" << endl;
		quit();
	}

	setFullScreen( true );
	hideCursor();

	mParams.hide();
	mUserManager.showParams( false );
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
		mUserManager.clearStrokes();
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
	gl::clear( Color::black() );

	mUserManager.draw();

	params::InterfaceGl::draw();
}

CINDER_APP_BASIC( ProthesisApp, RendererGl( RendererGl::AA_NONE ))

