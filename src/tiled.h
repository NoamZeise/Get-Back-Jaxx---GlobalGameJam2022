#ifndef TILED_H
#define TILED_H

#include "rapidxml.hpp"

#include <string>
#include <fstream>
#include <stdexcept>
#include <stdlib.h>
#include <vector>

#include <iostream>

namespace tiled
{

const std::string TILEMAP_LOCATION = "maps/";
const std::string TILED_TEXTURE_LOCATION = "textures/tilesets";
const std::string TILED_IMAGE_LOCATION = "textures/tiledimage";

static char* loadTextFile(std::string filename);

struct Properties
{
	bool collidable = false;
	bool camera = false;
	bool playerSpawn = false;
	bool enemySpawn = false;
	bool light = false;
	bool gap = false;
	bool reactorTP = false;
	bool reactorRoom = false;
	std::string message = "";
};

struct MapProperties
{
	std::string music;
};

static Properties fillPropStruct(rapidxml::xml_node<> *propertiesNode);

struct Layer
{
	Properties props;
	std::vector<unsigned int> data;
};

struct Object
{
	Properties props;
	double x = 0;
	double y = 0;
	double w = 0;
	double h = 0;
};

struct ObjectGroup
{
	Properties props;
	std::vector<Object> objs;
};

struct ImageLayer
{
	std::string source;
	double x = 0;
	double y = 0;
};

struct Tileset
{
	Tileset(std::string filename);

	unsigned int firstTileID = 0;

	std::string name;
	unsigned int tileWidth;
	unsigned int tileHeight;
	unsigned int tileCount;
	unsigned int columns;

	std::string imageSource;
	unsigned int imageWidth;
	unsigned int imageHeight;
};

struct Map
{
public:
	Map(std::string filename);
	Map() {}

	unsigned int width;
	unsigned int height;
	unsigned int tileWidth;
	unsigned int tileHeight;
	unsigned int totalTiles;

	std::vector<Tileset> tilesets;
	std::vector<Layer> layers;
	std::vector<ObjectGroup> objectGroups;
	std::vector<ImageLayer> imgLayer;
	
	MapProperties props;
};

} //namespace end

#endif