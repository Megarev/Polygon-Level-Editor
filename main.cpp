#define OLC_PGE_APPLICATION
#define OLC_PGEX_FUI
#include "editor.h"
#include "physics.h"
#include "olcPixelGameEngine.h"

class Game : public olc::PixelGameEngine {
public:
	PolygonEditor editor;
	Scene scene;
	int editor_state = 0;
	
	void EditorInput(const olc::vf2d& m_pos, float dt) {
		if (GetKey(olc::ENTER).bPressed && editor.polygons.size() > 0) {
			editor.ChangeEditorState(editor_state, scene);
		}
		
		bool is_pan = GetKey(olc::SPACE).bHeld;
		
		if (is_pan) {
			if (GetMouse(0).bPressed) editor.MousePressPan(m_pos);
			if (GetMouse(0).bHeld)    editor.MouseHoldPan(m_pos);
		}

		if (editor_state == 1) return;

		int key = (
			(int)GetKey(olc::W).bHeld << 0 |
			(int)GetKey(olc::A).bHeld << 1 |
			(int)GetKey(olc::D).bHeld << 2 |
			(int)GetKey(olc::S).bHeld << 3
			);

		// Input
		float m_wheel_speed = 5.0f * Sign((float)GetMouseWheel()) * dt;
		
		bool is_ctrl = GetKey(olc::CTRL).bHeld;

		if (is_ctrl) {
			if (GetKey(olc::S).bPressed) editor.Export("polygon_data.txt");
			if (GetKey(olc::O).bPressed) editor.Import("polygon_data.txt");
			if (GetKey(olc::E).bPressed) editor.polygons.clear();
		}
		
		if (!is_pan) {
			bool is_edit = GetMouse(0).bHeld;

			if (GetMouse(0).bReleased) editor.OnMouseReleaseEdit();
			if (GetKey(olc::E).bPressed) editor.ClearPolygon();
			if (is_edit) {
				if (GetMouse(0).bPressed)  editor.OnMousePressEdit(m_pos);
				if (GetMouse(0).bHeld)	   editor.OnMouseHoldEdit(m_pos, m_wheel_speed, key);
			}
			else if (!editor.is_constraint_add) {
				if (GetMouse(1).bPressed)  editor.OnMousePressPolygonAdd();
				if (GetMouse(1).bHeld)     editor.OnMouseHoldPolygonAdd(m_pos, m_wheel_speed, key);
				if (GetMouse(1).bReleased) editor.OnMouseReleasePolygonAdd();
			}
			else if (editor.is_constraint_add) {
				if (GetMouse(1).bPressed)  editor.OnMousePressConstraintAdd();
				if (GetMouse(1).bHeld)	   editor.OnMouseHoldConstraintAdd(m_pos);
				if (GetMouse(1).bReleased) editor.OnMouseReleaseConstraintAdd();
			}
		}
	}

	void PhysicsLogic(int iter, float dt) {
		if (editor_state == 0) return;

		for (int i = 0; i < iter; i++) scene.Update(dt);
	}
public:
	Game() {
		sAppName = "Editor";
	}

	bool OnUserCreate() override {

		editor.bounds = { ScreenWidth(), ScreenHeight() };
		scene.Initialize({ editor.bounds.x * 1.0f, editor.bounds.y * 1.0f });
		
		return true;
	}

	bool OnUserUpdate(float dt) override {

		const olc::vf2d& m_pos = GetMousePos();

		// Input
		EditorInput(m_pos, dt);

		// Logic
		PhysicsLogic(5, dt);
		Clear(olc::BLACK);
		if (editor_state == 0) editor.GUILogic(scene);

		// Render
		if (editor_state == 0) editor.Draw(this, m_pos);
		else if (editor_state == 1) scene.Draw(editor.offset, this, true);

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