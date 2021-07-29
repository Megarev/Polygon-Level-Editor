#pragma once
#include "olcPixelGameEngine.h"
#include "olcPGEX_FrostUI.h"
#include "physics.h"
#include <unordered_map>

float Sign(float a) { return a > 0.0f ? 1.0f : (a < 0.0f ? -1.0f : 0.0f); }


class PolygonObject {
public:
	olc::vf2d scale, position;
	float angle = 0.0f, cos_angle = 0.0f, sin_angle = 0.0f, len = 0.0f;
	int n_vertices = 0;
	std::vector<olc::vf2d> model, vertices;
	bool is_update = true;

	olc::Pixel color;

	float mass = 0.0f, inv_mass = 0.0f;
	float I = 0.0f, inv_I = 0.0f;
	float sf = 0.0f, df = 0.0f;
	float e = 0.0f;
	float angular_velocity = 0.0f;
	uint32_t id = 0;
public:
	PolygonObject() {}
	PolygonObject(int _n_vertices);

	void Update();
	void Draw(olc::PixelGameEngine* pge, const olc::vf2d& offset);
};

class ConstraintObject {
public:
	olc::vf2d pivot_pos, point_pos;
	float len = 0.0f, k = 0.4f, b = 0.2f;
	bool is_sling = false;

	uint32_t polygon_index = 0;
public:
	ConstraintObject() {}
	
	void Draw(olc::PixelGameEngine* pge, const olc::vf2d& offset);
};

class PolygonEditor {
public:
	std::vector<PolygonObject> polygons;
	std::vector<ConstraintObject> constraints;
	PolygonObject* selected_polygon = nullptr;

	olc::FrostUI gui;

	// Panning
	olc::vf2d offset, prev_pan_pos;

	bool is_preview = false;
	enum {
		POLY_ROTATE = 1,
		POLY_SCALE_X = 2,
		POLY_SCALE_Y = 4,
		POLY_LEN = 8
	};

	std::vector<olc::vf2d> vertices, model;
	olc::vf2d position, scale = { 1.0f, 1.0f };
	float angle = 0.0f, len = 10.0f;
	int n_vertices = 4;
	int r = 255, g = 255, b = 255;

	float mass = 0.0f;
	float sf = 0.0f, df = 0.0f;
	float e = 0.0f;

	olc::vi2d bounds;

	bool is_polygon_add = false;
	bool is_constraint_add = false, is_draw_constraint = false; // Preview constraint drawing
	olc::vf2d pivot_pos;
	uint32_t index_count = 0;
public:
	PolygonEditor();

	void Export(const std::string& filename);
	void Import(const std::string& filename);

	// Polygon Editing functions
	void OnMousePressEdit(const olc::vf2d& m_pos);
	void OnMouseHoldEdit(const olc::vf2d& m_pos, float m_wheel, int key);
	void OnMouseReleaseEdit();
	void ClearPolygon();

	// Polygon Adding functions
	void OnMousePressPolygonAdd();
	void OnMouseHoldPolygonAdd(const olc::vf2d& m_pos, float m_wheel, int key);
	void OnMouseReleasePolygonAdd();

	// Constraint Adding functions
	void OnMousePressConstraintAdd();
	void OnMouseHoldConstraintAdd(const olc::vf2d& m_pos);
	void OnMouseReleaseConstraintAdd();

	// Panning functions
	void MousePressPan(const olc::vf2d& m_pos);
	void MouseHoldPan(const olc::vf2d& m_pos);

	void ChangeEditorState(int& editor_state, Scene& scene);
	void GUILogic(Scene& scene);

	void PreviewRender(olc::PixelGameEngine* pge);
	void Draw(olc::PixelGameEngine* pge, const olc::vf2d& m_pos);
};

// PolygonObject Implementation
PolygonObject::PolygonObject(int _n_vertices)
	: n_vertices(_n_vertices) {
	for (int i = 0; i < n_vertices; i++) model.push_back({ cosf(2.0f * PI / n_vertices * i + PI / 4.0f), sinf(2.0f * PI / n_vertices * i + PI / 4.0f) });
	vertices.resize(n_vertices);
}

void PolygonObject::Update() {
	if (!is_update) return;

	std::vector<olc::vf2d> polygon_model = model;
	for (auto& p : polygon_model) p *= scale;

	cos_angle = cosf(angle);
	sin_angle = sinf(angle);

	for (int i = 0; i < n_vertices; i++) {
		vertices[i] = {
			polygon_model[i].x * cos_angle + polygon_model[i].y * sin_angle,
			polygon_model[i].x * sin_angle - polygon_model[i].y * cos_angle
		};

		vertices[i] *= len;

		vertices[i] += position;
	}

	is_update = false;
}

void PolygonObject::Draw(olc::PixelGameEngine* pge, const olc::vf2d& offset) {
	for (int i = 0; i < n_vertices; i++) {
		int j = (i + 1) % n_vertices;
		pge->DrawLine(vertices[i] + offset, vertices[j] + offset);
	}
	for (int i = 0; i < n_vertices - 2; i++) {
		int j = (i + 1) % n_vertices;
		int k = (i + 2) % n_vertices;

		pge->FillTriangle(vertices[0] + offset, vertices[j] + offset, vertices[k] + offset, color);
	}
}

// ConstraintObject implementation
void ConstraintObject::Draw(olc::PixelGameEngine* pge, const olc::vf2d& offset) {
	pge->DrawLine(pivot_pos + offset, point_pos + offset, olc::WHITE);
}

// PolygonEditor Implementation
PolygonEditor::PolygonEditor() {
	gui.create_window("window", "Physics Settings", { 0, 0 }, { 200, 200 });
	gui.set_active_window("window");
	gui.add_int_slider("vertices", "Vertices:", { 80, 10 }, { 80, 10 }, { 3, 10 }, &n_vertices);
	gui.add_int_slider("red", "R:", { 80, 25 }, { 80, 10 }, { 0, 255 }, &r);
	gui.add_int_slider("green", "G:", { 80, 40 }, { 80, 10 }, { 0, 255 }, &g);
	gui.add_int_slider("blue", "B:", { 80, 55 }, { 80, 10 }, { 0, 255 }, &b);
	gui.add_float_slider("mass", "Mass:", { 80, 70 }, { 80, 10 }, { 0.0f, 100.0f }, &mass);
	gui.add_float_slider("restitution", "e:", { 80, 85 }, { 80, 10 }, { 0.0f, 1.0f }, &e);
	gui.add_float_slider("static_friction", "sf:", { 80, 100 }, { 80, 10 }, { 0.0f, 1.0f }, &sf);
	gui.add_float_slider("dynamic_friction", "df:", { 80, 115 }, { 80, 10 }, { 0.0f, 1.0f }, &df);

	gui.add_button("constraint_polygon", "Rope", { 120, 135 }, { 64, 16 }, []() {});
	gui.find_element("constraint_polygon")->make_toggleable(&is_constraint_add);

	gui.add_int_slider("bounds_w", "W:", { 20, 135 }, { 60, 10 }, { 0, 1000 }, &bounds.x);
	gui.add_int_slider("bounds_h", "H:", { 20, 150 }, { 60, 10 }, { 0, 1000 }, &bounds.y);
}

void PolygonEditor::Export(const std::string& filename) {
	std::ofstream writer(filename);
	if (!writer.is_open()) return;

	/*for (auto& p : polygons) {
		writer << "pos:" << p.position.x + offset.x << "," << p.position.y + offset.y << "|";
		writer << "offset:" << offset.x << "," << offset.y << "|";
		writer << "angle:" << p.angle << "|";
		writer << "scale:" << p.scale.x << "," << p.scale.y << "|";
		writer << "len:" << p.len << "|";
		writer << "n:" << p.n_vertices << "|";
		writer << "rgb:" << (int)p.color.r << "," << (int)p.color.g << "," << (int)p.color.b << "|";
		writer << "mass:" << p.mass << "|";
		writer << "I:" << (p.mass * p.len * p.len / 12.0f) << "|";
		writer << "res:" << p.e << "|";
		writer << "sf:" << p.sf << "|";
		writer << "df:" << p.df << "\n";
	}*/
	writer << "Polygons\n";
	for (auto& p : polygons) {
		writer << "pos:" << p.position.x + offset.x << "," << p.position.y + offset.y << "|";
		writer << "offset:" << offset.x << "," << offset.y << "|";
		writer << "angle:" << p.angle << "|";
		writer << "scale:" << p.scale.x << "," << p.scale.y << "|";
		writer << "len:" << p.len << "|";
		writer << "n:" << p.n_vertices << "|";
		writer << "rgb:" << (int)p.color.r << "," << (int)p.color.g << "," << (int)p.color.b << "|";
		writer << "mass:" << p.mass << "|";
		writer << "I:" << (p.mass * p.len * p.len / 12.0f) << "|";
		writer << "res:" << p.e << "|";
		writer << "sf:" << p.sf << "|";
		writer << "df:" << p.df << "|";
		writer << "id:" << p.id << "\n";
	}

	writer << "Constraints\n";
	for (auto& c : constraints) {
		writer << "pivot:" << c.pivot_pos.x + offset.x << "," << c.pivot_pos.y + offset.y << "|";
		writer << "point:" << c.point_pos.x + offset.x << "," << c.point_pos.y + offset.y << "|";
		writer << "offset:" << offset.x << "," << offset.y << "|";
		writer << "len:" << c.len << "|";
		writer << "k:" << c.k << "|";
		writer << "b:" << c.b << "|";
		writer << "index:" << c.polygon_index << "|";
		writer << "sling:" << c.is_sling << "\n";
	}

	writer.close();
}

void PolygonEditor::Import(const std::string& filename) {
	std::ifstream reader(filename);
	if (!reader.is_open()) return;

	auto ParseStringDelimeter = [&](const std::string& str, const char& delimeter) -> std::vector<std::string> {
		std::vector<std::string> data;
		std::stringstream ss(str);
		std::string s;

		while (std::getline(ss, s, delimeter)) data.push_back(s);

		return data;
	};


	auto ParseFloatDelimeter = [&](const std::string& str, const char& delimeter) -> std::vector<float> {
		std::vector<float> data;
		std::stringstream ss(str);
		std::string s;

		while (std::getline(ss, s, delimeter)) data.push_back(std::stof(s));

		return data;
	};

	polygons.clear();
	std::string line;

	bool is_polygon_read = true;

	while (std::getline(reader, line)) {
		if (line == "Polygons") { is_polygon_read = true; continue; }
		else if (line == "Constraints") { is_polygon_read = false; continue; }
		const std::vector<std::string>& main_data = ParseStringDelimeter(line, '|');

		if (is_polygon_read) {

			// PolygonObject
			PolygonObject p;
			for (auto& data : main_data) {
				const std::vector<std::string>& sub_main_data = ParseStringDelimeter(data, ':');

				const std::string& data_type = sub_main_data[0];
				const std::vector<float>& data_value = ParseFloatDelimeter(sub_main_data[1], ',');

				if (data_type == "pos") p.position = { data_value[0], data_value[1] };
				//else if (data_type == "offset") offset = { data_value[0], data_value[1] };
				else if (data_type == "angle") p.angle = data_value[0];
				else if (data_type == "scale") p.scale = { data_value[0], data_value[1] };
				else if (data_type == "len") p.len = data_value[0];
				else if (data_type == "n") p.n_vertices = data_value[0];
				else if (data_type == "rgb") p.color = olc::Pixel(data_value[0], data_value[1], data_value[2]);
				else if (data_type == "mass") { p.mass = data_value[0]; p.inv_mass = p.mass == 0.0f ? 0.0f : 1.0f / p.mass; }
				else if (data_type == "I") { p.I = data_value[0]; p.inv_I = p.I == 0.0f ? 0.0f : 1.0f / p.I; }
				else if (data_type == "e") p.e = data_value[0];
				else if (data_type == "sf") p.sf = data_value[0];
				else if (data_type == "df") p.df = data_value[0];
				else if (data_type == "id") p.id = data_value[0];
			}
			
			index_count = std::fmaxf(index_count, p.id);
			
			p.model.resize(p.n_vertices);
			p.vertices.resize(p.n_vertices);

			for (int i = 0; i < p.n_vertices; i++) p.model[i] = { cosf(2.0f * PI / p.n_vertices * i + PI / 4.0f),
																sinf(2.0f * PI / p.n_vertices * i + PI / 4.0f) };
			p.Update();

			polygons.push_back(p);
		}
		else {
			// ConstraintObject
			ConstraintObject c;
			for (auto& data : main_data) {
				const std::vector<std::string>& sub_main_data = ParseStringDelimeter(data, ':');

				const std::string& data_type = sub_main_data[0];
				const std::vector<float>& data_value = ParseFloatDelimeter(sub_main_data[1], ',');

				if (data_type == "pivot") c.pivot_pos = { data_value[0], data_value[1] };
				else if (data_type == "point") c.point_pos = { data_value[0], data_value[1] };
				else if (data_type == "len") c.len = data_value[0];
				else if (data_type == "k") c.k = data_value[0];
				else if (data_type == "b") c.b = data_value[0];
				else if (data_type == "index") c.polygon_index = data_value[0];
				else if (data_type == "sling") c.is_sling = data_value[0];
			}
			constraints.push_back(c);
		}
	}
	offset = { 0.0f, 0.0f };

	reader.close();
}

void PolygonEditor::OnMousePressEdit(const olc::vf2d& m_pos) {
	for (auto& poly : polygons) {
		for (int i = 0; i < poly.n_vertices - 2; i++) {
			int j = (i + 1) % poly.n_vertices;
			int k = (i + 2) % poly.n_vertices;

			const olc::vf2d& ab = poly.vertices[j] - poly.vertices[0];
			const olc::vf2d& bc = poly.vertices[k] - poly.vertices[j];
			const olc::vf2d& ca = poly.vertices[0] - poly.vertices[k];

			const olc::vf2d& pa = poly.vertices[0] - (m_pos - offset);
			const olc::vf2d& pb = poly.vertices[j] - (m_pos - offset);
			const olc::vf2d& pc = poly.vertices[k] - (m_pos - offset);

			bool is_point_in_triangle = (ab.cross(pa) >= 0.0f && bc.cross(pb) >= 0.0f && ca.cross(pc) >= 0.0f);
			if (is_point_in_triangle) {
				selected_polygon = &poly;
				return;
			}
		}
	}
	selected_polygon = nullptr;
}

void PolygonEditor::OnMouseHoldEdit(const olc::vf2d& m_pos, float m_wheel, int key) {
	if (selected_polygon == nullptr) return;
	selected_polygon->is_update = true;
	selected_polygon->Update();

	selected_polygon->position = m_pos - offset;
	for (auto& c : constraints) {
		if (c.polygon_index == selected_polygon->id) { c.point_pos = selected_polygon->position; }
	}

	if (key == 0) return;

	if (key & POLY_ROTATE) { selected_polygon->angle += m_wheel; }
	else if (key & POLY_SCALE_X) { selected_polygon->scale.x = std::fminf(1.0f, std::fmaxf(selected_polygon->scale.x + m_wheel, 0.0f)); }
	else if (key & POLY_SCALE_Y) { selected_polygon->scale.y = std::fminf(1.0f, std::fmaxf(selected_polygon->scale.y + m_wheel, 0.0f)); }
	else if (key & POLY_LEN) { selected_polygon->len = std::fmaxf(selected_polygon->len + 5.0f * Sign(m_wheel), 1.0f); }
}

void PolygonEditor::OnMouseReleaseEdit() {}

void PolygonEditor::ClearPolygon() {
	if (selected_polygon == nullptr) return;
	for (int i = polygons.size() - 1; i >= 0; i--) {
		if (polygons[i].id == selected_polygon->id) {

			for (int j = constraints.size() - 1; j >= 0; j--) {
				if (constraints[j].polygon_index == selected_polygon->id) {
					constraints.erase(constraints.begin() + j);
				}
			}

			selected_polygon = nullptr;
			polygons.erase(polygons.begin() + i);
			return;
		}
	}
}

void PolygonEditor::OnMousePressPolygonAdd() {
	is_polygon_add = true;

	model.resize(n_vertices);
	vertices.resize(n_vertices);
	for (int i = 0; i < n_vertices; i++) model[i] = { cosf(2.0f * PI / n_vertices * i + PI / 4.0f), sinf(2.0f * PI / n_vertices * i + PI / 4.0f) };
}

void PolygonEditor::OnMouseHoldPolygonAdd(const olc::vf2d& m_pos, float m_wheel, int key) {
	position = m_pos - offset;

	float c = cosf(angle);
	float s = sinf(angle);

	std::vector<olc::vf2d> draw_model = model;
	for (auto& m : draw_model) m *= scale;

	for (int i = 0; i < n_vertices; i++) {
		vertices[i] = {
			draw_model[i].x * c + draw_model[i].y * s,
			draw_model[i].x * s - draw_model[i].y * c
		};

		vertices[i] *= len;

		vertices[i] += position;
	}

	if (key == 0) return;

	if (key & POLY_ROTATE) { angle += m_wheel; }
	else if (key & POLY_SCALE_X) { scale.x = std::fminf(1.0f, std::fmaxf(scale.x + m_wheel, 0.0f)); }
	else if (key & POLY_SCALE_Y) { scale.y = std::fminf(1.0f, std::fmaxf(scale.y + m_wheel, 0.0f)); }
	else if (key & POLY_LEN) { len = std::fmaxf(len + 5.0f * Sign(m_wheel), 1.0f); }
}

void PolygonEditor::OnMouseReleasePolygonAdd() {
	is_polygon_add = false;

	PolygonObject polygon(n_vertices);
	polygon.position = position;
	polygon.angle = angle;
	polygon.len = len;
	polygon.model = model;
	polygon.vertices = vertices;
	polygon.scale = scale;
	polygon.mass = mass;
	polygon.inv_mass = mass == 0.0f ? 0.0f : 1.0f / mass;
	polygon.I = mass * (len * len) / 12.0f;
	polygon.inv_I = polygon.I == 0.0f ? 0.0f : 1.0f / polygon.I;
	polygon.e = e;
	polygon.sf = sf;
	polygon.df = df;
	polygon.color = olc::Pixel(r, g, b);
	polygon.id = index_count++;

	polygons.push_back(polygon);
}

void PolygonEditor::OnMousePressConstraintAdd() {
	if (selected_polygon == nullptr) return;
	is_draw_constraint = true;
}

void PolygonEditor::OnMouseHoldConstraintAdd(const olc::vf2d& m_pos) {
	if (is_draw_constraint) pivot_pos = m_pos;
}

void PolygonEditor::OnMouseReleaseConstraintAdd() {
	if (!is_draw_constraint) return;

	is_draw_constraint = false;
	ConstraintObject constraint;
	constraint.pivot_pos = pivot_pos;
	constraint.point_pos = selected_polygon->position;
	constraint.len = (constraint.pivot_pos - constraint.point_pos).mag();
	constraint.k = 0.6f;
	constraint.b = 0.4f;
	constraint.is_sling = false;
	constraint.polygon_index = selected_polygon->id;

	constraints.push_back(constraint);
}

void PolygonEditor::MousePressPan(const olc::vf2d& m_pos) {
	prev_pan_pos = m_pos;
}

void PolygonEditor::MouseHoldPan(const olc::vf2d& m_pos) {
	offset += m_pos - prev_pan_pos;
	//if (offset.x > 0.0f) offset.x = 0.0f;
	//if (offset.y < 0.0f) offset.y = 0.0f;

	prev_pan_pos = m_pos;
}

void PolygonEditor::ChangeEditorState(int& editor_state, Scene& scene) {
	if (editor_state == 1) {
		scene.GetShapes().clear();
		editor_state = 0;
	}
	else if (editor_state == 0) {

		std::unordered_map<int, int> ids;

		for (auto& p : polygons) {
			if (p.position.x < 0.0f || p.position.y < 0.0f || p.position.x > (float)bounds.x || position.y > (float)bounds.y) continue;

			RigidBody rb(p.position, p.n_vertices, p.len, p.angle, 0.0f, p.mass, p.e, p.sf, p.df, p.id);
			rb.SetColor(p.color);
			rb.decal = nullptr;
			std::vector<olc::vf2d> polygon_model = p.model;
			for (auto& m : polygon_model) m *= p.scale;
			rb.SetModel(polygon_model);
			ids[p.id] = rb.GetID();
		
			scene.AddShape(rb);
		}

		for (auto& c : constraints) {
			Constraint constraint(c.pivot_pos, c.len, c.k, c.b, 5, c.is_sling);
			constraint.Attach(ids[c.polygon_index]);

			scene.AddConstraint(constraint);
		}
		editor_state = 1;
	}
}

void PolygonEditor::GUILogic(Scene& scene) {
	scene.SetBounds((olc::vf2d)bounds);
	gui.run();
}

void PolygonEditor::PreviewRender(olc::PixelGameEngine* pge) {
	if (is_polygon_add) {
		for (int i = 0; i < n_vertices; i++) {
			int j = (i + 1) % n_vertices;

			pge->DrawLine(vertices[i] + offset, vertices[j] + offset);
		}
	}

	if (is_draw_constraint && selected_polygon != nullptr) {
		pge->DrawLine(selected_polygon->position, pivot_pos);
	}
}

void PolygonEditor::Draw(olc::PixelGameEngine* pge, const olc::vf2d& m_pos) {
	for (auto& p : polygons) p.Draw(pge, offset);
	for (auto& c : constraints) c.Draw(pge, offset);
	PreviewRender(pge);
	if (selected_polygon != nullptr) {
		int circle_r = 10;
		pge->FillCircle((olc::vi2d)selected_polygon->position, circle_r, selected_polygon->color * 0.8f);
	}

	// Boundaries
	pge->DrawLine({ (int)offset.x, (int)offset.y },							{ (int)offset.x + bounds.x, (int)offset.y });
	pge->DrawLine({ (int)offset.x + bounds.x, (int)offset.y },				{ (int)offset.x + bounds.x, (int)offset.y + bounds.y });
	pge->DrawLine({ (int)offset.x + bounds.x, (int)offset.y + bounds.y },	{ (int)offset.x, (int)offset.y + bounds.y });
	pge->DrawLine({ (int)offset.x, (int)offset.y + bounds.y },				{ (int)offset.x, (int)offset.y });

	// Rendering mouse coordinates in world space
	const std::string& text = "(" + std::to_string((int)(m_pos.x + offset.x)) + ", " + std::to_string((int)(m_pos.y + offset.y)) + ")";
	pge->DrawString({ (int)m_pos.x, (int)m_pos.y + 15 }, text, olc::BLUE);
}