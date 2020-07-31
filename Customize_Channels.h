#pragma once
#include "Client.h"


namespace RealTimeBiosemiCpp {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;


	/// <summary>
	/// Summary for Customize_Channels
	/// </summary>
	public ref class Customize_Channels : public System::Windows::Forms::Form
	{

	public:
		Customize_Channels()
		{

			InitializeComponent();
			
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Customize_Channels()
		{
			if (components)
			{
				delete components;
			}
		}
	public protected: System::Windows::Forms::CheckedListBox^ Cust_Chan_Available;
	protected:

	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->Cust_Chan_Available = (gcnew System::Windows::Forms::CheckedListBox());
			this->SuspendLayout();
			// 
			// Cust_Chan_Available
			// 
			this->Cust_Chan_Available->AllowDrop = true;
			this->Cust_Chan_Available->CheckOnClick = true;
			this->Cust_Chan_Available->FormattingEnabled = true;
			this->Cust_Chan_Available->Items->AddRange(gcnew cli::array< System::Object^  >(72) {
				L"Fp1", L"AF7", L"AF3", L"F1", L"F3",
					L"F5", L"F7", L"FT7", L"FC5", L"FC3", L"FC1", L"C1", L"C3", L"C5", L"T7", L"TP7", L"CP5", L"CP3", L"CP1", L"P1", L"P3", L"P5",
					L"P7", L"P9", L"PO7", L"PO3", L"O1", L"Iz", L"Oz", L"POz", L"Pz", L"CPz", L"Fpz", L"Fp2", L"AF8", L"AF4", L"AFz", L"Fz", L"F2",
					L"F4", L"F6", L"F8", L"FT8", L"FC6", L"FC4", L"FC2", L"FCz", L"Cz", L"C2", L"C4", L"C6", L"T8", L"TP8", L"CP6", L"CP4", L"CP2",
					L"P2", L"P4", L"P6", L"P8", L"P10", L"PO8", L"PO4", L"O2", L"EXG1", L"EXG2", L"EXG3", L"EXG4", L"EXG5", L"EXG6", L"EXG7", L"EXG8"
			});
			this->Cust_Chan_Available->Location = System::Drawing::Point(66, 33);
			this->Cust_Chan_Available->Name = L"Cust_Chan_Available";
			this->Cust_Chan_Available->Size = System::Drawing::Size(120, 499);
			this->Cust_Chan_Available->TabIndex = 0;
			this->Cust_Chan_Available->SelectedIndexChanged += gcnew System::EventHandler(this, &Customize_Channels::Cust_Chan_Available_SelectedIndexChanged);
			// 
			// Customize_Channels
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 14);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::Lime;
			this->ClientSize = System::Drawing::Size(284, 575);
			this->Controls->Add(this->Cust_Chan_Available);
			this->Font = (gcnew System::Drawing::Font(L"Times New Roman", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->Name = L"Customize_Channels";
			this->Text = L"Customize_Channels";
			this->ResumeLayout(false);

		}
#pragma endregion

	public protected: System::Void Cust_Chan_Available_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {

		if (Cust_Chan_Available->CheckedItems->Count > 0)
		{
			
			int n_chan = Cust_Chan_Available->CheckedItems->Count;
			array<String^>^ chan_names = gcnew array<String^>(n_chan);

			for (int kk = 0; kk < Cust_Chan_Available->CheckedItems->Count; kk++)
			{

				chan_names[kk] = Convert::ToString(Cust_Chan_Available->Items[Cust_Chan_Available->CheckedIndices[kk]]);

			}

		}


	}

	};
}
