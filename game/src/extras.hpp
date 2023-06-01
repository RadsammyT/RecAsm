/*
 * This header file is primarily used so i can stuff
 * functions/vars/macros, unused or not, in here for use.
 *
 * Functions that don't depend on other functions tend to be declared first
 */

#include <raylib-cpp.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <tinyfiledialogs.h>
#include "rayImGui/imgui.h"

#define u64 unsigned long long int
#define MAX 9
#define HEIGHT 125
#define REC_THRESHOLD 15000
#define MOVE_SPEED 4
#define SPRINT_SPEED 10
char const *fp[1] = {"*.RecAsm"};
char const *ex[1] = {"*.h"};

struct settings {
		Vector2 gridOffset;
		int gridSpace;
		bool gridLine;
		bool tooltip;
		bool overlapHighlight;
		bool toolMainMenu;
		bool showOrigin;
		int currentMCT;
		int currentRLA;
		float camZoom;
		bool gridDrawOrder;

		bool config[11]; // NOTE: CHANGE FOR EVERY AVAILABLE SETTING. USED FOR SAVING USER-DEFINED CONFIGS TO DEFAULT
						// 0 = gridOffset
						// 1 = gridSpace
						// 2 = gridline
						// 3 = tooltip
						// 4 = currentMCT
						// 5 = overlapHighlight
						// 6 = toolMainMenu
						// 7 = currentRLA
						// 8 = showOrigin
						// 9 = camZoom
						// 10 = gridDrawOrder
};

// Struct that handles all rectangles in working world
struct RecBundle {
	raylib::Rectangle shape;
	raylib::Color color;
};

enum exportType {
	EXPORT_CPP_VECTOR = 0,
	EXPORT_C_ARRAY
};

enum MiddleClickTools {
	MDT_NONE = 0,
	MDT_COPY_COLOR,
	MDT_CAM_PAN,
};

enum RecListActions {
	RLA_DELETE = 0,
	RLA_COLOR,
	RLA_SWAP,
};

inline bool recBothNeg(Rectangle *in) {
	return (in->width < 0 && in->height < 0);
}

inline bool recSizeZero(Rectangle *in) {
	return (in->width == 0 || in->height == 0);
}

inline bool recXor(Rectangle *in) {
	return (in->width < 0 && in->height > 0) || (in->width > 0 && in->height < 0);
}

inline unsigned char ftouc(float in) {
	if(in > 255) {
		return (unsigned char)255;
	}

	if(in < 0) {
		return (unsigned char) 0;
	}

	return (unsigned char)in;
}

inline unsigned char dtouc(double in) {
	return (unsigned char)in;
}

inline float itof(int in) {
	return (float)in;
}

void InfoBox(const char* message) {
	tinyfd_messageBox(
				"Quick Message Box",
				message,
				"ok",
				"info",
				1
			);
}

std::string trunFloat(float in) {
	std::string out = std::to_string(in);
	for(auto i = out.end(); i != out.begin(); i--) {
		if(*i == '0' || *i == '.') {
			if(*i == '.') {
				out.pop_back();
				break;
			} else {
				out.pop_back();
			}
			
		} else if(std::isdigit(*i)) {
			break;
		}
	}

	return out;
}

unsigned char getZoomTrans(float *zoom, int *gridSpacing, float* factor) {
	// unsigned char zoomer = cam.zoom < 1 ? ftouc(255*cam.zoom) : 255;
	if(*zoom > 1)
		return 255;
	return ftouc((255+ *gridSpacing * *factor) * *zoom);
}

std::vector<Rectangle> getCollidingRectangles(std::vector<RecBundle> recs) {
	std::vector<Rectangle> result = {};
	for(int i = 0; i < recs.size(); i++) {
		for(int j = 0; j < recs.size(); j++) {
			if(CheckCollisionRecs(recs[i].shape, recs[j].shape) && i != j) {
				result.push_back(GetCollisionRec(recs[i].shape, recs[j].shape));
			}
		}
	}

	return result;

}

// yanked from imgui_demo.cpp
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}



float* RayColor2FloatP(Color base) {
	float ret[4] = {
		(float)base.r/255.0f,
		(float)base.g/255.0f,
		(float)base.b/255.0f,
		(float)base.a/255.0f,
	};

	return ret;
}

ImVec4 RayColor2ImVec(Color base) {
	return ImVec4 {
		(float)base.r/255.0f,
		(float)base.g/255.0f,
		(float)base.b/255.0f,
		(float)base.a/255.0f,
	};
}

Color FloatP2RayColor(float base[4]) {
	return Color {
		ftouc(base[0]*255),
		ftouc(base[1]*255),
		ftouc(base[2]*255),
		ftouc(base[3]*255),
	};
}

void color3(raylib::Color in, raylib::Color* out) {
	out->r = in.r;
	out->g = in.g;
	out->b = in.b;
	return;
}

void DrawGrid(settings* cfg, raylib::Camera2D* cam, int screenWidth, int screenHeight, float* factor, Color gridColor) {
	if(cfg->showOrigin)
		DrawCircleLines(0,0,10/cam->GetZoom(), RED);

	if(cfg->gridSpace != 0 && cfg->gridLine) { 
		int x = ((int)cam->GetTarget().x / cfg->gridSpace) * cfg->gridSpace;
		int y = ((int)cam->GetTarget().y / cfg->gridSpace) * cfg->gridSpace;
		//unsigned char zoomer = cam->zoom < 1 ? ftouc(255*cam->zoom) : 255;
		unsigned char zoomer = getZoomTrans(&cam->zoom, &cfg->gridSpace, factor);
		int gridLimit = (screenWidth/cfg->gridSpace/2+10) / cam->zoom;
		for(int iter = -gridLimit; iter <= gridLimit; iter++) {
			DrawLineV(
					Vector2 { 
						x + (iter * cfg->gridSpace) + cfg->gridOffset.x,
						cam->GetTarget().y - screenHeight / cam->GetZoom() / 2
						},
					Vector2 { 
						x + (iter*cfg->gridSpace) + cfg->gridOffset.x, 
						cam->GetTarget().y + screenHeight / cam->GetZoom()
						}, 
					Color{gridColor.r, gridColor.g, gridColor.b, zoomer}
					);
		}
		gridLimit = (screenHeight/cfg->gridSpace/2+10) / cam->zoom;
		for(int iter = -gridLimit; iter <= gridLimit; iter++) {
			DrawLineV(Vector2{cam->GetTarget().x - screenWidth / cam->GetZoom(),
					y + (iter * cfg->gridSpace) + cfg->gridOffset.y},
					Vector2{cam->GetTarget().x + screenWidth / cam->GetZoom(),
					y + (iter * cfg->gridSpace) + cfg->gridOffset.y}, 
					Color{gridColor.r, gridColor.g, gridColor.b, zoomer}
					);
		}
		if(cfg->showOrigin) {
			DrawLine(INT_MIN, 0, INT_MAX, 0, RED);
			DrawLine(0, INT_MIN, 0, INT_MAX, RED);
		}
	}
}

u64 CountFileLines(std::string filename) {
	u64 out = 0;
	std::ifstream file(filename);
	if(!file.is_open()) {
		InfoBox("ERROR: Cannot open file to count lines. Check console for file name ");
		printf("ERROR: Cannot open file \"%s\" to count lines\n", filename.c_str());
		exit(1);
	}

	while(std::getline(file, filename)) //troll coding
		out++;
	return out;
}

//yoinked from https://en.cppreference.com/w/cpp/string/basic_string/replace
std::size_t replace_all(std::string& inout, std::string_view what, std::string_view with) {
    std::size_t count{};
    for (std::string::size_type pos{};
         inout.npos != (pos = inout.find(what.data(), pos, what.length()));
         pos += with.length(), ++count) {
        inout.replace(pos, what.length(), with.data(), with.length());
    }
    return count;
}


std::string LoadFilePopup(char const*const* gp, int fpl = 1, const char* DEFPATH = NULL) {
	char* original =  tinyfd_openFileDialog(
				"Load .RecAsm file",
				DEFPATH,
				1,
				fp,
				"Rectangle Assembler Scenes",
				0
			);
	if(original == NULL) {
		return "NULL";
	} else {
		std::string out = original;
		replace_all(out, "\\", "/");
		return out;
	}	
}

std::string SaveFilePopup( char const*const* fp, bool sorex = false, int fpl = 1, const char* DEFPATH = NULL) {
	char *original = sorex ? tinyfd_saveFileDialog(
				"Export .RecAsm file",
				DEFPATH,
				fpl,
				ex,
				"Rectangle Assembler Exports"
			) 
			:
			 tinyfd_saveFileDialog(
				"Save .RecAsm file",
				DEFPATH,
				fpl,
				fp,
				"Rectangle Assembler Scenes"
			)
			;
	if(original == NULL) {
		return "NULL";
	} else {
		std::string out = original;
		replace_all(out, "\\", "/");
		return out;
	}
	printf("CRIT ERROR: SaveFilePopup did not return!\n-\nReturn Value == %s\n-\n", original);
	exit(1);
}

// -1 == cursor not hovering on a rectangle
int getHoveredRec(std::vector<RecBundle> recs, raylib::Camera2D *cam, bool reverse = false) {
	raylib::Vector2 temp = cam->GetScreenToWorld(GetMousePosition());
	if(!reverse) {
		for(int i = 0; i < recs.size(); i++)
			if(recs[i].shape.CheckCollision(temp))
				return i;
	} else if(reverse){
		for(int i = recs.size();i!=0; i--) {
			if(recs[i-1].shape.CheckCollision(temp))
				return i-1;
		}
	}
	return -1;	
}

// mouse input/translation
void mouse(std::vector<RecBundle> *recs, 	//
		std::vector<Rectangle> *overlap,		//
		bool *prevInput, 					//
		raylib::Vector2 *heldMouse,
		Vector2 *ply,		//
		raylib::Camera2D *cam, 				//
		raylib::Color *color, 				//
		float* imguiColor,					//
											//bool *rapidFire, 					//
		raylib::Window *win, 				//
		std::vector<RecBundle> *undo, 		//
		int* gridSpacing,					//
		Vector2* gridOffset,				//
		int* mouseJump,						//
		int* currentMCT,					//
		ImGuiIO& io,						//
		bool *change,
		bool *highlight)						//
{
	if (*prevInput != IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		//if (*prevInput && !raylib::Rectangle(0,-HEIGHT,win->GetWidth(), HEIGHT*2).CheckCollision(raylib::Mouse::GetPosition()))
		if(*prevInput && !io.WantCaptureMouse)
		{
			// make rectangle
			
			raylib::Rectangle temp(*heldMouse, (raylib::Vector2(cam->GetScreenToWorld(GetMousePosition())) - *heldMouse));
			
			if(*gridSpacing >= 1) {
				int x = ((int) cam->GetScreenToWorld(GetMousePosition()).x / (int)*gridSpacing) * *gridSpacing + gridOffset->x; 
				int y = ((int) cam->GetScreenToWorld(GetMousePosition()).y / (int)*gridSpacing) * *gridSpacing + gridOffset->y;
				temp.x = heldMouse->x;
				temp.y = heldMouse->y;
				temp.width = x - heldMouse->x;
				temp.height = y - heldMouse->y;
			}


			if (temp.GetWidth() < 0)
			{
				temp.SetX(temp.GetX() + temp.GetWidth());
				temp.SetWidth(-temp.GetWidth());
			}

			if (temp.GetHeight() < 0)
			{
				temp.SetY(temp.GetY() + temp.GetHeight());
				temp.SetHeight(-temp.GetHeight());
			}
			bool noplace = false;
			if(temp.height == 0)
				noplace = true;
			if(temp.width == 0)
				noplace = true;
			if(!noplace) {
				*change = true;
				recs->push_back(RecBundle{temp, *color});
				if(*highlight)
					*overlap = getCollidingRectangles(*recs);
			}
			*heldMouse = cam->GetScreenToWorld(GetMousePosition());
		}
		else
		{
			// init rectangle making
			if(*gridSpacing >= 1) {	
				int x = ((int) cam->GetScreenToWorld(GetMousePosition()).x / (int)*gridSpacing) * *gridSpacing + gridOffset->x; 
				int y = ((int) cam->GetScreenToWorld(GetMousePosition()).y / (int)*gridSpacing) * *gridSpacing + gridOffset->y;
				*heldMouse = raylib::Vector2(x,y);
			} else {
			
				*heldMouse = cam->GetScreenToWorld(GetMousePosition());
			}
		}
	}
	else
	{	// Draw Rectangle, should prolly use in testing idk
		//if (*prevInput && !raylib::Rectangle(0,0,win->GetWidth(), HEIGHT).CheckCollision(raylib::Mouse::GetPosition()))
		if(*prevInput && !io.WantCaptureMouse) {
			raylib::Rectangle temp(100,100,100,100);
			if(*gridSpacing >= 1) {
				int x = ((int) cam->GetScreenToWorld(GetMousePosition()).x / (int)*gridSpacing) * *gridSpacing + gridOffset->x; 
				int y = ((int) cam->GetScreenToWorld(GetMousePosition()).y / (int)*gridSpacing) * *gridSpacing + gridOffset->y;
				temp.x = heldMouse->x;
				temp.y = heldMouse->y;
				temp.width = x - heldMouse->x;
				temp.height = y - heldMouse->y;
			} else {
				temp = raylib::Rectangle(*heldMouse, 
raylib::Vector2(cam->GetScreenToWorld(GetMousePosition()).GetX() - heldMouse->GetX(), 
cam->GetScreenToWorld(GetMousePosition()).GetY() - heldMouse->GetY()));

			}
			if (temp.GetWidth() < 0) {
				temp.SetX(temp.GetX() + temp.GetWidth());
				temp.SetWidth(-temp.GetWidth());
			}

			if (temp.GetHeight() < 0) {
				temp.SetY(temp.GetY() + temp.GetHeight());
				temp.SetHeight(-temp.GetHeight());
			}
			DrawRectangleLinesEx(temp, 1/cam->GetZoom(), WHITE);
		}
	}
		if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !io.WantCaptureMouse) {
			int t = getHoveredRec(*recs, cam);
			if(t != -1) {
				*change = true;
				undo->push_back(recs->at(t));
				recs->erase(recs->begin() + t);
			} 
		}

		if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) && !io.WantCaptureMouse) {
			switch(*currentMCT) {
				case 0:
					break;
				case 1:
					if(IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) && getHoveredRec(*recs, cam) != -1) {
						*color = recs->at(getHoveredRec(*recs, cam)).color;
						imguiColor[0] = (float)color->r/255.0f; //cautious casting lmao
						imguiColor[1] = (float)color->g/255.0f;
						imguiColor[2] = (float)color->b/255.0f;
						imguiColor[3] = (float)color->a/255.0f;
					}
					break;
				case 2:
					ply->x += -GetMouseDelta().x / cam->zoom;
					ply->y += -GetMouseDelta().y / cam->zoom;
					break;

			}
		}

		if (IsKeyPressed(KEY_R) )
		{	

			if (recs->size() != 0){
				*change = true;
				undo->push_back(recs->back());
				recs->pop_back();
				if(*highlight)
				*overlap = getCollidingRectangles(*recs);
			}

		}

		if(IsKeyPressed(KEY_U) ) {
			if(undo->size() != 0) {
				recs->push_back(undo->back());
				undo->pop_back();

				if(*highlight)
				*overlap = getCollidingRectangles(*recs);
			}
		}

		// Color picking
		if(IsKeyPressed(KEY_LEFT)) 
			SetMousePosition(GetMouseX() - *mouseJump, GetMouseY());
		if(IsKeyPressed(KEY_RIGHT))
			SetMousePosition(GetMouseX() + *mouseJump, GetMouseY());
		if(IsKeyPressed(KEY_UP))
			SetMousePosition(GetMouseX(), GetMouseY() - *mouseJump);
		if(IsKeyPressed(KEY_DOWN))
			SetMousePosition(GetMouseX(), GetMouseY() + *mouseJump);
		int outline = getHoveredRec(*recs, cam, true);
		if(outline != -1) {
			DrawRectangleLinesEx(raylib::Rectangle(recs->at(outline).shape.GetX() - 16 / cam->GetZoom(), 
													recs->at(outline).shape.GetY() - 16 / cam->GetZoom(), 
													recs->at(outline).shape.GetWidth() + 16*2 / cam->GetZoom(), 
													recs->at(outline).shape.GetHeight() + 16*2 / cam->GetZoom()), 
													1/cam->GetZoom(), 
													WHITE);
		}


	*prevInput = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
}

std::string filterFile(std::string in) {
	std::string out = "";
	bool prime = false;
	for(auto i = in.size() - 1; i != 0; i--) {
		if(prime) {
			if(in.at(i) != '/') {
				out.push_back(in.at(i));
			} else {
				break;
			}
		}
		if(in.at(i) == '.') {
			prime = true;
		}
	}

	std::reverse(out.begin(), out.end());
	return out;
}

bool loadConfig(std::string config,
				settings* settings
		) {
	if(FileExists(config.c_str())) {
		std::ifstream f(config);
		std::string line;
		std::getline(f,line);
		if(!line.starts_with("//RecAsm Config File")) {
			printf("CONFIG: Config File has no check line\n");
			tinyfd_messageBox(
								"Rectangle Assembler",
								"ERROR: Config File has no check line. \nConfig Check Line: (//RecAsm Config File) \nexcluding parenthesis",
								"ok",
								"error",
								1
							);
			return false;
		}

		while(std::getline(f,line)) {
			if(line.starts_with("ox")) {
				line.erase(0,2);
				TraceLog(LOG_INFO, "Offset X Found");
				settings->config[0] = true;
				settings->gridOffset.x = stof(line);

			}

			if(line.starts_with("oy")) {
				line.erase(0,2);
				TraceLog(LOG_INFO, "Offset y Found");
				settings->config[0] = true;
				settings->gridOffset.y = stof(line);
			}

			if(line.starts_with("gridSpace ")) {
				line.erase(0,10);
				TraceLog(LOG_INFO, "gridSpace Found");
				settings->config[1] = true;
				settings->gridSpace = stoi(line);
			}

			if(line.starts_with("gridLine ")) {
				line.erase(0,9);
				TraceLog(LOG_INFO, "gridLine Found");
				settings->config[2] = true;
				settings->gridLine = stoi(line);
			}

			if(line.starts_with("tooltip ")) {
				line.erase(0,8); 
				TraceLog(LOG_INFO, "tooltip Found");
				settings->config[3] = true;
				settings->tooltip = stoi(line);
			}

			if(line.starts_with("mct ")) {
				line.erase(0,4);
				TraceLog(LOG_INFO, "Middle Click Tool Found");
				settings->config[4] = true;
				settings->currentMCT = stoi(line);
			}

			if(line.starts_with("hor ")) {
				line.erase(0,4);
				TraceLog(LOG_INFO, "overlapHighLight Found");
				settings->config[5] = true;
				settings->overlapHighlight = stoi(line);
			}

			if(line.starts_with("toolMainMenu ")) {
				line.erase(0,13);
				TraceLog(LOG_INFO, "Tool Main Menu Found");
				settings->config[6] = true;
				settings->toolMainMenu = stoi(line);
			}

			if(line.starts_with("grid_origin ")) {
				line.erase(0,12);
				TraceLog(LOG_INFO, "Show Origin Found");
				settings->config[8] = true;
				settings->showOrigin = stoi(line);
			}

			if(line.starts_with("cam zoom ")) {
				line.erase(0,9);
				TraceLog(LOG_INFO, "Cam Zoom Found");
				settings->config[9] = true;
				settings->camZoom = stof(line);
			}
		}

	} 
	else 
		printf("CONFIG: Config File does not exist\n");
	return false;
}

bool saveConfig(settings *cfg) {
	std::ofstream f("RecAsm.config");
	f << "//RecAsm Config File\n";
	if(cfg->config[0]) {
		f << "ox " << cfg->gridOffset.x << "\n";
		f << "oy " << cfg->gridOffset.y << "\n";
	}
	if(cfg->config[1])
		f << "gridSpace " << cfg->gridSpace << "\n";
	if(cfg->config[2])
		f << "gridLine " << cfg->gridLine << "\n";
	if(cfg->config[3])
		f << "tooltip " << cfg->tooltip << "\n";
	if(cfg->config[4])
		f << "mct " << cfg->currentMCT << "\n";
	if(cfg->config[5])
		f << "hor " << cfg->overlapHighlight << "\n";
	if(cfg->config[6])
		f << "toolMainMenu " << cfg->toolMainMenu << "\n";

	if(cfg->config[8])
		f << "grid_origin " << cfg->showOrigin << "\n";
	if(cfg->config[9])
		f << "cam zoom " << cfg->camZoom << "\n";
	return false;
}

bool exportToHeader(std::vector<RecBundle> recs, std::string *resultMessage, double* timer, std::string fileout, int exType = EXPORT_CPP_VECTOR)
{
	if(fileout == "NULL") {
		return false;
	}
	std::string filteredout = filterFile(fileout);
	std::ofstream f;
	f.open(fileout);
	if(exType == EXPORT_CPP_VECTOR) {
		f	<< "#include <vector> \n// EXPORTED WITH RECTANGLE ASSEMBLER\n"
			<< "// Export Type = C++ Vector\n"
			<< "// please include this struct in main code:\n" 
			<< "// struct RecBundle { Rectangle shape; Color color; };\n";
		f << "namespace RecAsmExports { std::vector<RecBundle>" << filteredout << " = {\n";

	} else if(exType == EXPORT_C_ARRAY) {
		f	<< "// EXPORTED WITH RECTANGLE ASSEMBLER\n"
			<< "// Export Type = C Array\n"
			<< "// Required struct:\n" 
			<< "typedef struct { Rectangle shape; Color color; } RecBundle;\n"
			<< "// You are encouraged to comment or modify this struct and the array\n"
			<< "// Because this format is untested af and I don't know if it works in C\n";
		f << "extern int SIZE = " <<  recs.size() << ";\n";
		f << "extern RecBundle " << filteredout <<"[" << recs.size() << "] = {\n";
	}

	for (RecBundle rec : recs)
	{
		f << "RecBundle{ Rectangle{"
		<< rec.shape.GetX() << ","
		<< rec.shape.GetY() << ","
		<< rec.shape.GetWidth() << ","
		<< rec.shape.GetHeight() << "}, Color{"
		<< (int)rec.color.r << ","
		<< (int)rec.color.g << ","
		<< (int)rec.color.b << ","
		<< (int)rec.color.a << "}},\n"; // i hope to god this works
	}
	if(exType == EXPORT_CPP_VECTOR) {
		f << "};\n}";
	} else if(exType == EXPORT_C_ARRAY) {
		f <<"};\n";
	}
	f.close();
	*resultMessage = TextFormat("Successful export. saved as %s", fileout.c_str());
	*timer = MAX;

	return true;
}

bool saveToFile(std::vector<RecBundle> recs, std::string *resultMessage, std::string filename, double* timer, bool* change) {
	//format each rec like this:
	//x,y,w,h,r,g,b,a\n
	if(filename == "NULL") {
		return false;
	} // DIOS MIO 2
	std::ofstream f;
	f.open(filename);
	f << "// Rectangle Assembler Scene, DO NOT EDIT. THIS LINE IS A CHECKSUM FOR VALIDATION\n";
	for (RecBundle rec : recs)
	{
		f
		<< rec.shape.GetX() << ","
		<< rec.shape.GetY() << ","
		<< rec.shape.GetWidth() << ","
		<< rec.shape.GetHeight() << ","
		<< (int)rec.color.r << ","
		<< (int)rec.color.g << ","
		<< (int)rec.color.b << ","
		<< (int)rec.color.a << ",\n";
	}
	f.close();
	*resultMessage = TextFormat("Successful save of scene. saved as %s", filename.c_str());
	*timer = MAX;
	*change = false;
	return true;
}

bool loadFromFile(std::vector<RecBundle>* recs, std::string* resultMessage, std::string file, double* timer) {
	if(file == "NULL") {
		return false;
	}
	
	////x,y,w,h,r,g,b,a\newvec
	if(!raylib::FileExists(file)) {
			*resultMessage = "ERROR: Scene file \'" + file + "\' does not exist.";
			*timer = MAX;
			return false;
	}

	

	std::vector<RecBundle> newvec = {};
	RecBundle newrb;
	std::string buf;
	std::string line;
	int step = 0;
	std::ifstream f(file);
	if(f.is_open()) {
		std::getline(f, line);
		if(!line.starts_with("// Rectangle")) {
			//TextPopupEx("ERROR: Selected file is not a Rectangle Assembler Scene.", timer, resultMessage);
			tinyfd_messageBox(
						"Rectangle Assembler Error",
						TextFormat("Selected File %s is not a Rectangle Assembler Scene.", file.c_str()),
						"ok",
						"error",
						1
					);
			return false;
		}
		int lineCount = CountFileLines(file);
		if(lineCount > REC_THRESHOLD) {

			char const* temp_message = TextFormat("The file you are trying to open has a very large amount of rectangles. Opening them may lead to performance issues.\nLine Count: %d\nAre you sure you want open this file?", lineCount);
				if(
						tinyfd_messageBox(
								"WARNING: Large Rectangle Count!",
								temp_message,
								"yesno",
								"warning",
								0
							) == 0
				) {
					
					return false;
				}
		}
		while(std::getline(f, line)) {
			buf = ""; 
			for (int i = 0; i < (int)line.size(); i++) {
				if(line[i] != ',' && line[i] != '\n' && line[i] != '\r') { // \r in there cuz windows
					buf += line[i];
				} else {
					if((line[i] == ',' || line[i] == '\n') && line[i] != '\r') {
						float temp = std::stof(buf);
						if(step == 0) {
							newrb.shape.SetX( temp);
						}

						if(step == 1) {
							newrb.shape.SetY( temp);
							
						}

						if(step == 2) {
							newrb.shape.SetWidth( temp);
							
						}

						if(step == 3) {
							newrb.shape.SetHeight( temp);
							
						} 

						if(step == 4) {
							newrb.color.SetR((unsigned char)temp);
							
						}

						if(step == 5) {
							newrb.color.SetG((unsigned char)temp);
							
						}

						if(step == 6) {
							newrb.color.SetB((unsigned char)temp);
							
						}

						if(step == 7) { 
							newrb.color.SetA((unsigned char)temp);
							
						}
						step++;
						buf = "";
					}
				}

			}
			newvec.push_back(newrb);
			step = 0;
		}
		f.close();

		*recs = newvec;
	} else {
			*resultMessage = "CRIT ERROR: Scene file \'" + file + "\' cannot be opened.";
			*timer = MAX;
			return false;
	}

	return 0;
}
