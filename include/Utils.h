#pragma once

#include <string>
#include <vector>

#include "cinder/gl/Texture.h"
#include "cinder/Filesystem.h"

namespace cinder {

//! Returns time stamp for current time.
std::string timeStamp();

std::vector< std::pair< std::string, ci::gl::Texture > > loadTextures( const fs::path &relativeDir );

} // namespace cinder
