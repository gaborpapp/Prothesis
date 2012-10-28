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

void Stroke::update( Vec2f pos )
{
	if( ! mActive )
		return;

	if( mPoints.empty() )
	{
		mPos = pos;
		mVel = Vec2f::zero();
	}

	Vec2f d = mPos - pos; // displacement from the cursor
	Vec2f f = -mK * d;    // Hooke's law F = - k * d
	Vec2f a = f / mMass;  // acceleration, F = ma

	mVel  = mVel + a;
	mVel *= mDamping;
	mPos += mVel;

	Vec2f ang( -mVel.y, mVel.x );
	ang.normalize();

	Vec2f scaledVel = mVel * Vec2f( mWindowSize );
	float s = math<float>::clamp( scaledVel.length(), 0, mMaxVelocity );
	ang *= mStrokeMinWidth + ( mStrokeMaxWidth - mStrokeMinWidth ) * easeInQuad( s / mMaxVelocity );
	mPoints.push_back( StrokePoint( mPos * Vec2f( mWindowSize ), ang ));
}

void Stroke::draw()
{
	if( ! mActive
	 || ! mBrush )
		return;

//	gl::enable( GL_TEXTURE_2D );
	mBrush.bind();
	glBegin( GL_QUAD_STRIP );
	size_t n = mPoints.size();
	float step = 1.0f / n;
	float u = 0.0f;

	for( list< StrokePoint >::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i )
	{
		const StrokePoint *s = &(*i);
		glTexCoord2f( u, 0 );
		gl::vertex( s->p + s->w );
		glTexCoord2f( u, 1 );
		gl::vertex( s->p - s->w );
		u += step;
	}

	glEnd();
	mBrush.unbind();

//	gl::disable( GL_TEXTURE_2D );
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

