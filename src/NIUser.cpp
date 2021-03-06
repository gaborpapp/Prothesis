#include <boost/foreach.hpp>
#include <boost/assign/std/vector.hpp>

#include "cinder/app/App.h"
#include "cinder/ip/grayscale.h"
#include "AntTweakBar.h"
#include "NIUser.h"
#include "Utils.h"
using namespace std;
using namespace ci;
using namespace ci::app;
using namespace mndl::ni;

namespace cinder
{

/*
visualize kinect joints

        o
        |
o--o--o---o--o--o
       \ /
        o
       / \
      |   |
      o   o
      |   |
      o   o
*/

User::User( UserManager *userManager )
: mUserManager( userManager )
{
}

void User::clearPoints()
{
	mJointPositions.clear();
}

void User::update()
{
	mStrokeManager.update();
}

void User::addPos( XnSkeletonJoint jointId, Vec2f pos )
{
	mJointPositions.insert( pair< XnSkeletonJoint, Vec2f >( jointId, pos ));

	Vec2f strokePos = pos / mUserManager->mOutputRect.getSize();

	mStrokeManager.setActive( jointId , mUserManager->getStrokeActive( jointId ));
	mStrokeManager.setBrush ( jointId , mUserManager->getStrokeBrush ( jointId ));
	mStrokeManager.addPos( jointId , strokePos );

	if( jointId == mUserManager->mJointRef )
		mPosRef = pos;
}

void User::addStroke( XnSkeletonJoint jointId )
{
	mStrokeManager.createStroke( jointId );
}

void User::clearStrokes()
{
	mStrokeManager.clear();
}

void User::drawStroke( const Calibrate &calibrate )
{
	Vec2f strokePos = mPosRef / mUserManager->mOutputRect.getSize();
	mStrokeManager.draw( calibrate, strokePos );
}

void User::drawBody( const Calibrate &calibrate )
{
	if( mUserManager->mJointShow )
		drawJoints( calibrate );
	if( mUserManager->mLineShow )
		drawLines( calibrate );
}

void User::drawJoints( const Calibrate &calibrate )
{
	gl::color( mUserManager->mJointColor );
	float sc = mUserManager->mOutputRect.getWidth() / 640.0f;
	float scaledJointSize = mUserManager->mJointSize * sc;
	RectMapping mapping( mUserManager->mOutputRect, mUserManager->mSourceBounds );

	for( JointPositions::const_iterator it = mJointPositions.begin(); it != mJointPositions.end(); ++it )
	{
		XnSkeletonJoint jointId = it->first;

		if( jointId == XN_SKEL_NECK
		 || jointId == XN_SKEL_LEFT_HIP
		 || jointId == XN_SKEL_RIGHT_HIP )
			continue;

		Vec2f pos = it->second;
		gl::drawSolidCircle( mapping.map( calibrate.transform( pos, mPosRef )), scaledJointSize );
	}
	gl::color( ColorA( 1, 1, 1, 1 ));
}

void User::drawLines( const Calibrate &calibrate )
{
	gl::color( mUserManager->mJointColor );

	drawLine( calibrate, XN_SKEL_LEFT_HAND     , XN_SKEL_LEFT_SHOULDER  );
	drawLine( calibrate, XN_SKEL_LEFT_SHOULDER , XN_SKEL_NECK           );
	drawLine( calibrate, XN_SKEL_RIGHT_SHOULDER, XN_SKEL_NECK           );
	drawLine( calibrate, XN_SKEL_RIGHT_HAND    , XN_SKEL_RIGHT_SHOULDER );
	drawLine( calibrate, XN_SKEL_HEAD          , XN_SKEL_NECK           );
	drawLine( calibrate, XN_SKEL_LEFT_SHOULDER , XN_SKEL_TORSO          );
	drawLine( calibrate, XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO          );
	drawLine( calibrate, XN_SKEL_TORSO         , XN_SKEL_LEFT_HIP       );
	drawLine( calibrate, XN_SKEL_TORSO         , XN_SKEL_RIGHT_HIP      );
	drawLine( calibrate, XN_SKEL_LEFT_HIP      , XN_SKEL_RIGHT_HIP      );
	drawLine( calibrate, XN_SKEL_LEFT_HIP      , XN_SKEL_LEFT_KNEE      );
	drawLine( calibrate, XN_SKEL_RIGHT_HIP     , XN_SKEL_RIGHT_KNEE     );
	drawLine( calibrate, XN_SKEL_LEFT_KNEE     , XN_SKEL_LEFT_FOOT      );
	drawLine( calibrate, XN_SKEL_RIGHT_KNEE    , XN_SKEL_RIGHT_FOOT     );

	gl::color( ColorA( 1, 1, 1, 1 ));
}

void User::drawLine( const Calibrate &calibrate, XnSkeletonJoint jointBeg, XnSkeletonJoint jointEnd )
{
	JointPositions::iterator it;
	it = mJointPositions.find( jointBeg );
	if( it == mJointPositions.end())
		return;

	Vec2f posBeg = it->second;

	it = mJointPositions.find( jointEnd );
	if( it == mJointPositions.end())
		return;

	Vec2f posEnd = it->second;

	RectMapping mapping( mUserManager->mOutputRect, mUserManager->mSourceBounds );
	gl::drawLine( mapping.map( calibrate.transform( posBeg, mPosRef )), mapping.map( calibrate.transform( posEnd, mPosRef )));
}

UserManager::UserManager()
: mJointColor( ColorA::hexA( 0x50ffffff ))
{
	mJoints.push_back( XN_SKEL_LEFT_HAND      );
	mJoints.push_back( XN_SKEL_LEFT_SHOULDER  );
	mJoints.push_back( XN_SKEL_HEAD           );
	mJoints.push_back( XN_SKEL_RIGHT_HAND     );
	mJoints.push_back( XN_SKEL_RIGHT_SHOULDER );
	mJoints.push_back( XN_SKEL_TORSO          );
	mJoints.push_back( XN_SKEL_LEFT_KNEE      );
	mJoints.push_back( XN_SKEL_RIGHT_KNEE     );
	mJoints.push_back( XN_SKEL_LEFT_FOOT      );
	mJoints.push_back( XN_SKEL_RIGHT_FOOT     );
	mJoints.push_back( XN_SKEL_NECK           );  // will not be visible only for body line
	mJoints.push_back( XN_SKEL_LEFT_HIP       );  // will not be visible only for body line
	mJoints.push_back( XN_SKEL_RIGHT_HIP      );  // will not be visible only for body line

	mJointRef = XN_SKEL_TORSO;
}

UserManager::~UserManager()
{
	mThread.join();
}

void UserManager::setup( const fs::path &path )
{
	mBrushes = loadTextures( "strokes" );
	for ( Brushes::iterator it = mBrushes.begin(); it != mBrushes.end(); ++it )
	{
		it->second.bind();
		it->second.setWrap( GL_REPEAT, GL_REPEAT );
		it->second.unbind();
	}

	mThread = thread( bind( &UserManager::openKinect, this, path ) );

	mParams = mndl::params::PInterfaceGl( "Kinect", Vec2i( 250, 500 ), Vec2i( 224, 16 ) );
	mParams.addPersistentSizeAndPosition();
	mParams.addText("Tracking");
	mKinectProgress = "Connecting...\0\0\0\0\0\0\0\0\0";
	mParams.addParam( "Kinect", &mKinectProgress, "", true );
	mParams.addPersistentParam( "Skeleton smoothing" , &mSkeletonSmoothing, 0.9, "min=0 max=1 step=.05");
	mParams.addPersistentParam( "Joint show"         , &mJointShow, true  );
	mParams.addPersistentParam( "Line show"          , &mLineShow , true  );
	mParams.addPersistentParam( "Mirror"             , &mVideoMirrored, false );
	mParams.addPersistentParam( "Video show"         , &mVideoShow, false );
	mParams.addPersistentParam( "Joint size"         , &mJointSize, 5.0, "min=0 max=50 step=.5" );
	mParams.addPersistentParam( "Joint Color"        , &mJointColor, ColorA::hexA( 0x50c81e1e ) );

	mParams.addSeparator();

	vector< string > strokes;
	strokes.push_back( "No Stroke" );
	for( Brushes::iterator it = mBrushes.begin(); it != mBrushes.end(); ++it )
		strokes.push_back( it->first );
	int strokePos  = 1;
	int strokeSize = mBrushes.size();

	using namespace boost::assign;
	vector< string > jointNames;
	jointNames += "Left hand", "Left shoulder", "Head", "Right hand", "Right shoulder",
		"Torso", "Left knee", "Right knee", "Left foot", "Right foot";
	std::vector< std::pair< std::string, boost::any > > vars;
	for( size_t i = 0; i < jointNames.size(); i++ )
	{
		mParams.addPersistentParam( jointNames[ i ] + " active", &mStrokeActive[ i ], true );
		mParams.addPersistentParam( jointNames[ i ], strokes, &mStrokeSelect[ i ],
				strokePos <= strokeSize ? strokePos : 0 ); ++strokePos;
		vars.push_back( make_pair( jointNames[ i ] + " active", &mStrokeActive[ i ] ) );
		vars.push_back( make_pair( jointNames[ i ], &mStrokeSelect[ i ] ) );
	}
	vars.push_back( make_pair( "Skeleton smoothing", &mSkeletonSmoothing ) );
	mParams.addSeparator();
	mParams.addPresets( vars );

	mParams.setOptions( "", "refresh=.3" );


	mSourceBounds = app::getWindowBounds();
	setBounds( mSourceBounds );
}

void UserManager::openKinect( const fs::path &path )
{
	try
	{
		mndl::ni::OpenNI kinect;
		if ( path.empty() )
			kinect = OpenNI( OpenNI::Device() );
		else
			kinect = OpenNI( path );
		{
			std::lock_guard< std::mutex > lock( mMutex );
			mNI = kinect;
		}
	}
	catch ( ... )
	{
		if ( path.empty() )
			mKinectProgress = "No device detected";
		else
			mKinectProgress = "Recording not found";
		return;
	}

	if ( path.empty() )
		mKinectProgress = "Connected";
	else
		mKinectProgress = "Recording loaded";

	{
		std::lock_guard< std::mutex > lock( mMutex );
		mNI.setDepthAligned();
		mNI.start();
		mNIUserTracker = mNI.getUserTracker();
		mNIUserTracker.addListener( this );
	}
}

void UserManager::update()
{
	for( Users::const_iterator it = mUsers.begin(); it != mUsers.end(); ++it )
	{
		UserRef user = it->second;

		user->update();
	}

	{
		std::lock_guard< std::mutex > lock( mMutex );
		if ( !mNI )
			return;
	}

	mNIUserTracker.setSmoothing( mSkeletonSmoothing );
	if ( mNI.isMirrored() != mVideoMirrored )
		mNI.setMirrored( mVideoMirrored );

	vector< unsigned > users = mNIUserTracker.getUsers();
	for( vector< unsigned >::const_iterator it = users.begin(); it != users.end(); ++it )
	{
		unsigned userId = *it;
		UserRef user = findUser( userId );
		if( ! user )
			continue;

		user->clearPoints();
		for( Joints::const_iterator it = mJoints.begin(); it != mJoints.end(); ++it )
		{
			XnSkeletonJoint jointId = *it;
			float conf = 0;
			Vec2f jointPos = mNIUserTracker.getJoint2d( userId, jointId, &conf );

			if( conf > .9 )
			{
				user->addPos( jointId, mOutputMapping.map( jointPos ));
			}
		}
	}
}

void UserManager::drawStroke( const Calibrate &calibrate )
{
	gl::enableAlphaBlending();

	for( Users::iterator it = mUsers.begin(); it != mUsers.end(); ++it )
	{
		it->second->drawStroke( calibrate );
	}

	gl::disableAlphaBlending();
}

void UserManager::drawBody( const Calibrate &calibrate )
{
	gl::enableAlphaBlending();


	if( mNI && mNI.checkNewVideoFrame())
	{
		mNITexture = gl::Texture( mNI.getVideoImage() );
	}

	if( mNITexture && mVideoShow )
	{
		gl::color( Color::white() );
		gl::draw( mNITexture, mSourceBounds );
	}

	for( Users::iterator it = mUsers.begin(); it != mUsers.end(); ++it )
	{
		it->second->drawBody( calibrate );
	}

	gl::disableAlphaBlending();
}

void UserManager::setBounds( const Rectf &rect )
{
	mOutputRect = rect;

	Rectf kRect( 0, 0, 640, 480 ); // kinect image rect
	Rectf dRect = kRect.getCenteredFit( mOutputRect, true );
	if( mOutputRect.getAspectRatio() > dRect.getAspectRatio() )
		dRect.scaleCentered( mOutputRect.getWidth() / dRect.getWidth() );
	else
		dRect.scaleCentered( mOutputRect.getHeight() / dRect.getHeight() );

	mOutputMapping = RectMapping( kRect, dRect, true );
}

void UserManager::setSourceBounds( const Area &area )
{
	mSourceBounds = area;
}

void UserManager::clearStrokes()
{
	for( Users::iterator it = mUsers.begin(); it != mUsers.end(); ++it )
	{
		it->second->clearStrokes();
	}
}

void UserManager::createUser( unsigned userId )
{
	if( findUser( userId ))
		return;

	mUsers[ userId ] = UserRef( new User( this ));

	for( Joints::const_iterator it = mJoints.begin(); it != mJoints.end(); ++it )
	{
		XnSkeletonJoint jointId = *it;

		if( jointId == XN_SKEL_NECK
		 || jointId == XN_SKEL_LEFT_HIP
		 || jointId == XN_SKEL_RIGHT_HIP )
			continue;

		mUsers[ userId ]->addStroke( jointId );
	}
}

void UserManager::destroyUser( unsigned userId )
{
	if( ! findUser( userId ))
		return;

	mUsers.erase( userId );
}

UserManager::UserRef UserManager::findUser( unsigned userId )
{
	Users::iterator it = mUsers.find( userId );
	if( it == mUsers.end())
		return UserRef();

	return it->second;
}

bool UserManager::getStrokeActive( XnSkeletonJoint jointId )
{
	switch( jointId )
	{
	case XN_SKEL_LEFT_HAND      : return mStrokeActive[ LEFT_HAND      ];
	case XN_SKEL_LEFT_SHOULDER  : return mStrokeActive[ LEFT_SHOULDER  ];
	case XN_SKEL_HEAD           : return mStrokeActive[ HEAD           ];
	case XN_SKEL_RIGHT_HAND     : return mStrokeActive[ RIGHT_HAND     ];
	case XN_SKEL_RIGHT_SHOULDER : return mStrokeActive[ RIGHT_SHOULDER ];
	case XN_SKEL_TORSO          : return mStrokeActive[ TORSO          ];
	case XN_SKEL_LEFT_KNEE      : return mStrokeActive[ LEFT_KNEE      ];
	case XN_SKEL_RIGHT_KNEE     : return mStrokeActive[ RIGHT_KNEE     ];
	case XN_SKEL_LEFT_FOOT      : return mStrokeActive[ LEFT_FOOT      ];
	case XN_SKEL_RIGHT_FOOT     : return mStrokeActive[ RIGHT_FOOT     ];
	default: return false;
	}

	return false;
}

gl::Texture UserManager::getStrokeBrush( XnSkeletonJoint jointId )
{
	switch( jointId )
	{
	case XN_SKEL_LEFT_HAND      : if( mStrokeSelect[ LEFT_HAND      ] != 0 ) return mBrushes[ mStrokeSelect[ LEFT_HAND      ] - 1 ].second; break;
	case XN_SKEL_LEFT_SHOULDER  : if( mStrokeSelect[ LEFT_SHOULDER  ] != 0 ) return mBrushes[ mStrokeSelect[ LEFT_SHOULDER  ] - 1 ].second; break;
	case XN_SKEL_HEAD           : if( mStrokeSelect[ HEAD           ] != 0 ) return mBrushes[ mStrokeSelect[ HEAD           ] - 1 ].second; break;
	case XN_SKEL_RIGHT_HAND     : if( mStrokeSelect[ RIGHT_HAND     ] != 0 ) return mBrushes[ mStrokeSelect[ RIGHT_HAND     ] - 1 ].second; break;
	case XN_SKEL_RIGHT_SHOULDER : if( mStrokeSelect[ RIGHT_SHOULDER ] != 0 ) return mBrushes[ mStrokeSelect[ RIGHT_SHOULDER ] - 1 ].second; break;
	case XN_SKEL_TORSO          : if( mStrokeSelect[ TORSO          ] != 0 ) return mBrushes[ mStrokeSelect[ TORSO          ] - 1 ].second; break;
	case XN_SKEL_LEFT_KNEE      : if( mStrokeSelect[ LEFT_KNEE      ] != 0 ) return mBrushes[ mStrokeSelect[ LEFT_KNEE      ] - 1 ].second; break;
	case XN_SKEL_RIGHT_KNEE     : if( mStrokeSelect[ RIGHT_KNEE     ] != 0 ) return mBrushes[ mStrokeSelect[ RIGHT_KNEE     ] - 1 ].second; break;
	case XN_SKEL_LEFT_FOOT      : if( mStrokeSelect[ LEFT_FOOT      ] != 0 ) return mBrushes[ mStrokeSelect[ LEFT_FOOT      ] - 1 ].second; break;
	case XN_SKEL_RIGHT_FOOT     : if( mStrokeSelect[ RIGHT_FOOT     ] != 0 ) return mBrushes[ mStrokeSelect[ RIGHT_FOOT     ] - 1 ].second; break;
	default: return gl::Texture();
	}

	return gl::Texture();
}

void UserManager::newUser( UserTracker::UserEvent event )
{
	console() << "new user: " << event.id << endl;
}

void UserManager::calibrationBeg( UserTracker::UserEvent event )
{
	console() << "user calib beg: " << event.id << endl;
}

void UserManager::calibrationEnd( UserTracker::UserEvent event )
{
	console() << "app calib end: " << event.id << endl;
	createUser( event.id );
}

void UserManager::lostUser( UserTracker::UserEvent event )
{
	console() << "lost user: " << event.id << endl;
	destroyUser( event.id );
}

bool UserManager::mouseDown( ci::app::MouseEvent event )
{
	mUsers[ 0 ] = UserRef( new User( this ));
	mUsers[ 0 ]->addStroke( XN_SKEL_LEFT_HAND );
	RectMapping mapping( mSourceBounds, mOutputRect );
	mUsers[ 0 ]->addPos( XN_SKEL_LEFT_HAND, mapping.map( event.getPos() ) );

	return true;
}

bool UserManager::mouseDrag( ci::app::MouseEvent event )
{
	RectMapping mapping( mSourceBounds, mOutputRect );
	if ( mUsers.find( 0 ) != mUsers.end() )
	mUsers[ 0 ]->addPos( XN_SKEL_LEFT_HAND, mapping.map( event.getPos() ) );

	return true;
}

bool UserManager::mouseUp( ci::app::MouseEvent event )
{
	destroyUser( 0 );

	return true;
}

void UserManager::keyUp( KeyEvent event )
{
	switch( event.getCode())
	{
	case KeyEvent::KEY_0 :
	case KeyEvent::KEY_1 :
	case KeyEvent::KEY_2 :
	case KeyEvent::KEY_3 :
	case KeyEvent::KEY_4 :
	case KeyEvent::KEY_5 :
	case KeyEvent::KEY_6 :
	case KeyEvent::KEY_7 :
	case KeyEvent::KEY_8 :
	case KeyEvent::KEY_9 :
		{
			int pos = event.getCode() - KeyEvent::KEY_0;
			mStrokeActive[ pos ] = ! mStrokeActive[ pos ];
		}
		break;
	}
}

} // namespace cinder
