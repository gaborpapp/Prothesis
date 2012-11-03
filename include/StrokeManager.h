#pragma once

#include <map>

#include "cinder/app/App.h"
#include "cinder/gl/Texture.h"
#include "cinder/Vector.h"

#include "Stroke.h"
#include "PParams.h"
#include "Calibrate.h"

class StrokeManager
{
typedef std::shared_ptr< Stroke > StrokeRef;
typedef std::map< int, StrokeRef > Strokes;

public:
	static void setup( ci::Vec2i size );

	void update();
	void draw( const Calibrate &calibrate );

	void addPos( int id, ci::Vec2f pos );
	void setActive( int id, bool active );
	void setBrush( int id, ci::gl::Texture brush );
	void clear();

	int  createStroke ( int id = -1 );
	void destroyStroke( int id );

private:
	StrokeRef findStroke( int id );
	int       generateStrokeId();

private:
	Strokes                  mStrokes;

	static const int         sGenerateid;

	// params
	static ci::params::PInterfaceGl mParams;
	static float                    mK;
	static float                    mDamping;
	static float                    mStrokeMinWidth;
	static float                    mStrokeMaxWidth;
	static float                    mMaxVelocity;
	static ci::Vec2i                mSize;
};

