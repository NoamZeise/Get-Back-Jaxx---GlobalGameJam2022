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
#include <fstream>
#include <string>

//#define SEE_COLLIDERS;

struct Tile
{
	glm::vec4 tileRect;
	Resource::Texture texture;
};

enum EnemyTypes
{
	Basic
};

struct MapEnemy
{
	MapEnemy(glm::vec2 spawn, EnemyTypes type)
	{
		this->spawn = spawn;
		this->type = type;
	}
	MapEnemy() {}
	glm::vec2 spawn;
	EnemyTypes type;
};

struct MapMessage
{
	MapMessage(glm::vec4 rect, std::string fileName)
	{
		this->rect = rect;
		
    	std::ifstream file(fileName);
    	std::string line; 
    	while (std::getline(file, line))
    	{
			messages.push_back(line);
    	}
	}
	MapMessage() {}
	glm::vec4 rect;
	std::vector<std::string> messages;
};

class Map
{
public:
	Map(std::string filename, Render &render);
	Map(){}
	void Update(glm::vec4 cameraRect);
	void Draw(Render &render);
	glm::vec4 getMapRect() {return mapRect; }
	std::vector<glm::vec4> getCameraRects() { return cameraRects; }
	std::vector<glm::vec4> getMapColliders() {return colliders;}
	std::vector<glm::vec4> getGapColliders() { return gaps; }
	std::vector<MapEnemy> getEnemySpawns() {return enemySpawns;}
	std::vector<MapMessage> getMapMessages() {return messageAreas;}
	glm::vec2 getPlayerSpawn() { return playerSpawn; }
	std::string getMusic() {return map.props.music;}
	glm::vec4 getReactorRoom() { return reactorRoom; }
	glm::vec4 getReactorTP() { return reactorTP; }


	std::string name;
	std::vector<glm::vec4> items;
	std::vector<glm::vec4> checkpoints;
	glm::vec4 lastCheckpoint = glm::vec4(0);
	std::vector<glm::vec2> lights;

	std::vector<glm::vec2> doors;
	std::vector<glm::vec2> scientist;

private:
	tiled::Map map;
	std::vector<glm::mat4> tileMats;
	std::vector<bool> toDraw;
	std::vector<glm::vec4> tileRects;
	std::vector<Tile> tiles;
	std::vector<glm::vec4> cameraRects;
	glm::vec4 mapRect;

	std::vector<glm::vec4> colliders;
	std::vector<glm::vec4> gaps;
	std::vector<MapEnemy> enemySpawns;
	glm::vec2 playerSpawn;
	std::vector<MapMessage> messageAreas;

	glm::vec4 reactorRoom = glm::vec4(0);
	glm::vec4 reactorTP = glm::vec4(0);

};



#endif