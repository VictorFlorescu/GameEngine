#pragma once
#include "raylib.h"

struct Sprite
{
	Texture2D texture = {};
	Rectangle srcRect = { 0,0,0,0 }; // source region - 0 means use full texture
	Vector2 origin = { 0.5f, 0.5f }; // normalised pivot (0.5, 0.5 = centre)
	Color tint = WHITE;
	int layer = 0; // draw order - lower = behind
	bool flipX = false;
	bool flipY = false;
	bool visible = true;

	// Returns the actual source rect, defaulting to full texture if unset

	Rectangle GetSrcRect() const
	{
		if (srcRect.width == 0 && srcRect.height == 0)
			return { 0,0, static_cast<float>(texture.width), static_cast<float>(texture.height) };
		return srcRect;
	}

	// Pivot in pixels relative to the source rect
	Vector2 GetOriginPixels() const
	{
		Rectangle r = GetSrcRect();
		return { origin.x * r.width, origin.y * r.height };
	}
};