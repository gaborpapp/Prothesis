#pragma once

#include <map>
#include "cinder/app/App.h"
#include "cinder/Color.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"

#include "CiNI.h"
#include "PParams.h"
#include "StrokeManager.h"
#include "Calibrate.h"

#define USE_KINECT        0
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
	void draw( const Calibrate &calibrate );

private:
	void drawJoints( const Calibrate &calibrate );
	void drawBodyLines( const Calibrate &calibrate );
	void drawBodyLine( const Calibrate &calibrate, XnSkeletonJoint jointBeg, XnSkeletonJoint jointEnd );

private:
	UserManager     *mUserManager;
	JointPositions   mJointPositions;
	StrokeManager    mStrokeManager;
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
	void draw( const Calibrate &calibrate );

	void setBounds( const Rectf &rect );
	void clearStrokes();

	void newUser       ( mndl::ni::UserTracker::UserEvent event );
	void lostUser      ( mndl::ni::UserTracker::UserEvent event );
	void calibrationBeg( mndl::ni::UserTracker::UserEvent event );
	void calibrationEnd( mndl::ni::UserTracker::UserEvent event );

	bool mouseDown( ci::app::MouseEvent event );
	bool mouseDrag( ci::app::MouseEvent event );
	bool mouseUp( ci::app::MouseEvent event );

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
	mndl::ni::OpenNI      mNI;
	mndl::ni::UserTracker mNIUserTracker;

	Joints  mJoints;
	Brushes mBrushes;

	ci::Rectf       mOutputRect;
	ci::RectMapping mOutputMapping;

	ci::gl::Texture     mNITexture;

	ci::gl::Fbo         mFbo;

	// params
	ci::params::PInterfaceGl mParams;
	float                    mSkeletonSmoothing;
	bool                     mJointShow;
	bool                     mBodyLineShow;
	bool                     mVideoShow;
	float                    mJointSize;
	ci::ColorA               mJointColor;

	int                      mStrokeLeftHand;
	int                      mStrokeLeftShoulder;
	int                      mStrokeHead;
	int                      mStrokeRightHand;
	int                      mStrokeRightShoulder;
	int                      mStrokeTorso;
	int                      mStrokeLeftKnee;
	int                      mStrokeRightKnee;
	int                      mStrokeLeftFoot;
	int                      mStrokeRightFoot;

	Users                    mUsers;

	friend class User;
};

} // namespace cinder