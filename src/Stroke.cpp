#include "cinder/CinderMath.h"
#include "cinder/Easing.h"

#include "Stroke.h"

using namespace std;
using namespace ci;
using namespace ci::app;

Stroke::Stroke()
: mActive( true )
, mK( 0.06f )
, mDamping( 0.7f )
, mMass( 1.0f )
, mStrokeMinWidth( 1.0f )
, mStrokeMaxWidth( 15.0f )
{
}

void Stroke::resize( ResizeEvent event )
{
	mWindowSize = event.getSize();
}

void Stroke::addPos( Vec2f pos )
{
	mTarget = pos;
}

void Stroke::update()
{
	if( ! mActive )
		return;

	if ( mPoints.empty() )
	{
		mPos = mTarget;
		mVel = Vec2f::zero();
		mU = 0.f;
		mLastDrawn = 0;
	}

	Vec2f d = mPos - mTarget; // displacement from the target

	// no new point if the target is close
	if ( ( d.lengthSquared() < .001f ) && ( !mPoints.empty() ) )
		return;

	Vec2f f = -mK * d; // Hooke's law F = - k * d
	Vec2f a = f / mMass; // acceleration, F = ma

	mVel = mVel + a;
	mVel *= mDamping;
	mPos += mVel;
	mU += mVel.length();

	Vec2f ang( -mVel.y, mVel.x );
	ang.safeNormalize();

	Vec2f scaledVel = mVel * Vec2f( mWindowSize );
	float s = math<float>::clamp( scaledVel.length(), 0, mMaxVelocity );
	ang *= mStrokeMinWidth + ( mStrokeMaxWidth - mStrokeMinWidth ) * easeInQuad( s / mMaxVelocity );
	mPoints.push_back( StrokePoint( mPos * Vec2f( mWindowSize ), ang, mU ) );
}

void Stroke::draw( const Calibrate &calibrate )
{
	if( ! mActive
	 || ! mBrush )
		return;

	gl::enable( GL_TEXTURE_2D );
	mBrush.bind();
	gl::color( ColorA::white() );

	// subdivison to overcome texturing artifacts
	size_t subdivCount = 8;
	float coeffStep = 2.f / subdivCount;
	float coeff = -1.f;

	for ( size_t i = 0; i < subdivCount; i++ )
	{
		float c0 = coeff;
		float c1 = coeff + coeffStep;
		float v0 = c0 *.5f + .5f;
		float v1 = c1 *.5f + .5f;
		if ( mPoints.size() >= 2 )
		{
			glBegin( GL_QUAD_STRIP );
			// add only the stroke points not drawn already
			for( vector< StrokePoint >::const_iterator i = mPoints.begin() + mLastDrawn;
					i != mPoints.end(); ++i )
			{
				const StrokePoint *s = &(*i);
				glTexCoord2f( s->u, v0 );
				gl::vertex( calibrate.transform( s->p + c0 * s->w ));
				glTexCoord2f( s->u, v1 );
				gl::vertex( calibrate.transform( s->p + c1 * s->w ));
			}
			glEnd();
		}
		coeff += coeffStep;
	}

	mBrush.unbind();
	gl::disable( GL_TEXTURE_2D );

	if ( !mPoints.empty() )
		mLastDrawn = mPoints.size() - 1;
}

void Stroke::setActive( bool active )
{
	if( mActive != active )
	{
		mActive = active;

		if( ! mActive )
			clear();
	}
}

