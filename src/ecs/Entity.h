#pragma once
#include <cstdint>

struct Entity
{
	static constexpr uint32_t INDEX_BITS = 20;
	static constexpr uint32_t GEN_BITS = 12;
	static constexpr uint32_t INDEX_MASK = (1u << INDEX_BITS) - 1; // 0x000fffff
	static constexpr uint32_t GEN_MASK = (1u << GEN_BITS) - 1; // 0x00000fff
	static constexpr uint32_t MAX_ENTITIES = 1u << INDEX_BITS; // 1,048,576

	uint32_t id = 0; // [31..12] Generation | [11..0] Index

	// Pack: upper 12 bits = generation, lower 20 bits = index
	static Entity Make(uint32_t index, uint32_t generation)
	{
		return { (generation << INDEX_BITS) | (index & INDEX_MASK) };
	}

	uint32_t Index() const { return id & INDEX_MASK; }
	uint32_t Generation() const { return (id >> INDEX_BITS) & GEN_MASK; }

	bool operator==(Entity other) const { return id == other.id; }
	bool operator!=(Entity other) const { return id != other.id; }

	static const Entity Null; // sentinel for "no entity"
};

inline const Entity Entity::Null = Entity{ UINT32_MAX };