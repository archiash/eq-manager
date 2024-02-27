#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "string"
#include "unordered_map"
#include "Calculate.cpp"
#include "EquationManager.h"
#include "LaTexCaller.cpp"

using namespace std;

std::vector<EquationData> equations;
bool isLaTexUsable = false;

float Red = 0;
float Green = 0;
float Blue = 0;

static void DeleteEquation(int index) {
	equations.erase(equations.begin() + index);
}

class ExampleLayer : public Walnut::Layer
{
public:
	string S;
	std::shared_ptr<Walnut::Image> image;
	string resultValue;
	string resultVariable;
	int menu = 0;
	float k = 0.3;
	float R = 2.25;
	unordered_map<unsigned, std::shared_ptr<Walnut::Image>> buttonImage;

	std::shared_ptr<Walnut::Image> GetImage(string equation) {
		unsigned id = Hashing(equation);
		string filename = to_string(id) + ".png";
		if (buttonImage[id]) {
			return buttonImage[id];
		}
		else if (!CheckFile(filename)) {
			try {
				if (GenarateImage(ToLaTexFormat(equation), to_string(id)) == 2) {
					throw(2);
				}
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

	virtual void OnAttach() {
		image = make_shared<Walnut::Image>("PK.jpg");
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Dobby's Calculation");

		// Get Dobby's Calculation Window Pos and Size
		ImVec2 screen = ImGui::GetWindowViewport()->Pos;
		ImVec2 screenSize = ImGui::GetWindowViewport()->Size;

		// Draw Background
		ImDrawList* background = ImGui::GetWindowDrawList();
		background->AddImage(image->GetDescriptorSet(), screen, ImVec2(screen.x + screenSize.x, screenSize.y + screen.y));
		
		ImGui::Text("##TopPadding");

		// Set Main Color Theme of Application
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); //text black color
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.5f, 0.8f, 1.0f)); //change blue button

		// Set Column 0 : Left Margin 1% of screen 
		ImGui::Columns(3, "MyLayout", false);
		ImGui::SetColumnWidth(0, (float)screenSize.x * 0.01);
		
		// Set Column 1 : Equation List 39% of screen
		ImGui::NextColumn();
		ImGui::SetColumnWidth(1, (float)screenSize.x * 0.39);
		ImGui::Text("Equations List");

		// Create List of Equation
		for (int i = 0; i < equations.size(); i++) {
			if (isLaTexUsable) {
				auto img = GetImage(equations[i].getFormula());
				double width = img->GetWidth() / (R + 2) ;
				double height = img->GetHeight() / (R + 2);
				float offsetX = ((float)screenSize.x * 0.3 - width) / 2.0;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { offsetX, 10 });
				if (ImGui::ImageButton(img->GetDescriptorSet(), ImVec2(width, height), {0,0}, {1,1})) {
					menu = 2;
					S = equations[i].getFormula();
					variable.clear();
					vector<string> var = GetInputVariablesList(S);
					resultVariable = var[0];
					for (int i = 1; i < var.size(); i++) {
						variable[var[i]] = 0;
					}
					resultValue = "";
				}
				ImGui::PopStyleVar();
				ImGui::SameLine();
				ImGui::PushID(i);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
				if (ImGui::Button("EDIT", ImVec2(50, height + 20))) {
					menu = 3;
				}
				ImGui::PopID();
				ImGui::PopStyleColor();
			}
			else {
				if (ImGui::Button(equations[i].getFormula().c_str(), ImVec2((float)screenSize.x * 0.3, 30)))
				{
					menu = 2;
					S = equations[i].getFormula();
					variable.clear();

					vector<string> var = GetInputVariablesList(S);
					resultVariable = var[0];
					for (int i = 1; i < var.size(); i++) {
						variable[var[i]] = 0;
					}
					resultValue = "";
				}

				ImGui::SameLine();
				ImGui::PushID(i);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
				if (ImGui::Button("EDIT", ImVec2(50, 30))) {
					menu = 3;
				}
				/*if (ImGui::Button("EDIT", ImVec2(50, 30))) {
					menu = 3;
				}*/
				ImGui::PopID();
				ImGui::PopStyleColor();
			}

		}
		
		if (ImGui::Button("s = v * t", ImVec2((float)screenSize.x * 0.3, 30))) //default function 1
		{
			menu = 2;
			S = "s = v * t";
			variable.clear();

			vector<string> var = GetInputVariablesList(S);
			resultVariable = var[0];
			for (int i = 1; i < var.size(); i++) {
				variable[var[i]] = 0;
			}
			resultValue = "";
		}
		if (ImGui::Button("s = u + a * t", ImVec2((float)screenSize.x * 0.3, 30))) //default function 1
		{
			menu = 2;
			S = "s = u + a * t";
			variable.clear();

			vector<string> var = GetInputVariablesList(S);
			resultVariable = var[0];
			for (int i = 1; i < var.size(); i++) {
				variable[var[i]] = 0;
			}
			resultValue = "";
		}
		if (ImGui::Button("f = m * x + b", ImVec2((float)screenSize.x * 0.3, 30))) //default function 1
		{
			menu = 2;
			S = "f = m * x + b";
			variable.clear();

			vector<string> var = GetInputVariablesList(S);
			resultVariable = var[0];
			for (int i = 1; i < var.size(); i++) {
				variable[var[i]] = 0;
			}
			resultValue = "";
		}	

		if (ImGui::Button("+", ImVec2((float)screenSize.x * 0.3, 30)))
		{
			menu = 1;
		}			

		ImGui::NextColumn();
		ImGui::SetColumnWidth(2, (float)screenSize.x * 0.6);
		if (menu == 1) {
			ImGui::PushStyleColor(ImGuiCol_TextDisabled, IM_COL32(0, 255, 0, 255));
			ImGui::Text("Input Equation");
			ImGui::InputTextWithHint("##InputEquation", "Enter Equation", inputEquation, 255);
			ImGui::Text("Description of Equation");
			ImGui::InputText("##InputDesc", inputDescription, 255);
			if (ImGui::Button("Add")) {
				if (inputEquation[0] != '\0') {
					equations.push_back(EquationData(inputEquation, inputDescription));
					EquationManager::SaveEquations(equations);
					inputEquation[0] = '\0';
				}
			}
			ImGui::PopStyleColor();
			

		}
		else if (menu == 2) {
			ImGui::Text(("Equation: " + S).c_str());
			for (auto i = variable.begin(); i != variable.end(); i++) {
					ImGui::Text(i->first.c_str());
					ImGui::SameLine();
					ImGui::InputDouble(("##" + i->first).c_str(), &i->second);
			}
			ImGui::Text((resultVariable + " = " + resultValue).c_str());
			if (ImGui::Button("Calculate")) {
				if (S != "") resultValue = to_string(CalcualteEquation(S, variable));
			}
		}

		else if (menu == 3) {
			ImGui::PushStyleColor(ImGuiCol_TextDisabled, IM_COL32(0, 255, 0, 255));
			ImGui::Text("Edit your Equation");
			ImGui::InputTextWithHint("##InputEquation", "Enter Equation", inputEquation, 255);
			ImGui::Text("Edit your Description of Equation");
			ImGui::InputText("##InputDesc", inputDescription, 255);
			if (ImGui::Button("SAVE")) {
				if (inputEquation[0] != '\0') {
					equations.push_back(EquationData(inputEquation, inputDescription));
					EquationManager::SaveEquations(equations);
					inputEquation[0] = '\0';
				}
			}
			for (int i = 0; i < equations.size(); i++)
				if (ImGui::Button("DELETE", ImVec2(100, 30))) {
				DeleteEquation(i);
				EquationManager::SaveEquations(equations);
				}
			ImGui::PopStyleColor();
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::End();
	}
private:
	char inputEquation[255];
	unordered_map<string, double> variable;
	char inputDescription[255];
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";
	Walnut::Application* app = new Walnut::Application(spec);
	isLaTexUsable = CheckFile("LaTex\\LaTex.exe");
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}

			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("Equation")) {
			if (ImGui::MenuItem("Load"))
			{
				EquationManager::LoadEquations(equations);
			}
			if (ImGui::MenuItem("Save"))
			{
				EquationManager::SaveEquations(equations);
			}
			ImGui::EndMenu();
		}
	});
	return app;
}