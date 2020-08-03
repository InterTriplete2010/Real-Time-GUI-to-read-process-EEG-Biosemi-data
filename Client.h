#pragma once
#include <stdio.h> 
#include <stdlib.h> 
#include <iostream>
#include <string.h> 
#include <fstream>
#include <chrono>
#include <ctime>
#include <ws2tcpip.h>
#include <msclr\marshal_cppstd.h>
#include <Math.h>
#include <numeric>
#include <vector> 
#include <complex.h>
#include <thread>
#include "Customize_Channels.h"


//#define ARMA_DONT_USE_CXX11
#define ARMA_DONT_USE_CXX11_MUTEX
#include <armadillo>
#include "sigpack.h"
//#include <fftw/fftw.h>
//#include <fftw3.h>

#include "IIR_Butterworth/IIR_Butterworth.h"

#pragma comment (lib, "ws2_32.lib")

#define PI 3.141592653589793
#define max_chan_analysis	4	//Maximum number of channels allowed 

namespace RealTimeBiosemiCpp {
	
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Threading;
	using namespace System::Windows::Forms::DataVisualization::Charting;
	//using namespace arma;	//Don't use this namespace, otherwise it creates a conflict when creating array of string in non-native language
	//using namespace std;	//Don't use this namespace, otherwise it creates a conflict when creating array of string in non-native language
	//using namespace sp;	//This namespace seems to be working fine, but I would not use it, in case it still creates a conflict down the road
	
	/// <summary>
	/// Summary for Client
	/// </summary>
	public ref class Client : public System::Windows::Forms::Form
	{
		
	//Initialize the global variables
	private: int enter_code = 13;    //Code for enter
	
	public: int size_samples;	//Total number of samples per package 
	public protected: int bytes_sample;	//Bytes/sample
	private: int port;	//Port used to connect to the server
	private: SOCKET sock; //Declare Socket object
	private: tm* day_time;	//Keep track of the time the connection is opened and closed
	private: char* buf_data;	//Array used to store the bytes read in char format
	public protected: array<String^>^ chan_names;	//Array used to save the name of the channels
	private: int sampl_freq;	//Sampling frequency
	private: static double cal_fact; //Calibration factor 
	private: int* chan_analysis;    //Array used to store the indeces of the channels used for the analysis
	private: static int chan_analysis_biosemi;	//Number of channels to be plotted
	private: int decimation_fact;
	private: int track_pos_sample;   //Track the position of the sample being read
	private: array<String^>^ temp_ref_chan_string;	//Store the names of the reference channels 
	private: int res_plot;	//Variable used to determine if the average needs to be plotted
	private: bool reset_matrices;	//Variable used to reset to zero the matrices used for filtering and for the FFT
	private: double art_rej_threshold; //Variable used for the artifact rejection threshold

	//Parameters used for the filter and for the FFT
	private: int samples_FFT;	//Number of samples for the FFT
	private: std::complex<double>** fft_output;	//2D vector used to store the FFT data
	private: double** filter_coeff_pointer;	//This 2D vector is used to have the filter coefficients as global variables. This is done so that we can use Multi-threads 
	private: int temp_filt_coeff_length;
	private: double** data_biosemi_plot_value_copy_Chans_filt;	//2D vector where to save the filtered data
	private: double** data_biosemi_plot_value_copy_Chans_filt_final;	//2D vector where to save the filtered data
	private: bool filter_stable;	//Variable used to check the stability of the filter
	private: bool filter_par_error;	//Variable used to inform the user that an error has been made in selecting the parameters of the filter

	//Variables used to read the data
	public protected: int n_chan; //Number of channels
	private: int* temp_bytes_trig;    //This array is used to store 2 bytes for the trigger line
	private: int track_bytes;	//Variable used to track the position of the bytes for each channel
	public protected: int size_package;	//Size of the package to read from Biosemi
	private: uint8_t read_bytes_trig;	//Variable used to temporarily store the trigger code
	private: uint8_t plot_read_bytes_trig;	//Variable used to temporarily store the trigger code (for plotting purpose, only)private: int plot_read_bytes_trig;	//Variable used to temporarily store the trigger code (for plotting purpose, only)
	private: int* read_bytes;	//Array used to read the 4 bytes that are used to encode each sample;
	private: bool connected_true;	//Variable used to check if the connection is open
	private: String^ temp_trig_string;	//String where to temprarily save the trigger code val
	private: int* trig_code_val;	//Array used to store the trigger codes chosen by the user
	private: int track_trig_position;    //Keep track of the position of the trigger inside the package of data 
	private: bool trig_detected;	//Variable used to flag if a trigger has been detected
	private: int temp_size_samples;  //This is the number of samples to be read from each package of data. It will be affected by the position of the trigger
	private: int* ref_index;			//Array used to store the indeces of the reference channels
	private: int track_data_plot;	//Track the number of samples read to determine if data need to be plotted
	private: int track_av;   //Count the number of sweeps accepted
	private: int track_sweeps_rej;   //Count the number of sweeps rejected

	//Variables used to create the time and frequency domain of the plot 
	private: double* time_domain;  //Time domain for the x-axis
	private: double* freq_axis;		//Frequency domain for the x-axis
	private: static int time_w;   //Number of samples needed before refreshing the plot
	private: int refresh_time_w;   //Variable used to track the refreshing time for plotting the data
	private: int track_time_cont_mode;	//Variable used to update the time domain in the continuous mode
	private: int cycle_end;	//Variable used to make sure the correct number of samples are plotted
	private: double pol_data; //Variable used to determine the polarity of the data

	//Variables used to adjust the axis of the FFT
	private: bool FFT_axis_bool;
	private: int min_FFT_axis;
	private: int max_FFT_axis;

	private: System::Windows::Forms::DataVisualization::Charting::Chart^ Time_Domain_Plot_Chart_I;
	private: System::Windows::Forms::DataVisualization::Charting::Chart^ Time_Domain_Plot_Chart_II;
	private: System::Windows::Forms::DataVisualization::Charting::Chart^ Time_Domain_Plot_Chart_III;
	private: System::Windows::Forms::DataVisualization::Charting::Chart^ Time_Domain_Plot_Chart_IV;
	private: System::Windows::Forms::Button^ Plot_Filter_Response;
	private: System::Windows::Forms::DataVisualization::Charting::Chart^ Magnitude_Response_Filter;
	private: System::Windows::Forms::DataVisualization::Charting::Chart^ Frequency_Domain_Plot_Chart_I;
	private: System::Windows::Forms::DataVisualization::Charting::Chart^ Frequency_Domain_Plot_Chart_II;
	private: System::Windows::Forms::DataVisualization::Charting::Chart^ Frequency_Domain_Plot_Chart_III;
	private: System::Windows::Forms::DataVisualization::Charting::Chart^ Frequency_Domain_Plot_Chart_IV;

	private: System::Windows::Forms::ComboBox^ Continuous_Av_Data;
	private: System::Windows::Forms::Label^ label1;
	
	//Initialize the threads
	private: ThreadStart^ Thread_Read_C_Data_S = gcnew ThreadStart(this, &Client::Read_Continuous_Data);
	private: ThreadStart^ Thread_Read_Data_S = gcnew ThreadStart(this, &Client::Read_Data);
	private: ThreadStart^ Thread_compute_FFT_S = gcnew ThreadStart(this, &Client::compute_FFT);
	private: ThreadStart^ Thread_Customize_Channels_S = gcnew ThreadStart(this, &Client::Customize_Chan_Update);
	
	private: System::Windows::Forms::Label^ Label_Frequency_Axis;

	private: System::Windows::Forms::Label^ Label_Time_Axis;
	private: System::Windows::Forms::TextBox^ Min_Val_Freq;

	private: System::Windows::Forms::TextBox^ Max_Val_Freq;
	private: System::Windows::Forms::Label^ Min_Val_Freq_Text;
	private: System::Windows::Forms::Label^ Max_Val_Freq_Text;
	private: System::Windows::Forms::Label^ label12;

	private: System::Windows::Forms::Label^ label13;
	private: System::Windows::Forms::TextBox^ Decimated_SF;
	private: System::Windows::Forms::ComboBox^ Dec_Factor;
	private: System::Windows::Forms::TextBox^ AA_SF;
	private: System::Windows::Forms::TextBox^ Trig_Code_Detected;
	private: System::Windows::Forms::VScrollBar^ Art_Rej;
	private: System::Windows::Forms::Label^ Max_Val_Art_Rej;
	private: System::Windows::Forms::Label^ Scale_Art_Rej;

	private: System::Windows::Forms::Label^ label19;
	private: System::Windows::Forms::TextBox^ Sweeps_Rejected;
private: System::Windows::Forms::ComboBox^ Polarity_Data;
private: System::Windows::Forms::Label^ label14;
private: System::Windows::Forms::GroupBox^ groupBox4;
private: System::Windows::Forms::Label^ label20;
private: System::Windows::Forms::GroupBox^ groupBox5;
private: System::Windows::Forms::Label^ label21;


	//Object used to handle the secondary window for the customized channels
	private: Customize_Channels^ cust_chan_obj = gcnew Customize_Channels;
		   
	public:
		Client(void)
		{
			InitializeComponent();

			sampl_freq = Convert::ToInt32(SF_Data->Text);	//Sampling frequency

			Initialize_Variables();

		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Client()
		{
			if (components)
			{
				delete components;
				
			}
		}
	private: System::Windows::Forms::GroupBox^ groupBox1;
	private: System::Windows::Forms::TextBox^ Bytes_Sample;
	private: System::Windows::Forms::TextBox^ Samples_Package;
	private: System::Windows::Forms::TextBox^ Port_Host;
	private: System::Windows::Forms::TextBox^ IP_Address;
	private: System::Windows::Forms::ProgressBar^ Status_Connection_Open_Close;
public protected: System::Windows::Forms::TextBox^ Bytes_Package;

public protected: System::Windows::Forms::TextBox^ Chan_Number_Biosemi;

	private: System::Windows::Forms::Button^ Connect_Biosemi;
	private: System::Windows::Forms::Label^ label6;
	private: System::Windows::Forms::Label^ label5;
	private: System::Windows::Forms::Label^ label4;
	private: System::Windows::Forms::Label^ label3;
	private: System::Windows::Forms::Label^ label2;
	private: System::Windows::Forms::Label^ Status_Connection_Label;

	private: System::Windows::Forms::Button^ Close_Biosemi;
	private: System::Windows::Forms::Label^ label7;
	private: System::Windows::Forms::GroupBox^ groupBox2;
public protected: System::Windows::Forms::ComboBox^ Forty_Seventy_Two_Chan;

	private: System::Windows::Forms::ComboBox^ SF_Data;
	private: System::Windows::Forms::TextBox^ Trig_Code;
	private: System::Windows::Forms::Label^ label10;
	private: System::Windows::Forms::Label^ label9;
	private: System::Windows::Forms::Label^ label8;
	private: System::Windows::Forms::Label^ label11;
	private: System::Windows::Forms::TextBox^ Ref_Chan;
	private: System::Windows::Forms::GroupBox^ groupBox3;
	private: System::Windows::Forms::ComboBox^ Type_Filter;
	private: System::Windows::Forms::TextBox^ LF_Cut;
	private: System::Windows::Forms::TextBox^ Order_Filt;
	private: System::Windows::Forms::TextBox^ HF_Cut;
	private: System::Windows::Forms::Label^ label15;
private: System::Windows::Forms::Label^ Label_Order_Filt;


private: System::Windows::Forms::Label^ Label_LF_Cut;

private: System::Windows::Forms::Label^ Label_HF_Cut;



	private: System::Windows::Forms::RichTextBox^ Status_Connection;
	private: System::Windows::Forms::TextBox^ Sweeps_Averaged;
	private: System::Windows::Forms::TextBox^ Refreshing_Rate;
	private: System::Windows::Forms::TextBox^ Time_Window;
	private: System::Windows::Forms::Label^ label16;
	private: System::Windows::Forms::Label^ label17;
	private: System::Windows::Forms::Label^ label18;
public protected: System::Windows::Forms::ComboBox^ Chan_Biosemi_I;
public protected: System::Windows::Forms::ComboBox^ Chan_Biosemi_II;
public protected: System::Windows::Forms::ComboBox^ Chan_Biosemi_III;
public protected: System::Windows::Forms::ComboBox^ Chan_Biosemi_IV;

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

		//Initiliaze the global variables
		void Initialize_Variables() {
			
			size_samples = Convert::ToInt32(Samples_Package->Text);
			bytes_sample = Convert::ToInt32(Bytes_Sample->Text);
			n_chan = Convert::ToInt32(Chan_Number_Biosemi->Text);

			sampl_freq = Convert::ToInt32(SF_Data->Text);

			Continuous_Av_Data->Text = Convert::ToString(Continuous_Av_Data->Items[0]);
			Refreshing_Rate->Enabled = false;

			Label_LF_Cut->ForeColor = Color().Green;
			LF_Cut->Font->Bold;
			Label_HF_Cut->ForeColor = Color().Green;
			HF_Cut->Font->Bold;
			Label_Order_Filt->ForeColor = Color().Green;
			Order_Filt->Font->Bold;

			FFT_axis_bool = true;

			Bytes_Package->Text = Convert::ToString(size_samples * bytes_sample * n_chan);
			Type_Filter->SelectedIndex = 0;
			cal_fact = (0.031249942257997/256.0);    //Calibration factor. ~(1/32) is the resolution, while 256 is needed because the data in Biosemi are 24 bits, but here they are converted into 32 bits
			//cal_fact = 1.0 / (32.0 * 256.0);    //Calibration factor. 1/32 is the resolution, while 256 is needed because the data in Biosemi are 24 bits, but here they are converted into 32 bits
			//cal_fact = 0.0078125 / (256);	//This value is for the simulation in Matlab

			Name_chan_data();
			Initialize_Charts();
			
		}

		void InitializeComponent(void)
		{
			System::Windows::Forms::DataVisualization::Charting::ChartArea^ chartArea1 = (gcnew System::Windows::Forms::DataVisualization::Charting::ChartArea());
			System::Windows::Forms::DataVisualization::Charting::Legend^ legend1 = (gcnew System::Windows::Forms::DataVisualization::Charting::Legend());
			System::Windows::Forms::DataVisualization::Charting::Series^ series1 = (gcnew System::Windows::Forms::DataVisualization::Charting::Series());
			System::Windows::Forms::DataVisualization::Charting::ChartArea^ chartArea2 = (gcnew System::Windows::Forms::DataVisualization::Charting::ChartArea());
			System::Windows::Forms::DataVisualization::Charting::Legend^ legend2 = (gcnew System::Windows::Forms::DataVisualization::Charting::Legend());
			System::Windows::Forms::DataVisualization::Charting::Series^ series2 = (gcnew System::Windows::Forms::DataVisualization::Charting::Series());
			System::Windows::Forms::DataVisualization::Charting::ChartArea^ chartArea3 = (gcnew System::Windows::Forms::DataVisualization::Charting::ChartArea());
			System::Windows::Forms::DataVisualization::Charting::Legend^ legend3 = (gcnew System::Windows::Forms::DataVisualization::Charting::Legend());
			System::Windows::Forms::DataVisualization::Charting::Series^ series3 = (gcnew System::Windows::Forms::DataVisualization::Charting::Series());
			System::Windows::Forms::DataVisualization::Charting::ChartArea^ chartArea4 = (gcnew System::Windows::Forms::DataVisualization::Charting::ChartArea());
			System::Windows::Forms::DataVisualization::Charting::Legend^ legend4 = (gcnew System::Windows::Forms::DataVisualization::Charting::Legend());
			System::Windows::Forms::DataVisualization::Charting::Series^ series4 = (gcnew System::Windows::Forms::DataVisualization::Charting::Series());
			System::Windows::Forms::DataVisualization::Charting::ChartArea^ chartArea5 = (gcnew System::Windows::Forms::DataVisualization::Charting::ChartArea());
			System::Windows::Forms::DataVisualization::Charting::Legend^ legend5 = (gcnew System::Windows::Forms::DataVisualization::Charting::Legend());
			System::Windows::Forms::DataVisualization::Charting::Series^ series5 = (gcnew System::Windows::Forms::DataVisualization::Charting::Series());
			System::Windows::Forms::DataVisualization::Charting::ChartArea^ chartArea6 = (gcnew System::Windows::Forms::DataVisualization::Charting::ChartArea());
			System::Windows::Forms::DataVisualization::Charting::Legend^ legend6 = (gcnew System::Windows::Forms::DataVisualization::Charting::Legend());
			System::Windows::Forms::DataVisualization::Charting::Series^ series6 = (gcnew System::Windows::Forms::DataVisualization::Charting::Series());
			System::Windows::Forms::DataVisualization::Charting::ChartArea^ chartArea7 = (gcnew System::Windows::Forms::DataVisualization::Charting::ChartArea());
			System::Windows::Forms::DataVisualization::Charting::Legend^ legend7 = (gcnew System::Windows::Forms::DataVisualization::Charting::Legend());
			System::Windows::Forms::DataVisualization::Charting::Series^ series7 = (gcnew System::Windows::Forms::DataVisualization::Charting::Series());
			System::Windows::Forms::DataVisualization::Charting::ChartArea^ chartArea8 = (gcnew System::Windows::Forms::DataVisualization::Charting::ChartArea());
			System::Windows::Forms::DataVisualization::Charting::Legend^ legend8 = (gcnew System::Windows::Forms::DataVisualization::Charting::Legend());
			System::Windows::Forms::DataVisualization::Charting::Series^ series8 = (gcnew System::Windows::Forms::DataVisualization::Charting::Series());
			System::Windows::Forms::DataVisualization::Charting::ChartArea^ chartArea9 = (gcnew System::Windows::Forms::DataVisualization::Charting::ChartArea());
			System::Windows::Forms::DataVisualization::Charting::Legend^ legend9 = (gcnew System::Windows::Forms::DataVisualization::Charting::Legend());
			System::Windows::Forms::DataVisualization::Charting::Series^ series9 = (gcnew System::Windows::Forms::DataVisualization::Charting::Series());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->Status_Connection_Label = (gcnew System::Windows::Forms::Label());
			this->Close_Biosemi = (gcnew System::Windows::Forms::Button());
			this->Connect_Biosemi = (gcnew System::Windows::Forms::Button());
			this->Status_Connection_Open_Close = (gcnew System::Windows::Forms::ProgressBar());
			this->Bytes_Package = (gcnew System::Windows::Forms::TextBox());
			this->Chan_Number_Biosemi = (gcnew System::Windows::Forms::TextBox());
			this->Bytes_Sample = (gcnew System::Windows::Forms::TextBox());
			this->Samples_Package = (gcnew System::Windows::Forms::TextBox());
			this->Port_Host = (gcnew System::Windows::Forms::TextBox());
			this->IP_Address = (gcnew System::Windows::Forms::TextBox());
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			this->label13 = (gcnew System::Windows::Forms::Label());
			this->groupBox3 = (gcnew System::Windows::Forms::GroupBox());
			this->Plot_Filter_Response = (gcnew System::Windows::Forms::Button());
			this->label15 = (gcnew System::Windows::Forms::Label());
			this->Label_Order_Filt = (gcnew System::Windows::Forms::Label());
			this->Label_LF_Cut = (gcnew System::Windows::Forms::Label());
			this->Label_HF_Cut = (gcnew System::Windows::Forms::Label());
			this->Type_Filter = (gcnew System::Windows::Forms::ComboBox());
			this->LF_Cut = (gcnew System::Windows::Forms::TextBox());
			this->Order_Filt = (gcnew System::Windows::Forms::TextBox());
			this->HF_Cut = (gcnew System::Windows::Forms::TextBox());
			this->Decimated_SF = (gcnew System::Windows::Forms::TextBox());
			this->Ref_Chan = (gcnew System::Windows::Forms::TextBox());
			this->label10 = (gcnew System::Windows::Forms::Label());
			this->label9 = (gcnew System::Windows::Forms::Label());
			this->label8 = (gcnew System::Windows::Forms::Label());
			this->Trig_Code = (gcnew System::Windows::Forms::TextBox());
			this->SF_Data = (gcnew System::Windows::Forms::ComboBox());
			this->Forty_Seventy_Two_Chan = (gcnew System::Windows::Forms::ComboBox());
			this->groupBox4 = (gcnew System::Windows::Forms::GroupBox());
			this->label19 = (gcnew System::Windows::Forms::Label());
			this->label14 = (gcnew System::Windows::Forms::Label());
			this->Sweeps_Rejected = (gcnew System::Windows::Forms::TextBox());
			this->Polarity_Data = (gcnew System::Windows::Forms::ComboBox());
			this->label18 = (gcnew System::Windows::Forms::Label());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->Continuous_Av_Data = (gcnew System::Windows::Forms::ComboBox());
			this->Time_Window = (gcnew System::Windows::Forms::TextBox());
			this->label17 = (gcnew System::Windows::Forms::Label());
			this->label16 = (gcnew System::Windows::Forms::Label());
			this->Status_Connection = (gcnew System::Windows::Forms::RichTextBox());
			this->groupBox5 = (gcnew System::Windows::Forms::GroupBox());
			this->label21 = (gcnew System::Windows::Forms::Label());
			this->Trig_Code_Detected = (gcnew System::Windows::Forms::TextBox());
			this->label20 = (gcnew System::Windows::Forms::Label());
			this->Dec_Factor = (gcnew System::Windows::Forms::ComboBox());
			this->label11 = (gcnew System::Windows::Forms::Label());
			this->AA_SF = (gcnew System::Windows::Forms::TextBox());
			this->label12 = (gcnew System::Windows::Forms::Label());
			this->Sweeps_Averaged = (gcnew System::Windows::Forms::TextBox());
			this->Refreshing_Rate = (gcnew System::Windows::Forms::TextBox());
			this->Chan_Biosemi_I = (gcnew System::Windows::Forms::ComboBox());
			this->Chan_Biosemi_II = (gcnew System::Windows::Forms::ComboBox());
			this->Chan_Biosemi_III = (gcnew System::Windows::Forms::ComboBox());
			this->Chan_Biosemi_IV = (gcnew System::Windows::Forms::ComboBox());
			this->Time_Domain_Plot_Chart_I = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->Time_Domain_Plot_Chart_II = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->Time_Domain_Plot_Chart_III = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->Time_Domain_Plot_Chart_IV = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->Magnitude_Response_Filter = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->Frequency_Domain_Plot_Chart_I = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->Frequency_Domain_Plot_Chart_II = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->Frequency_Domain_Plot_Chart_III = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->Frequency_Domain_Plot_Chart_IV = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->Label_Frequency_Axis = (gcnew System::Windows::Forms::Label());
			this->Label_Time_Axis = (gcnew System::Windows::Forms::Label());
			this->Min_Val_Freq = (gcnew System::Windows::Forms::TextBox());
			this->Max_Val_Freq = (gcnew System::Windows::Forms::TextBox());
			this->Min_Val_Freq_Text = (gcnew System::Windows::Forms::Label());
			this->Max_Val_Freq_Text = (gcnew System::Windows::Forms::Label());
			this->Art_Rej = (gcnew System::Windows::Forms::VScrollBar());
			this->Max_Val_Art_Rej = (gcnew System::Windows::Forms::Label());
			this->Scale_Art_Rej = (gcnew System::Windows::Forms::Label());
			this->groupBox1->SuspendLayout();
			this->groupBox2->SuspendLayout();
			this->groupBox3->SuspendLayout();
			this->groupBox4->SuspendLayout();
			this->groupBox5->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Time_Domain_Plot_Chart_I))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Time_Domain_Plot_Chart_II))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Time_Domain_Plot_Chart_III))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Time_Domain_Plot_Chart_IV))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Magnitude_Response_Filter))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Frequency_Domain_Plot_Chart_I))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Frequency_Domain_Plot_Chart_II))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Frequency_Domain_Plot_Chart_III))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Frequency_Domain_Plot_Chart_IV))->BeginInit();
			this->SuspendLayout();
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->label7);
			this->groupBox1->Controls->Add(this->label6);
			this->groupBox1->Controls->Add(this->label5);
			this->groupBox1->Controls->Add(this->label4);
			this->groupBox1->Controls->Add(this->label3);
			this->groupBox1->Controls->Add(this->label2);
			this->groupBox1->Controls->Add(this->Status_Connection_Label);
			this->groupBox1->Controls->Add(this->Close_Biosemi);
			this->groupBox1->Controls->Add(this->Connect_Biosemi);
			this->groupBox1->Controls->Add(this->Status_Connection_Open_Close);
			this->groupBox1->Controls->Add(this->Bytes_Package);
			this->groupBox1->Controls->Add(this->Chan_Number_Biosemi);
			this->groupBox1->Controls->Add(this->Bytes_Sample);
			this->groupBox1->Controls->Add(this->Samples_Package);
			this->groupBox1->Controls->Add(this->Port_Host);
			this->groupBox1->Controls->Add(this->IP_Address);
			this->groupBox1->Font = (gcnew System::Drawing::Font(L"Times New Roman", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->groupBox1->Location = System::Drawing::Point(12, 6);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(576, 216);
			this->groupBox1->TabIndex = 0;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Parameters TCP/IP";
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->Location = System::Drawing::Point(452, 131);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(74, 14);
			this->label7->TabIndex = 15;
			this->label7->Text = L"Bytes/Package";
			// 
			// label6
			// 
			this->label6->AutoSize = true;
			this->label6->Location = System::Drawing::Point(296, 131);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(101, 14);
			this->label6->TabIndex = 14;
			this->label6->Text = L"Channels# + Trigger";
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Location = System::Drawing::Point(172, 131);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(69, 14);
			this->label5->TabIndex = 13;
			this->label5->Text = L"Bytes/Sample";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(26, 131);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(85, 14);
			this->label4->TabIndex = 12;
			this->label4->Text = L"Samples/Package";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(194, 47);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(28, 14);
			this->label3->TabIndex = 11;
			this->label3->Text = L"Port";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(42, 46);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(55, 14);
			this->label2->TabIndex = 10;
			this->label2->Text = L"IP Address";
			// 
			// Status_Connection_Label
			// 
			this->Status_Connection_Label->AutoSize = true;
			this->Status_Connection_Label->Location = System::Drawing::Point(327, 27);
			this->Status_Connection_Label->Name = L"Status_Connection_Label";
			this->Status_Connection_Label->Size = System::Drawing::Size(37, 14);
			this->Status_Connection_Label->TabIndex = 9;
			this->Status_Connection_Label->Text = L"Closed";
			// 
			// Close_Biosemi
			// 
			this->Close_Biosemi->BackColor = System::Drawing::Color::Transparent;
			this->Close_Biosemi->Enabled = false;
			this->Close_Biosemi->Font = (gcnew System::Drawing::Font(L"Times New Roman", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->Close_Biosemi->Location = System::Drawing::Point(466, 59);
			this->Close_Biosemi->Name = L"Close_Biosemi";
			this->Close_Biosemi->Size = System::Drawing::Size(75, 25);
			this->Close_Biosemi->TabIndex = 8;
			this->Close_Biosemi->Text = L"Close";
			this->Close_Biosemi->UseVisualStyleBackColor = false;
			this->Close_Biosemi->Click += gcnew System::EventHandler(this, &Client::Close_Biosemi_Click);
			// 
			// Connect_Biosemi
			// 
			this->Connect_Biosemi->BackColor = System::Drawing::Color::Transparent;
			this->Connect_Biosemi->Font = (gcnew System::Drawing::Font(L"Times New Roman", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->Connect_Biosemi->ForeColor = System::Drawing::Color::Black;
			this->Connect_Biosemi->Location = System::Drawing::Point(330, 58);
			this->Connect_Biosemi->Name = L"Connect_Biosemi";
			this->Connect_Biosemi->Size = System::Drawing::Size(75, 25);
			this->Connect_Biosemi->TabIndex = 7;
			this->Connect_Biosemi->Text = L"Connect";
			this->Connect_Biosemi->UseVisualStyleBackColor = false;
			this->Connect_Biosemi->Click += gcnew System::EventHandler(this, &Client::Connect_Biosemi_Click);
			// 
			// Status_Connection_Open_Close
			// 
			this->Status_Connection_Open_Close->Location = System::Drawing::Point(378, 20);
			this->Status_Connection_Open_Close->Name = L"Status_Connection_Open_Close";
			this->Status_Connection_Open_Close->Size = System::Drawing::Size(100, 25);
			this->Status_Connection_Open_Close->TabIndex = 6;
			// 
			// Bytes_Package
			// 
			this->Bytes_Package->Enabled = false;
			this->Bytes_Package->Location = System::Drawing::Point(441, 152);
			this->Bytes_Package->Name = L"Bytes_Package";
			this->Bytes_Package->Size = System::Drawing::Size(100, 20);
			this->Bytes_Package->TabIndex = 5;
			this->Bytes_Package->Text = L"0";
			this->Bytes_Package->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// Chan_Number_Biosemi
			// 
			this->Chan_Number_Biosemi->Enabled = false;
			this->Chan_Number_Biosemi->Location = System::Drawing::Point(297, 153);
			this->Chan_Number_Biosemi->Name = L"Chan_Number_Biosemi";
			this->Chan_Number_Biosemi->Size = System::Drawing::Size(100, 20);
			this->Chan_Number_Biosemi->TabIndex = 4;
			this->Chan_Number_Biosemi->Text = L"41";
			this->Chan_Number_Biosemi->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// Bytes_Sample
			// 
			this->Bytes_Sample->Enabled = false;
			this->Bytes_Sample->Location = System::Drawing::Point(158, 152);
			this->Bytes_Sample->Name = L"Bytes_Sample";
			this->Bytes_Sample->Size = System::Drawing::Size(100, 20);
			this->Bytes_Sample->TabIndex = 3;
			this->Bytes_Sample->Text = L"3";
			this->Bytes_Sample->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// Samples_Package
			// 
			this->Samples_Package->Enabled = false;
			this->Samples_Package->Location = System::Drawing::Point(18, 152);
			this->Samples_Package->Name = L"Samples_Package";
			this->Samples_Package->Size = System::Drawing::Size(100, 20);
			this->Samples_Package->TabIndex = 2;
			this->Samples_Package->Text = L"16";
			this->Samples_Package->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// Port_Host
			// 
			this->Port_Host->Location = System::Drawing::Point(158, 64);
			this->Port_Host->Name = L"Port_Host";
			this->Port_Host->Size = System::Drawing::Size(100, 20);
			this->Port_Host->TabIndex = 1;
			this->Port_Host->Text = L"8888";
			this->Port_Host->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// IP_Address
			// 
			this->IP_Address->Location = System::Drawing::Point(18, 63);
			this->IP_Address->Name = L"IP_Address";
			this->IP_Address->Size = System::Drawing::Size(100, 20);
			this->IP_Address->TabIndex = 0;
			this->IP_Address->Text = L"127.0.0.1";
			this->IP_Address->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// groupBox2
			// 
			this->groupBox2->Controls->Add(this->label13);
			this->groupBox2->Controls->Add(this->groupBox3);
			this->groupBox2->Controls->Add(this->Decimated_SF);
			this->groupBox2->Controls->Add(this->Ref_Chan);
			this->groupBox2->Controls->Add(this->label10);
			this->groupBox2->Controls->Add(this->label9);
			this->groupBox2->Controls->Add(this->label8);
			this->groupBox2->Controls->Add(this->Trig_Code);
			this->groupBox2->Controls->Add(this->SF_Data);
			this->groupBox2->Controls->Add(this->Forty_Seventy_Two_Chan);
			this->groupBox2->Controls->Add(this->groupBox4);
			this->groupBox2->Controls->Add(this->Status_Connection);
			this->groupBox2->Controls->Add(this->groupBox5);
			this->groupBox2->Location = System::Drawing::Point(663, 10);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(807, 247);
			this->groupBox2->TabIndex = 1;
			this->groupBox2->TabStop = false;
			this->groupBox2->Text = L"Parameters EEG Data";
			// 
			// label13
			// 
			this->label13->AutoSize = true;
			this->label13->Location = System::Drawing::Point(288, 30);
			this->label13->Name = L"label13";
			this->label13->Size = System::Drawing::Size(70, 14);
			this->label13->TabIndex = 40;
			this->label13->Text = L"Decimated SF";
			// 
			// groupBox3
			// 
			this->groupBox3->Controls->Add(this->Plot_Filter_Response);
			this->groupBox3->Controls->Add(this->label15);
			this->groupBox3->Controls->Add(this->Label_Order_Filt);
			this->groupBox3->Controls->Add(this->Label_LF_Cut);
			this->groupBox3->Controls->Add(this->Label_HF_Cut);
			this->groupBox3->Controls->Add(this->Type_Filter);
			this->groupBox3->Controls->Add(this->LF_Cut);
			this->groupBox3->Controls->Add(this->Order_Filt);
			this->groupBox3->Controls->Add(this->HF_Cut);
			this->groupBox3->Location = System::Drawing::Point(483, 20);
			this->groupBox3->Name = L"groupBox3";
			this->groupBox3->Size = System::Drawing::Size(318, 107);
			this->groupBox3->TabIndex = 21;
			this->groupBox3->TabStop = false;
			this->groupBox3->Text = L"IIR filter";
			// 
			// Plot_Filter_Response
			// 
			this->Plot_Filter_Response->Location = System::Drawing::Point(102, 77);
			this->Plot_Filter_Response->Name = L"Plot_Filter_Response";
			this->Plot_Filter_Response->Size = System::Drawing::Size(130, 25);
			this->Plot_Filter_Response->TabIndex = 21;
			this->Plot_Filter_Response->Text = L"Impulse Filter Response";
			this->Plot_Filter_Response->UseVisualStyleBackColor = true;
			this->Plot_Filter_Response->Click += gcnew System::EventHandler(this, &Client::Plot_Filter_Response_Click);
			// 
			// label15
			// 
			this->label15->AutoSize = true;
			this->label15->Location = System::Drawing::Point(247, 19);
			this->label15->Name = L"label15";
			this->label15->Size = System::Drawing::Size(31, 14);
			this->label15->TabIndex = 20;
			this->label15->Text = L"Type";
			// 
			// Label_Order_Filt
			// 
			this->Label_Order_Filt->AutoSize = true;
			this->Label_Order_Filt->Location = System::Drawing::Point(99, 19);
			this->Label_Order_Filt->Name = L"Label_Order_Filt";
			this->Label_Order_Filt->Size = System::Drawing::Size(33, 14);
			this->Label_Order_Filt->TabIndex = 19;
			this->Label_Order_Filt->Text = L"Order";
			// 
			// Label_LF_Cut
			// 
			this->Label_LF_Cut->AutoSize = true;
			this->Label_LF_Cut->Location = System::Drawing::Point(173, 20);
			this->Label_LF_Cut->Name = L"Label_LF_Cut";
			this->Label_LF_Cut->Size = System::Drawing::Size(27, 14);
			this->Label_LF_Cut->TabIndex = 18;
			this->Label_LF_Cut->Text = L"Low";
			// 
			// Label_HF_Cut
			// 
			this->Label_HF_Cut->AutoSize = true;
			this->Label_HF_Cut->Location = System::Drawing::Point(26, 19);
			this->Label_HF_Cut->Name = L"Label_HF_Cut";
			this->Label_HF_Cut->Size = System::Drawing::Size(29, 14);
			this->Label_HF_Cut->TabIndex = 17;
			this->Label_HF_Cut->Text = L"High";
			// 
			// Type_Filter
			// 
			this->Type_Filter->FormattingEnabled = true;
			this->Type_Filter->Items->AddRange(gcnew cli::array< System::Object^  >(4) { L"Band Pass", L"Band Stop", L"High Pass", L"Low Pass" });
			this->Type_Filter->Location = System::Drawing::Point(232, 40);
			this->Type_Filter->Name = L"Type_Filter";
			this->Type_Filter->Size = System::Drawing::Size(80, 22);
			this->Type_Filter->TabIndex = 10;
			this->Type_Filter->SelectedIndexChanged += gcnew System::EventHandler(this, &Client::Type_Filter_SelectedIndexChanged);
			// 
			// LF_Cut
			// 
			this->LF_Cut->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->LF_Cut->Location = System::Drawing::Point(162, 40);
			this->LF_Cut->Name = L"LF_Cut";
			this->LF_Cut->Size = System::Drawing::Size(51, 20);
			this->LF_Cut->TabIndex = 9;
			this->LF_Cut->Text = L"30";
			this->LF_Cut->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			this->LF_Cut->TextChanged += gcnew System::EventHandler(this, &Client::LF_Cut_TextChanged);
			// 
			// Order_Filt
			// 
			this->Order_Filt->Location = System::Drawing::Point(89, 40);
			this->Order_Filt->Name = L"Order_Filt";
			this->Order_Filt->Size = System::Drawing::Size(51, 20);
			this->Order_Filt->TabIndex = 8;
			this->Order_Filt->Text = L"2";
			this->Order_Filt->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			this->Order_Filt->TextChanged += gcnew System::EventHandler(this, &Client::Order_Filt_TextChanged);
			// 
			// HF_Cut
			// 
			this->HF_Cut->Location = System::Drawing::Point(15, 40);
			this->HF_Cut->Name = L"HF_Cut";
			this->HF_Cut->Size = System::Drawing::Size(51, 20);
			this->HF_Cut->TabIndex = 7;
			this->HF_Cut->Text = L"1";
			this->HF_Cut->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			this->HF_Cut->TextChanged += gcnew System::EventHandler(this, &Client::HF_Cut_TextChanged);
			// 
			// Decimated_SF
			// 
			this->Decimated_SF->Enabled = false;
			this->Decimated_SF->Location = System::Drawing::Point(295, 50);
			this->Decimated_SF->Name = L"Decimated_SF";
			this->Decimated_SF->Size = System::Drawing::Size(60, 20);
			this->Decimated_SF->TabIndex = 39;
			this->Decimated_SF->Text = L"2048";
			this->Decimated_SF->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// Ref_Chan
			// 
			this->Ref_Chan->Location = System::Drawing::Point(15, 81);
			this->Ref_Chan->Name = L"Ref_Chan";
			this->Ref_Chan->Size = System::Drawing::Size(84, 20);
			this->Ref_Chan->TabIndex = 19;
			this->Ref_Chan->Text = L"EXG1 EXG2";
			this->Ref_Chan->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			this->Ref_Chan->TextChanged += gcnew System::EventHandler(this, &Client::Ref_Chan_TextChanged);
			this->Ref_Chan->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &Client::Extract_chan_ref_Enter);
			// 
			// label10
			// 
			this->label10->AutoSize = true;
			this->label10->Location = System::Drawing::Point(29, 32);
			this->label10->Name = L"label10";
			this->label10->Size = System::Drawing::Size(49, 14);
			this->label10->TabIndex = 18;
			this->label10->Text = L"Channels";
			// 
			// label9
			// 
			this->label9->AutoSize = true;
			this->label9->Location = System::Drawing::Point(132, 30);
			this->label9->Name = L"label9";
			this->label9->Size = System::Drawing::Size(123, 14);
			this->label9->TabIndex = 17;
			this->label9->Text = L"Sampling Frequency (Hz)";
			// 
			// label8
			// 
			this->label8->AutoSize = true;
			this->label8->Location = System::Drawing::Point(402, 31);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(53, 14);
			this->label8->TabIndex = 16;
			this->label8->Text = L"Trig Code";
			// 
			// Trig_Code
			// 
			this->Trig_Code->Location = System::Drawing::Point(397, 51);
			this->Trig_Code->Name = L"Trig_Code";
			this->Trig_Code->Size = System::Drawing::Size(74, 20);
			this->Trig_Code->TabIndex = 6;
			this->Trig_Code->Text = L"1 2";
			this->Trig_Code->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// SF_Data
			// 
			this->SF_Data->FormattingEnabled = true;
			this->SF_Data->Items->AddRange(gcnew cli::array< System::Object^  >(4) { L"2048  ", L"4096  ", L"8192   ", L"16384 " });
			this->SF_Data->Location = System::Drawing::Point(155, 50);
			this->SF_Data->Name = L"SF_Data";
			this->SF_Data->Size = System::Drawing::Size(69, 22);
			this->SF_Data->TabIndex = 1;
			this->SF_Data->Text = L"2048  ";
			this->SF_Data->SelectedIndexChanged += gcnew System::EventHandler(this, &Client::SF_Data_SelectedIndexChanged);
			// 
			// Forty_Seventy_Two_Chan
			// 
			this->Forty_Seventy_Two_Chan->FormattingEnabled = true;
			this->Forty_Seventy_Two_Chan->Items->AddRange(gcnew cli::array< System::Object^  >(3) { L"40 Channels", L"72 Channels", L"Customize" });
			this->Forty_Seventy_Two_Chan->Location = System::Drawing::Point(15, 51);
			this->Forty_Seventy_Two_Chan->Name = L"Forty_Seventy_Two_Chan";
			this->Forty_Seventy_Two_Chan->Size = System::Drawing::Size(84, 22);
			this->Forty_Seventy_Two_Chan->TabIndex = 0;
			this->Forty_Seventy_Two_Chan->Text = L"40 Channels";
			this->Forty_Seventy_Two_Chan->SelectedIndexChanged += gcnew System::EventHandler(this, &Client::Forty_Seventy_Two_Chan_SelectedIndexChanged);
			// 
			// groupBox4
			// 
			this->groupBox4->BackColor = System::Drawing::Color::Aqua;
			this->groupBox4->Controls->Add(this->label19);
			this->groupBox4->Controls->Add(this->label14);
			this->groupBox4->Controls->Add(this->Sweeps_Rejected);
			this->groupBox4->Controls->Add(this->Polarity_Data);
			this->groupBox4->Controls->Add(this->label18);
			this->groupBox4->Controls->Add(this->label1);
			this->groupBox4->Controls->Add(this->Continuous_Av_Data);
			this->groupBox4->Controls->Add(this->Time_Window);
			this->groupBox4->Controls->Add(this->label17);
			this->groupBox4->Controls->Add(this->label16);
			this->groupBox4->Location = System::Drawing::Point(6, 132);
			this->groupBox4->Name = L"groupBox4";
			this->groupBox4->Size = System::Drawing::Size(471, 111);
			this->groupBox4->TabIndex = 44;
			this->groupBox4->TabStop = false;
			this->groupBox4->Text = L"Parameters display data";
			// 
			// label19
			// 
			this->label19->AutoSize = true;
			this->label19->BackColor = System::Drawing::Color::Red;
			this->label19->Location = System::Drawing::Point(17, 68);
			this->label19->Name = L"label19";
			this->label19->Size = System::Drawing::Size(81, 14);
			this->label19->TabIndex = 50;
			this->label19->Text = L"Sweeps Rejected";
			// 
			// label14
			// 
			this->label14->AutoSize = true;
			this->label14->Location = System::Drawing::Point(350, 68);
			this->label14->Name = L"label14";
			this->label14->Size = System::Drawing::Size(67, 14);
			this->label14->TabIndex = 53;
			this->label14->Text = L"Polarity data";
			// 
			// Sweeps_Rejected
			// 
			this->Sweeps_Rejected->Enabled = false;
			this->Sweeps_Rejected->Location = System::Drawing::Point(9, 87);
			this->Sweeps_Rejected->Name = L"Sweeps_Rejected";
			this->Sweeps_Rejected->Size = System::Drawing::Size(100, 20);
			this->Sweeps_Rejected->TabIndex = 49;
			this->Sweeps_Rejected->Text = L"0";
			this->Sweeps_Rejected->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// Polarity_Data
			// 
			this->Polarity_Data->FormattingEnabled = true;
			this->Polarity_Data->Items->AddRange(gcnew cli::array< System::Object^  >(2) { L"Up", L"Down" });
			this->Polarity_Data->Location = System::Drawing::Point(335, 85);
			this->Polarity_Data->Name = L"Polarity_Data";
			this->Polarity_Data->Size = System::Drawing::Size(100, 22);
			this->Polarity_Data->TabIndex = 52;
			this->Polarity_Data->Text = L"Up";
			// 
			// label18
			// 
			this->label18->AutoSize = true;
			this->label18->Location = System::Drawing::Point(183, 68);
			this->label18->Name = L"label18";
			this->label18->Size = System::Drawing::Size(97, 14);
			this->label18->TabIndex = 21;
			this->label18->Text = L"Time Window (ms)";
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(350, 23);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(70, 14);
			this->label1->TabIndex = 36;
			this->label1->Text = L"Display mode";
			// 
			// Continuous_Av_Data
			// 
			this->Continuous_Av_Data->FormattingEnabled = true;
			this->Continuous_Av_Data->Items->AddRange(gcnew cli::array< System::Object^  >(2) { L"Continuous", L"Average" });
			this->Continuous_Av_Data->Location = System::Drawing::Point(335, 40);
			this->Continuous_Av_Data->Name = L"Continuous_Av_Data";
			this->Continuous_Av_Data->Size = System::Drawing::Size(100, 22);
			this->Continuous_Av_Data->TabIndex = 35;
			this->Continuous_Av_Data->SelectedIndexChanged += gcnew System::EventHandler(this, &Client::Continuous_Av_Data_SelectedIndexChanged);
			// 
			// Time_Window
			// 
			this->Time_Window->Location = System::Drawing::Point(180, 85);
			this->Time_Window->Name = L"Time_Window";
			this->Time_Window->Size = System::Drawing::Size(100, 20);
			this->Time_Window->TabIndex = 9;
			this->Time_Window->Text = L"1000";
			this->Time_Window->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label17
			// 
			this->label17->AutoSize = true;
			this->label17->Location = System::Drawing::Point(172, 23);
			this->label17->Name = L"label17";
			this->label17->Size = System::Drawing::Size(119, 14);
			this->label17->TabIndex = 20;
			this->label17->Text = L"Refreshing rate (sweeps)";
			// 
			// label16
			// 
			this->label16->AutoSize = true;
			this->label16->Location = System::Drawing::Point(17, 23);
			this->label16->Name = L"label16";
			this->label16->Size = System::Drawing::Size(84, 14);
			this->label16->TabIndex = 19;
			this->label16->Text = L"Sweeps Accepted";
			// 
			// Status_Connection
			// 
			this->Status_Connection->Location = System::Drawing::Point(483, 154);
			this->Status_Connection->Name = L"Status_Connection";
			this->Status_Connection->Size = System::Drawing::Size(318, 64);
			this->Status_Connection->TabIndex = 2;
			this->Status_Connection->Text = L"";
			// 
			// groupBox5
			// 
			this->groupBox5->Controls->Add(this->label21);
			this->groupBox5->Controls->Add(this->Trig_Code_Detected);
			this->groupBox5->Controls->Add(this->label20);
			this->groupBox5->Controls->Add(this->Dec_Factor);
			this->groupBox5->Controls->Add(this->label11);
			this->groupBox5->Controls->Add(this->AA_SF);
			this->groupBox5->Controls->Add(this->label12);
			this->groupBox5->Location = System::Drawing::Point(6, 19);
			this->groupBox5->Name = L"groupBox5";
			this->groupBox5->Size = System::Drawing::Size(471, 108);
			this->groupBox5->TabIndex = 46;
			this->groupBox5->TabStop = false;
			this->groupBox5->Text = L"Parameters Biosemi";
			// 
			// label21
			// 
			this->label21->AutoSize = true;
			this->label21->Location = System::Drawing::Point(396, 87);
			this->label21->Name = L"label21";
			this->label21->Size = System::Drawing::Size(68, 14);
			this->label21->TabIndex = 46;
			this->label21->Text = L"Trig detected";
			// 
			// Trig_Code_Detected
			// 
			this->Trig_Code_Detected->Enabled = false;
			this->Trig_Code_Detected->Location = System::Drawing::Point(391, 62);
			this->Trig_Code_Detected->Name = L"Trig_Code_Detected";
			this->Trig_Code_Detected->Size = System::Drawing::Size(74, 20);
			this->Trig_Code_Detected->TabIndex = 43;
			this->Trig_Code_Detected->Text = L"N/A";
			this->Trig_Code_Detected->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label20
			// 
			this->label20->AutoSize = true;
			this->label20->Location = System::Drawing::Point(140, 87);
			this->label20->Name = L"label20";
			this->label20->Size = System::Drawing::Size(90, 14);
			this->label20->TabIndex = 45;
			this->label20->Text = L"Anti-aliasing (Hz)";
			// 
			// Dec_Factor
			// 
			this->Dec_Factor->FormattingEnabled = true;
			this->Dec_Factor->Items->AddRange(gcnew cli::array< System::Object^  >(3) { L"1", L"2", L"4" });
			this->Dec_Factor->Location = System::Drawing::Point(289, 60);
			this->Dec_Factor->Name = L"Dec_Factor";
			this->Dec_Factor->Size = System::Drawing::Size(60, 22);
			this->Dec_Factor->TabIndex = 41;
			this->Dec_Factor->Text = L"1";
			this->Dec_Factor->SelectedIndexChanged += gcnew System::EventHandler(this, &Client::Dec_Factor_SelectedIndexChanged);
			// 
			// label11
			// 
			this->label11->AutoSize = true;
			this->label11->Location = System::Drawing::Point(13, 87);
			this->label11->Name = L"label11";
			this->label11->Size = System::Drawing::Size(76, 14);
			this->label11->TabIndex = 20;
			this->label11->Text = L"Ref Channel(s)";
			// 
			// AA_SF
			// 
			this->AA_SF->Enabled = false;
			this->AA_SF->Location = System::Drawing::Point(149, 62);
			this->AA_SF->Name = L"AA_SF";
			this->AA_SF->Size = System::Drawing::Size(69, 20);
			this->AA_SF->TabIndex = 42;
			this->AA_SF->Text = L"417";
			this->AA_SF->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label12
			// 
			this->label12->AutoSize = true;
			this->label12->Location = System::Drawing::Point(291, 87);
			this->label12->Name = L"label12";
			this->label12->Size = System::Drawing::Size(58, 14);
			this->label12->TabIndex = 38;
			this->label12->Text = L"Dec Factor";
			// 
			// Sweeps_Averaged
			// 
			this->Sweeps_Averaged->Enabled = false;
			this->Sweeps_Averaged->Location = System::Drawing::Point(678, 182);
			this->Sweeps_Averaged->Name = L"Sweeps_Averaged";
			this->Sweeps_Averaged->Size = System::Drawing::Size(100, 20);
			this->Sweeps_Averaged->TabIndex = 7;
			this->Sweeps_Averaged->Text = L"0";
			this->Sweeps_Averaged->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// Refreshing_Rate
			// 
			this->Refreshing_Rate->Location = System::Drawing::Point(850, 182);
			this->Refreshing_Rate->Name = L"Refreshing_Rate";
			this->Refreshing_Rate->Size = System::Drawing::Size(100, 20);
			this->Refreshing_Rate->TabIndex = 8;
			this->Refreshing_Rate->Text = L"20";
			this->Refreshing_Rate->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// Chan_Biosemi_I
			// 
			this->Chan_Biosemi_I->FormattingEnabled = true;
			this->Chan_Biosemi_I->Location = System::Drawing::Point(150, 263);
			this->Chan_Biosemi_I->Name = L"Chan_Biosemi_I";
			this->Chan_Biosemi_I->Size = System::Drawing::Size(84, 22);
			this->Chan_Biosemi_I->TabIndex = 22;
			// 
			// Chan_Biosemi_II
			// 
			this->Chan_Biosemi_II->FormattingEnabled = true;
			this->Chan_Biosemi_II->Location = System::Drawing::Point(504, 263);
			this->Chan_Biosemi_II->Name = L"Chan_Biosemi_II";
			this->Chan_Biosemi_II->Size = System::Drawing::Size(84, 22);
			this->Chan_Biosemi_II->TabIndex = 23;
			// 
			// Chan_Biosemi_III
			// 
			this->Chan_Biosemi_III->FormattingEnabled = true;
			this->Chan_Biosemi_III->Location = System::Drawing::Point(855, 263);
			this->Chan_Biosemi_III->Name = L"Chan_Biosemi_III";
			this->Chan_Biosemi_III->Size = System::Drawing::Size(84, 22);
			this->Chan_Biosemi_III->TabIndex = 24;
			// 
			// Chan_Biosemi_IV
			// 
			this->Chan_Biosemi_IV->FormattingEnabled = true;
			this->Chan_Biosemi_IV->Location = System::Drawing::Point(1202, 263);
			this->Chan_Biosemi_IV->Name = L"Chan_Biosemi_IV";
			this->Chan_Biosemi_IV->Size = System::Drawing::Size(84, 22);
			this->Chan_Biosemi_IV->TabIndex = 25;
			// 
			// Time_Domain_Plot_Chart_I
			// 
			chartArea1->AxisY->Title = L"Amplitude (muV)";
			chartArea1->AxisY->TitleFont = (gcnew System::Drawing::Font(L"Times New Roman", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			chartArea1->Name = L"ChartArea1";
			this->Time_Domain_Plot_Chart_I->ChartAreas->Add(chartArea1);
			legend1->Name = L"Legend1";
			this->Time_Domain_Plot_Chart_I->Legends->Add(legend1);
			this->Time_Domain_Plot_Chart_I->Location = System::Drawing::Point(8, 291);
			this->Time_Domain_Plot_Chart_I->Name = L"Time_Domain_Plot_Chart_I";
			series1->ChartArea = L"ChartArea1";
			series1->ChartType = System::Windows::Forms::DataVisualization::Charting::SeriesChartType::FastLine;
			series1->IsVisibleInLegend = false;
			series1->Legend = L"Legend1";
			series1->Name = L"Fp1";
			this->Time_Domain_Plot_Chart_I->Series->Add(series1);
			this->Time_Domain_Plot_Chart_I->Size = System::Drawing::Size(343, 284);
			this->Time_Domain_Plot_Chart_I->TabIndex = 26;
			this->Time_Domain_Plot_Chart_I->Text = L"chart1";
			// 
			// Time_Domain_Plot_Chart_II
			// 
			chartArea2->Name = L"ChartArea1";
			this->Time_Domain_Plot_Chart_II->ChartAreas->Add(chartArea2);
			legend2->Name = L"Legend1";
			this->Time_Domain_Plot_Chart_II->Legends->Add(legend2);
			this->Time_Domain_Plot_Chart_II->Location = System::Drawing::Point(361, 291);
			this->Time_Domain_Plot_Chart_II->Name = L"Time_Domain_Plot_Chart_II";
			series2->ChartArea = L"ChartArea1";
			series2->ChartType = System::Windows::Forms::DataVisualization::Charting::SeriesChartType::FastLine;
			series2->IsVisibleInLegend = false;
			series2->Legend = L"Legend1";
			series2->Name = L"AF3";
			this->Time_Domain_Plot_Chart_II->Series->Add(series2);
			this->Time_Domain_Plot_Chart_II->Size = System::Drawing::Size(343, 284);
			this->Time_Domain_Plot_Chart_II->TabIndex = 27;
			this->Time_Domain_Plot_Chart_II->Text = L"chart1";
			// 
			// Time_Domain_Plot_Chart_III
			// 
			chartArea3->Name = L"ChartArea1";
			this->Time_Domain_Plot_Chart_III->ChartAreas->Add(chartArea3);
			legend3->Name = L"Legend1";
			this->Time_Domain_Plot_Chart_III->Legends->Add(legend3);
			this->Time_Domain_Plot_Chart_III->Location = System::Drawing::Point(721, 291);
			this->Time_Domain_Plot_Chart_III->Name = L"Time_Domain_Plot_Chart_III";
			series3->ChartArea = L"ChartArea1";
			series3->ChartType = System::Windows::Forms::DataVisualization::Charting::SeriesChartType::FastLine;
			series3->IsVisibleInLegend = false;
			series3->Legend = L"Legend1";
			series3->Name = L"F7";
			this->Time_Domain_Plot_Chart_III->Series->Add(series3);
			this->Time_Domain_Plot_Chart_III->Size = System::Drawing::Size(343, 284);
			this->Time_Domain_Plot_Chart_III->TabIndex = 28;
			this->Time_Domain_Plot_Chart_III->Text = L"chart1";
			// 
			// Time_Domain_Plot_Chart_IV
			// 
			chartArea4->Name = L"ChartArea1";
			this->Time_Domain_Plot_Chart_IV->ChartAreas->Add(chartArea4);
			legend4->Name = L"Legend1";
			this->Time_Domain_Plot_Chart_IV->Legends->Add(legend4);
			this->Time_Domain_Plot_Chart_IV->Location = System::Drawing::Point(1077, 291);
			this->Time_Domain_Plot_Chart_IV->Name = L"Time_Domain_Plot_Chart_IV";
			series4->ChartArea = L"ChartArea1";
			series4->ChartType = System::Windows::Forms::DataVisualization::Charting::SeriesChartType::FastLine;
			series4->IsVisibleInLegend = false;
			series4->Legend = L"Legend1";
			series4->Name = L"F3";
			this->Time_Domain_Plot_Chart_IV->Series->Add(series4);
			this->Time_Domain_Plot_Chart_IV->Size = System::Drawing::Size(343, 284);
			this->Time_Domain_Plot_Chart_IV->TabIndex = 29;
			this->Time_Domain_Plot_Chart_IV->Text = L"chart1";
			// 
			// Magnitude_Response_Filter
			// 
			chartArea5->AxisX->Title = L"Frequency (Hz)";
			chartArea5->AxisX->TitleFont = (gcnew System::Drawing::Font(L"Times New Roman", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			chartArea5->AxisY->Title = L"Magnitude (dB)";
			chartArea5->AxisY->TitleFont = (gcnew System::Drawing::Font(L"Times New Roman", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			chartArea5->Name = L"ChartArea1";
			this->Magnitude_Response_Filter->ChartAreas->Add(chartArea5);
			legend5->Name = L"Legend1";
			this->Magnitude_Response_Filter->Legends->Add(legend5);
			this->Magnitude_Response_Filter->Location = System::Drawing::Point(8, 291);
			this->Magnitude_Response_Filter->Name = L"Magnitude_Response_Filter";
			series5->ChartArea = L"ChartArea1";
			series5->ChartType = System::Windows::Forms::DataVisualization::Charting::SeriesChartType::Line;
			series5->Legend = L"Legend1";
			series5->Name = L"Magnitude (dB)";
			this->Magnitude_Response_Filter->Series->Add(series5);
			this->Magnitude_Response_Filter->Size = System::Drawing::Size(1412, 591);
			this->Magnitude_Response_Filter->TabIndex = 30;
			this->Magnitude_Response_Filter->Text = L"chart1";
			this->Magnitude_Response_Filter->Visible = false;
			// 
			// Frequency_Domain_Plot_Chart_I
			// 
			chartArea6->AxisY->Title = L"Power (muV^2)";
			chartArea6->AxisY->TitleFont = (gcnew System::Drawing::Font(L"Times New Roman", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			chartArea6->Name = L"ChartArea1";
			this->Frequency_Domain_Plot_Chart_I->ChartAreas->Add(chartArea6);
			legend6->Name = L"Legend1";
			this->Frequency_Domain_Plot_Chart_I->Legends->Add(legend6);
			this->Frequency_Domain_Plot_Chart_I->Location = System::Drawing::Point(8, 598);
			this->Frequency_Domain_Plot_Chart_I->Name = L"Frequency_Domain_Plot_Chart_I";
			series6->ChartArea = L"ChartArea1";
			series6->ChartType = System::Windows::Forms::DataVisualization::Charting::SeriesChartType::FastLine;
			series6->IsVisibleInLegend = false;
			series6->Legend = L"Legend1";
			series6->Name = L"Fp1";
			this->Frequency_Domain_Plot_Chart_I->Series->Add(series6);
			this->Frequency_Domain_Plot_Chart_I->Size = System::Drawing::Size(343, 284);
			this->Frequency_Domain_Plot_Chart_I->TabIndex = 31;
			this->Frequency_Domain_Plot_Chart_I->Text = L"chart1";
			// 
			// Frequency_Domain_Plot_Chart_II
			// 
			chartArea7->Name = L"ChartArea1";
			this->Frequency_Domain_Plot_Chart_II->ChartAreas->Add(chartArea7);
			legend7->Name = L"Legend1";
			this->Frequency_Domain_Plot_Chart_II->Legends->Add(legend7);
			this->Frequency_Domain_Plot_Chart_II->Location = System::Drawing::Point(361, 598);
			this->Frequency_Domain_Plot_Chart_II->Name = L"Frequency_Domain_Plot_Chart_II";
			series7->ChartArea = L"ChartArea1";
			series7->ChartType = System::Windows::Forms::DataVisualization::Charting::SeriesChartType::FastLine;
			series7->IsVisibleInLegend = false;
			series7->Legend = L"Legend1";
			series7->Name = L"Fp1";
			this->Frequency_Domain_Plot_Chart_II->Series->Add(series7);
			this->Frequency_Domain_Plot_Chart_II->Size = System::Drawing::Size(343, 284);
			this->Frequency_Domain_Plot_Chart_II->TabIndex = 32;
			this->Frequency_Domain_Plot_Chart_II->Text = L"chart1";
			// 
			// Frequency_Domain_Plot_Chart_III
			// 
			chartArea8->Name = L"ChartArea1";
			this->Frequency_Domain_Plot_Chart_III->ChartAreas->Add(chartArea8);
			legend8->Name = L"Legend1";
			this->Frequency_Domain_Plot_Chart_III->Legends->Add(legend8);
			this->Frequency_Domain_Plot_Chart_III->Location = System::Drawing::Point(721, 598);
			this->Frequency_Domain_Plot_Chart_III->Name = L"Frequency_Domain_Plot_Chart_III";
			series8->ChartArea = L"ChartArea1";
			series8->ChartType = System::Windows::Forms::DataVisualization::Charting::SeriesChartType::FastLine;
			series8->IsVisibleInLegend = false;
			series8->Legend = L"Legend1";
			series8->Name = L"Fp1";
			this->Frequency_Domain_Plot_Chart_III->Series->Add(series8);
			this->Frequency_Domain_Plot_Chart_III->Size = System::Drawing::Size(343, 284);
			this->Frequency_Domain_Plot_Chart_III->TabIndex = 33;
			this->Frequency_Domain_Plot_Chart_III->Text = L"chart1";
			// 
			// Frequency_Domain_Plot_Chart_IV
			// 
			chartArea9->Name = L"ChartArea1";
			this->Frequency_Domain_Plot_Chart_IV->ChartAreas->Add(chartArea9);
			legend9->Name = L"Legend1";
			this->Frequency_Domain_Plot_Chart_IV->Legends->Add(legend9);
			this->Frequency_Domain_Plot_Chart_IV->Location = System::Drawing::Point(1077, 598);
			this->Frequency_Domain_Plot_Chart_IV->Name = L"Frequency_Domain_Plot_Chart_IV";
			series9->ChartArea = L"ChartArea1";
			series9->ChartType = System::Windows::Forms::DataVisualization::Charting::SeriesChartType::FastLine;
			series9->IsVisibleInLegend = false;
			series9->Legend = L"Legend1";
			series9->Name = L"Fp1";
			this->Frequency_Domain_Plot_Chart_IV->Series->Add(series9);
			this->Frequency_Domain_Plot_Chart_IV->Size = System::Drawing::Size(343, 284);
			this->Frequency_Domain_Plot_Chart_IV->TabIndex = 34;
			this->Frequency_Domain_Plot_Chart_IV->Text = L"chart1";
			// 
			// Label_Frequency_Axis
			// 
			this->Label_Frequency_Axis->AutoSize = true;
			this->Label_Frequency_Axis->BackColor = System::Drawing::Color::White;
			this->Label_Frequency_Axis->Font = (gcnew System::Drawing::Font(L"Times New Roman", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->Label_Frequency_Axis->Location = System::Drawing::Point(651, 885);
			this->Label_Frequency_Axis->Name = L"Label_Frequency_Axis";
			this->Label_Frequency_Axis->Size = System::Drawing::Size(111, 19);
			this->Label_Frequency_Axis->TabIndex = 36;
			this->Label_Frequency_Axis->Text = L"Frequency (Hz)";
			this->Label_Frequency_Axis->Visible = false;
			// 
			// Label_Time_Axis
			// 
			this->Label_Time_Axis->AutoSize = true;
			this->Label_Time_Axis->BackColor = System::Drawing::Color::White;
			this->Label_Time_Axis->Font = (gcnew System::Drawing::Font(L"Times New Roman", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->Label_Time_Axis->Location = System::Drawing::Point(670, 577);
			this->Label_Time_Axis->Name = L"Label_Time_Axis";
			this->Label_Time_Axis->Size = System::Drawing::Size(76, 19);
			this->Label_Time_Axis->TabIndex = 38;
			this->Label_Time_Axis->Text = L"Time (ms)";
			this->Label_Time_Axis->Visible = false;
			// 
			// Min_Val_Freq
			// 
			this->Min_Val_Freq->Location = System::Drawing::Point(1423, 694);
			this->Min_Val_Freq->Name = L"Min_Val_Freq";
			this->Min_Val_Freq->Size = System::Drawing::Size(47, 20);
			this->Min_Val_Freq->TabIndex = 40;
			this->Min_Val_Freq->Text = L"0";
			this->Min_Val_Freq->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			this->Min_Val_Freq->Visible = false;
			this->Min_Val_Freq->TextChanged += gcnew System::EventHandler(this, &Client::Min_Val_Freq_TextChanged);
			// 
			// Max_Val_Freq
			// 
			this->Max_Val_Freq->Location = System::Drawing::Point(1423, 749);
			this->Max_Val_Freq->Name = L"Max_Val_Freq";
			this->Max_Val_Freq->Size = System::Drawing::Size(47, 20);
			this->Max_Val_Freq->TabIndex = 41;
			this->Max_Val_Freq->Text = L"1023";
			this->Max_Val_Freq->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			this->Max_Val_Freq->Visible = false;
			this->Max_Val_Freq->WordWrap = false;
			this->Max_Val_Freq->TextChanged += gcnew System::EventHandler(this, &Client::Max_Val_Freq_TextChanged);
			// 
			// Min_Val_Freq_Text
			// 
			this->Min_Val_Freq_Text->AutoSize = true;
			this->Min_Val_Freq_Text->BackColor = System::Drawing::SystemColors::ButtonHighlight;
			this->Min_Val_Freq_Text->Location = System::Drawing::Point(1431, 677);
			this->Min_Val_Freq_Text->Name = L"Min_Val_Freq_Text";
			this->Min_Val_Freq_Text->Size = System::Drawing::Size(26, 14);
			this->Min_Val_Freq_Text->TabIndex = 42;
			this->Min_Val_Freq_Text->Text = L"Min";
			this->Min_Val_Freq_Text->Visible = false;
			// 
			// Max_Val_Freq_Text
			// 
			this->Max_Val_Freq_Text->AutoSize = true;
			this->Max_Val_Freq_Text->BackColor = System::Drawing::SystemColors::ButtonHighlight;
			this->Max_Val_Freq_Text->Location = System::Drawing::Point(1433, 733);
			this->Max_Val_Freq_Text->Name = L"Max_Val_Freq_Text";
			this->Max_Val_Freq_Text->Size = System::Drawing::Size(28, 14);
			this->Max_Val_Freq_Text->TabIndex = 43;
			this->Max_Val_Freq_Text->Text = L"Max";
			this->Max_Val_Freq_Text->Visible = false;
			// 
			// Art_Rej
			// 
			this->Art_Rej->LargeChange = 1;
			this->Art_Rej->Location = System::Drawing::Point(1432, 288);
			this->Art_Rej->Maximum = 500000;
			this->Art_Rej->Name = L"Art_Rej";
			this->Art_Rej->Size = System::Drawing::Size(23, 287);
			this->Art_Rej->TabIndex = 45;
			this->Art_Rej->Value = 499970;
			this->Art_Rej->Visible = false;
			this->Art_Rej->Scroll += gcnew System::Windows::Forms::ScrollEventHandler(this, &Client::Art_Rej_Scroll);
			// 
			// Max_Val_Art_Rej
			// 
			this->Max_Val_Art_Rej->AutoSize = true;
			this->Max_Val_Art_Rej->BackColor = System::Drawing::SystemColors::Control;
			this->Max_Val_Art_Rej->Location = System::Drawing::Point(1412, 271);
			this->Max_Val_Art_Rej->Name = L"Max_Val_Art_Rej";
			this->Max_Val_Art_Rej->Size = System::Drawing::Size(19, 14);
			this->Max_Val_Art_Rej->TabIndex = 46;
			this->Max_Val_Art_Rej->Text = L"30";
			this->Max_Val_Art_Rej->Visible = false;
			// 
			// Scale_Art_Rej
			// 
			this->Scale_Art_Rej->AutoSize = true;
			this->Scale_Art_Rej->BackColor = System::Drawing::SystemColors::Control;
			this->Scale_Art_Rej->Location = System::Drawing::Point(1451, 271);
			this->Scale_Art_Rej->Name = L"Scale_Art_Rej";
			this->Scale_Art_Rej->Size = System::Drawing::Size(19, 14);
			this->Scale_Art_Rej->TabIndex = 47;
			this->Scale_Art_Rej->Text = L"uV";
			this->Scale_Art_Rej->Visible = false;
			// 
			// Client
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 14);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::Aqua;
			this->ClientSize = System::Drawing::Size(1475, 911);
			this->Controls->Add(this->Scale_Art_Rej);
			this->Controls->Add(this->Max_Val_Art_Rej);
			this->Controls->Add(this->Art_Rej);
			this->Controls->Add(this->Max_Val_Freq_Text);
			this->Controls->Add(this->Min_Val_Freq_Text);
			this->Controls->Add(this->Max_Val_Freq);
			this->Controls->Add(this->Min_Val_Freq);
			this->Controls->Add(this->Label_Time_Axis);
			this->Controls->Add(this->Label_Frequency_Axis);
			this->Controls->Add(this->Frequency_Domain_Plot_Chart_IV);
			this->Controls->Add(this->Frequency_Domain_Plot_Chart_III);
			this->Controls->Add(this->Frequency_Domain_Plot_Chart_II);
			this->Controls->Add(this->Magnitude_Response_Filter);
			this->Controls->Add(this->Frequency_Domain_Plot_Chart_I);
			this->Controls->Add(this->Time_Domain_Plot_Chart_IV);
			this->Controls->Add(this->Time_Domain_Plot_Chart_III);
			this->Controls->Add(this->Time_Domain_Plot_Chart_II);
			this->Controls->Add(this->Time_Domain_Plot_Chart_I);
			this->Controls->Add(this->Chan_Biosemi_IV);
			this->Controls->Add(this->Chan_Biosemi_III);
			this->Controls->Add(this->Chan_Biosemi_II);
			this->Controls->Add(this->Chan_Biosemi_I);
			this->Controls->Add(this->Refreshing_Rate);
			this->Controls->Add(this->Sweeps_Averaged);
			this->Controls->Add(this->groupBox2);
			this->Controls->Add(this->groupBox1);
			this->Font = (gcnew System::Drawing::Font(L"Times New Roman", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->ForeColor = System::Drawing::SystemColors::ActiveCaptionText;
			this->IsMdiContainer = true;
			this->Name = L"Client";
			this->Text = L"Client";
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			this->groupBox2->ResumeLayout(false);
			this->groupBox2->PerformLayout();
			this->groupBox3->ResumeLayout(false);
			this->groupBox3->PerformLayout();
			this->groupBox4->ResumeLayout(false);
			this->groupBox4->PerformLayout();
			this->groupBox5->ResumeLayout(false);
			this->groupBox5->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Time_Domain_Plot_Chart_I))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Time_Domain_Plot_Chart_II))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Time_Domain_Plot_Chart_III))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Time_Domain_Plot_Chart_IV))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Magnitude_Response_Filter))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Frequency_Domain_Plot_Chart_I))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Frequency_Domain_Plot_Chart_II))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Frequency_Domain_Plot_Chart_III))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->Frequency_Domain_Plot_Chart_IV))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

		//------------------------------------------------------------------------------------------------//
		//------------------------------------------------------------------------------------------------//

		//Initialize the charts
		private: void Initialize_Charts() 
		{

			Time_Domain_Plot_Chart_I->Series[0]->Points->Clear();
			Time_Domain_Plot_Chart_II->Series[0]->Points->Clear();
			Time_Domain_Plot_Chart_III->Series[0]->Points->Clear();
			Time_Domain_Plot_Chart_IV->Series[0]->Points->Clear();

			Time_Domain_Plot_Chart_I->Visible = true;
			Time_Domain_Plot_Chart_II->Visible = true;
			Time_Domain_Plot_Chart_III->Visible = true;
			Time_Domain_Plot_Chart_IV->Visible = true;

			Time_Domain_Plot_Chart_I->Series[0]->ToolTip = "Amplitude (uV) = #VALY\nTime (ms) = #VALX";   //Show the value of the data point pointed by the mouse
			Time_Domain_Plot_Chart_II->Series[0]->ToolTip = "Amplitude (uV) = #VALY\nTime (ms) = #VALX";   //Show the value of the data point pointed by the mouse
			Time_Domain_Plot_Chart_III->Series[0]->ToolTip = "Amplitude (uV) = #VALY\nTime (ms) = #VALX";   //Show the value of the data point pointed by the mouse
			Time_Domain_Plot_Chart_IV->Series[0]->ToolTip = "Amplitude (uV) = #VALY\nTime (ms) = #VALX";   //Show the value of the data point pointed by the mouse


			Frequency_Domain_Plot_Chart_I->Series[0]->Points->Clear();
			Frequency_Domain_Plot_Chart_II->Series[0]->Points->Clear();
			Frequency_Domain_Plot_Chart_III->Series[0]->Points->Clear();
			Frequency_Domain_Plot_Chart_IV->Series[0]->Points->Clear();

			Frequency_Domain_Plot_Chart_I->Series[0]->ToolTip = "Power (uV^2) = #VALY\nFrequency (Hz) = #VALX";   //Show the value of the data point pointed by the mouse
			Frequency_Domain_Plot_Chart_II->Series[0]->ToolTip = "Power (uV^2) = #VALY\nFrequency (Hz) = #VALX";   //Show the value of the data point pointed by the mouse
			Frequency_Domain_Plot_Chart_III->Series[0]->ToolTip = "Power (uV^2) = #VALY\nFrequency (Hz) = #VALX";   //Show the value of the data point pointed by the mouse
			Frequency_Domain_Plot_Chart_IV->Series[0]->ToolTip = "Power (uV^2) = #VALY\nFrequency (Hz) = #VALX";   //Show the value of the data point pointed by the mouse

			if (FFT_axis_bool)
			{
			
				Max_Val_Freq->Text = Convert::ToString((sampl_freq / 2) - 1);
			
				FFT_axis_bool = false;

			}

			Frequency_Domain_Plot_Chart_I->Visible = true;
			Frequency_Domain_Plot_Chart_II->Visible = true;
			Frequency_Domain_Plot_Chart_III->Visible = true;
			Frequency_Domain_Plot_Chart_IV->Visible = true;
			
			Magnitude_Response_Filter->Series[0]->ToolTip = "Power (dB/Hz) = #VALY\nFrequency (Hz) = #VALX";   //Show the value of the data point pointed by the mouse
			Magnitude_Response_Filter->Visible = false;
			
		}


	//Functions used to avoid cross-threading
	//Update the number of sweeps accepted
	void Update_Sweeps_Averaged()	
	{

		Sweeps_Averaged->Text = Convert::ToString(track_av);
		Trig_Code_Detected->BackColor = Color::White;
	}

	//Update the number of sweeps rejected
	void Update_Sweeps_Rejected()
	{

		Sweeps_Rejected->Text = Convert::ToString(track_sweeps_rej);
		Trig_Code_Detected->BackColor = Color::White;

	}

	//Check if data need to be plotted
	void Get_Refreshing_Rate()
	{

		Math::DivRem(track_av, Convert::ToInt32(Refreshing_Rate->Text), res_plot);
		
	}

	//Check if data need to be plotted
	void Display_Current_Trigger()
	{
		
		Trig_Code_Detected->Text = Convert::ToString(plot_read_bytes_trig);
		Trig_Code_Detected->BackColor = Color::LawnGreen;

	}
	
	//Plot the data in the time and frequency domain
	void Plot_Time_Frequency_Domain()
	{

		Time_Domain_Plot_Chart_I->Series[0]->Points->Clear();
		Time_Domain_Plot_Chart_II->Series[0]->Points->Clear();
		Time_Domain_Plot_Chart_III->Series[0]->Points->Clear();
		Time_Domain_Plot_Chart_IV->Series[0]->Points->Clear();

		Frequency_Domain_Plot_Chart_I->Series[0]->Points->Clear();
		Frequency_Domain_Plot_Chart_II->Series[0]->Points->Clear();
		Frequency_Domain_Plot_Chart_III->Series[0]->Points->Clear();
		Frequency_Domain_Plot_Chart_IV->Series[0]->Points->Clear();

		//Set the x-axis of the FFT
		if ((!Equals(Min_Val_Freq->Text, "")) & (!Equals(Max_Val_Freq->Text, "")))
		{

			min_FFT_axis = binary_search_function(Convert::ToInt32(Min_Val_Freq->Text), freq_axis, 0, (samples_FFT / 2) - 4);
			max_FFT_axis = binary_search_function(Convert::ToInt32(Max_Val_Freq->Text), freq_axis, 0, (samples_FFT / 2) - 4);

		}

		//Update the the x-axis of the time domain, if in continuous mode
		if (Continuous_Av_Data->SelectedIndex == 0)
		{

			for (int kk = 0; kk < refresh_time_w; kk++)
			{

				time_domain[kk] = ((double)kk / sampl_freq) + ((double)refresh_time_w*track_time_cont_mode)/ sampl_freq;

			}


			track_time_cont_mode++;
		}

				
		if (samples_FFT / 2 - 1 > refresh_time_w)
		{

			cycle_end = samples_FFT / 2 - 1;

		}

		else
		{

			cycle_end = refresh_time_w;

		}

		for (int kk = 0; kk < cycle_end; kk++)
		{
			
			if (kk < refresh_time_w)
			{

				for (int bb = 0; bb < chan_analysis_biosemi; bb++)
				{

					switch (bb)
					{

					case 0:

						Time_Domain_Plot_Chart_I->Series[0]->Points->AddXY(time_domain[kk], data_biosemi_plot_value_copy_Chans_filt_final[bb][kk] * pol_data);

						break;

					case 1:

						Time_Domain_Plot_Chart_II->Series[0]->Points->AddXY(time_domain[kk], data_biosemi_plot_value_copy_Chans_filt_final[bb][kk] * pol_data);

						break;

					case 2:

						Time_Domain_Plot_Chart_III->Series[0]->Points->AddXY(time_domain[kk], data_biosemi_plot_value_copy_Chans_filt_final[bb][kk] * pol_data);

						break;

					case 3:

						Time_Domain_Plot_Chart_IV->Series[0]->Points->AddXY(time_domain[kk], data_biosemi_plot_value_copy_Chans_filt_final[bb][kk] * pol_data);

						break;

					}

				}
			}

			if (kk < samples_FFT / 2 - 1)
			{

				for (int bb = 0; bb < chan_analysis_biosemi; bb++)
				{

					switch (bb)
					{

					case 0:

						Frequency_Domain_Plot_Chart_I->Series[0]->Points->AddXY(freq_axis[kk], Math::Pow(abs(fft_output[bb][kk]) / (samples_FFT / 2),2));
						
						if ((kk == 0) & (!Equals(Min_Val_Freq->Text, "")) & (!Equals(Max_Val_Freq->Text, "")))
						{

							Frequency_Domain_Plot_Chart_I->ChartAreas[0]->AxisX->Minimum = min_FFT_axis;
							Frequency_Domain_Plot_Chart_I->ChartAreas[0]->AxisX->Maximum = max_FFT_axis;

						}

						break;

					case 1:

						Frequency_Domain_Plot_Chart_II->Series[0]->Points->AddXY(freq_axis[kk], Math::Pow(abs(fft_output[bb][kk]) / (samples_FFT / 2), 2));

						if ((kk == 0) & (!Equals(Min_Val_Freq->Text, "")) & (!Equals(Max_Val_Freq->Text, "")))
						{

						Frequency_Domain_Plot_Chart_II->ChartAreas[0]->AxisX->Minimum = min_FFT_axis;
						Frequency_Domain_Plot_Chart_II->ChartAreas[0]->AxisX->Maximum = max_FFT_axis;

						}

						break;

					case 2:

						Frequency_Domain_Plot_Chart_III->Series[0]->Points->AddXY(freq_axis[kk], Math::Pow(abs(fft_output[bb][kk]) / (samples_FFT / 2), 2));

						if ((kk == 0) & (!Equals(Min_Val_Freq->Text, "")) & (!Equals(Max_Val_Freq->Text, "")))
						{

							Frequency_Domain_Plot_Chart_III->ChartAreas[0]->AxisX->Minimum = min_FFT_axis;
							Frequency_Domain_Plot_Chart_III->ChartAreas[0]->AxisX->Maximum = max_FFT_axis;

						}

						break;

					case 3:

						Frequency_Domain_Plot_Chart_IV->Series[0]->Points->AddXY(freq_axis[kk], Math::Pow(abs(fft_output[bb][kk]) / (samples_FFT / 2), 2));


						if ((kk == 0) & (!Equals(Min_Val_Freq->Text, "")) & (!Equals(Max_Val_Freq->Text, "")))
						{

							Frequency_Domain_Plot_Chart_IV->ChartAreas[0]->AxisX->Minimum = min_FFT_axis;
							Frequency_Domain_Plot_Chart_IV->ChartAreas[0]->AxisX->Maximum = max_FFT_axis;

						}

						break;

					}

				}

			}
		}
	
		/*
		std::ofstream myfile;
		myfile.open("example.txt");
		myfile << Math::Pow(abs(fft_output[0][kk]) / (samples_FFT / 2), 2) << "\n";
		myfile.close();
		*/

		Refresh();
		/*
		Frequency_Domain_Plot_Chart_I->ChartAreas[0]->RecalculateAxesScale();
		Time_Domain_Plot_Chart_II->ChartAreas[0]->RecalculateAxesScale();
		Time_Domain_Plot_Chart_III->ChartAreas[0]->RecalculateAxesScale();
		Time_Domain_Plot_Chart_IV->ChartAreas[0]->RecalculateAxesScale();
		*/

		
	}


	//Connect to the server
	private: System::Void Connect_Biosemi_Click(System::Object^ sender, System::EventArgs^ e) {		
		std::string ip = msclr::interop::marshal_as<std::string>(IP_Address->Text);	//Read the IP address
		
		if (ip.empty())
		{


			MessageBox::Show("The IP address is not in a correct format", "Error");

			return;

		}

		//Handling exceptions for the port
		try
		{

			port = Convert::ToInt32(Port_Host->Text);	//Read the port

		}

		catch (System::FormatException^ e) { MessageBox::Show("The port number is not in a correct format", "Error"); return; }

		//Handling exceptions for the Time Window
		try
		{

			if (Convert::ToInt32(Time_Window->Text) <= 0)
			{

				MessageBox::Show("The time window has a value <= 0", "Error");
				return;

			}
		}

		catch (System::FormatException^ e) { MessageBox::Show("The time window is not in a correct format", "Error"); return; }

		//Handling exceptions for the Refreshing rate
		try
		{

			if ((Convert::ToInt32(Refreshing_Rate->Text) <= 0) & (Continuous_Av_Data->SelectedIndex == 1))
			{

				MessageBox::Show("The refreshing rate has a value <= 0", "Error");
				return;

			}
		}

		catch (System::FormatException^ e) { MessageBox::Show("The refreshing rate is not in a correct format", "Error"); return; }

		//Handling exceptions for the Trig code
		if (Equals(Trig_Code->Text, ""))
		{


			MessageBox::Show("The trigger code is not in a correct format", "Error");

			return;

		}

		size_package = Convert::ToInt32(Bytes_Package->Text);	
		size_samples = Convert::ToInt32(Samples_Package -> Text);
		bytes_sample = Convert::ToInt32(Bytes_Sample->Text);

		res_plot = -1;
		
		Connect_Biosemi->Enabled = false;
		Connect_Biosemi->ForeColor = Color().Green;
		Close_Biosemi->Enabled = true;
		Close_Biosemi->ForeColor = Color().Red;

		Chan_Biosemi_I->Enabled = false;
		Chan_Biosemi_II->Enabled = false;
		Chan_Biosemi_III->Enabled = false;
		Chan_Biosemi_IV->Enabled = false;
		
		Status_Connection_Open_Close->Value = 100;
		Status_Connection_Label->Text = "Open";

		//Initiliaze winsock. Winsok is a technical specification that defines how Windows network software should access network services, especially TCP/IP
		WSADATA data_biosemi_Winsock;
		WORD word_start = MAKEWORD(2, 2);	//Version of the Win Socket (2.2)

		int start_S = WSAStartup(word_start, &data_biosemi_Winsock);	//Start the winsock. &data_biosemi_Winsock is a pointer to my data

		//Terminate the request to connect to the server, if Winsock cannot be initialized
		if (start_S != 0) {

			MessageBox::Show("Winsock cannot be initialized", "Warning");
			return;

		}
		
		//create a socket
		sock = socket(AF_INET, SOCK_STREAM, 0);

		if (sock == INVALID_SOCKET) {

			MessageBox::Show("Invalid socket", "Warning");	//The socket could not be created

			WSACleanup();
			return;

		}


		//Get the current date and time
		time_t time_now = time(NULL);

		// convert "time_now" to string form
		day_time = localtime(&time_now);
		Status_Connection->Text = "Connecting to the server..." + " - Time: " + day_time->tm_hour + ":" + day_time->tm_min + ":" + day_time->tm_sec + "\n";

		//Add the socket
		SOCKADDR_IN hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(port);
		inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);
	
		decimation_fact = Convert::ToInt32(Dec_Factor->Text);
		sampl_freq = Convert::ToInt32(SF_Data->Text)/decimation_fact;

		Forty_Seventy_Two_Chan->Enabled = false;
		SF_Data -> Enabled = false;
		Trig_Code -> Enabled = false;
		Ref_Chan -> Enabled = false;
		Refreshing_Rate -> Enabled = false;
		Time_Window->Enabled = false;
		connected_true = true;
		HF_Cut->Enabled = false;
		LF_Cut->Enabled = false;
		Order_Filt->Enabled = false;
		Type_Filter->Enabled = false;
		Dec_Factor->Enabled = false;
		Plot_Filter_Response->Enabled = false;
		Polarity_Data->Enabled = false;
		Continuous_Av_Data->Enabled = false;
		
		temp_size_samples = 0;
		trig_detected = false;
		track_trig_position = -1;
		
		Store_trig_code();

		read_bytes = new int[bytes_sample + 1];	//Biosemi transmit 3 bytes, so an addtional byte of 0x00 in the LSB position needs t be added to have a 32 bit conversion
		buf_data = new char[size_package];	//Array used to store the bytes read in char format
		
		Extract_chan_ref();
		Initialize_time_frequency_domain();
		Initialize_Charts();

		//Initialize variables for the data acquisition
		track_data_plot = 0;
		track_av = 0;
		track_sweeps_rej = 0;
		track_trig_position = -1;
		temp_size_samples = 0;
		Sweeps_Averaged->Text = Convert::ToString(0);
		Sweeps_Rejected->Text = Convert::ToString(0);
		art_rej_threshold = Convert::ToDouble(Art_Rej->Maximum) - Convert::ToDouble(Art_Rej->Value);

		//Select the polarity of the data
		if (Polarity_Data->SelectedIndex == 0)
		{

			pol_data = 1.0;

		}

		else

		{

			pol_data = -1.0;

		}

		//Checking on how many channels will be plotted. 
		chan_analysis_biosemi = 0;
		for (int kk = 0; kk < max_chan_analysis; kk++)
		{

			try
			{
				if (!Equals(Convert::ToString(Chan_Biosemi_I->Items[kk]), " "))
				{

					chan_analysis_biosemi++;

				}
			}

			catch(ArgumentOutOfRangeException^ e){}
		}
		
		if (chan_analysis_biosemi == 0)
		{

			MessageBox::Show("No channel has been selected", "Warning");

			Close_Biosemi->PerformClick();

		}

		chan_analysis = new int[chan_analysis_biosemi + temp_ref_chan_string->Length];
		
		for (int kk = 0; kk < chan_analysis_biosemi; kk++)
		{

			switch (kk)
			{

			case 0:

				chan_analysis[kk] = Chan_Biosemi_I->SelectedIndex;

				break;

			case 1:

				chan_analysis[kk] = Chan_Biosemi_II->SelectedIndex;

				break;

			case 2:

				chan_analysis[kk] = Chan_Biosemi_III->SelectedIndex;

				break;

			case 3:

				chan_analysis[kk] = Chan_Biosemi_IV->SelectedIndex;

				break;


			}

		}
		
		//Check if one or more of the channels to be analyzed is also used as a reference. If that is the case, that channel will be removed from the analysis
		for (int kk = 0; kk < chan_analysis_biosemi; kk++)
		{



		}

		//Adding the reference channels
		for (int kk = chan_analysis_biosemi; kk < chan_analysis_biosemi + temp_ref_chan_string->Length; kk++)
		{

			chan_analysis[kk] = ref_index[kk - chan_analysis_biosemi];

		}


		data_biosemi_plot_value_copy_Chans_filt = new double* [chan_analysis_biosemi];
		data_biosemi_plot_value_copy_Chans_filt_final = new double* [chan_analysis_biosemi];
		fft_output = new std::complex<double>* [chan_analysis_biosemi];

		Initialize_FFT();

		//Zero-padding for the FFT analysis, if necessary
			freq_axis = new double[samples_FFT / 2];

			for (double hh = 0; hh < samples_FFT / 2; hh++)
			{

				freq_axis[(int)hh] = hh * (sampl_freq) / samples_FFT;

			}

			for (int kk = 0; kk < chan_analysis_biosemi; kk++)
			{

				data_biosemi_plot_value_copy_Chans_filt[kk] = new double[samples_FFT]();
				data_biosemi_plot_value_copy_Chans_filt_final[kk] = new double[samples_FFT]();
				fft_output[kk] = new std::complex<double>[samples_FFT / 2]();

			}


		//Connect to the server
		int connr = connect(sock, (sockaddr*)&hint, sizeof(hint));

		if (connr == SOCKET_ERROR) {

			Close_Biosemi->PerformClick();

			//MessageBox::Show("Connection failed. ERR#: " + WSAGetLastError(),"Warning");	//Connection to the server failed

			WSACleanup();
			return;

		}
		
		//Extract the filter coefficients
		std::vector<std::vector<double> > save_filt_coeff;	//Array used to save the coefficients of the filter
		if ((!Equals(HF_Cut->Text, "")) & (!Equals(LF_Cut->Text, "")) & (!Equals(Order_Filt->Text, "")))
		{

			save_filt_coeff = Initialize_Filter(false);

		}

		else
		{

			Close_Biosemi->PerformClick();
			return;

		}

		//Save the filter coefficients into a global variable
		filter_coeff_pointer = new double* [save_filt_coeff.size()];
		temp_filt_coeff_length = save_filt_coeff[0].size();

		//Save the filter coefficients into a global variable
		for (int kk = 0; kk < save_filt_coeff.size(); kk++)
		{
			
			filter_coeff_pointer[kk] = new double[save_filt_coeff[0].size()];

		}

		for (int kk = 0; kk < save_filt_coeff.size(); kk++)
		{

			for (int ll = 0; ll < save_filt_coeff[0].size(); ll++)
			{

				filter_coeff_pointer[kk][ll] = save_filt_coeff[kk][ll];

			}
		}

		reset_matrices = false;

		Art_Rej->Visible = true;
		Max_Val_Art_Rej->Visible = true;
		Scale_Art_Rej->Visible = true;

		Label_Time_Axis->Visible = true;
		Label_Frequency_Axis->Visible = true;

		Min_Val_Freq_Text->Visible = true;
		Max_Val_Freq_Text->Visible = true;
		Min_Val_Freq->Visible = true;
		Max_Val_Freq->Visible = true;

		Status_Connection->Text += "Connection to the server established" + " - Time: " + day_time->tm_hour + ":" + day_time->tm_min + ":" + day_time->tm_sec + "\n";
		
		if (Continuous_Av_Data->SelectedIndex == 1)
		{

			//Set-up and start the thread to read the Biosemi data for the average 
			Thread^ Thread_Read_Data = gcnew Thread(Thread_Read_Data_S);
			Thread_Read_Data->Priority = ThreadPriority::Normal;
			Thread_Read_Data->Start();	//Start reading the Biosemi data

		}

		else
		{

			track_time_cont_mode = 0;

			//Set-up and start the thread to read the continuous Biosemi data 
			Thread^ Thread_Read_C_Data = gcnew Thread(Thread_Read_C_Data_S);
			Thread_Read_C_Data->Priority = ThreadPriority::Normal;
			Thread_Read_C_Data->Start();	//Start reading the Biosemi data

		}
	}

	
	//Close the connection
	private: System::Void Close_Biosemi_Click(System::Object^ sender, System::EventArgs^ e) {

	Forty_Seventy_Two_Chan->Enabled = true;
	SF_Data->Enabled = true;
	Trig_Code->Enabled = true;
	Ref_Chan->Enabled = true;
	Time_Window->Enabled = true;
	connected_true = false;

	if (Continuous_Av_Data->SelectedIndex == 1)
	{

		Refreshing_Rate->Enabled = true;

	}

	HF_Cut->Enabled = true;
	LF_Cut->Enabled = true;
	Order_Filt->Enabled = true;
	Type_Filter->Enabled = true;

	Dec_Factor->Enabled = true;
	Plot_Filter_Response->Enabled = true;
	
	Check_filter_parameters();

	switch (Type_Filter->SelectedIndex)
	{

	case 0:
		LF_Cut->Enabled = true;
		Label_LF_Cut->ForeColor = Color().Green;
		HF_Cut->Enabled = true;
		Label_HF_Cut->ForeColor = Color().Green;

		break;

	case 1:
		LF_Cut->Enabled = true;
		Label_LF_Cut->ForeColor = Color().Green;
		HF_Cut->Enabled = true;
		Label_HF_Cut->ForeColor = Color().Green;

		break;

	case 2:
		LF_Cut->Enabled = false;
		Label_LF_Cut->ForeColor = Color().Black;
		HF_Cut->Enabled = true;
		Label_HF_Cut->ForeColor = Color().Green;

		break;

	case 3:
		LF_Cut->Enabled = true;
		Label_LF_Cut->ForeColor = Color().Green;
		HF_Cut->Enabled = false;
		Label_HF_Cut->ForeColor = Color().Black;

		break;

	}

	Connect_Biosemi->Enabled = true;
	Connect_Biosemi->ForeColor = Color().Black;
	Close_Biosemi->Enabled = false;
	Close_Biosemi->ForeColor = Color().Black;

	Chan_Biosemi_I->Enabled = true;
	Chan_Biosemi_II->Enabled = true;
	Chan_Biosemi_III->Enabled = true;
	Chan_Biosemi_IV->Enabled = true;

	Polarity_Data->Enabled = true;
	Continuous_Av_Data->Enabled = true;

	Status_Connection_Open_Close->Value = 0;
	Status_Connection_Label->Text = "Closed";

	
	//Get the current date and time
	time_t time_now = time(NULL);

	// convert "time_now" to string form
	day_time = localtime(&time_now);
	Status_Connection->Text += "The connection has been closed" + " Time: " + day_time->tm_hour + ":" + day_time->tm_min + ":" + day_time->tm_sec + "\n";
	MessageBox::Show("The connection has been closed. If the connection was automatically closed, please check that the parameters are correct and that Biosemi is running", "Warning");	//Connection to the server failed
	

	closesocket(sock);
		WSACleanup();

}

	//Reads the packages of data, as they become available from Biosemi. This method is used for the continuous raw data
	   private: void Read_Continuous_Data()
	   {


		   //Initialize the matrix used to save the data
		   std::vector<std::vector<double> > data_biosemi;
		   std::vector<std::vector<double> > data_biosemi_plot_value_copy_Chans;
		   std::vector<double> data_biosemi_ref_av(refresh_time_w);	//Array used to save the average of the reference channel, if more than one channel has been selected. Otherwise, it will be the values of the selected channel
		   double temp_value;	//Variable used for the complement 2 conversion
		   double mean_chan;	//Variable used to demean the data
		   double mean_ref;		//Variable used to demean the ref channel(s)

		   //Variables used to store the bytes converted from char to double
		   double byte_1;
		   double byte_2;
		   double byte_3;

			   std::vector<double> temp_v;

			   for (int ff = 0; ff < refresh_time_w; ff++)
			   {

				   temp_v.push_back(0);

			   }

			   for (int hh = 0; hh < chan_analysis_biosemi + temp_ref_chan_string->Length; hh++)
			   {

					data_biosemi.push_back(temp_v);

					if (hh < chan_analysis_biosemi)
					{

						data_biosemi_plot_value_copy_Chans.push_back(temp_v);

					}

				}


		   //Start reading the data
		   while (connected_true)
		   {

			   size_t rec_bytes = 0;
			   size_t bytesrec = 0;

			   //Reading the package of bytes in "char" format
			   while (rec_bytes < size_package)
			   {

				   bytesrec = recv(sock, buf_data + rec_bytes, size_package, MSG_WAITALL);

				   rec_bytes += bytesrec;


			   }

			   //Terminate the thread, if the user disconnected the communication with Biosemi
			   if (!connected_true)
			   {

				   break;

			   }


				   //Read and store the data
				   for (int temp_chan = 0; temp_chan < chan_analysis_biosemi + temp_ref_chan_string->Length; temp_chan++)    //Loop through all the channels
				   {

					   track_bytes = (chan_analysis[temp_chan] * bytes_sample);
					   track_pos_sample = 0;

					   for (int temp_track_sample = 0; temp_track_sample < size_samples; temp_track_sample += decimation_fact)  //Loop through all the samples per each channel
					   {

						   //Convert each byte into double and save them into a matrix
						   if (track_pos_sample + track_data_plot < refresh_time_w)
						   {
							   
							   //Convert the bytes from char to double
							   byte_1 = (double)buf_data[track_bytes];
							   if (byte_1 < 0)
							   {

								   byte_1 += 256;

							   }

							   byte_2 = (double)buf_data[track_bytes + 1];
							   if (byte_2 < 0)
							   {

								   byte_2 += 256;

							   }

							   byte_3 = (double)buf_data[track_bytes + 2];
							   if (byte_3 < 0)
							   {

								   byte_3 += 256;

							   }

							   temp_value = byte_3 * pow(2, 24) + byte_2 * pow(2, 16) + byte_1 * pow(2, 8);

							   //Complement 2 conversion
							   if (temp_value >= pow(2, 31))
							   {

								   //This is used with Biosemi.
								   data_biosemi[temp_chan][track_pos_sample + track_data_plot] = (temp_value - pow(2, 32)) * cal_fact;

								   //This conversion is used with the Matlab simulation.
								   //data_biosemi[temp_chan][track_pos_sample + track_data_plot] = -(temp_value - pow(2, 31)) * cal_fact;


							   }

							   else
							   {

								   data_biosemi[temp_chan][track_pos_sample + track_data_plot] = temp_value * cal_fact;

							   }

						   }

						   track_pos_sample++;
						   track_bytes += bytes_sample * n_chan * decimation_fact;   //Jump to the next sample for the same channel

					   }    //Closing the "temp_track_sample" loop


				   } //Closing the temp_chan loop


				   
				   track_data_plot += (size_samples / decimation_fact);
				   rec_bytes = 0;	//Reset the number of received bytes


				   if (track_data_plot >= refresh_time_w)
				   {

					   track_data_plot = 0;

					   //Reset the reference vector 
					   std::fill(data_biosemi_ref_av.begin(), data_biosemi_ref_av.end(), 0);

					   //Making a copy of the data for further analysis
					   
						   for (int kk = chan_analysis_biosemi + temp_ref_chan_string->Length - 1; kk >= 0; kk--)
						   {

							   //Save the data of the reference channel(s)
							   if (kk > chan_analysis_biosemi - 1)
							   {

								   for (int hh = 0; hh < refresh_time_w; hh++)
								   {

									   data_biosemi_ref_av[hh] += data_biosemi[kk][hh];

								   }

							   }

							   else
							   {

								   //Make a copy of the EEG data, reference
								   for (int hh = 0; hh < refresh_time_w; hh++)
								   {

									   if (hh == 0)	//Demean the data to minimize edge effects due to the filter
									   {

										   mean_chan = std::accumulate(data_biosemi[kk].begin(), data_biosemi[kk].end(),0) / refresh_time_w;
										   
										   if (kk == chan_analysis_biosemi - 1)
										   {

											   mean_ref = std::accumulate(data_biosemi_ref_av.begin(), data_biosemi_ref_av.end(), 0) / refresh_time_w;

										   }

									   }

									   data_biosemi_plot_value_copy_Chans[kk][hh] = (((data_biosemi[kk][hh] - mean_chan) - ((data_biosemi_ref_av[hh] - mean_ref) / temp_ref_chan_string->Length)));
									  									   
								   }
							   }
						   }

						   if (reset_matrices)
						   {
							   //Resetting the values of the 2D vectors to zero for a new analysis
							   for (int kk = 0; kk < chan_analysis_biosemi; kk++)
							   {

								   memset(data_biosemi_plot_value_copy_Chans_filt_final[kk], 0, sizeof(double) * samples_FFT);
								   memset(fft_output[kk], 0, sizeof(std::complex<double>) * (samples_FFT / 2));

							   }
						   }

						   else
						   {

							   reset_matrices = true;

						   }

						   //Filter the data
						   //Filter_Data_Biosemi();
						   Filter_Data_Biosemi_Direct_Form_II_Transpose(data_biosemi_plot_value_copy_Chans);
						   
						   //Compute the FFT
						   //Set-up the thread for the FFT analysis
						   Thread^ Thread_compute_FFT = gcnew Thread(Thread_compute_FFT_S);
						   Thread_compute_FFT->Priority = ThreadPriority::Normal;
						   Thread_compute_FFT->Start();	//Start reading the Biosemi data

				   }
		   }

	   }

	   //Reads the packages of data, as they become available from Biosemi. This method is used for the average
	   private: void Read_Data()
	   {
		   
		   std::ofstream myfile;
		   
		   //Initialize the matrix used to save the data
		   std::vector<std::vector<double> > data_biosemi_pre_art;	//2D vector used to temporarily save single sweep to check if they are within the artifact rejection threshold
		   std::vector<std::vector<double> > data_biosemi;	//2D vector used to save the data that are within the artifact threshold
		   std::vector<std::vector<double> > data_biosemi_plot_value_copy_Chans;
		   std::vector<double> data_biosemi_ref_av(refresh_time_w);	//Array used to save the average of the reference channel, if more than one channel has been selected. Otherwise, it will be the values of the selected channel
		   double temp_value;	//Variable used for the complement 2 conversion
		   double mean_chan;	//Mean of each channel calculated to reduce edge effects due to filtering
		   double mean_ref;	//Mean of the ref channel(s) calculated to reduce edge effects due to filtering
		   int track_bytes_save = 0;
		   std::vector<double> save_bytes(refresh_time_w*3);

		   double byte_1;
		   double byte_2;
		   double byte_3;

			std::vector<double> temp_v;

			   for (int ff = 0; ff < refresh_time_w; ff++)
			   {

				   temp_v.push_back(0);

			   }

			   for (int hh = 0; hh < chan_analysis_biosemi + temp_ref_chan_string->Length; hh++)
			   {

				   data_biosemi_pre_art.push_back(temp_v);

				   if (hh < chan_analysis_biosemi)
				   {

					   data_biosemi.push_back(temp_v);
					   data_biosemi_plot_value_copy_Chans.push_back(temp_v);

				   }

				}

		  
		   //Start reading the data
		   while (connected_true)
		   {

			   size_t rec_bytes = 0;
			   size_t bytesrec = 0;

			   //Reading the package of bytes in "char" format
			   while (rec_bytes < size_package)
			   {

				   bytesrec = recv(sock, buf_data + rec_bytes, size_package, MSG_WAITALL);

				   rec_bytes += bytesrec;


			   }

			   //Terminate the thread, if the user disconnects the communication with Biosemi
			   if (!connected_true)
			   {

				   break;

			   }


			   //Check if a trigger is present. If it is present, then starts reading the data
			   track_bytes = (n_chan - 1) * bytes_sample;
			   for (int temp_track_sample_trig = 0; temp_track_sample_trig < size_samples; temp_track_sample_trig++)  //Loop through all the samples per each channel
			   {

				   //read_bytes_trig = (uint8_t)(buf_data[track_bytes + 1] << 8 | buf_data[track_bytes]);
				   
				   byte_2_trig = (uint8_t)(buf_data[track_bytes + 1]);
				   byte_1_trig = (uint8_t)(buf_data[track_bytes]);

				   read_bytes_trig = byte_2_trig * pow(2, 8) + byte_1_trig;

 
				   //Check if one of the selected triggers has been detected
				   for (int trig_check = 0; trig_check < temp_trig_string->Length; trig_check++)
				   {

					   if (read_bytes_trig == trig_code_val[trig_check] && track_trig_position == -1) //Check if a trigger has been detected
					   {

						   trig_detected = true;
					   plot_read_bytes_trig = read_bytes_trig;	
						   
					   if (track_trig_position == -1)
						   {

							   track_trig_position = temp_track_sample_trig;
							   temp_size_samples = track_trig_position;

						   }
					   }
				   }

				   track_bytes += bytes_sample * n_chan;   //Jump to the next sample for the same channel

			   }
			   
			   //Start storing the data, if a trigger has been detected
			   while (trig_detected)
			   {

				   if (track_data_plot > 0)
				   {

					   //Reading the package of bytes in "char" format
					   while (rec_bytes < size_package)
					   {

						   bytesrec = recv(sock, buf_data + rec_bytes, size_package, MSG_WAITALL);

						   rec_bytes += bytesrec;

					   }

				   }


				   //Read and store the data, if a trigger has been detected
				   for (int temp_chan = 0; temp_chan < chan_analysis_biosemi + temp_ref_chan_string->Length; temp_chan++)    //Loop through all the channels
				   {

					   track_bytes = (chan_analysis[temp_chan] * bytes_sample) + (bytes_sample * n_chan * track_trig_position);
					   track_pos_sample = 0;

					   for (int temp_track_sample = 0; temp_track_sample < size_samples - temp_size_samples; temp_track_sample += decimation_fact)  //Loop through all the samples per each channel
					   {

						   //Convert each byte into double and save them into a matrix, if they are within the artifact rejection threshold 
						   if (track_pos_sample + track_data_plot < refresh_time_w)
						   {
							   
							   //Convert the bytes from char to double
							   byte_1 = (double)buf_data[track_bytes];
							   if (byte_1 < 0)
							   {

								   byte_1 += 256;

							   }

							   byte_2 = (double)buf_data[track_bytes + 1];
							   if (byte_2 < 0)
							   {

								   byte_2 += 256;

							   }

							   byte_3 = (double)buf_data[track_bytes + 2];
							   if (byte_3 < 0)
							   {

								   byte_3 += 256;

							   }

							   temp_value = byte_3 * pow(2, 24) + byte_2 * pow(2, 16) + byte_1 * pow(2, 8);

							   //Complement 2 conversion
							   if (temp_value >= pow(2, 31))
							   {

								   //This is used with Biosemi
								   data_biosemi_pre_art[temp_chan][track_pos_sample + track_data_plot] = (temp_value - pow(2, 32)) * cal_fact;

								   //This conversion is used with the Matlab simulation. 
								   //data_biosemi_pre_art[temp_chan][track_pos_sample + track_data_plot] = -(temp_value - pow(2, 31)) * cal_fact;


							   }

							   else
							   {

								   data_biosemi_pre_art[temp_chan][track_pos_sample + track_data_plot] = temp_value * cal_fact;

							   }

						   }			  

						   track_pos_sample++;
						   track_bytes += bytes_sample * n_chan * decimation_fact;   //Jump to the next sample for the same channel

					   }    //Closing the "temp_track_sample" loop


				   } //Closing the temp_chan loop

				 

					 //If the decimation factor is > 1, we need to make sure that the first sample from the next package will be read from the correct position, which is not necessarily "0"
				   	if (decimation_fact > 1)
					{

						if (track_data_plot == 0)
						{
							
							Math::DivRem(track_trig_position, decimation_fact, track_trig_position);

						}
					}

					else
					{
					
						track_trig_position = 0; //Resetting this variable to zero, so the next package of data will be read from position '0' until the time window is filled
					
					}

					track_data_plot += track_pos_sample;

				   temp_size_samples = 0; //Resetting this variable to zero, so the next package of data will be read from position '0' until the time window is filled
				   rec_bytes = 0;	//Reset the number of received bytes

				   
				   if (track_data_plot >= refresh_time_w)
				   {
					   
					   track_data_plot = 0;
					   trig_detected = false;
					   track_trig_position = -1;

					   //Display the current trigger being read
					   this->Invoke(gcnew MethodInvoker(this, &Client::Display_Current_Trigger));	//Used to avoid cross-threading


					//---------------------------------------------------------------------------------------------------------------------//
					//---------------------------------------------------------------------------------------------------------------------//
					//Check if the sweep is within the artifact rejection threshold. If it is, add it to the 2D vector "data_biosemi"
					  //Reset the reference vector 
					   std::fill(data_biosemi_ref_av.begin(), data_biosemi_ref_av.end(), 0);

					   for (int kk = chan_analysis_biosemi + temp_ref_chan_string->Length - 1; kk >= 0; kk--)
					   {

						   //Save the data of the reference channel(s)
						   if (kk > chan_analysis_biosemi - 1)
						   {

							   for (int hh = 0; hh < refresh_time_w; hh++)
							   {

								   data_biosemi_ref_av[hh] += data_biosemi_pre_art[kk][hh];

							   }

						   }

						   else
						   {


							   //Make a copy of the EEG data and reference them
							   for (int hh = 0; hh < refresh_time_w; hh++)
							   {

								   if (hh == 0)
								   {

									   //mean_chan = std::accumulate(data_biosemi_pre_art[kk].begin(), data_biosemi_pre_art[kk].end(), 0) / refresh_time_w;
									   
									   //if (kk == chan_analysis_biosemi - 1)
									   //{

										  // mean_ref = std::accumulate(data_biosemi_ref_av.begin(), data_biosemi_ref_av.end(), 0) / refresh_time_w;

									   //}

								   }

								   //data_biosemi_plot_value_copy_Chans[kk][hh] = (data_biosemi_pre_art[kk][hh] - mean_chan) - ((data_biosemi_ref_av[hh] - mean_ref) / temp_ref_chan_string->Length);
								   data_biosemi_plot_value_copy_Chans[kk][hh] = (data_biosemi_pre_art[kk][hh]) - ((data_biosemi_ref_av[hh]) / temp_ref_chan_string->Length);

							   }
						   }
					   }

					   //Demean the data to reduce egde effects of the filter
					   for (int kk = 0; kk < chan_analysis_biosemi; kk++)
					   {
						   
						   mean_chan = std::accumulate(data_biosemi_plot_value_copy_Chans[kk].begin(), data_biosemi_plot_value_copy_Chans[kk].end(), 0) / refresh_time_w;
						   
						   for (int hh = 0; hh < refresh_time_w; hh++)
						   {

							   data_biosemi_plot_value_copy_Chans[kk][hh] = data_biosemi_plot_value_copy_Chans[kk][hh] - mean_chan;

						   }

					   }

					   if (reset_matrices)
					   {
						   //Resetting the values of the 2D vectors to zero for a new analysis
						   for (int kk = 0; kk < chan_analysis_biosemi; kk++)
						   {

							   memset(data_biosemi_plot_value_copy_Chans_filt_final[kk], 0, sizeof(double) * samples_FFT);

						   }

					   }

					   else
					   {

						   reset_matrices = true;

					   }

					   //Filter the data
					   Filter_Data_Biosemi_Direct_Form_II_Transpose(data_biosemi_plot_value_copy_Chans);

					   //Now check the max and min amplitude vs the art rejection val for each channel
					   bool temp_max = true;
					   for (int art_index = 0; art_index < chan_analysis_biosemi; art_index++)
					   {

						   if (check_max(data_biosemi_plot_value_copy_Chans_filt_final[art_index]) == false)
						   {

							   temp_max = false;

							   break;

						   }

					   }

					   //---------------------------------------------------------------------------------------------------------------------//
					   //---------------------------------------------------------------------------------------------------------------------//


					   if (temp_max)
					   {

						   
						   //Add the filtered sweep to "data_biosemi"
						    for (int biosemi_index = 0; biosemi_index < chan_analysis_biosemi; biosemi_index++)
						   {

							 
							   for (int samples_index = 0; samples_index < refresh_time_w; samples_index++)
							   {

								   data_biosemi[biosemi_index][samples_index] += data_biosemi_plot_value_copy_Chans_filt_final[biosemi_index][samples_index];

							   }

						   }
						   
							track_av++;	//Update the number of sweeps accepted

						   this->Invoke(gcnew MethodInvoker(this, &Client::Update_Sweeps_Averaged));	//Used to avoid cross-threading

						   //Check if the data need to be plotted
						   this->Invoke(gcnew MethodInvoker(this, &Client::Get_Refreshing_Rate));	//Used to avoid cross-threading

						   //Making a copy of the data for further analysis
						   if (res_plot == 0)
						   {

							   for (int kk = 0; kk < chan_analysis_biosemi; kk++)
							   {
 
									   
								//Copy the EEG data into "data_biosemi_plot_value_copy_Chans_filt_final" for the FFT analysis
								   for (int hh = 0; hh < refresh_time_w; hh++)
								   {

									   data_biosemi_plot_value_copy_Chans_filt_final[kk][hh] = data_biosemi[kk][hh] / track_av;

								   }

							   }

							   if (reset_matrices)
							   {
								   //Resetting the values of the 2D vectors to zero for a new analysis
								   for (int kk = 0; kk < chan_analysis_biosemi; kk++)
								   {

									    memset(fft_output[kk], 0, sizeof(std::complex<double>) * (samples_FFT / 2));

								   }
							   }

							   else
							   {

								   reset_matrices = true;

							   }

							   
							   //Compute the FFT
							   //Set-up the thread for the FFT analysis
							   Thread^ Thread_compute_FFT = gcnew Thread(Thread_compute_FFT_S);
							   Thread_compute_FFT->Priority = ThreadPriority::Normal;
							   Thread_compute_FFT->Start();	//Start reading the Biosemi data
							   							  
						   }

					   }

					   else
					   {

						   track_sweeps_rej++;
						   this->Invoke(gcnew MethodInvoker(this, &Client::Update_Sweeps_Rejected));	//Used to avoid cross-threading

					   }
				   }
			   }
		   }
	   }


	 //Method to determine if the max value of a 1D array is within the artifact rejection threshold
	   private: bool check_max(double* temp_array)
	   {

		   bool temp_max_val_check = true;
		  

		   for (int kk = 0; kk < refresh_time_w; kk++)
		   {

			   if (Math::Abs(temp_array[kk]) > art_rej_threshold)
			   {

				   temp_max_val_check = false;
				   
				   break;

			   }
		   }

		   return temp_max_val_check;

	   }

	 //This method is used to store the triggers that will be used for the average
		private: void Store_trig_code()
		{
			
		temp_trig_string = Trig_Code->Text;
		array<String^>^ temp_ref_chan_string_trig_code = temp_trig_string->Split(' ');
				   
				  
		//Store the trigger codes for the analysis
		trig_code_val = new int[temp_trig_string->Length]; 

			for (int kk = 0; kk < temp_ref_chan_string_trig_code->Length; kk++)
			{
					  
			trig_code_val[kk] = Convert::ToInt32(temp_ref_chan_string_trig_code[kk]);
					  
			}

		}



		//Extract the channels used as reference
		private: void Extract_chan_ref()
		{

		String^ ref_chan_text = Ref_Chan->Text;
		temp_ref_chan_string = ref_chan_text->Split(' ');
		ref_index = new int[temp_ref_chan_string->Length]; 
						
		int count_triggers_recognized = 0;

			for (int kk = 0; kk < temp_ref_chan_string->Length; kk++)
			{

				for (int hh = 0; hh < chan_names->Length; hh++)
				{

					if (Equals(temp_ref_chan_string[kk],(chan_names[hh])))
					{

					ref_index[kk] = hh;
					count_triggers_recognized++;
					break;

					}

				}

			}

				if (count_triggers_recognized != temp_ref_chan_string->Length)
				{

				 MessageBox::Show("One or more reference channels were not found. Select valid reference channels before connecting to the server", "warning");

				 Close_Biosemi->PerformClick();

				}

				else
				{

				Ref_Chan->BackColor = Color::White;

				}

		}

		

		//Box used to select the reference channels
		private: System::Void Ref_Chan_TextChanged(System::Object^ sender, System::EventArgs^ e) 
		{
		
			Ref_Chan->BackColor = Color::Red;
			

		}

			   //Executes when the enter key is pressed
			   private: System::Void Extract_chan_ref_Enter(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e) 
			   {
			   
				   if (e->KeyChar == (char)enter_code)
				   {

					   Extract_chan_ref();

				   }
			   
			   }


			//Populate the dropdown menu of the channels when the GUI is initialized
			private: void Name_chan_data()
			{

			chan_names = gcnew array<String^>(n_chan - 1);
			chan_names[0] = "Fp1";
			chan_names[1] = "AF3";
			chan_names[2] = "F7";
			chan_names[3] = "F3";
			chan_names[4] = "FC1";
			chan_names[5] = "FC5";
			chan_names[6] = "T7";
			chan_names[7] = "C3";
			chan_names[8] = "CP1";
			chan_names[9] = "CP5";
			chan_names[10] = "P7";
			chan_names[11] = "P3";
			chan_names[12] = "Pz";
			chan_names[13] = "PO3";
			chan_names[14] = "O1";
			chan_names[15] = "Oz";
			chan_names[16] = "O2";
			chan_names[17] = "PO4";
			chan_names[18] = "P4";
			chan_names[19] = "P8";
			chan_names[20] = "CP6";
			chan_names[21] = "CP2";
			chan_names[22] = "C4";
			chan_names[23] = "T8";
			chan_names[24] = "FC6";
			chan_names[25] = "FC2";
			chan_names[26] = "F4";
			chan_names[27] = "F8";
			chan_names[28] = "AF4";
			chan_names[29] = "FP2";
			chan_names[30] = "Fz";
			chan_names[31] = "Cz";
			chan_names[32] = "EXG1";
			chan_names[33] = "EXG2";
			chan_names[34] = "EXG3";
			chan_names[35] = "EXG4";
			chan_names[36] = "EXG5";
			chan_names[37] = "EXG6";
			chan_names[38] = "EXG7";
			chan_names[39] = "EXG8";


			Chan_Biosemi_I->Items->AddRange(chan_names);
			Chan_Biosemi_I->Text = Convert::ToString(chan_names[0]);

			Chan_Biosemi_II->Items->AddRange(chan_names);
			Chan_Biosemi_II->Text = Convert::ToString(chan_names[1]);

			Chan_Biosemi_III->Items->AddRange(chan_names);
			Chan_Biosemi_III->Text = Convert::ToString(chan_names[2]);

			Chan_Biosemi_IV->Items->AddRange(chan_names);
			Chan_Biosemi_IV->Text = Convert::ToString(chan_names[3]);

			Chan_Number_Biosemi->Text = Convert::ToString(n_chan); //Number of channels + the trigger line
			Bytes_Package->Text = Convert::ToString(n_chan * size_samples * bytes_sample);

			}

		
		//Change the size of the package of bytes based on the number of channels selected
		private: System::Void Forty_Seventy_Two_Chan_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) 
		{

		Chan_Biosemi_I->Items->Clear();
		Chan_Biosemi_II->Items->Clear();
		Chan_Biosemi_III->Items->Clear();
		Chan_Biosemi_IV->Items->Clear();

		switch (Forty_Seventy_Two_Chan->SelectedIndex)
		{


		case 0:
			n_chan = 41;
			chan_names = gcnew array<String^>(n_chan - 1);
			chan_names[0] = "Fp1";
			chan_names[1] = "AF3";
			chan_names[2] = "F7";
			chan_names[3] = "F3";
			chan_names[4] = "FC1";
			chan_names[5] = "FC5";
			chan_names[6] = "T7";
			chan_names[7] = "C3";
			chan_names[8] = "CP1";
			chan_names[9] = "CP5";
			chan_names[10] = "P7";
			chan_names[11] = "P3";
			chan_names[12] = "Pz";
			chan_names[13] = "PO3";
			chan_names[14] = "O1";
			chan_names[15] = "Oz";
			chan_names[16] = "O2";
			chan_names[17] = "PO4";
			chan_names[18] = "P4";
			chan_names[19] = "P8";
			chan_names[20] = "CP6";
			chan_names[21] = "CP2";
			chan_names[22] = "C4";
			chan_names[23] = "T8";
			chan_names[24] = "FC6";
			chan_names[25] = "FC2";
			chan_names[26] = "F4";
			chan_names[27] = "F8";
			chan_names[28] = "AF4";
			chan_names[29] = "FP2";
			chan_names[30] = "Fz";
			chan_names[31] = "Cz";
			chan_names[32] = "EXG1";
			chan_names[33] = "EXG2";
			chan_names[34] = "EXG3";
			chan_names[35] = "EXG4";
			chan_names[36] = "EXG5";
			chan_names[37] = "EXG6";
			chan_names[38] = "EXG7";
			chan_names[39] = "EXG8";


			Chan_Biosemi_I->Items->AddRange(chan_names);
			Chan_Biosemi_I->Text = Convert::ToString(chan_names[0]);

			Chan_Biosemi_II->Items->AddRange(chan_names);
			Chan_Biosemi_II->Text = Convert::ToString(chan_names[1]);

			Chan_Biosemi_III->Items->AddRange(chan_names);
			Chan_Biosemi_III->Text = Convert::ToString(chan_names[2]);

			Chan_Biosemi_IV->Items->AddRange(chan_names);
			Chan_Biosemi_IV->Text = Convert::ToString(chan_names[3]);

			Chan_Number_Biosemi->Text = Convert::ToString(n_chan); //Number of channels + the trigger line
			Bytes_Package->Text = Convert::ToString(n_chan * size_samples * bytes_sample);


			if (cust_chan_obj == nullptr)
			{

				cust_chan_obj = gcnew Customize_Channels();
				cust_chan_obj->Hide();

			}

			else
			{

				cust_chan_obj->Hide();

			}


			break;

		case 1:
			n_chan = 73;
			chan_names = gcnew array<String^>(n_chan - 1);
			chan_names[0] = "Fp1";
			chan_names[1] = "AF7";
			chan_names[2] = "AF3";
			chan_names[3] = "F1";
			chan_names[4] = "F3";
			chan_names[5] = "F5";
			chan_names[6] = "F7";
			chan_names[7] = "FT7";
			chan_names[8] = "FC5";
			chan_names[9] = "FC3";
			chan_names[10] = "FC1";
			chan_names[11] = "C1";
			chan_names[12] = "C3";
			chan_names[13] = "C5";
			chan_names[14] = "T7";
			chan_names[15] = "TP7";
			chan_names[16] = "CP5";
			chan_names[17] = "CP3";
			chan_names[18] = "CP1";
			chan_names[19] = "P1";
			chan_names[20] = "P3";
			chan_names[21] = "P5";
			chan_names[22] = "P7";
			chan_names[23] = "P9";
			chan_names[24] = "PO7";
			chan_names[25] = "PO3";
			chan_names[26] = "O1";
			chan_names[27] = "Iz";
			chan_names[28] = "Oz";
			chan_names[29] = "POz";
			chan_names[30] = "Pz";
			chan_names[31] = "CPz";
			chan_names[32] = "FPz";
			chan_names[33] = "Fp2";
			chan_names[34] = "AF8";
			chan_names[35] = "AF4";
			chan_names[36] = "AFz";
			chan_names[37] = "Fz";
			chan_names[38] = "F2";
			chan_names[39] = "F4";
			chan_names[40] = "F6";
			chan_names[41] = "F8";
			chan_names[42] = "FT8";
			chan_names[43] = "FC6";
			chan_names[44] = "FC4";
			chan_names[45] = "FC2";
			chan_names[46] = "FCz";
			chan_names[47] = "Cz";
			chan_names[48] = "C2";
			chan_names[49] = "C4";
			chan_names[50] = "C6";
			chan_names[51] = "T8";
			chan_names[52] = "TP8";
			chan_names[53] = "CP6";
			chan_names[54] = "CP4";
			chan_names[55] = "CP2";
			chan_names[56] = "P2";
			chan_names[57] = "P4";
			chan_names[58] = "P6";
			chan_names[59] = "P8";
			chan_names[60] = "P10";
			chan_names[61] = "PO8";
			chan_names[62] = "PO4";
			chan_names[63] = "O2";
			chan_names[64] = "EXG1";
			chan_names[65] = "EXG2";
			chan_names[66] = "EXG3";
			chan_names[67] = "EXG4";
			chan_names[68] = "EXG5";
			chan_names[69] = "EXG6";
			chan_names[70] = "EXG7";
			chan_names[71] = "EXG8";


			Chan_Biosemi_I->Items->AddRange(chan_names);
			Chan_Biosemi_I->Text = Convert::ToString(chan_names[0]);

			Chan_Biosemi_II->Items->AddRange(chan_names);
			Chan_Biosemi_II->Text = Convert::ToString(chan_names[1]);

			Chan_Biosemi_III->Items->AddRange(chan_names);
			Chan_Biosemi_III->Text = Convert::ToString(chan_names[2]);

			Chan_Biosemi_IV->Items->AddRange(chan_names);
			Chan_Biosemi_IV->Text = Convert::ToString(chan_names[3]);

			Chan_Number_Biosemi->Text = Convert::ToString(n_chan); //Number of channels + the trigger line
			Bytes_Package->Text = Convert::ToString(n_chan * size_samples * bytes_sample);

			if (cust_chan_obj->IsDisposed)
			{

				cust_chan_obj = gcnew Customize_Channels();
				cust_chan_obj->Hide();

			}

			else
			{

				cust_chan_obj->Hide();

			}

			break;

		case 2:
			
				//Cleaning up the channels available
				Chan_Biosemi_I->Text = " ";
				Chan_Biosemi_II->Text = " ";
				Chan_Biosemi_III->Text = " ";
				Chan_Biosemi_IV->Text = " ";

			//Check if the Form with the customized channels need to be open
			if (cust_chan_obj->IsDisposed)
			{

				cust_chan_obj = gcnew Customize_Channels();
				cust_chan_obj->Activate();
				cust_chan_obj->Show();

			}

			else
			{

				cust_chan_obj->Show();

			}

			//Set-up and start the thread to update the customized channels selected
			Thread^ Thread_Customize_Channels = gcnew Thread(Thread_Customize_Channels_S);
			Thread_Customize_Channels->Priority = ThreadPriority::Normal;
			Thread_Customize_Channels->SetApartmentState(ApartmentState::STA);	 //Using single threaded apartments
			Thread_Customize_Channels->Start();	//Start updating the customized channels selected
		
		break;

	}

}

	//Populate the customized channels
	private: void Fill_Customized_Channels()
	{
		try
		{

			Chan_Biosemi_I->Items->AddRange(chan_names);
			Chan_Biosemi_I->Text = Convert::ToString(chan_names[0]);

			Chan_Biosemi_II->Items->AddRange(chan_names);
			Chan_Biosemi_II->Text = Convert::ToString(chan_names[1]);

			Chan_Biosemi_III->Items->AddRange(chan_names);
			Chan_Biosemi_III->Text = Convert::ToString(chan_names[2]);

			Chan_Biosemi_IV->Items->AddRange(chan_names);
			Chan_Biosemi_IV->Text = Convert::ToString(chan_names[3]);

			Chan_Number_Biosemi->Text = Convert::ToString(n_chan); //Number of channels + the trigger line
			Bytes_Package->Text = Convert::ToString(n_chan * size_samples * bytes_sample);

		}

			catch (IndexOutOfRangeException^ e)
			{

				Chan_Number_Biosemi->Text = Convert::ToString(n_chan); //Number of channels + the trigger line
				Bytes_Package->Text = Convert::ToString(n_chan * size_samples * bytes_sample);

			}

	}

	//Method used to update the customized channels selected
	private: void Customize_Chan_Update()
	{
	
		while (!cust_chan_obj->IsDisposed)
		{


			if (cust_chan_obj->Cust_Chan_Available->CheckedItems->Count > 1)
			{

				n_chan = cust_chan_obj->Cust_Chan_Available->CheckedItems->Count + 1;
				chan_names = gcnew array<String^>(n_chan - 1);

				for (int temp_char_name = 0; temp_char_name < chan_names->Length; temp_char_name++)
				{

					chan_names[temp_char_name] = Convert::ToString(cust_chan_obj->Cust_Chan_Available->Items[cust_chan_obj->Cust_Chan_Available->CheckedIndices[temp_char_name]]);

				}

			}

		}

		if (cust_chan_obj->Cust_Chan_Available->CheckedItems->Count > 1)
		{

			this->Invoke(gcnew MethodInvoker(this, &Client::Fill_Customized_Channels));	//Used to avoid cross-threading
		}

		else
		{

			MessageBox::Show("You need to select a minimum of 2 channels, as one channel serves as reference", "Warning");

		}

	}

//Method used to initialize the filter
	   private: std::vector<std::vector<double> >  Initialize_Filter(bool filt_resp_f)
	   {
		   
			   filter_par_error = true;

			   IIR_B_F::IIR_Butterworth iir_b;

			   arma::vec filt_coeff_num;
			   arma::vec filt_coeff_den;
			   arma::vec magnitude_filt = arma::zeros<arma::vec>(sampl_freq / 2);
			   arma::vec phase_filt = arma::zeros<arma::vec>(sampl_freq / 2);

			   std::vector<std::vector<double> > save_filt_coeff;	//Array used to save the coefficients of the filter
			   std::vector<std::vector<double> >  save_magnitude_filt;
			   std::vector<std::vector<double> >  save_phase_filt;

			   std::vector<double> temp_v;
			   for (int ff = 0; ff < sampl_freq / 2; ff++)
			   {

				   temp_v.push_back(0);

			   }

			   save_magnitude_filt.push_back(temp_v);
			   save_phase_filt.push_back(temp_v);

			   switch (Type_Filter->SelectedIndex)
			   {

			   case 0:	//Band pass filter

				   filt_coeff_num = arma::zeros<arma::vec>(2 * Convert::ToInt32(Order_Filt->Text) + 1);
				   filt_coeff_den = arma::zeros<arma::vec>(2 * Convert::ToInt32(Order_Filt->Text) + 1);

				   for (int ff = 0; ff < 2 * Convert::ToInt32(Order_Filt->Text) + 1; ff++)
				   {

					   temp_v.push_back(0);

				   }

				   for (int hh = 0; hh < 2; hh++)
				   {

					   save_filt_coeff.push_back(temp_v);

				   }

				   save_filt_coeff = iir_b.lp2bp(Convert::ToDouble(HF_Cut->Text) / (double)(sampl_freq / 2), Convert::ToDouble(LF_Cut->Text) / (double)(sampl_freq / 2), Convert::ToInt32(Order_Filt->Text));

				   filter_stable = iir_b.check_stability_iir(save_filt_coeff);

				   for (int kk = 0; kk < 2 * Convert::ToInt32(Order_Filt->Text) + 1; kk++)
				   {

					   filt_coeff_num(kk) = save_filt_coeff[0][kk];
					   filt_coeff_den(kk) = save_filt_coeff[1][kk];

				   }

				   magnitude_filt = 20 * log10(sp::freqz(filt_coeff_num, filt_coeff_den, sampl_freq / 2));
				   phase_filt = sp::phasez(filt_coeff_num, filt_coeff_den, sampl_freq / 2);

				   for (int kk = 0; kk < sampl_freq / 2; kk++)
				   {

					   save_magnitude_filt[0][kk] = magnitude_filt.mem[kk];
					   save_phase_filt[0][kk] = phase_filt.mem[kk];

					   if (isinf(magnitude_filt.mem[kk]))
					   {

						   save_magnitude_filt[0][kk] = -100;

					   }

				   }



				   break;

			   case 1:	//Band stop filter

				   filt_coeff_num = arma::zeros<arma::vec>(2 * Convert::ToInt32(Order_Filt->Text) + 1);
				   filt_coeff_den = arma::zeros<arma::vec>(2 * Convert::ToInt32(Order_Filt->Text) + 1);

				   for (int ff = 0; ff < 2 * Convert::ToInt32(Order_Filt->Text) + 1; ff++)
				   {

					   temp_v.push_back(0);

				   }

				   for (int hh = 0; hh < 2; hh++)
				   {


					   save_filt_coeff.push_back(temp_v);

				   }


				   save_filt_coeff = iir_b.lp2bs(Convert::ToDouble(HF_Cut->Text) / (double)(sampl_freq / 2), Convert::ToDouble(LF_Cut->Text) / (double)(sampl_freq / 2), Convert::ToInt32(Order_Filt->Text));

				   filter_stable = iir_b.check_stability_iir(save_filt_coeff);

				   for (int kk = 0; kk < 2 * Convert::ToInt32(Order_Filt->Text) + 1; kk++)
				   {

					   filt_coeff_num(kk) = save_filt_coeff[0][kk];
					   filt_coeff_den(kk) = save_filt_coeff[1][kk];

				   }

				   magnitude_filt = 20 * log10(sp::freqz(filt_coeff_num, filt_coeff_den, sampl_freq / 2));
				   phase_filt = sp::phasez(filt_coeff_num, filt_coeff_den, sampl_freq / 2);

				   for (int kk = 0; kk < sampl_freq / 2; kk++)
				   {

					   save_magnitude_filt[0][kk] = magnitude_filt.mem[kk];
					   save_phase_filt[0][kk] = phase_filt.mem[kk];

					   if (isinf(magnitude_filt.mem[kk]))
					   {

						   save_magnitude_filt[0][kk] = -100;

					   }

				   }



				   break;


			   case 2:	//High pass filter

				   filt_coeff_num = arma::zeros<arma::vec>(Convert::ToInt32(Order_Filt->Text) + 1);
				   filt_coeff_den = arma::zeros<arma::vec>(Convert::ToInt32(Order_Filt->Text) + 1);

				   for (int ff = 0; ff < Convert::ToInt32(Order_Filt->Text) + 1; ff++)
				   {

					   temp_v.push_back(0);

				   }

				   for (int hh = 0; hh < 2; hh++)
				   {

					   save_filt_coeff.push_back(temp_v);

				   }


				   save_filt_coeff = iir_b.lp2hp(Convert::ToDouble(HF_Cut->Text) / (double)(sampl_freq / 2), Convert::ToInt32(Order_Filt->Text));

				   filter_stable = iir_b.check_stability_iir(save_filt_coeff);

				   for (int kk = 0; kk < Convert::ToInt32(Order_Filt->Text) + 1; kk++)
				   {

					   filt_coeff_num(kk) = save_filt_coeff[0][kk];
					   filt_coeff_den(kk) = save_filt_coeff[1][kk];

				   }

				   magnitude_filt = 20 * log10(sp::freqz(filt_coeff_num, filt_coeff_den, sampl_freq / 2));
				   phase_filt = sp::phasez(filt_coeff_num, filt_coeff_den, sampl_freq / 2);

				   for (int kk = 0; kk < sampl_freq / 2; kk++)
				   {

					   save_magnitude_filt[0][kk] = magnitude_filt.mem[kk];
					   save_phase_filt[0][kk] = phase_filt.mem[kk];

					   if (isinf(magnitude_filt.mem[kk]))
					   {

						   save_magnitude_filt[0][kk] = -100;

					   }

				   }


				   break;

			   case 3:	//Low pass filter

				   filt_coeff_num = arma::zeros<arma::vec>(Convert::ToInt32(Order_Filt->Text) + 1);
				   filt_coeff_den = arma::zeros<arma::vec>(Convert::ToInt32(Order_Filt->Text) + 1);

				   for (int ff = 0; ff < Convert::ToInt32(Order_Filt->Text) + 1; ff++)
				   {

					   temp_v.push_back(0);

				   }

				   for (int hh = 0; hh < 2; hh++)
				   {


					   save_filt_coeff.push_back(temp_v);

				   }


				   save_filt_coeff = iir_b.lp2lp(Convert::ToDouble(LF_Cut->Text) / (double)(sampl_freq / 2), Convert::ToInt32(Order_Filt->Text));

				   filter_stable = iir_b.check_stability_iir(save_filt_coeff);

				   for (int kk = 0; kk < Convert::ToInt32(Order_Filt->Text) + 1; kk++)
				   {

					   filt_coeff_num(kk) = save_filt_coeff[0][kk];
					   filt_coeff_den(kk) = save_filt_coeff[1][kk];

				   }

				   magnitude_filt = 20 * log10(sp::freqz(filt_coeff_num, filt_coeff_den, sampl_freq / 2));
				   phase_filt = sp::phasez(filt_coeff_num, filt_coeff_den, sampl_freq / 2);

				   for (int kk = 0; kk < sampl_freq / 2; kk++)
				   {

					   save_magnitude_filt[0][kk] = magnitude_filt.mem[kk];
					   save_phase_filt[0][kk] = phase_filt.mem[kk];

					   if (isinf(magnitude_filt.mem[kk]))
					   {

						   save_magnitude_filt[0][kk] = -100;

					   }

				   }



				   break;


			   }


			   if (filt_resp_f)
			   {

				   return save_magnitude_filt;

			   }

			   else
			   {

				   return save_filt_coeff;

			   }

	   }


	//Plot the filter response
	private: System::Void Plot_Filter_Response_Click(System::Object^ sender, System::EventArgs^ e) 
	{

		if ((!Equals(HF_Cut->Text, "")) & (!Equals(LF_Cut->Text, "")) & (!Equals(Order_Filt->Text, "")))
		{

			std::vector<std::vector<double> >  save_magnitude_filt;	//Array used to save the coefficients of the filter
			std::vector<double> temp_v;
			for (int ff = 0; ff < sampl_freq / 2; ff++)
			{

				temp_v.push_back(0);

			}

			save_magnitude_filt.push_back(temp_v);


			save_magnitude_filt = Initialize_Filter(true);

			Time_Domain_Plot_Chart_I->Visible = false;
			Time_Domain_Plot_Chart_II->Visible = false;
			Time_Domain_Plot_Chart_III->Visible = false;
			Time_Domain_Plot_Chart_IV->Visible = false;

			Frequency_Domain_Plot_Chart_I->Visible = false;
			Frequency_Domain_Plot_Chart_II->Visible = false;
			Frequency_Domain_Plot_Chart_III->Visible = false;
			Frequency_Domain_Plot_Chart_IV->Visible = false;


			Label_Time_Axis->Visible = false;
			Label_Frequency_Axis->Visible = false;

			Art_Rej->Visible = false;
			Max_Val_Art_Rej->Visible = false;
			Scale_Art_Rej->Visible = false;

			Min_Val_Freq_Text->Visible = false;
			Max_Val_Freq_Text->Visible = false;
			Min_Val_Freq->Visible = false;
			Max_Val_Freq->Visible = false;

			Magnitude_Response_Filter->Visible = true;
			Magnitude_Response_Filter->Series[0]->Points->Clear();


			//Plot the magnitude of the impulse response of the filter 
			for (int kk = 0; kk < sampl_freq / 2; kk++) {

				Magnitude_Response_Filter->Series[0]->Points->AddXY(kk, save_magnitude_filt[0][kk]);

			}

			if (filter_stable)
			{

				Magnitude_Response_Filter->Series[0]->LegendText = "The filter is stable";

			}

			else
			{

				Magnitude_Response_Filter->Series[0]->LegendText = "The filter is untable";

			}

		}

		else
		{

			MessageBox::Show("Wrong selection of the filter parameters. One or more parameters are missing", "Warning");
			
		}

	}

	/*
	//Filter the data by using the Direct-Form I, as explained in the Matlab documentation
	private: void Filter_Data_Biosemi()
	{
		
		
		std::vector<double> temp_val;
		std::vector<double> temp_val_zeros(temp_filt_coeff_length);

		std::vector<double> temp_val_y;
		std::vector<double> temp_val_y_zeros(temp_filt_coeff_length);

		//Loop through the channels 
		for (int ll = 0; ll < chan_analysis_biosemi; ll++)
		{

			//Resetting the vectors, before starting filtering a new channel
			if (temp_val.size() > 0)
			{

				temp_val.erase(temp_val.begin(), temp_val.begin() + temp_val.size());
				std::fill(temp_val.begin(), temp_val.end(), 0);	//Setting the values to zero

				temp_val_y.erase(temp_val_y.begin(), temp_val_y.begin() + temp_val_y.size());
				std::fill(temp_val_y.begin(), temp_val_y.end(), 0);	//Setting the values to zero
				
				temp_val_zeros.erase(temp_val_zeros.begin() + temp_filt_coeff_length, temp_val_zeros.begin() + temp_val_zeros.size());
				std::fill(temp_val_zeros.begin(), temp_val_zeros.end(), 0);	//Setting the values to zero

				temp_val_y_zeros.erase(temp_val_y_zeros.begin() + temp_filt_coeff_length, temp_val_y_zeros.begin() + temp_val_y_zeros.size());
				std::fill(temp_val_y_zeros.begin(), temp_val_y_zeros.end(), 0);	//Setting the values to zero
				
			}

			//Convolution product to filter the data. Since the signal has not been flipped, the index starts from the end 
			for (int kk = refresh_time_w - 1; kk >= 0; kk--)
			{
				
				temp_val.insert(temp_val.begin(), data_biosemi_plot_value_copy_Chans[ll][kk]);
				
				if (refresh_time_w - 1 - kk > 0)
				{

					temp_val_y.insert(temp_val_y.begin(),data_biosemi_plot_value_copy_Chans_filt[ll][refresh_time_w - kk - 2]);

				}

				else
				{

					temp_val_y.insert(temp_val_y.begin(),data_biosemi_plot_value_copy_Chans_filt[ll][refresh_time_w - kk]);

				}

				if (refresh_time_w - 1 - kk < temp_filt_coeff_length)
				{

					for (int pp = 0; pp < temp_val.size(); pp++)
					{

						temp_val_zeros[pp] = temp_val[pp];
						temp_val_y_zeros[pp] = temp_val_y[pp];

					}

				}

				else
				{

					for (int pp = 0; pp < temp_filt_coeff_length; pp++)
					{

						temp_val_zeros = temp_val;
						temp_val_y_zeros = temp_val_y;

					}

				}

				for (int ww = 0; ww < temp_filt_coeff_length; ww++)
				{

					if (ww < temp_filt_coeff_length - 1)
					{

						data_biosemi_plot_value_copy_Chans_filt[ll][refresh_time_w - kk - 1] += temp_val_zeros[ww] * filter_coeff_pointer[0][ww] - temp_val_y_zeros[ww] * filter_coeff_pointer[1][ww + 1];

					}

					else
					{

						data_biosemi_plot_value_copy_Chans_filt[ll][refresh_time_w - kk - 1] += temp_val_zeros[ww] * filter_coeff_pointer[0][ww];

					}

				}

				data_biosemi_plot_value_copy_Chans_filt_final[ll][kk] = data_biosemi_plot_value_copy_Chans_filt[ll][refresh_time_w - kk - 1];

			}

		}
		
		   }
		   */

	//Filter the data by using the Direct-Form II Transpose, as explained in the Matlab documentation
	private: void Filter_Data_Biosemi_Direct_Form_II_Transpose(std::vector<std::vector<double>> data_biosemi_temp_filt)
	{

		std::vector<std::vector<double> > w_val;

		std::vector<double> temp_v;

		for (int ff = 0; ff < refresh_time_w; ff++)
		{

			temp_v.push_back(0);

		}

		for (int hh = 0; hh < temp_filt_coeff_length; hh++)
		{

			w_val.push_back(temp_v);

		}

		//Loop through the channels
		for (int ll = 0; ll < chan_analysis_biosemi; ll++)
		{

			//Convolution product to filter the data
			for (int kk = 0; kk < refresh_time_w; kk++)
			{

				if (kk == 0)
				{

					data_biosemi_plot_value_copy_Chans_filt_final[ll][kk] = data_biosemi_temp_filt[ll][kk] * filter_coeff_pointer[0][0];


					for (int ww = 1; ww < temp_filt_coeff_length; ww++)
					{

						w_val[ww - 1][kk] = data_biosemi_temp_filt[ll][kk] * filter_coeff_pointer[0][ww] - data_biosemi_plot_value_copy_Chans_filt_final[ll][kk] * filter_coeff_pointer[1][ww];


					}

				}

				else
				{

					data_biosemi_plot_value_copy_Chans_filt_final[ll][kk] = data_biosemi_temp_filt[ll][kk] * filter_coeff_pointer[0][0] + w_val[0][kk - 1];

					for (int ww = 1; ww < temp_filt_coeff_length; ww++)
					{

						w_val[ww - 1][kk] = data_biosemi_temp_filt[ll][kk] * filter_coeff_pointer[0][ww] + w_val[ww][kk - 1] - data_biosemi_plot_value_copy_Chans_filt_final[ll][kk] * filter_coeff_pointer[1][ww];

						if (ww == temp_filt_coeff_length - 1)
						{
							
							w_val[ww - 1][kk] = data_biosemi_temp_filt[ll][kk] * filter_coeff_pointer[0][ww] - data_biosemi_plot_value_copy_Chans_filt_final[ll][kk] * filter_coeff_pointer[1][ww];

						}

					}

				}

				

			}

		}

	}


	//Binary search method 
	private: int binary_search_function(int samples, std::vector<int> array_binary, int temp_index_array_min, int temp_index_array_max)
	{
   
	int mid_index;

	 while (temp_index_array_max - temp_index_array_min != 0)
		{
		
		 mid_index = ceil(((double)temp_index_array_min + (double)temp_index_array_max) / 2);

		 if (array_binary[mid_index] > samples)
		 {

			 temp_index_array_max = mid_index - 1;

		 }

		 else
		 {

			 temp_index_array_min = mid_index;

		 }

		 if (array_binary[temp_index_array_min] == samples)
		 {

			 return array_binary[temp_index_array_min];

		 }

		}

	 //If an exact match is not found, use the index that returns a number > samples
	 if (array_binary[temp_index_array_min] > samples)
	 {
		 
		 return array_binary[temp_index_array_min];

	 }

	 else
	 {

		 return array_binary[temp_index_array_min + 1];

	 }
}

	//Overload for binary_search_function
	private: int binary_search_function(int samples, double* array_binary, int temp_index_array_min, int temp_index_array_max)
		   {

		int mid_index;

		while (temp_index_array_max - temp_index_array_min != 0)
		{

			mid_index = ceil(((double)temp_index_array_min + (double)temp_index_array_max) / 2);

			if (array_binary[mid_index] > samples)
			{

				temp_index_array_max = mid_index - 1;

			}

			else
			{

				temp_index_array_min = mid_index;

			}

			if (array_binary[temp_index_array_min] == samples)
			{

				return array_binary[temp_index_array_min];

			}

		}

		//If an exact match is not found, use the index that returns a number > samples
		if (array_binary[temp_index_array_min] > samples)
		{

			return array_binary[temp_index_array_min];

		}

		else
		{

			return array_binary[temp_index_array_min + 1];

		}

	}

	//Method used to initiliaze the x-axis of the FFT
	private: void Initialize_FFT()
	{

		int const array_length = 24;	//We have 24 different values
		std::vector<int> array_fft;

		{

			for (double ff = 0; ff < array_length; ff++)
			{

				array_fft.push_back(pow(2, ff + 1));

			}

		}

		samples_FFT = binary_search_function(refresh_time_w, array_fft, 0, array_length - 1);	 //find the optimal number of samples for the FFT, which is the highest power of 2 closest to the samples
   
	}

	//Method to copute the FFT. The radix-2 algorithm is used
	private: void compute_FFT()
	{

		std::complex<double> complex_imag_fft(0.0, -1.0);
		std::complex<double> W;
		std::complex<double> W_n_k;
		

		int track_index;

		for (double kk = 0; kk < samples_FFT/2 - 1; kk++)
			
			//The DFT assumes the signal to be periodic = > the fundamental frequency is 1 / N * T, where N is the number
			//of samples chosen for the FFT (samples_FFT in our case) and T is the sampling period, which has to be equivalent to the number of samples to have periodicity.

		{

			track_index = 0;

			W_n_k = cos(2.0 * PI * kk / (double)samples_FFT) - complex_imag_fft * sin(2.0 * PI * kk / (double)samples_FFT);

			//The second loop is the for the samples of the signal. This loop populates the matrix
			//with the cos / sin values

			for (double nn = 0; nn < samples_FFT/2 - 1; nn++)
			{

				W = cos(2.0 * PI * kk * nn / ((double)samples_FFT/2)) - complex_imag_fft * sin(2.0 * PI * kk * nn / ((double)samples_FFT/2));
			
				for (int bb = 0; bb < chan_analysis_biosemi; bb++)
				{

					switch (bb)
					{

					case 0:

						fft_output[bb][(int)kk] += data_biosemi_plot_value_copy_Chans_filt_final[bb][track_index] * W + W_n_k * data_biosemi_plot_value_copy_Chans_filt_final[bb][track_index + 1] * W;

						break;

					case 1:

						fft_output[bb][(int)kk] += data_biosemi_plot_value_copy_Chans_filt_final[bb][track_index] * W + W_n_k * data_biosemi_plot_value_copy_Chans_filt_final[bb][track_index + 1] * W;

						break;

					case 2:

						fft_output[bb][(int)kk] += data_biosemi_plot_value_copy_Chans_filt_final[bb][track_index] * W + W_n_k * data_biosemi_plot_value_copy_Chans_filt_final[bb][track_index + 1] * W;

						break;

					case 3:

						fft_output[bb][(int)kk] += data_biosemi_plot_value_copy_Chans_filt_final[bb][track_index] * W + W_n_k * data_biosemi_plot_value_copy_Chans_filt_final[bb][track_index + 1] * W;

						break;

					}

				}

				track_index += 2;

			}
			
		}
		
				
		this->Invoke(gcnew MethodInvoker(this, &Client::Plot_Time_Frequency_Domain));	//Used to avoid cross-threading
		
	}

//Method used to initialize the time and frequency domain
private: void Initialize_time_frequency_domain()
{

	//Initialize the time domain
	time_w = Math::Round((Convert::ToDouble(Time_Window->Text) * sampl_freq) / 1000);    //Number of samples needed before refreshing the plot

	//Make sure the time domain has an even number of data points. This simplifies checking if the plot needs to be refreshed
	int res_div;
	Math::DivRem(Convert::ToInt32(time_w), 2, res_div);

	if (res_div == 0)
	{
	
		time_domain = new double[Convert::ToInt32(time_w)];
		
		refresh_time_w = Convert::ToInt32(time_w);
	
	}

	else
	{

		time_domain = new double[Convert::ToInt32(time_w - 1)];
		
		refresh_time_w = Convert::ToInt32(time_w - 1);

	}


	//Populate the x-axis of the time and frequency domain
	for (int kk = 0; kk < refresh_time_w; kk++)
	{

		time_domain[kk] = 1000 * ((double)kk / sampl_freq);

	}


}

	
	 //Change the sample size per package based on the sampling frequency
	private: System::Void SF_Data_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) 
	{

	switch (SF_Data->SelectedIndex) 
	{

	case 0: 

		sampl_freq = Convert::ToInt32(SF_Data->Text)/ Convert::ToInt32(Dec_Factor->Text);
		Decimated_SF->Text = Convert::ToString(Convert::ToInt32(SF_Data->Text) / Convert::ToInt32(Dec_Factor->Text));
		Samples_Package->Text = "16";
		size_samples = Convert::ToInt32(Samples_Package->Text);
		Bytes_Package->Text = Convert::ToString(size_samples * bytes_sample * n_chan);

		if (Convert::ToDouble(LF_Cut->Text) >= sampl_freq / 2)
		{

			LF_Cut->Text = Convert::ToString(Math::Abs(sampl_freq / 2 - 0.001));

		}

		Max_Val_Freq->Text = Convert::ToString(Math::Abs(sampl_freq / 2 - 1));

		AA_SF->Text = "417";

		break;

	case 1:

		sampl_freq = Convert::ToInt32(SF_Data->Text) / Convert::ToInt32(Dec_Factor->Text);
		Decimated_SF->Text = Convert::ToString(Convert::ToInt32(SF_Data->Text) / Convert::ToInt32(Dec_Factor->Text));
		Samples_Package->Text = "32";
		size_samples = Convert::ToInt32(Samples_Package->Text);
		Bytes_Package->Text = Convert::ToString(size_samples * bytes_sample * n_chan);

		if (Convert::ToDouble(LF_Cut->Text) >= sampl_freq / 2)
		{

			LF_Cut->Text = Convert::ToString(Math::Abs(sampl_freq / 2 - 0.001));

		}

		Max_Val_Freq->Text = Convert::ToString(Math::Abs(sampl_freq / 2 - 1));

		AA_SF->Text = "834";

		break;

	case 2:

		sampl_freq = Convert::ToInt32(SF_Data->Text) / Convert::ToInt32(Dec_Factor->Text);
		Decimated_SF->Text = Convert::ToString(Convert::ToInt32(SF_Data->Text) / Convert::ToInt32(Dec_Factor->Text));
		Samples_Package->Text = "64";
		size_samples = Convert::ToInt32(Samples_Package->Text);
		Bytes_Package->Text = Convert::ToString(size_samples * bytes_sample * n_chan);

		if (Convert::ToDouble(LF_Cut->Text) >= sampl_freq / 2)
		{

			LF_Cut->Text = Convert::ToString(Math::Abs(sampl_freq / 2 - 0.001));

		}

		Max_Val_Freq->Text = Convert::ToString(Math::Abs(sampl_freq / 2 - 1));

		AA_SF->Text = "1667";

		break;

	case 3:

		sampl_freq = Convert::ToInt32(SF_Data->Text) / Convert::ToInt32(Dec_Factor->Text);
		Decimated_SF->Text = Convert::ToString(Convert::ToInt32(SF_Data->Text) / Convert::ToInt32(Dec_Factor->Text));
		Samples_Package->Text = "128";
		size_samples = Convert::ToInt32(Samples_Package->Text);
		Bytes_Package->Text = Convert::ToString(size_samples * bytes_sample * n_chan);

		if (Convert::ToDouble(LF_Cut->Text) >= sampl_freq / 2)
		{

			LF_Cut->Text = Convert::ToString(Math::Abs(sampl_freq / 2 - 0.001));

		}

		Max_Val_Freq->Text = Convert::ToString(Math::Abs(sampl_freq / 2 - 1));

		AA_SF->Text = "3334";

		break;

	}

}

	   //Type of the filter to be applied to the data
private: System::Void Type_Filter_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) 
{
	
	Check_filter_parameters();

	switch (Type_Filter -> SelectedIndex)
	{

	case 0:
		LF_Cut -> Enabled = true;
		Label_LF_Cut->ForeColor = Color().Green;
		HF_Cut -> Enabled = true;
		Label_HF_Cut->ForeColor = Color().Green;

		break;

	case 1:
		LF_Cut->Enabled = true;
		Label_LF_Cut->ForeColor = Color().Green;
		HF_Cut->Enabled = true;
		Label_HF_Cut->ForeColor = Color().Green;

		break;

	case 2:
		LF_Cut->Enabled = false;
		Label_LF_Cut->ForeColor = Color().Black;
		HF_Cut->Enabled = true;
		Label_HF_Cut->ForeColor = Color().Green;

		break;

	case 3:
		LF_Cut->Enabled = true;
		Label_LF_Cut->ForeColor = Color().Green;
		HF_Cut->Enabled = false;
		Label_HF_Cut->ForeColor = Color().Black;

	
		break;

	}


}

	   //Depending on which filter is selected, check the following: 1) cutoff frequencies are not higher than the Nyquist frequency, 2) The HF is lower than LH.
	   private: void Check_filter_parameters()
	   {

		   try
		   {

			   if (Convert::ToDouble(HF_Cut -> Text) >= sampl_freq / 2)
			   {

				   HF_Cut -> Text = Convert::ToString(Math::Abs(sampl_freq / 2 - 0.01));

			   }

			   if (Convert::ToDouble(LF_Cut -> Text) >= sampl_freq / 2)
			   {

				   LF_Cut -> Text = Convert::ToString(Math::Abs(sampl_freq / 2 - 0.01));

			   }

			   if (Convert::ToDouble(HF_Cut->Text) <= 0)
			   {

				   HF_Cut->Text = Convert::ToString(0.01);

			   }

			   if (Convert::ToDouble(LF_Cut->Text) <= 0)
			   {

				   LF_Cut->Text = Convert::ToString(0.1);

			   }

			   if (Convert::ToDouble(HF_Cut -> Text) >= Convert::ToDouble(LF_Cut -> Text) & Type_Filter -> SelectedIndex < 2)
			   {

				   HF_Cut -> Text = Convert::ToString(Math::Abs(Convert::ToDouble(LF_Cut -> Text) - 0.01));

			   }

		   }

		   catch (FormatException ^ e) {}

	   }


//High cutoff frequency for the filter
private: System::Void HF_Cut_TextChanged(System::Object^ sender, System::EventArgs^ e) {

	Check_filter_parameters();

}

	   //Low cutoff frequency for the filter
	   private: System::Void LF_Cut_TextChanged(System::Object^ sender, System::EventArgs^ e) 
{

	Check_filter_parameters();
	//test();
	

}

//Select if to display the continous data or the average
private: System::Void Continuous_Av_Data_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) 
{

	if (Continuous_Av_Data->SelectedIndex == 0)
	{

		Refreshing_Rate->Enabled = false;

	}

	else
	{

		Refreshing_Rate->Enabled = true;

	}

}

//Update the sampling frequency based on the decimation factor selected
private: System::Void Dec_Factor_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) 
{

	Decimated_SF->Text = Convert::ToString(Convert::ToInt32(SF_Data->Text)/Convert::ToInt32(Dec_Factor->Text));
	sampl_freq = Convert::ToInt32(SF_Data->Text) / Convert::ToInt32(Dec_Factor->Text);
	
	if (Convert::ToDouble(LF_Cut->Text) >= sampl_freq / 2)
	{

		LF_Cut->Text = Convert::ToString(Math::Abs(sampl_freq / 2 - 0.01));

	}

	Max_Val_Freq->Text = Convert::ToString(Math::Abs(sampl_freq / 2 - 1));
}

//Change the value of the artifact rejection threshold
private: System::Void Art_Rej_Scroll(System::Object^ sender, System::Windows::Forms::ScrollEventArgs^ e) 
{

	art_rej_threshold = Convert::ToDouble(Art_Rej->Maximum) - Convert::ToDouble(Art_Rej->Value);
	Max_Val_Art_Rej->Text = Convert::ToString(art_rej_threshold);

}


//Min value of the x-axis of the FFT
private: System::Void Min_Val_Freq_TextChanged(System::Object^ sender, System::EventArgs^ e) 
{

	try
	{

		if (Convert::ToInt32(Min_Val_Freq->Text) >= Convert::ToInt32(Max_Val_Freq->Text))
		{

			Min_Val_Freq->Text = Convert::ToString(Convert::ToInt32(Max_Val_Freq->Text) - 1);

		}

	}

	catch (System::FormatException^ e) {}

}

//Max value of the x-axis of the FFT
private: System::Void Max_Val_Freq_TextChanged(System::Object^ sender, System::EventArgs^ e) 
{

	try
	{

		if (Convert::ToInt32(Min_Val_Freq->Text) >= Convert::ToInt32(Max_Val_Freq->Text))
		{

			Min_Val_Freq->Text = Convert::ToString(Convert::ToInt32(Max_Val_Freq->Text) - 1);

		}
	}

	catch (System::FormatException^ e) {}

}

//Order of the filter
private: System::Void Order_Filt_TextChanged(System::Object^ sender, System::EventArgs^ e) 
{

	try
	{

		if (Convert::ToInt32(Order_Filt->Text) <= 0)
		{

			Order_Filt->Text = Convert::ToString(1);

		}

	}

	catch(System::FormatException ^e){}

}

};
}


