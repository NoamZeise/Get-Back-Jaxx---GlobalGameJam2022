#ifndef GAME_HELPER_H
#define GAME_HELPER_H

#include <glm/glm.hpp>

namespace gh
{
	static bool colliding(glm::vec4 a, glm::vec4 b)
	{
		return  a.x < b.x + b.z &&
				a.x + a.z > b.x && 
				a.y < b.y + b.w &&
				a.y + a.w > b.y;
	}

	static bool aInB(glm::vec4 a, glm::vec4 b)
	{
		return a.x > b.x && a.x < b.x + b.z &&
				a.y > b.y && a.y < b.y + b.z;
	}

	static bool contains(glm::vec2 p, glm::vec4 r)
	{
		return r.x < p.x && p.x < r.x + r.z  && r.y < p.y && p.y < r.y + r.w;   
	}


} //end namespace



#endif