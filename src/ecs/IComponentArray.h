#pragma once
#include "Entity.h"

// Type-erased base - lets ComponentStore call RemoveIfExists 
// on all arrays without knowing their type

class IComponentArray
{
public:
	virtual ~IComponentArray() = default;
	virtual void RemoveIfExists(Entity entity) = 0;
};