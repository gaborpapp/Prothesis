#include <boost/foreach.hpp>

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

void User::update( XnSkeletonJoint jointId, Vec2f pos )
{
	mJointPositions.insert( pair< XnSkeletonJoint, Vec2f >( jointId, pos ));

	Vec2f strokePos = pos / Vec2f( 640, 480 );
	Stroke *stroke = findStroke( jointId );
	if( stroke )
	{
		stroke->setStiffness( mUserManager->mK );
		stroke->setDamping( mUserManager->mDamping );
		stroke->setStrokeMinWidth( mUserManager->mStrokeMinWidth );
		stroke->setStrokeMaxWidth( mUserManager->mStrokeMaxWidth );
		stroke->setMaxVelocity( mUserManager->mMaxVelocity );

		stroke->setActive( mUserManager->getStrokeActive( jointId ));
		stroke->setBrush ( mUserManager->getStrokeBrush ( jointId ));

		stroke->update( strokePos );
	}
}

void User::addStroke( XnSkeletonJoint jointId )
{
	Stroke *stroke = createStroke( jointId );

	stroke->resize( ResizeEvent( mUserManager->mFbo.getSize()));
	stroke->setStiffness( mUserManager->mK );
	stroke->setDamping( mUserManager->mDamping );
	stroke->setStrokeMinWidth( mUserManager->mStrokeMinWidth );
	stroke->setStrokeMaxWidth( mUserManager->mStrokeMaxWidth );
	stroke->setMaxVelocity( mUserManager->mMaxVelocity );
}

void User::clearStrokes()
{
	for( JointStrokes::iterator it = mJointStrokes.begin(); it != mJointStrokes.end(); ++it )
	{
		it->second->clear();
	}
}

Stroke *User::createStroke( XnSkeletonJoint jointId )
{
	Stroke *stroke = findStroke( jointId );

	if( ! stroke )
	{
		stroke = new Stroke();
		mJointStrokes[ jointId ] = shared_ptr< Stroke >( stroke );
	}

	return stroke;
}

void User::destroyStroke( XnSkeletonJoint jointId )
{
	if( ! findStroke( jointId ))
		return;

	mJointStrokes.erase( jointId );
}

Stroke *User::findStroke( XnSkeletonJoint jointId )
{
	JointStrokes::iterator it = mJointStrokes.find( jointId );
	if( it == mJointStrokes.end())
		return 0;

	return &(*it->second);
}

void User::draw()
{
	if( mUserManager->mJointShow )
		drawJoints();
	if( mUserManager->mBodyLineShow )
		drawBodyLines();

	drawStrokes();
}

void User::drawStrokes()
{
	for( JointStrokes::iterator it = mJointStrokes.begin(); it != mJointStrokes.end(); ++it )
		it->second->draw();
}

void User::drawJoints()
{
	gl::color( mUserManager->mJointColor );
	float sc = mUserManager->mOutputRect.getWidth() / 640.0f;
	float scaledJointSize = mUserManager->mJointSize * sc;
	for( JointPositions::const_iterator it = mJointPositions.begin(); it != mJointPositions.end(); ++it )
	{
		XnSkeletonJoint jointId = it->first;

		if( jointId == XN_SKEL_NECK
		 || jointId == XN_SKEL_LEFT_HIP
		 || jointId == XN_SKEL_RIGHT_HIP )
			continue;

		Vec2f pos = it->second;
		gl::drawSolidCircle( pos, scaledJointSize );
	}
	gl::color( ColorA( 1, 1, 1, 1 ));
}

void User::drawBodyLines()
{
	gl::color( mUserManager->mJointColor );

	drawBodyLine( XN_SKEL_LEFT_HAND     , XN_SKEL_LEFT_SHOULDER  );
	drawBodyLine( XN_SKEL_LEFT_SHOULDER , XN_SKEL_NECK           );
	drawBodyLine( XN_SKEL_RIGHT_SHOULDER, XN_SKEL_NECK           );
	drawBodyLine( XN_SKEL_RIGHT_HAND    , XN_SKEL_RIGHT_SHOULDER );
	drawBodyLine( XN_SKEL_HEAD          , XN_SKEL_NECK           );
	drawBodyLine( XN_SKEL_LEFT_SHOULDER , XN_SKEL_TORSO          );
	drawBodyLine( XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO          );
	drawBodyLine( XN_SKEL_TORSO         , XN_SKEL_LEFT_HIP       );
	drawBodyLine( XN_SKEL_TORSO         , XN_SKEL_RIGHT_HIP      );
	drawBodyLine( XN_SKEL_LEFT_HIP      , XN_SKEL_RIGHT_HIP      );
	drawBodyLine( XN_SKEL_LEFT_HIP      , XN_SKEL_LEFT_KNEE      );
	drawBodyLine( XN_SKEL_RIGHT_HIP     , XN_SKEL_RIGHT_KNEE     );
	drawBodyLine( XN_SKEL_LEFT_KNEE     , XN_SKEL_LEFT_FOOT      );
	drawBodyLine( XN_SKEL_RIGHT_KNEE    , XN_SKEL_RIGHT_FOOT     );

	gl::color( ColorA( 1, 1, 1, 1 ));
}

void User::drawBodyLine( XnSkeletonJoint jointBeg, XnSkeletonJoint jointEnd )
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

	gl::drawLine( posBeg, posEnd );
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
}

void UserManager::setup( const fs::path &path )
{
	mBrushes = loadTextures( "strokes" );

	gl::Fbo::Format format;
	format.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
	//	mFbo = gl::Fbo( 1024, 768, format );
	mFbo = gl::Fbo( 640, 480, format );

#if USE_KINECT
	if ( path.empty() )
		mNI = OpenNI( OpenNI::Device());
	else
		mNI = OpenNI( path );

	mNI.setMirrored( true );
	mNI.setDepthAligned();
	mNI.start();
	mNIUserTracker = mNI.getUserTracker();
	mNIUserTracker.addListener( this );
#endif

	mParams = params::PInterfaceGl( "Kinect", Vec2i( 300, 400 ) );
	mParams.addPersistentSizeAndPosition();
	mParams.addText("Tracking");
	mParams.addPersistentParam( "Skeleton smoothing" , &mSkeletonSmoothing, 0.9, "min=0 max=1 step=.05");
	mParams.addPersistentParam( "Joint show"         , &mJointShow   , true );
	mParams.addPersistentParam( "Line show"          , &mBodyLineShow, true );
	mParams.addPersistentParam( "Video show"         , &mVideoShow   , true );
	mParams.addPersistentParam( "Joint size"         , &mJointSize, 5.0, "min=0 max=50 step=.5" );
	mParams.addPersistentParam( "Joint Color"        , &mJointColor, ColorA::hexA( 0x50ffffff ) );

	mParams.addText("Stroke");
	mParams.addPersistentParam( "Stiffness"          , &mK             , 0.06f, "min=   0.01 max=   0.2   step= 0.01" );
	mParams.addPersistentParam( "Damping"            , &mDamping       , 0.7f , "min=   0.25 max=   0.999 step= 0.02" );
	mParams.addPersistentParam( "Stroke min"         , &mStrokeMinWidth, 6.0f , "min=   0    max=  50     step= 0.5"  );
	mParams.addPersistentParam( "Stroke width"       , &mStrokeMaxWidth, 16.0f, "min= -50    max=  50     step= 0.5"  );
	mParams.addPersistentParam( "Velocity max"       , &mMaxVelocity   , 40.0f, "min=   1    max= 100"                );
	mParams.addSeparator();

	vector< string > strokes;
	strokes.push_back( "OFF" );
	for( Brushes::iterator it = mBrushes.begin(); it != mBrushes.end(); ++it )
		strokes.push_back( it->first );

	mParams.addPersistentParam( "Left hand"     , strokes, &mStrokeLeftHand     , 1 );
	mParams.addPersistentParam( "Left shoulder" , strokes, &mStrokeLeftShoulder , 0 );
	mParams.addPersistentParam( "Head"          , strokes, &mStrokeHead         , 0 );
	mParams.addPersistentParam( "Right hand"    , strokes, &mStrokeRightHand    , 0 );
	mParams.addPersistentParam( "Right shoulder", strokes, &mStrokeRightShoulder, 0 );
	mParams.addPersistentParam( "Torso"         , strokes, &mStrokeTorso        , 0 );
	mParams.addPersistentParam( "Left knee"     , strokes, &mStrokeLeftKnee     , 0 );
	mParams.addPersistentParam( "Right knee"    , strokes, &mStrokeRightKnee    , 0 );
	mParams.addPersistentParam( "Left foot"     , strokes, &mStrokeLeftFoot     , 0 );
	mParams.addPersistentParam( "Right foot"    , strokes, &mStrokeRightFoot    , 0 );

	setBounds( app::getWindowBounds());
}

void UserManager::update()
{
#if USE_KINECT
	mNIUserTracker.setSmoothing( mSkeletonSmoothing );

	vector< unsigned > users = mNIUserTracker.getUsers();
	for( vector< unsigned >::const_iterator it = users.begin(); it != users.end(); ++it )
	{
		unsigned userId = *it;
		User *user = findUser( userId );
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
				user->update( jointId, mOutputMapping.map( jointPos ));
			}
		}
	}
#endif
}

void UserManager::draw()
{
	gl::enable( GL_SCISSOR_TEST );
	// FIXME: video is one pixel smaller?
	glScissor( (GLint)mOutputRect.getX1(), (GLint)mOutputRect.getY1() + 1, (GLsizei)mOutputRect.getWidth(), (GLsizei)mOutputRect.getHeight() - 2 );

//	gl::enableDepthRead();
//	gl::enableDepthWrite();
	gl::enableAlphaBlending();

#if USE_KINECT
	if( mNI.checkNewVideoFrame())
	{
		Surface NISurface = mNI.getVideoImage();
		mNITexture = Channel( NISurface );
	}

	if( mNITexture && mVideoShow )
		gl::draw( mNITexture, mOutputRect );
#endif

	for( Users::iterator it = mUsers.begin(); it != mUsers.end(); ++it )
	{
		it->second->draw();
	}

	gl::disableAlphaBlending();
//	gl::disableDepthRead();
//	gl::disableDepthWrite();

	gl::disable( GL_SCISSOR_TEST );
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

void UserManager::showParams( bool show )
{
	if( show )
		TwDefine( "Kinect visible=true" );
	else
		TwDefine( "Kinect visible=false" );
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

	mUsers[ userId ] = shared_ptr< User >( new User( this ));

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

User *UserManager::findUser( unsigned userId )
{
	Users::iterator it = mUsers.find( userId );
	if( it == mUsers.end())
		return 0;

	return &(*it->second);
}

bool UserManager::getStrokeActive( XnSkeletonJoint jointId )
{
	switch( jointId )
	{
	case XN_SKEL_LEFT_HAND      : return mStrokeLeftHand      == 0 ? false : true;
	case XN_SKEL_LEFT_SHOULDER  : return mStrokeLeftShoulder  == 0 ? false : true;
	case XN_SKEL_HEAD           : return mStrokeHead          == 0 ? false : true;
	case XN_SKEL_RIGHT_HAND     : return mStrokeRightHand     == 0 ? false : true;
	case XN_SKEL_RIGHT_SHOULDER : return mStrokeRightShoulder == 0 ? false : true;
	case XN_SKEL_TORSO          : return mStrokeTorso         == 0 ? false : true;
	case XN_SKEL_LEFT_KNEE      : return mStrokeLeftKnee      == 0 ? false : true;
	case XN_SKEL_RIGHT_KNEE     : return mStrokeRightKnee     == 0 ? false : true;
	case XN_SKEL_LEFT_FOOT      : return mStrokeLeftFoot      == 0 ? false : true;
	case XN_SKEL_RIGHT_FOOT     : return mStrokeRightFoot     == 0 ? false : true;
	}

	return false;
}

gl::Texture UserManager::getStrokeBrush( XnSkeletonJoint jointId )
{
	switch( jointId )
	{
	case XN_SKEL_LEFT_HAND      : if( mStrokeLeftHand      != 0 ) return mBrushes[mStrokeLeftHand      - 1 ].second; break;
	case XN_SKEL_LEFT_SHOULDER  : if( mStrokeLeftShoulder  != 0 ) return mBrushes[mStrokeLeftShoulder  - 1 ].second; break;
	case XN_SKEL_HEAD           : if( mStrokeHead          != 0 ) return mBrushes[mStrokeHead          - 1 ].second; break;
	case XN_SKEL_RIGHT_HAND     : if( mStrokeRightHand     != 0 ) return mBrushes[mStrokeRightHand     - 1 ].second; break;
	case XN_SKEL_RIGHT_SHOULDER : if( mStrokeRightShoulder != 0 ) return mBrushes[mStrokeRightShoulder - 1 ].second; break;
	case XN_SKEL_TORSO          : if( mStrokeTorso         != 0 ) return mBrushes[mStrokeTorso         - 1 ].second; break;
	case XN_SKEL_LEFT_KNEE      : if( mStrokeLeftKnee      != 0 ) return mBrushes[mStrokeLeftKnee      - 1 ].second; break;
	case XN_SKEL_RIGHT_KNEE     : if( mStrokeRightKnee     != 0 ) return mBrushes[mStrokeRightKnee     - 1 ].second; break;
	case XN_SKEL_LEFT_FOOT      : if( mStrokeLeftFoot      != 0 ) return mBrushes[mStrokeLeftFoot      - 1 ].second; break;
	case XN_SKEL_RIGHT_FOOT     : if( mStrokeRightFoot     != 0 ) return mBrushes[mStrokeRightFoot     - 1 ].second; break;
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
	mUsers[ 0 ] = shared_ptr< User >( new User( this ));
	mUsers[ 0 ]->addStroke( XN_SKEL_LEFT_HAND );

	return true;
}

bool UserManager::mouseDrag( ci::app::MouseEvent event )
{
	mUsers[ 0 ]->update( XN_SKEL_LEFT_HAND, event.getPos() );

	return true;
}

bool UserManager::mouseUp( ci::app::MouseEvent event )
{
	destroyUser( 0 );

	return true;
}

} // namespace cinder

