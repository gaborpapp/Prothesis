#pragma once

#include <map>
#include "cinder/app/App.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Area.h"
#include "cinder/Color.h"
#include "cinder/Rect.h"
#include "cinder/Thread.h"

#include "CiNI.h"
#include "PParams.h"
#include "StrokeManager.h"
#include "Calibrate.h"

#define USE_KINECT_RECORD 0

namespace cinder {

class UserManager;

class User
{
typedef std::map< XnSkeletonJoint, ci::Vec2f > JointPositions;

public:
	User( UserManager *userManager );

	void update();
	void addPos( XnSkeletonJoint jointId, ci::Vec2f pos );

	void clearPoints();
	void addStroke( XnSkeletonJoint jointId );
	void clearStrokes();
	void drawStroke( const Calibrate &calibrate );
	void drawBody  ( const Calibrate &calibrate );

private:
	void drawJoints( const Calibrate &calibrate );
	void drawLines ( const Calibrate &calibrate );
	void drawLine  ( const Calibrate &calibrate, XnSkeletonJoint jointBeg, XnSkeletonJoint jointEnd );

private:
	UserManager     *mUserManager;
	JointPositions   mJointPositions;
	StrokeManager    mStrokeManager;
	ci::Vec2f        mPosRef;
};

class UserManager : mndl::ni::UserTracker::Listener
{
typedef std::shared_ptr< User >                                  UserRef;
typedef std::map< unsigned, UserRef >                            Users;
typedef std::vector< std::pair< std::string, ci::gl::Texture > > Brushes;
typedef std::vector< XnSkeletonJoint >                           Joints;
public:
	UserManager();

	void setup( const ci::fs::path &path = "" );
	void update();
	void drawStroke( const Calibrate &calibrate );
	void drawBody  ( const Calibrate &calibrate );

	void setBounds( const Rectf &rect );
	void setSourceBounds( const ci::Area &area );
	void clearStrokes();

	void newUser       ( mndl::ni::UserTracker::UserEvent event );
	void lostUser      ( mndl::ni::UserTracker::UserEvent event );
	void calibrationBeg( mndl::ni::UserTracker::UserEvent event );
	void calibrationEnd( mndl::ni::UserTracker::UserEvent event );

	bool mouseDown( ci::app::MouseEvent event );
	bool mouseDrag( ci::app::MouseEvent event );
	bool mouseUp( ci::app::MouseEvent event );
	void keyUp( ci::app::KeyEvent event );

	void setFbo( const ci::gl::Fbo &fbo )
	{
		mFbo = fbo;
		setBounds( mFbo.getBounds());
	}

private:
	void    createUser ( unsigned userId );
	void    destroyUser( unsigned userId );
	UserRef findUser   ( unsigned userId );

	bool            getStrokeActive( XnSkeletonJoint jointId );
	ci::gl::Texture getStrokeBrush ( XnSkeletonJoint jointId );

private:
	enum JointType
	{
		LEFT_HAND      = 0,
		LEFT_SHOULDER  = 1,
		HEAD           = 2,
		RIGHT_HAND     = 3,
		RIGHT_SHOULDER = 4,
		TORSO          = 5,
		LEFT_KNEE      = 6,
		RIGHT_KNEE     = 7,
		LEFT_FOOT      = 8,
		RIGHT_FOOT     = 9,
	};

	mndl::ni::OpenNI      mNI;
	mndl::ni::UserTracker mNIUserTracker;

	Joints  mJoints;
	Brushes mBrushes;
	XnSkeletonJoint mJointRef;

	ci::Rectf       mOutputRect;
	ci::RectMapping mOutputMapping;
	ci::Area        mSourceBounds;

	ci::gl::Texture     mNITexture;

	ci::gl::Fbo         mFbo;

	// params
	ci::params::PInterfaceGl mParams;
	std::string              mKinectProgress;
	float                    mSkeletonSmoothing;
	bool                     mJointShow;
	bool                     mLineShow;
	bool                     mVideoShow;
	bool                     mVideoMirrored;
	float                    mJointSize;
	ci::ColorA               mJointColor;

	int                      mStrokeSelect[10];
	bool                     mStrokeActive[10];

	std::thread              mThread;
	std::mutex               mMutex;
	void                     openKinect( const ci::fs::path &path );

	Users                    mUsers;

	friend class User;
};

} // namespace cinder
