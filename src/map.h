#ifndef MAP_H
#define MAP_H

#include "tiled.h"
#include "gamehelper.h"
#include "vulkan-render/vkhelper.h"
#include "vulkan-render/render.h"

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <map>

//#define SEE_COLLIDERS;

struct Tile
{
	glm::vec4 tileRect;
	Resource::Texture texture;
};

class Map
{
public:
	Map(std::string filename, Render &render);
	Map(){}
	void Update(glm::vec4 cameraRect);
	void Draw(Render &render);
	glm::vec4 getMapRect();
	glm::vec4 getMapRect(glm::vec2 pos);
	std::vector<glm::vec4> getCameraRects();
private:
	tiled::Map map;
	std::vector<glm::vec4> colliders;
	std::vector<glm::mat4> tileMats;
	std::vector<bool> toDraw;
	std::vector<glm::vec4> tileRects;
	std::vector<Tile> tiles;
	std::vector<glm::vec4> cameraRects;
	glm::vec4 mapRect;
};



#endif