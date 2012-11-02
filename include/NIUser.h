#pragma once

#include <map>
#include "cinder/app/App.h"
#include "cinder/Color.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"

#include "CiNI.h"
#include "PParams.h"
#include "Stroke.h"

#define USE_KINECT 1
#define USE_KINECT_RECORD 0

namespace cinder {

class UserManager;

class User
{
typedef std::map< XnSkeletonJoint, ci::Vec2f >                 JointPositions;
typedef std::map< XnSkeletonJoint, std::shared_ptr< Stroke > > JointStrokes;

public:
	User( UserManager *userManager );

	void update( XnSkeletonJoint jointId, ci::Vec2f pos );

	void clearPoints();
	void addStroke( XnSkeletonJoint jointId );
	void clearStrokes();
	void draw();

private:
	Stroke *createStroke ( XnSkeletonJoint jointId );
	void    destroyStroke( XnSkeletonJoint jointId );
	Stroke *findStroke   ( XnSkeletonJoint jointId );

	void drawJoints();
	void drawBodyLines();
	void drawBodyLine( XnSkeletonJoint jointBeg, XnSkeletonJoint jointEnd );
	void drawStrokes();

private:
	UserManager     *mUserManager;
	JointPositions   mJointPositions;
	JointStrokes     mJointStrokes;
};

class UserManager : mndl::ni::UserTracker::Listener
{
typedef std::map< unsigned, std::shared_ptr< User > >            Users;
typedef std::vector< std::pair< std::string, ci::gl::Texture > > Brushes;
typedef std::vector< XnSkeletonJoint >                           Joints;
public:
	UserManager();

	void setup( const ci::fs::path &path = "" );
	void update();
	void draw();

	void setBounds( const Rectf &rect );
	void showParams( bool show );
	void clearStrokes();

	void newUser       ( mndl::ni::UserTracker::UserEvent event );
	void lostUser      ( mndl::ni::UserTracker::UserEvent event );
	void calibrationBeg( mndl::ni::UserTracker::UserEvent event );
	void calibrationEnd( mndl::ni::UserTracker::UserEvent event );

	bool mouseDown( ci::app::MouseEvent event );
	bool mouseDrag( ci::app::MouseEvent event );
	bool mouseUp( ci::app::MouseEvent event );

private:
	void  createUser ( unsigned userId );
	void  destroyUser( unsigned userId );
	User *findUser   ( unsigned userId );

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

	float                    mK;
	float                    mDamping;
	float                    mStrokeMinWidth;
	float                    mStrokeMaxWidth;
	float                    mMaxVelocity;

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
