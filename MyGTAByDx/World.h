/** 
 * The World is the top level object representing a map or a sandbox in which Actors and Components will exist and be rendered.  
 *
 * A World can be a single Persistent Level with an optional list of streaming levels that are loaded and unloaded via volumes and blueprint functions
 * or it can be a collection of levels organized with a World Composition.
 *
 * In a standalone game, generally only a single World exists except during seamless area transitions when both a destination and current world exists.
 * In the editor many Worlds exist: The level being edited, each PIE instance, each editor tool which has an interactive rendered viewport, and many more.
 *
 */

#pragma once
#include "Object.h"
#include "Actor.h"
#include <vector>

class World : public Object {
public:
    World();
	~World();

	// World开始，以及调用World中所有Actor的BeginPlay函数
	virtual void BeginPlay();


	// GameInstance


	// World中的Actor
    std::vector<Actor*> Actors;

	// 添加删除Actor
    void AddActor(Actor& InActor);

	void RemoveActor(int ActorIndex);



};
