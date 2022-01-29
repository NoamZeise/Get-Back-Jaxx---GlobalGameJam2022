#ifndef VULKAN_RENDERER_CONFIG_H
#define VULKAN_RENDERER_CONFIG_H


//#define NDEBUG //uncomment for release mode
#define ONLY_2D

namespace settings
{

const bool SRGB = false;
const bool MIP_MAPPING = false;
const bool PIXELATED = true;
const bool VSYNC = true;
const bool MULTISAMPLING = false;
const bool SAMPLE_SHADING = true;

const bool FIXED_RATIO = true;
const int TARGET_WIDTH = 480;
const int TARGET_HEIGHT = 270;


#ifndef NDEBUG
const bool ERROR_ONLY = true;
#endif
}


#endif