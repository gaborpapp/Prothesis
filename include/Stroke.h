#pragma once

#include <vector>

#include "cinder/Vector.h"
#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "Calibrate.h"

class Stroke
{
	public:
		Stroke();

		void resize( const ci::Vec2i &size );

		void addPos( ci::Vec2f point );
		void update();
		void draw( const Calibrate &calibrate, const ci::Vec2f &posRef );

		void setActive( bool active );

		void clear() { mPoints.clear(); }

		void setBrush( ci::gl::Texture brush ) { mBrush = brush; };

		void  setStiffness( float s ) { mK = s; }
		float getStiffness() { return mK; }

		void  setDamping( float d ) { mDamping = d; }
		float getDamping() { return mDamping; }

		void  setStrokeMinWidth( float w ) { mStrokeMinWidth = w; }
		float getStrokeMinWidth() const { return mStrokeMinWidth; }

		void  setStrokeMaxWidth( float w ) { mStrokeMaxWidth = w; }
		float getStrokeMaxWidth() const { return mStrokeMaxWidth; }

		void  setMaxVelocity( float v ) { mMaxVelocity = v; }
		float getMaxVelocity() const { return mMaxVelocity; }

	private:
		struct StrokePoint
		{
			StrokePoint( ci::Vec2f _p, ci::Vec2f _w, float _u ) :
				p( _p ), w( _w ), u( _u ) {}

			ci::Vec2f p;
			ci::Vec2f w;
			float u;
		};

		bool mActive;
		bool mEmpty; // if it has already a target position or not

		std::vector<StrokePoint> mPoints;

		ci::Vec2f mPos;     // spring position
		ci::Vec2f mTarget;  // target position
		ci::Vec2f mVel;     // velocity
		float     mK;       // spring stiffness
		float     mDamping; // friction
		float     mMass;

		float     mStrokeMinWidth;
		float     mStrokeMaxWidth;
		float     mMaxVelocity;

		float     mU; // u texture coord
		size_t    mLastDrawn; // index of the last drawn point

		ci::Vec2i       mWindowSize;
		ci::gl::Texture mBrush;
};

