#include "Client.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThreadAttribute]
void Main(array<String^>^ args) {

	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	
	RealTimeBiosemiCpp::Client form;
	Application::Run(% form);

}

