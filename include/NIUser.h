#pragma once

#include <map>
#include "cinder/Color.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"

#include "NI.h"
#include "PParams.h"
#include "Stroke.h"

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

class UserManager : UserTracker::Listener
{
typedef std::map< unsigned, std::shared_ptr< User > >            Users;
typedef std::vector< std::pair< std::string, ci::gl::Texture > > Brushes;
typedef std::vector< XnSkeletonJoint >                           Joints;
public:
	UserManager();

	void setup();
	void update();
	void draw();

	void setBounds( const Rectf &rect );
	void showParams( bool show );
	void clearStrokes();

	void newUser       ( UserTracker::UserEvent event );
	void lostUser      ( UserTracker::UserEvent event );
	void calibrationBeg( UserTracker::UserEvent event );
	void calibrationEnd( UserTracker::UserEvent event );

private:
	void  createUser ( unsigned userId );
	void  destroyUser( unsigned userId );
	User *findUser   ( unsigned userId );

	bool            getStrokeActive( XnSkeletonJoint jointId );
	ci::gl::Texture getStrokeBrush ( XnSkeletonJoint jointId );

private:
	OpenNI      mNI;
	UserTracker mNIUserTracker;

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
