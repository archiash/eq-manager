#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "string"
#include "unordered_map"
#include "Calculate.cpp"
#include "EquationManager.h"
#include "LaTexCaller.cpp"
#include <imgui_internal.h>
#include "misc/cpp/imgui_stdlib.cpp"
#include <charconv>


using namespace std;

//Custom Input Multiline with hint
namespace ImGui
{
	bool InputTextMultilineWithHint(const char* label, const char* hint, char* buf, size_t buf_size, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
		return InputTextEx(label, hint, buf, (int)buf_size, size, flags | ImGuiInputTextFlags_Multiline, callback, user_data);
	}


	int rotation_start_index;
	void ImRotateStart()
	{
		rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
	}

	ImVec2 ImRotationCenter()
	{
		ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX); // bounds

		const auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
		for (int i = rotation_start_index; i < buf.Size; i++)
			l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

		return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2); // or use _ClipRectStack?
	}

	ImVec2 operator-(const ImVec2& l, const ImVec2& r) { return{ l.x - r.x, l.y - r.y }; }

	void ImRotateEnd(float rad, ImVec2 center = ImRotationCenter())
	{
		float s = sin(rad), c = cos(rad);
		center = ImRotate(center, s, c) - center;

		auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
		for (int i = rotation_start_index; i < buf.Size; i++)
			buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
	}

};

// List of User Equation
std::vector<EquationData> equations;

bool isLaTexUsable = false;

// Control Hint Color
float Saturation = 0;
float Colorola = 0;

// Function For Delete User Equation
static void DeleteEquation(int index) {
	equations.erase(equations.begin() + index);
}

// Use to turn on/off UI Debug Tool
bool debugEnable = false;

class DobbyLayer : public Walnut::Layer
{
	// Use to send equation between menu 
	string onWorkFormula;
	string onWorkDesc;

	string inputWarnning = "";

	//Background Image
	std::shared_ptr<Walnut::Image> backgroundImage;

	//Images
	std::shared_ptr<Walnut::Image> eqmImage;

	std::shared_ptr<Walnut::Image> victora;
	double posila = 2000;
	bool calleso = false;

	// Use to store about of calulation
	string resultValue;
	string resultVariable;
	unordered_map<string, double> variable;
	unordered_map<string, string> variableString;
	
	// Current Right Menu
	int menu = 0;

	// Current Equation to edit 
	int edit_index;

	//Use to Controll LaTex Equation Button
	float lbWidth = 0.3; // Width Size
	float lbHeight = 1.75; // Font Size Dont Effect Button Size

	// List of LaTex Equation Image
	unordered_map<unsigned, std::shared_ptr<Walnut::Image>> buttonImage;

	//Controll Columns size
	float C0 = 36;
	float C1 = 0.5;
	float C2 = 100 - C0 - C1;

	//Padding
	float PaX = 30;
	float PaY = 30;

	//Open Debug Menu
	bool setting = false;
	bool debugChildBorder = false;

	//Screen Pos and Size 
	ImVec2 screen;
	ImVec2 screenSize;

	//Variable to store input data
	char inputEquation[255];
	char inputDescription[1024];

	//Function to Get LaTex Equation Image
	std::shared_ptr<Walnut::Image> GetImage(string equation) {
		unsigned id = Hashing(equation);
		string filename = to_string(id) + ".png";
		if (buttonImage[id]) {
			return buttonImage[id];
		}
		else if (!CheckFile(filename)) {
			try {
				int result = GenarateImage(ToLaTexFormat(equation), to_string(id));
				if (result) throw(result);			
			}
			catch (int x)
			{
				isLaTexUsable = false;
				return 0;
			}
		}

		buttonImage[id] = make_shared<Walnut::Image>(filename);
		return buttonImage[id];
	}
	
	//Function to send equation to calulate menu
	void OnEquationButton(string equationFormula, string equationDesc) {
		menu = 2;
		onWorkFormula = equationFormula;
		onWorkDesc = equationDesc;
		variable.clear();
		variableString.clear();
		vector<string> var = GetInputVariablesList(onWorkFormula);
		resultVariable = var[0];
		for (int i = 1; i < var.size(); i++) {
			variable[var[i]] = 0;
			variableString[var[i]] = "";
		}
		resultValue = "";
	}

	//Function to Create LaTex Equation Button
	void LaTexEquationButton(string equationFormula, string equationDesc) {
		if (!isLaTexUsable) return;
		auto img = GetImage(equationFormula);
		double width = img->GetWidth() / (lbHeight + 2);
		double height = img->GetHeight() / (lbHeight + 2);
		float offsetX = ((float)screenSize.x * lbWidth - width) / 2.0;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { offsetX, 10 });
		bool button = ImGui::ImageButton(img->GetDescriptorSet(), ImVec2(width, height), { 0,0 }, { 1,1 });
		if (button) OnEquationButton(equationFormula, equationDesc);
		ImGui::PopStyleVar();
	}

	//Function to Draw Image of LaTex Equation
	void DrawLaTexEquation(string equationFormula) {
		if (!isLaTexUsable) return;
		auto img = GetImage(equationFormula);
		double width = img->GetWidth() * 0.6f;
		double height = img->GetHeight() * 0.6f;
		float offsetX = ((float)screenSize.x * 0.4f - width) * 0.5f;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);
		ImGui::Image(img->GetDescriptorSet(), ImVec2(width, height), ImVec2(0,0), ImVec2(1,1), ImVec4(0,0,0,1));
	}

	//Funtion to open editor menu of i-th equation 
	void OnEditEquationButton(int equationIndex) {
		menu = 3;
		edit_index = equationIndex;
		inputWarnning = "";
		strcpy(inputEquation, equations[equationIndex].getFormula().c_str());
		strcpy(inputDescription, equations[equationIndex].getDescription().c_str());
	}

public:
	virtual void OnAttach() {
		eqmImage = make_shared<Walnut::Image>("EQM.png");
	}

	virtual void OnUIRender() override
	{
		//Setting Window Pos size to Full Viewport
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(28, 29, 32)); //SC1

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); //SV1
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.f, 0.f)); //SV2
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0); //SV3
		ImGui::Begin("Dobby's Calculator", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
		
		ImVec2 cursor = ImGui::GetCursorPos();

		ImFont* largeFont = ImGui::GetIO().Fonts->Fonts[1];
		ImFont* mediumFont = ImGui::GetIO().Fonts->Fonts[2];

		ImDrawList* background = ImGui::GetWindowDrawList();
		ImDrawList* foreground = ImGui::GetForegroundDrawList();

		screen = ImGui::GetWindowViewport()->Pos;
		screenSize = ImGui::GetWindowViewport()->Size;

		ImGui::BeginHorizontal(4);

		float colWidth1 = (float)screenSize.x * 0.05f;
		background->AddRectFilled(ImVec2(screen.x, screen.y), ImVec2(screen.x + colWidth1, screen.y + screenSize.y), ImColor(255, 255, 255));
		ImVec2 eqmSize = ImVec2(eqmImage->GetWidth() * 0.8f, eqmImage->GetHeight() * 0.8f);
		foreground->AddImage(eqmImage->GetDescriptorSet(), ImVec2(screen.x + (colWidth1 - eqmSize.x) * 0.5f, screen.y + (screenSize.y - eqmSize.y) * 0.5f)
			, ImVec2(screen.x + eqmSize.x + (colWidth1 - eqmSize.x) * 0.5f, screen.y + eqmSize.y + (screenSize.y - eqmSize.y) * 0.5f));

		float colWidth2 = (float)screenSize.x * 0.25f;
		background->AddRectFilled(ImVec2(screen.x + colWidth1, screen.y), ImVec2(screen.x + colWidth1 + colWidth2, screen.y + screenSize.y), ImColor(61, 63, 69));
		ImGui::SetCursorPosX(colWidth1);
		ImGui::BeginChild(64001, ImVec2(colWidth2, screenSize.y));
		ImGui::BeginVertical(64002);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		ImGui::SetCursorPosY(15);
		for (int i = 0; i < equations.size(); i++) {
			// If can use LaTex use Image from LaTex		
			bool bottonCreate = false;
			if (isLaTexUsable) {
				auto img = GetImage(equations[i].getFormula());
				if (img != 0) {
					double width = img->GetWidth() * 0.35f;
					double height = img->GetHeight() * 0.35f;
					float offsetX = ((float)screenSize.x * 0.25f - width) / 2.0;
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { offsetX, 10 });

					if (ImGui::ImageButton(img->GetDescriptorSet(), ImVec2(width, height), {0,0}, {1,1}))
						OnEquationButton(equations[i].getFormula(), equations[i].getDescription());
					if (ImGui::IsItemHovered()) {
						foreground->AddLine(ImVec2(screen.x + ImGui::GetCursorPosX() + colWidth1 + 25, screen.y + ImGui::GetCursorPosY() - 10), ImVec2(screen.x - 25 + colWidth1 + ImGui::GetCursorPosX() + screenSize.x * 0.25f, screen.y + ImGui::GetCursorPosY() - 10), ImColor(255, 255, 255), 1);
					}
					else {
						foreground->AddLine(ImVec2(screen.x + ImGui::GetCursorPosX() + colWidth1 + 25, screen.y + ImGui::GetCursorPosY() - 10), ImVec2(screen.x - 25 + colWidth1 + ImGui::GetCursorPosX() + screenSize.x * 0.25f, screen.y + ImGui::GetCursorPosY() - 10), ImColor(255, 255, 255, 125), 1);
					}
					
					ImGui::PopStyleVar();
					bottonCreate = true;
				}
			}
			if (!bottonCreate) {
				if (ImGui::Button(equations[i].getFormula().c_str(), ImVec2((float)screenSize.x * 0.3, 30)))
					OnEquationButton(equations[i].getFormula(), equations[i].getDescription());
			}
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
		ImGui::PushFont(mediumFont);
		if (ImGui::Button("Add Equation", ImVec2((float)screenSize.x * 0.25, 0)))
		{
			inputEquation[0] = '\0';
			inputDescription[0] = '\0';
			inputWarnning = "";
			menu = 1;
		}
		if (ImGui::IsItemHovered()) {
			foreground->AddLine(ImVec2(screen.x + ImGui::GetCursorPosX() + colWidth1 + 25, screen.y + ImGui::GetCursorPosY()), ImVec2(screen.x - 25 + colWidth1 + ImGui::GetCursorPosX() + screenSize.x * 0.25f, screen.y + ImGui::GetCursorPosY()), ImColor(255, 255, 255), 1);
		}
		else {
			foreground->AddLine(ImVec2(screen.x + ImGui::GetCursorPosX() + colWidth1 + 25, screen.y + ImGui::GetCursorPosY()), ImVec2(screen.x - 25 + colWidth1 + ImGui::GetCursorPosX() + screenSize.x * 0.25f, screen.y + ImGui::GetCursorPosY()), ImColor(255, 255, 255, 125), 1);
		}
		ImGui::PopFont();

		ImGui::PopStyleColor(3);
		ImGui::EndVertical();
		ImGui::EndChild();

		float colWidth3 = (float)screenSize.x * 0.25f;

		float colWidth4 = (float)screenSize.x * 0.45f;
		ImGui::PushStyleColor(ImGuiCol_ChildBg, (ImVec4)ImColor(61, 63, 69));
		ImGui::SetCursorPos(ImVec2(colWidth1 + colWidth2 + colWidth3, 0));
		ImGui::BeginChild(64011, ImVec2(colWidth4, screenSize.y));
		if (menu == 2)
		{
			ImGui::SetCursorPosX(cursor.x + colWidth4 * 0.5f);
			ImGui::SetCursorPosY(cursor.y + screenSize.y * 0.2f);
			ImGui::BeginVertical(901);
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(255, 255, 255));
			ImGui::Text("Description");
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(255, 255, 255, 200));
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
			ImGui::InputTextMultiline("##DescShow", &onWorkDesc, ImVec2(screenSize.x * 0.2f,500), ImGuiInputTextFlags_ReadOnly);
			ImGui::PopStyleColor(3);
			ImGui::EndVertical();
		}
		ImGui::PopStyleColor();
		ImGui::EndChild();

		if(menu == 2)
		{
			int px = 40;
			int py = 30;
			ImGui::SetCursorPos(ImVec2(screenSize.x * 0.35f, screenSize.y * 0.1f));
			ImGui::PushStyleColor(ImGuiCol_ChildBg, (ImVec4)ImColor(255, 255, 255));
			ImGui::BeginChild(64012, ImVec2(screenSize.x * 0.4f, screenSize.y * 0.8f));
			DrawLaTexEquation(onWorkFormula);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + px);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + py);
			ImGui::BeginVertical(64013);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
			double x = ImGui::GetCursorScreenPos().x;
			double l = screenSize.x * 0.4f - 2 * px;
			float maxWidth = 0;
			for (auto i = variableString.begin(); i != variableString.end(); i++) {
				maxWidth = max(ImGui::CalcTextSize(i->first.c_str()).x, maxWidth);
			}
			maxWidth += 20;
			double k = (screenSize.x * 0.4f) - (2 * px) - maxWidth;
			for (auto i = variableString.begin(); i != variableString.end(); i++) {
				if (i->first.size() > 2) if (i->first[0] == 'e' && i->first[1] == ':') continue;
				
				ImGui::Text(i->first.c_str());
				ImGui::SameLine(maxWidth + px);
				ImGui::PushItemWidth(k);
				ImGui::InputText(("##" + i->first).c_str(), &i->second);
				ImGui::PopItemWidth();
			}

			ImGui::Text((resultVariable + " = " + resultValue).c_str());
			if (ImGui::Button("Calculate", ImVec2(l, 0))) {
				string result = "";
				variable = ConvertInputVariable(variableString, 0, &result);
				if (result != "Has Variable") {
					double calValue = CalcualteEquation(onWorkFormula, variable, &result);
					if (onWorkFormula != "") {
						resultValue = to_string_exact(calValue);
						if (calValue < 1 || calValue > 10) resultValue += " = " + to_scientific_form(calValue);
					}
					if (result == "Wrong Format") resultValue = "Equation Wrong Format or not Compatible";
				}
				else resultValue = "Input variable can not have variable";
			}
			ImGui::PopStyleColor();
			ImGui::EndVertical();
			ImGui::PopStyleColor();
			ImGui::EndChild();
		}


		//background->AddRectFilled(ImVec2(screen.x + colWidth1 + colWidth2 + colWidth3, screen.y), ImVec2(screen.x + colWidth1 + colWidth2 + colWidth3 + colWidth4, screen.y + screenSize.y), ImColor(61, 63, 69));

		ImGui::EndHorizontal();
		ImGui::PopStyleVar(3); //SV1,2,3
		ImGui::PopStyleColor();//SC1
		ImGui::End();

	}
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Dobby's Calculator";
	Walnut::Application* app = new Walnut::Application(spec);
	EquationManager::LoadEquations(equations);
	isLaTexUsable = CheckFile("LaTex\\LaTex.exe");
	if (isLaTexUsable) isLaTexUsable = CheckLaTex() == 0;
	app->PushLayer<DobbyLayer>();
	return app;
}