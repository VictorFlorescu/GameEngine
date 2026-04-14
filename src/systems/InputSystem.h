#pragma once 

#include "raylib.h"
#include "raymath.h"

class InputSystem
{
public:
	InputSystem() = default;

	void Update() {
		m_prevMousePos = m_mousePos;
		m_mousePos = GetMousePosition();
		m_mouseWheel = GetMouseWheelMove();
	}

	// Keyboard

	bool IsKeyDown(int key) const { return ::IsKeyDown(key); }
	bool IsKeyUp(int key) const { return ::IsKeyUp(key); }
	bool IsKeyPressed(int key) const { return ::IsKeyPressed(key); }
	bool IsKeyReleased(int key) const { return ::IsKeyReleased(key); }

	// Mouse

	bool IsMouseDown(int button) const { return ::IsMouseButtonDown(button); }
	bool IsMousePressed(int button) const { return ::IsMouseButtonPressed(button); }
	bool IsMouseReleased(int button) const { return ::IsMouseButtonReleased(button); }

	Vector2 GetMousePos() const { return m_mousePos; }
	Vector2 GetMouseDelta() const { return Vector2Subtract(m_mousePos, m_prevMousePos); }
	float GetMouseWheel() const { return m_mouseWheel; }

	// gamepad (pad 0 by default)

	bool IsGamepadAvailable(int pad = 0) const { return ::IsGamepadAvailable(pad); }
	bool IsButtonDown(int button, int pad = 0) const { return ::IsGamepadButtonDown(pad, button); }
	bool IsButtonPressed(int button, int pad = 0) const { return ::IsGamepadButtonPressed(pad, button); }
	bool IsButtonReleased(int button, int pad = 0) const { return ::IsGamepadButtonReleased(pad, button); }
	bool GetAxis (int axis, int pad = 0) const { return ::GetGamepadAxisMovement(pad, axis); }

private:
	Vector2 m_mousePos = { 0,0 };
	Vector2 m_prevMousePos = { 0,0 };
	float m_mouseWheel = 0.0f;
};