#include "cinder/gl/Texture.h"
#include "Calibrate.h"
#include "Utils.h"

using namespace ci;
using namespace ci::app;

const float Calibrate::MIN_TRANSLATE  = -1000.0f;
const float Calibrate::MAX_TRANSLATE  =  1000.0f;
const float Calibrate::STEP_TRANSLATE =  1.0f;
const float Calibrate::MIN_SCALE      =  0.1f;
const float Calibrate::MAX_SCALE      =  10.0f;
const float Calibrate::STEP_SCALE     =  0.001f;

const int   Calibrate::MIN_COVER      = 0;
const int   Calibrate::MAX_COVER      = 400;
const int   Calibrate::STEP_COVER     = 1;

Calibrate::Calibrate()
: mTranslateX( 0.0f )
, mTranslateY( 0.0f )
, mScaleX( 1.0f )
, mScaleY( 1.0f )
, mCoverLeft( 0 )
, mCoverRight( 0 )
, mCoverTop( 0 )
, mCoverBottom( 0 )
{
}

void Calibrate::setup()
{
	mParams = params::PInterfaceGl( "Calibrate", Vec2i( 200, 250 ) );
	mParams.setPosition( Vec2i( 480, 16 ) );
	mParams.addPersistentSizeAndPosition();
	mParams.addText( "Translate", "help='Left mouse button'" );
	mParams.addPersistentParam( "Translate X" , &mTranslateX    , 0.0f, getMinMaxStepString<float>( MIN_TRANSLATE, MAX_TRANSLATE, STEP_TRANSLATE ));
	mParams.addPersistentParam( "Translate Y" , &mTranslateY    , 0.0f, getMinMaxStepString<float>( MIN_TRANSLATE, MAX_TRANSLATE, STEP_TRANSLATE ));
	mParams.addText( "Scale", "help='Right mouse button'" );
	mParams.addPersistentParam( "Scale X"     , &mScaleX        , 1.0f, getMinMaxStepString<float>( MIN_SCALE    , MAX_SCALE    , STEP_SCALE     ));
	mParams.addPersistentParam( "Scale Y"     , &mScaleY        , 1.0f, getMinMaxStepString<float>( MIN_SCALE    , MAX_SCALE    , STEP_SCALE     ));
	mParams.addText( "Cover", "help='Middle mouse button'" );
	mParams.addPersistentParam( "Cover Left"  , &mCoverLeft     , 0   , getMinMaxStepString<int  >( MIN_COVER    , MAX_COVER    , STEP_COVER     ));
	mParams.addPersistentParam( "Cover Right" , &mCoverRight    , 0   , getMinMaxStepString<int  >( MIN_COVER    , MAX_COVER    , STEP_COVER     ));
	mParams.addPersistentParam( "Cover Top"   , &mCoverTop      , 0   , getMinMaxStepString<int  >( MIN_COVER    , MAX_COVER    , STEP_COVER     ));
	mParams.addPersistentParam( "Cover Bottom", &mCoverBottom   , 0   , getMinMaxStepString<int  >( MIN_COVER    , MAX_COVER    , STEP_COVER     ));
	mParams.addSeparator();
	mParams.addButton( "Reset", std::bind( &Calibrate::reset, this ));
}

void Calibrate::mouseDown( MouseEvent event )
{
	mMousePos = event.getPos();
}

void Calibrate::mouseDrag( MouseEvent event )
{
	Vec2i mousePosAct = event.getPos();

	if( event.isLeftDown())
	{
		mTranslateX  = math<float>::min( math<float>::max( mTranslateX + ( mousePosAct.x - mMousePos.x ) * STEP_TRANSLATE, MIN_TRANSLATE ), MAX_TRANSLATE );
		mTranslateY  = math<float>::min( math<float>::max( mTranslateY + ( mousePosAct.y - mMousePos.y ) * STEP_TRANSLATE, MIN_TRANSLATE ), MAX_TRANSLATE );
	}
	else if( event.isRightDown())
	{
		mScaleX      = math<float>::min( math<float>::max( mScaleX     + ( mousePosAct.x - mMousePos.x ) * STEP_SCALE    , MIN_SCALE     ), MAX_SCALE     );
		mScaleY      = math<float>::min( math<float>::max( mScaleY     + ( mousePosAct.y - mMousePos.y ) * STEP_SCALE    , MIN_SCALE     ), MAX_SCALE     );
	}
	else if( event.isMiddleDown())
	{
		mCoverLeft   = math<int  >::min( math<int  >::max( mCoverLeft  + ( mousePosAct.x - mMousePos.x ) * STEP_COVER    , MIN_COVER     ), MAX_COVER     );
		mCoverRight  = mCoverLeft;
		mCoverTop    = math<int  >::min( math<int  >::max( mCoverTop   + ( mousePosAct.y - mMousePos.y ) * STEP_COVER    , MIN_COVER     ), MAX_COVER     );
		mCoverBottom = mCoverTop;
	}

	mMousePos = mousePosAct;
}

void Calibrate::mouseUp( MouseEvent event )
{
	mMousePos = event.getPos();
}

const Vec2f Calibrate::transform( const Vec2f pos ) const
{
	return ( pos + getTranslate()) * getScale();
}

const Vec2f Calibrate::getTranslate() const
{
	return Vec2f( mTranslateX, mTranslateY );
}

const Vec2f Calibrate::getScale() const
{
	return Vec2f( mScaleX, mScaleY );
}

const Area Calibrate::getCoverLeft() const
{
	Area window = getWindowBounds();
	return Area( 0, 0, mCoverLeft, window.getHeight());
}

const Area Calibrate::getCoverRight() const
{
	Area window = getWindowBounds();
	return Area( window.getWidth() - mCoverRight, 0, window.getWidth(), window.getHeight());
}

const Area Calibrate::getCoverTop() const
{
	Area window = getWindowBounds();
	return Area( 0, 0, window.getWidth(), mCoverTop );
}

const Area Calibrate::getCoverBottom() const
{
	Area window = getWindowBounds();
	return Area( 0, window.getHeight() - mCoverBottom, window.getWidth(), window.getHeight());
}

void Calibrate::reset()
{
	mTranslateX  = 0.0f;
	mTranslateY  = 0.0f;
	mScaleX      = 1.0f;
	mScaleY      = 1.0f;

	mCoverLeft   = 0;
	mCoverRight  = 0;
	mCoverTop    = 0;
	mCoverBottom = 0;
}
