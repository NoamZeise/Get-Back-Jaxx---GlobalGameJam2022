#include "map.h"

Map::Map(std::string filename, Render &render)
{
	map = tiled::Map(filename);

	tileMats.resize(map.width * map.height);
	toDraw.resize(map.width * map.height);
	tileRects.resize(map.width * map.height);
	int index = 0;
	for(unsigned int y = 0; y < map.height; y++)
		for(unsigned int x = 0; x < map.width; x++)
		{
			tileRects[index] = glm::vec4(x * map.tileWidth, y * map.tileHeight, map.tileWidth, map.tileHeight);
			tileMats[index] = vkhelper::calcMatFromRect(tileRects[index], 0);
			toDraw[index++] = false;
		}
		

	for(const auto &layer: map.layers)
	{
		if(layer.props.collidable || layer.props.gap)
		{
			//TODO: make colliders more efficient by merging rects 
			size_t i = 0;
			for(unsigned int y = 0; y < map.height; y++)
				for(unsigned int x = 0; x < map.width; x++)
				{
					if(layer.data[i] != 0 && layer.props.collidable)
						colliders.push_back(glm::vec4(x * map.tileWidth, y * map.tileHeight, map.tileWidth, map.tileHeight));
					if(layer.data[i] != 0 && layer.props.gap)
						gaps.push_back(glm::vec4(x * map.tileWidth, y * map.tileHeight, map.tileWidth, map.tileHeight/3));
					i++;
				}
		}
	}

	tiles.resize(map.totalTiles + 1);
	tiles[0] = Tile();
	tiles[0].tileRect = glm::vec4(0, 0, 1, 1);
	for(const auto &tileset: map.tilesets)
	{
		Resource::Texture tex = render.LoadTexture(tileset.imageSource);
		unsigned int id = tileset.firstTileID;
		for(unsigned int y = 0; y < tileset.imageHeight / tileset.tileHeight; y++)
			for(unsigned int x = 0; x < tileset.columns; x++)
			{
				tiles[id] = Tile();
				tiles[id].texture = tex;
				tiles[id++].tileRect = vkhelper::calcTexOffset(glm::vec2(tileset.imageWidth, tileset.imageHeight),
					glm::vec4(x * tileset.tileWidth, y * tileset.tileHeight, tileset.tileWidth, tileset.tileHeight));
			}
	}

	for(const auto &objGroup: map.objectGroups)
	{
		for(const auto &obj: objGroup.objs)
		{
			if(obj.props.collidable || objGroup.props.collidable)
				colliders.push_back(glm::vec4(obj.x, obj.y, obj.w, obj.h));
			if(obj.props.camera || objGroup.props.camera)
				cameraRects.push_back(glm::vec4(obj.x, obj.y, obj.w, obj.h));
			if(obj.props.message != "")
				messageAreas.push_back(MapMessage(glm::vec4(obj.x, obj.y, obj.w, obj.h), obj.props.message));
			if(obj.props.playerSpawn)
				playerSpawn = glm::vec2(obj.x, obj.y);
			if(obj.props.enemySpawn || objGroup.props.enemySpawn)
				enemySpawns.push_back(MapEnemy(glm::vec2(obj.x, obj.y), EnemyTypes::Basic));
			if(obj.props.light || objGroup.props.light)
				lights.push_back(glm::vec2(obj.x, obj.y));
			if(obj.props.reactorRoom || objGroup.props.reactorRoom)
				reactorRoom = glm::vec4(obj.x, obj.y, obj.w, obj.h);
			if(obj.props.reactorTP || objGroup.props.reactorTP)
				reactorTP = glm::vec4(obj.x, obj.y, obj.w, obj.h);
			if(obj.props.item || objGroup.props.item)
				items.push_back(glm::vec4(obj.x, obj.y, obj.w, obj.h));
			if(obj.props.checkpoint || objGroup.props.checkpoint)
				checkpoints.push_back(glm::vec4(obj.x, obj.y, obj.w, obj.h));
			if(obj.props.door || objGroup.props.door)
				doors.push_back(glm::vec4(obj.x, obj.y, obj.w, obj.h));
			if(obj.props.scientist || objGroup.props.scientist)
				scientist.push_back(glm::vec2(obj.x, obj.y));
		}
	}

	mapRect = glm::vec4(0, 0, map.width * map.tileWidth, map.height * map.tileHeight);
}

void Map::Update(glm::vec4 cameraRect)
{
	for(unsigned int i = 0; i < toDraw.size(); i++)
	{
		if(gh::colliding(cameraRect, tileRects[i]))
			toDraw[i] = true;
		else
			toDraw[i] = false;
	}
}

void Map::Draw(Render &render)
{
	#ifdef SEE_COLLIDERS
	for(const auto &rect: colliders)
	{
		render.DrawQuad(Resource::Texture(), vkhelper::getModelMatrix(rect, 0), glm::vec4(1.0f));
	}
	#endif
	for(unsigned int i = map.layers.size(); i > 0; i--)
	{
		for(unsigned int j = 0; j < map.layers[i - 1].data.size(); j++)
		{
			if(!toDraw[j])
				continue;
			if(map.layers[i - 1].data[j] != 0)
				render.DrawQuad(tiles[map.layers[i - 1].data[j]].texture, tileMats[j], glm::vec4(1.0f), tiles[map.layers[i - 1].data[j]].tileRect);
		}
	}
}