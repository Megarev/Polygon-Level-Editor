#define OLC_PGE_APPLICATION
#define OLC_PGEX_FUI
#include "editor.h"
#include "olcPixelGameEngine.h"

class Game : public olc::PixelGameEngine {
public:
	PolygonEditor editor;
public:
	Game() {
		sAppName = "Editor";
	}

	bool OnUserCreate() override {

		return true;
	}

	bool OnUserUpdate(float dt) override {

		const olc::vf2d& m_pos = GetMousePos();
		int key = (
			(int)GetKey(olc::W).bHeld << 0 |
			(int)GetKey(olc::A).bHeld << 1 |
			(int)GetKey(olc::D).bHeld << 2 |
			(int)GetKey(olc::S).bHeld << 3
		);

		// Input
		float m_wheel_speed = 5.0f * Sign((float)GetMouseWheel()) * dt;

		bool is_pan = GetKey(olc::SPACE).bHeld;
		bool is_ctrl = GetKey(olc::CTRL).bHeld;

		if (is_ctrl) {
			if (GetKey(olc::S).bPressed) editor.Export("polygon_data.txt");
			if (GetKey(olc::O).bPressed) editor.Import("polygon_data.txt");
			if (GetKey(olc::E).bPressed) editor.polygons.clear();
		}

		if (is_pan) {
			if (GetMouse(0).bPressed) editor.MousePressPan(m_pos);
			if (GetMouse(0).bHeld)    editor.MouseHoldPan(m_pos);
		}
		else {
			bool is_edit = GetMouse(0).bHeld;

			if (GetMouse(0).bReleased) editor.OnMouseReleaseEdit();
			if (is_edit) {
				if (GetMouse(0).bPressed)  editor.OnMousePressEdit(m_pos);
				if (GetMouse(0).bHeld)	   editor.OnMouseHoldEdit(m_pos, m_wheel_speed, key);
			}
			else {
				if (GetMouse(1).bPressed)  editor.OnMousePressAdd();
				if (GetMouse(1).bHeld)     editor.OnMouseHoldAdd(m_pos, m_wheel_speed, key);
				if (GetMouse(1).bReleased) editor.OnMouseReleaseAdd();
			}
		}
		// Logic

		// Render
		Clear(olc::BLACK);
		editor.Draw(this, m_pos);

		return true;
	}
};

int main() {

	Game game;
	if (game.Construct(512, 512, 1, 1, false, true)) {
		game.Start();
	}

	return 0;
}