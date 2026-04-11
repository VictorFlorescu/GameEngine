#pragma once
#include "raylib.h"

struct MeshRenderer
{
	Model model = {};
	Color tint = WHITE;
	bool visible = true;
	bool castShadow = true; // reserved for future shadow pass
	bool  receiveShadow = true;

	// Convenience - create directly from a file path
	static MeshRenderer FromFile(const char* path)
	{
		MeshRenderer r;
		r.model = LoadModel(path);
		return r;
	}

	// Convenience - create from a generated raylib primitive
	static MeshRenderer FromMesh(::Mesh mesh)
	{
		MeshRenderer r;
		r.model = LoadModelFromMesh(mesh);
		return r;
	}

	void SetMaterial(Material mat, int slot = 0)
	{
		model.materials[slot] = mat;
	}

	void Unload()
	{
		UnloadModel(model);
	}
};