/*
 *	Rectangle Assembler
 *	By RadsammyT
 *	IMPORTANT: C++ Standard must be C++20, otherwise RecAsm won't compile!
 *	cuz std library sh*t lmao
 */

#define RESOURCE_USED 0
#define RAYGUI_IMPLEMENTATION

#include <raylib-cpp.hpp>
#include <string>
#include <vector>
#include <time.h>
#include <raygui.h>
#include "extras.hpp"
#include <stdexcept>
#include <climits>
#include <tinyfiledialogs.h>
#include <cfloat>
#include <cmath>
#include "rayImGui/rlImGui.h"
const std::string rayver = RAYLIB_VERSION;

int screenWidth = 800*1;
int screenHeight = 450*1;

bool InfoOn = false;
bool ImDemoOn = false;
bool gridOffsetSlider = false;
float gridOffsetDI = 0.01f;
float color[4] = {1,1,1,1}; // R G B A
float ImOverlapColor[4] = {1,0,0,1};
float ImRecListColor[4] = {1,1,1,1};
Rectangle *hoveredRecList = NULL;

bool changesMade = false;
std::string resultMessage;
double current = 0;
int sample = 0;
raylib::Vector2 ply(0, 0);
raylib::Color tc(LIGHTGRAY);
raylib::Color workingColor(255,255,255,255);
		Color overlapColor = {255,0,0,255};
		Color recListColor = {255,255,255,255};
float factor = 1;

settings cfg = {
	Vector2 {0,0},
	50,
	true,
	true,
	false,
	0,
	0,
	0,
	{0,0,0,0,0,0,0}
};


int main(int argc, char* argv[]) {
	InitAudioDevice();
    
	raylib::Window win(screenWidth, screenHeight, "Rectangle Assembler");
	loadConfig("RecAsm.config", &cfg);
	rlImGuiSetup(true);
	
	ImGuiIO& io = ImGui::GetIO();

	//icon image	
	#if RESOURCE_USED
	if(!FileExists("resources/iconc.png")) {
		throw std::runtime_error("Icon does not exist");
	}

	if(!FileExists("resources/Load.wav")) {
		throw std::runtime_error("Load sound does not exist");
	}
		
	Image im = LoadImage("resources/iconc.png"); 
	ImageFormat(&im, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

	Texture imt = LoadTextureFromImage(im);
	//icon image
	raylib::Sound load("resources/Load.wav");
	
	#endif
	raylib::Camera2D cam(raylib::Vector2(screenWidth/2,screenHeight/2), ply);
    win.SetState(FLAG_WINDOW_RESIZABLE);
	win.SetMinSize(800,450);
	#if RESOURCE_USED
	SetWindowIcon(im);
	#endif
	
    std::vector<RecBundle> recs = {
        // RecBundle{raylib::Rectangle(100, 100, -100, -100), raylib::Color(255, 255, 255, 255)},
    };
	std::vector<RecBundle> undo;
	std::vector<Rectangle> overlap;

	int gridSpacing = 50;	
	int mouseJump = 1;

    bool prevInput = false; //previous LMB input. false = not down, true = down
    bool rapidFire = false; // if color picking slides or increments by 1
    raylib::Vector2 heldMouse(0, 0);
    SetTargetFPS(60);
	#if RESOUCE_USED
	load.Play();
	#endif
    while (!win.ShouldClose()) // Detect window close button or ESC key
    {
		if(IsWindowResized()) {
			screenHeight = GetScreenHeight();
			screenWidth = GetScreenWidth();
			cam.SetOffset(raylib::Vector2(screenWidth/2, screenHeight/2));
		}



        current -= GetFrameTime();
        if(current < 0) {
            current = MAX;
            resultMessage = "";
        }
		if(!io.WantCaptureKeyboard) {
		
		
			if(IsKeyDown(KEY_D)) {
				if(IsKeyDown(KEY_LEFT_SHIFT))
					ply.SetX(ply.GetX() + SPRINT_SPEED * 1/cam.zoom);
				else
					ply.SetX(ply.GetX() + MOVE_SPEED* 1/cam.zoom);
			}
			if(IsKeyDown(KEY_A)) {
				if(IsKeyDown(KEY_LEFT_SHIFT))
					ply.SetX(ply.GetX() - SPRINT_SPEED* 1/cam.zoom);
				else
					ply.SetX(ply.GetX() - MOVE_SPEED* 1/cam.zoom);
			}
			if(IsKeyDown(KEY_S)) {
				if(IsKeyDown(KEY_LEFT_SHIFT))
					ply.SetY(ply.GetY() + SPRINT_SPEED* 1/cam.zoom);
				else
					ply.SetY(ply.GetY() + MOVE_SPEED* 1/cam.zoom);
			}
			if(IsKeyDown(KEY_W)){
				if(IsKeyDown(KEY_LEFT_SHIFT))
					ply.SetY(ply.GetY() - SPRINT_SPEED* 1/cam.zoom);
				else
					ply.SetY(ply.GetY() - MOVE_SPEED* 1/cam.zoom);
			}
			if(IsKeyPressed(KEY_SPACE)) {
				printf("DEBUG TIME"); // manual breakpoint for gdb
			}

			if(GetMouseWheelMove() != 0 && !io.WantCaptureMouse) {
				if(GetMouseWheelMove() > 0)
					cam.SetZoom(cam.GetZoom() + (GetMouseWheelMove() / 10) + (cam.zoom / 10));
				if(GetMouseWheelMove() < 0)
					cam.SetZoom(cam.GetZoom() + (GetMouseWheelMove() / 10) - (cam.zoom / 10));
			}

			if(cam.GetZoom() < 0.1) {
				cam.SetZoom(0.1);
			}

			if(IsKeyPressed(KEY_C) ) {
				cam.SetZoom(1.0f);
			}
			if(IsKeyPressed(KEY_B))
				InfoOn = !InfoOn;
		}
		#if 0 
        if(IsKeyPressed(KEY_M)) {
			for(int i = 0; i < mouseJump * 1000; i++) {
				recs.pop_back();
			}
        }
		#endif

        // i thought it would be more fancy if code were to be indented on drawing and camera modes
        BeginDrawing();
            ClearBackground(BLACK);
            cam.BeginMode(); //Drawing functions called during this point are drawn on world
				cam.SetTarget(ply);

				DrawCircleLines(0,0,10/cam.GetZoom(), RED);

				if(gridSpacing != 0 && cfg.gridLine) { 
					int x = ((int)cam.GetTarget().x / gridSpacing) * gridSpacing;
					int y = ((int)cam.GetTarget().y / gridSpacing) * gridSpacing;
					//unsigned char zoomer = cam.zoom < 1 ? ftouc(255*cam.zoom) : 255;
					unsigned char zoomer = getZoomTrans(&cam.zoom, &gridSpacing, &factor);
					int gridLimit = (screenWidth/gridSpacing/2+10) / cam.zoom;
					for(int iter = -gridLimit; iter <= gridLimit; iter++) {
						DrawLineV(
								Vector2 { 
									x + (iter * gridSpacing) + cfg.gridOffset.x,
									cam.GetTarget().y - screenHeight / cam.GetZoom() / 2
									},
								Vector2 { 
									x + (iter*gridSpacing) + cfg.gridOffset.x, 
									cam.GetTarget().y + screenHeight / cam.GetZoom()
									}, 
								Color{130,130,130,zoomer}
								);
					}
					gridLimit = (screenHeight/gridSpacing/2+10) / cam.zoom;
					for(int iter = -gridLimit; iter <= gridLimit; iter++) {
						DrawLineV(Vector2{cam.GetTarget().x - screenWidth / cam.GetZoom(),
								y + (iter * gridSpacing) + cfg.gridOffset.y},
								Vector2{cam.GetTarget().x + screenWidth / cam.GetZoom(),
								y + (iter * gridSpacing) + cfg.gridOffset.y}, Color{130,130,130,zoomer});
					}

					DrawLine(INT_MIN, 0, INT_MAX, 0, RED);
					DrawLine(0, INT_MIN, 0, INT_MAX, RED);
				}
				for(RecBundle rec: recs) {
					rec.shape.Draw(rec.color);
				}

				for(Rectangle rec: overlap) {
					DrawRectangleRec(rec, overlapColor);
				}

				mouse(&recs, &overlap, &prevInput, &heldMouse, &ply,&cam, &workingColor, color, &rapidFire, &win, &undo, &gridSpacing
						,&cfg.gridOffset,&mouseJump, &cfg.currentMCT, io, &changesMade, &cfg.overlapHighlight);

			if(IsKeyDown(KEY_RIGHT_SHIFT)) {
				for(int i = 0; i < recs.size(); i++) {
				
					DrawRectangleLinesEx(raylib::Rectangle(recs.at(i).shape.GetX() - 16 / cam.GetZoom(), 
														recs.at(i).shape.GetY() - 16 / cam.GetZoom(), 
														recs.at(i).shape.GetWidth() + 16*2 / cam.GetZoom(), 
														recs.at(i).shape.GetHeight() + 16*2 / cam.GetZoom()), 
														1/cam.GetZoom(), 
														WHITE);
					//DrawText(std::to_string(i).c_str(), recs[i].shape.x,
														//recs[i].shape.y - 16/cam.GetZoom(),
														//16 / cam.zoom,
														//WHITE);
					
					DrawRectangleV(Vector2 {recs[i].shape.x, recs[i].shape.y - 16 / cam.zoom},
							Vector2{(std::to_string(i).size() * 16) / cam.zoom, (16/cam.zoom)}, Color {0,0,0,127});
					DrawTextEx(GetFontDefault(), std::to_string(i).c_str(), 
							Vector2 {recs[i].shape.x, recs[i].shape.y-16/cam.GetZoom()}, 
							16/cam.zoom, 
							(16/cam.zoom)/10, 
							WHITE);
				}
			}

			if(hoveredRecList != NULL) {
				DrawRectangleLinesEx(raylib::Rectangle(hoveredRecList->x - 16/ cam.GetZoom(), 
					hoveredRecList->y - 16 / cam.GetZoom(), 
					hoveredRecList->width + 16*2 / cam.GetZoom(), 
					hoveredRecList->height + 16*2 / cam.GetZoom()), 
					1/cam.GetZoom(), 
					WHITE);
			
				DrawRectangleV(
						Vector2 {
							hoveredRecList->x,
							hoveredRecList->y - 16 / cam.zoom
						},
						Vector2 {
							3 * 16 / cam.zoom,
						   16/cam.zoom	
						},
						Color { 0, 0, 0, 127 }
					);
				DrawTextEx(GetFontDefault(), "HVR", Vector2 { hoveredRecList->x,
					hoveredRecList->y - 16/cam.GetZoom()},
					16 / cam.zoom,
					(16/cam.zoom)/10,
					WHITE);
			
			}

				if(gridSpacing >= 1 && !io.WantCaptureMouse) {
					int x = ((int) cam.GetScreenToWorld(GetMousePosition()).x / (int)gridSpacing) * gridSpacing + cfg.gridOffset.x; 
					int y = ((int) cam.GetScreenToWorld(GetMousePosition()).y / (int)gridSpacing) * gridSpacing + cfg.gridOffset.y;
					DrawCircleLines(x,y,10 / cam.GetZoom(),WHITE);
				}
				cam.EndMode();
				rlImGuiBegin();

				if(InfoOn) {
					ImGui::Begin("Info", &InfoOn, 0);
						ImGui::Checkbox("ImGui Demo", &ImDemoOn);
						if(ImGui::BeginTabBar("info_tab_bar")) {
								if(ImGui::BeginTabItem("Scene")) {
										ImGui::Text("Rectangle Count: %d", recs.size());
										ImGui::Text("Mouse X (World): %f", cam.GetScreenToWorld(GetMousePosition()).x);
										ImGui::Text("Mouse Y (World): %f", cam.GetScreenToWorld(GetMousePosition()).y);
										ImGui::Text("Cam X (World): %f", cam.target.x);
										ImGui::Text("Cam Y (World): %f", cam.target.y);
									ImGui::EndTabItem();
								}

								if(ImGui::BeginTabItem("Build")) {
										ImGui::Text("C++ Standard: %d", __cplusplus);
										ImGui::Text("Raylib Ver: %s", rayver.c_str());
										ImGui::Text("Build Time: %s %s", __DATE__, __TIME__); // macros are defined on preprocessor run
									ImGui::EndTabItem();
								}

								if(ImGui::BeginTabItem("Perf")) {
										ImGui::Text("FPS: %f", 1/GetFrameTime());
										ImGui::Text("FT (ms): %f", GetFrameTime()*1000);
										ImGui::Text("FT - 1/60 (ns): %d",(int) ((GetFrameTime()-1.0f/60.0f)*1'000'000'000));
										if(ImGui::Button("Sample from FT-1/60")) {
											sample = (int) ((GetFrameTime()-1.0f/60.0f)*1'000'000'000);
										}
										ImGui::Text("Sample: %d", sample);
									ImGui::EndTabItem();
								}

								if(ImGui::BeginTabItem("Rec. List")) {
									
									bool ceasePointer = false;
									bool hoverCheck = false;
									ImGui::Combo("RecList Action", &cfg.currentRLA,
											"Delete Rectangle\0"
											"Edit Color of Rectangle (See Top Color Tab)\0"
										);
									switch(cfg.currentRLA) {
										case RLA_DELETE:
												ImGui::Text("Click on button to delete");
											break;
										
										case RLA_COLOR:

        										if(ImGui::ColorEdit4("RecList Color", ImRecListColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
													recListColor = FloatP2RayColor(ImRecListColor);
												}
												//ImGui::ColorButton("Rec List Color", RayColor2ImVec(recListColor), ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(20,20));
												ImGui::SameLine();
												ImGui::Text("Click on button to change color");
											break;

									}
									for(int i = 0; i < recs.size(); i++) {
										if(ImGui::Button(TextFormat("%d",i))) { // cuz if all buttons use the same label, all buttons but the one with index 0
																				// wont work because they have the same FUCKING LABEL THATS IN FACT AN ID
																				// IMGUI LIED TO ME, IT FUCKING LIED TO ME. SHAME. SHAAAAAAAAAAAAAAAAME.
											switch(cfg.currentRLA) {
												case RLA_DELETE:
														recs.erase(recs.begin() + i);
														hoveredRecList = NULL;
														ceasePointer = true;
													break;
												
												case RLA_COLOR:
													//InfoBox("Unimplemented Function: RLA_COLOR in Rec List Switch case");
														recs[i].color = recListColor;
													break;
											}
											
										} 
										
										if(ImGui::IsItemHovered() && !ceasePointer) {
											hoveredRecList = &recs[i].shape;
											hoverCheck = true;
										} else if(!hoverCheck) {
											hoveredRecList = NULL; 
										}
										
										ImGui::SameLine();

										if(!ceasePointer) {
											ImGui::Text("X:%f, Y:%f, W:%f, H:%f", i, recs[i].shape.x,
													recs[i].shape.y,
													recs[i].shape.width,
													recs[i].shape.height);
											ImGui::SameLine();
											ImGui::ColorButton(TextFormat("Rectangle %d", i), RayColor2ImVec(recs[i].color), ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(20,20));
										}

									}

									ImGui::EndTabItem();
								}

								if(ImGui::BeginTabItem("Etc.")) {
									
									ImGui::Text("Cam Zoom: %.2f", cam.zoom);
									ImGui::Text("Zoom Transparency: %d", getZoomTrans(&cam.zoom, &gridSpacing, &factor));
									ImGui::SliderFloat("Zoom Factor", &factor, 0.01, 10);				
			
									#if RESOURCE_USED
									rlImGuiImageSize(&imt,100,100);
									#endif

									if(ImGui::Button("Activate Tiny Mode")) {
										gridSpacing = 1;
										cam.zoom = 50.0f;
									}
									ImGui::EndTabItem();
								}
							ImGui::EndTabBar();
						}
					ImGui::End();
				}

				if(ImDemoOn) {
					bool thing = true;
					ImGui::ShowDemoWindow(&thing);
				}

				if(cfg.tooltip && getHoveredRec(recs, &cam, true) != -1) {
					int index = getHoveredRec(recs, &cam, true);
					ImGui::BeginTooltip();
						ImGui::Text("X: %s", trunFloat(recs[index].shape.x).c_str());
						ImGui::SameLine();
						ImGui::Text("Y: %s", trunFloat(recs[index].shape.y).c_str());
						ImGui::Text("W: %s", trunFloat(recs[index].shape.width).c_str());
						ImGui::SameLine();
						ImGui::Text("H: %s", trunFloat(recs[index].shape.height).c_str());

						ImGui::Text("RGBA: %d, %d, %d, %d",
								recs[index].color.r,
								recs[index].color.g,
								recs[index].color.b,
								recs[index].color.a
								);
						ImGui::SameLine();
						ImGui::ColorButton("colorPreviewTooltip", RayColor2ImVec(recs[index].color), ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(20,20));
					ImGui::EndTooltip();
				}

				if(ImGui::BeginMainMenuBar()) {
					if(ImGui::BeginMenu("File")) {
						if(ImGui::MenuItem("Save to...")) {
							saveToFile(recs, &resultMessage, SaveFilePopup(fp), &current, &changesMade);
						}
						
						if(ImGui::MenuItem("Load")) {
							loadFromFile(&recs, &resultMessage, LoadFilePopup(fp), &current);
						}

						if(ImGui::BeginMenu("Export as")) {

							if(ImGui::MenuItem("C++ Vector")) {
								exportToHeader(recs, &resultMessage, &current, SaveFilePopup(ex,true));
							}

							if(ImGui::MenuItem("C Array (Plus Length Constant(UNTESTED AF))")) {
								exportToHeader(recs, &resultMessage, &current, SaveFilePopup(ex,true), EXPORT_C_ARRAY);
							}

							ImGui::EndMenu();
						}

						ImGui::Separator();

						if(ImGui::BeginMenu("Help")) {
							
							if(ImGui::MenuItem("Keybind Help")) {
								tinyfd_messageBox(
									"Keybind Help",
									"WASD: Move Camera (Left shift to move fast)\nLeft Click(& Drag): Create Rectangle\nRight Click: Delete Hovered Rectangle\nRight Shift: Highlight?? all rectangles w/ index\nC: Reset Zoom\nB: Toggle Info Menu\nR: Undo\nU: Redo",
									"ok",
									"info",
									1		
								);
							}

						if(ImGui::MenuItem("About RecAsm")) {
							tinyfd_messageBox(
								"Rectangle Assembler dev",
								"made by RadsammyT in raylib\nLibraries:\n\tTinyFileDialogs (By Guillarme Vareille) \n\traylib-cpp, a raylib c++ wrapper (By RobLoach)\n\tImGui (by ocornut)\n\trlImGui (by raylib-extras)",
								"ok",
								"info",
								1
							);
						}

							ImGui::EndMenu();
						}

						
						ImGui::EndMenu();
					}

					if(ImGui::BeginMenu("Color")) {
						if(ImGui::BeginTabBar("color_tabs")) {
							if(ImGui::BeginTabItem("Working Color"))	{
								if(ImGui::ColorPicker4("Color", color, 
											ImGuiColorEditFlags_AlphaBar |
											ImGuiColorEditFlags_AlphaPreviewHalf
											)) {
									workingColor =	FloatP2RayColor(color);
								
								}
								ImGui::EndTabItem();
							}

							if(ImGui::BeginTabItem("Overlap Color"))	{
								if(ImGui::ColorPicker4("OColor", ImOverlapColor,
											ImGuiColorEditFlags_AlphaBar |
											ImGuiColorEditFlags_AlphaPreviewHalf)) {
											//overlapColor = Color {
												//ftouc(ImOverlapColor[0] * 255),
												//ftouc(ImOverlapColor[1] * 255),
												//ftouc(ImOverlapColor[2] * 255),
												//ftouc(ImOverlapColor[3] * 255),
											//};
											overlapColor = FloatP2RayColor(ImOverlapColor); 
								}
								ImGui::EndTabItem();
							}

							if(ImGui::BeginTabItem("Rec List Color")) {
								if(ImGui::ColorPicker4("RLColor", ImRecListColor,
											ImGuiColorEditFlags_AlphaBar|
											ImGuiColorEditFlags_AlphaPreviewHalf)) {
										recListColor = FloatP2RayColor(ImRecListColor);
								}
								ImGui::EndTabItem();
							}
							ImGui::EndTabBar();					
						}
					
						ImGui::EndMenu();
					}
					
					//ImGui::ColorButton("MyColor##3c", *(ImVec4*)&color, misc_flags | (no_border ? ImGuiColorEditFlags_NoBorder : 0), ImVec2(80, 80));
					ImGui::ColorButton("\0", ImVec4(color[0], color[1], color[2], color[3]), ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(20,25));
					
					
					if(ImGui::BeginMenu("Settings")) {

						ImGui::Checkbox("Info Menu", &InfoOn);
						
						if(ImGui::BeginTabBar("settings_tab")) {
							
							if(ImGui::BeginTabItem("Grid")) {
								ImGui::DragInt("Grid Space", &gridSpacing, 1.0f, 0, INT_MAX);
								ImGui::DragInt("Mouse Jump", &mouseJump, 1.0f, 0, INT_MAX);
								if(gridOffsetSlider) {
									ImGui::SliderFloat("Grid Offset X", &cfg.gridOffset.x, 0, gridSpacing);
									ImGui::SliderFloat("Grid Offset Y", &cfg.gridOffset.y, 0, gridSpacing);
								} else {
									ImGui::DragFloat("Grid Offset X", &cfg.gridOffset.x, gridOffsetDI, 0, gridSpacing);
									ImGui::DragFloat("Grid Offset Y", &cfg.gridOffset.y, gridOffsetDI, 0, gridSpacing);
								}
								
								ImGui::Checkbox("Grid Outline", &cfg.gridLine);
								ImGui::EndTabItem();
							}
							if(ImGui::BeginTabItem("Tools")) {
								if(ImGui::Combo("Middle Click Tool", &cfg.currentMCT, 
											"None\0"
											"Copy Color\0"
											"Pan Camera\0"
											)) {
									
								}
								
								if(ImGui::Checkbox("Highlight Overlapping Rectangles", &cfg.overlapHighlight)) {
									if(!cfg.overlapHighlight) {
										overlap.clear();
									} else {
										overlap = getCollidingRectangles(recs);
									}
								}
								ImGui::Checkbox("Rectangle Tooltip", &cfg.tooltip);
								ImGui::EndTabItem();
							}

							if(ImGui::BeginTabItem("Serialization")) {
								ImGui::Checkbox("Save Grid Offset", &cfg.config[0]);
								ImGui::Checkbox("Save Grid Space", &cfg.config[1]);
								ImGui::Checkbox("Save Grid Outline", &cfg.config[2]);
								ImGui::Checkbox("Save Rectangle Toolip", &cfg.config[3]);
								ImGui::Checkbox("Save Overlap Highlight", &cfg.config[5]);
								ImGui::Checkbox("Save Middle Click Tool", &cfg.config[4]);
								ImGui::Checkbox("Save Tool Main Menu", &cfg.config[6]);
								if(ImGui::Button("Save Config to Default")) {
									saveConfig(&cfg);
								}
								ImGui::EndTabItem();
							}

							if(ImGui::BeginTabItem("Meta")) {
								ImGui::Checkbox("Grid Offset Sliders?", &gridOffsetSlider);
								ImGui::SameLine();
								HelpMarker("While DragFloat uses 0.01f as its increment and is able to"
										   " use direct input, SliderFloat direct input "
										   "is broken (at least on my machine)."
										   "\nUseful for if you commonly use direct inputs for your workflow.");
								if(gridOffsetSlider)
									ImGui::BeginDisabled();
								ImGui::SliderFloat("Grid Offset Drag Increment", &gridOffsetDI, 0.01f, 1);
								
								if(gridOffsetSlider)
									ImGui::EndDisabled();

								ImGui::Checkbox("Tool in Main Menu Bar", &cfg.toolMainMenu);
								ImGui::EndTabItem();
							}
							ImGui::EndTabBar();
							}
						ImGui::EndMenu();

					}
					if(cfg.toolMainMenu) {
						ImGui::SetNextItemWidth(125);
						ImGui::Combo("Middle Click Tool", &cfg.currentMCT, 
											"None\0"
											"Copy Color\0"
											"Pan Camera\0"
											);	
					}

					ImGui::EndMainMenuBar();
				}
			rlImGuiEnd();
        EndDrawing();
	}

	if(changesMade) {
		int res = tinyfd_messageBox("Unsaved Changes!",
									"You have unsaved changes in your assembler.\n\nSave now? (You cannot cancel the exit.)",
									"yesno",
									"warning",
									1);

		if(res) 
			saveToFile(recs, &resultMessage, SaveFilePopup(fp), &current, &changesMade);
	}

	rlImGuiShutdown();
	CloseAudioDevice();				
    return 0;
}				
				
