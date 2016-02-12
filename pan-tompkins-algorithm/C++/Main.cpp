/*********************************************************************
* Filename:   main.cpp
* Author:     Farshad Momtaz (Fdoktorm@uci.edu)
* Copyright:
* Details:    
*********************************************************************/

#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;

void convolve(const double Signal[], size_t SignalLen, const double Kernel[], size_t KernelLen, double Result[])
{
	size_t n;

	for (n = 0; n < SignalLen + KernelLen - 1; n++)
	{
		size_t kmin, kmax, k;

		Result[n] = 0;

		kmin = (n >= KernelLen - 1) ? n - (KernelLen - 1) : 0;
		kmax = (n < SignalLen - 1) ? n : SignalLen - 1;

		for (k = kmin; k <= kmax; k++)
		{
			Result[n] += Signal[k] * Kernel[n - k];
		}
	}
}

void normalize(double vector[], size_t size)
{
	double max = -1000;

	for (size_t i = 0; i < size; i++)
		if (vector[i] > max)
			max = vector[i];

	for (size_t i = 0; i < size; i++)
		vector[i] = vector[i] / max;
}

int main(void) {
	// input variables
	int signal_length = 0;
	double redundancy_val_per = 0;

	cout << "Enter Signal Length: ";
	cin >> signal_length;

	int fs;

	cout << "Enter Sampling Frequency: ";
	cin >> fs;

	cout << "Enter Redundancy Percentage: ";
	cin >> redundancy_val_per;

	double *ECG_input = new double[signal_length];

	// read data from cin
	for (int i = 0; i < signal_length; i++) {
		cin >> ECG_input[i];
	}
	cout << "Values Accurid" << endl;

	// set up derived variables
	int redundancy_check = 250; //(int)((signal_length * redundancy_val_per) / 100);
	int working_signal_length = signal_length + 105;
	double impulse_response_filter[105] = { 0.000699350,0.003932829,0.010797257,0.020402692,0.031231391,0.042113326,0.052182515,0.060834791,0.067687468,0.072541339,0.075345396,0.076164570,0.075150717,0.072516977,0.068515542,0.063418812,0.057503802,0.051039655,0.044278007,0.037445969,0.030741429,0.024330366,0.018345885,0.012888659,0.008028507,0.003806814,0.000239567,-0.002679219,-0.004973884,-0.006683468,-0.007858163,-0.008555968,-0.008839629,-0.008773929,-0.008423345,-0.007850106,-0.007112633,-0.006264364,-0.005352926,-0.004419630,-0.003499251,-0.002620048,-0.001803993,-0.001067151,-0.000420193,0.000131017,0.000584771,0.000942859,0.001209895,0.001392686,0.001499624,0.001540142,0.001524225,0.001461990,0.001363332,0.001237640,0.001093579,0.000938930,0.000780491,0.000624021,0.000474232,0.000334817,0.000208502,0.000097124,0.000001723,-0.000077356,-0.000140352,-0.000187987,-0.000221352,-0.000241816,-0.000250928,-0.000250340,-0.000241736,-0.000226768,-0.000207013,-0.000183933,-0.000158845,-0.000132907,-0.000107105,-0.000082252,-0.000058991,-0.000037803,-0.000019019,-0.000002836,0.000010667,0.000021511,0.000029802,0.000035707,0.000039445,0.000041264,0.000041436,0.000040237,0.000037939,0.000034807,0.000031084,0.000026992,0.000022727,0.000018457,0.000014321,0.000010429,0.000006866,0.000003692,0.000000942,-0.000001368,-0.000003238 };
	double impulse_response_derviative_filter[5] = { -0.125, -0.25, 0, 0.25, 0.125 };
	double impulse_response_moving_window[31] = { 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451, 0.03225806451 };

	double *working_signal = new double[working_signal_length];
	for (int i = 0; i < working_signal_length; i++) {
		working_signal[i] = 0;
	}

	// Filter - Butterworth Lowpass filter
	convolve(ECG_input, signal_length, impulse_response_filter, 105, working_signal);
	normalize(working_signal, signal_length);

	for (int i = 0; i < signal_length; i++) {
		ECG_input[i] = working_signal[i];
	}

	// Pan Tompkins Alg
	// PT - Derviative Filter
	for (int i = 0; i < working_signal_length; i++)
		working_signal[i] = 0;

	convolve(ECG_input, signal_length, impulse_response_derviative_filter, 5, working_signal);

	double *temp_signal = new double[signal_length];
	for (int i = 0; i < working_signal_length; i++)
	{
		if (i > 1 && i <= (signal_length + 1))
			temp_signal[i - 2] = working_signal[i];

		working_signal[i] = 0;
	}

	normalize(temp_signal, signal_length);

	// PT - Square results
	for (int i = 0; i < signal_length; i++)
	{
		temp_signal[i] = temp_signal[i] * temp_signal[i];
	}

	normalize(temp_signal, signal_length);

	// PT - Moving Window Int.
	convolve(temp_signal, signal_length, impulse_response_moving_window, 31, working_signal);

	for (int i = 0; i < signal_length; i++)
	{
		temp_signal[i] = working_signal[i + 14];
	}

	normalize(temp_signal, signal_length);

	// destroy working signal for memory
	//delete[] working_signal;

	// fix max and avg
	double max = -1000;
	double sum = 0;
	for (int i = 0; i < signal_length; i++)
	{
		if (temp_signal[i] > max)
			max = temp_signal[i];

		sum += temp_signal[i];
	}
	double threshold = (sum / signal_length) * max;

	// finding edges
	for (int i = 0; i < signal_length; i++)
	{
		if (temp_signal[i] >= threshold)
			temp_signal[i] = 1;
		else
			temp_signal[i] = 0;
	}

	//cout << max << endl;
	//cout << (sum/signal_length) << endl;
	//cout << threshold << endl;

	vector<int> rising;
	vector<int> falling;

	double past = -1000;
	for (int i = 0; i < signal_length; i++)
	{
		if (past != temp_signal[i] && past != -1000)
		{
			if (past < temp_signal[i]) {
				rising.push_back(i);
				cout << "rising -- " << i << endl;
			}

			if (past > temp_signal[i]) {
				falling.push_back(i);
				cout << "falling -- " << i << endl;
			}
		}

		past = temp_signal[i];
	}

	// redundancy wrong number of edges between falling and rising edges
	if (rising.size() != falling.size() && (rising[0] > falling[0])) {
		cout << "Remove first element to counter starting with falling edge" << endl;
		falling.erase(falling.begin());
	}

	if (rising.size() != falling.size() && (rising[rising.size() - 1] > falling[falling.size() - 1])) {
		cout << "Remove last element to counter ending with rising edge" << endl;
		rising.pop_back();
	}

	if (rising.size() != falling.size()) {
		cout << "Error - Number of falling and rising edges don't match" << endl;
		exit;
	}

	int size = rising.size();

	// create array for RQS
	int *r_location = new int[size];
	int *q_location = new int[size];
	int *s_location = new int[size];

	int size_trim = 0;

	// find QRS values
	for (int i = 0; i < size; i++) {
		double r_value = -1000;
		double s_value = 1000;
		double q_value = 1000;
		int r_loc, s_loc, q_loc;

		for (int l = rising[i]; l < falling[i]; l++) {
			if (ECG_input[l] > r_value) {
				r_value = ECG_input[l];
				r_loc = l;
			}

			if (ECG_input[l] < s_value) {
				s_value = ECG_input[l];
				s_loc = l;
			}
		}

		for (int l = rising[i]; l < r_loc; l++) {
			if (ECG_input[l] < q_value) {
				q_value = ECG_input[l];
				q_loc = l;
			}
		}

		if (size_trim != 0 && (r_loc - r_location[size_trim - 1]) < redundancy_check) {
			cout << "Redundancy check -  Skipped value at " << redundancy_check << " -- " << (r_loc - r_location[size_trim - 1]) << " -- " << i << endl;
			cout << "   R -- Start: " << setw(7) << rising[i] << " END:" << setw(7) << falling[i] << "  R Loc:" << setw(7) << r_loc << "  R:" << setw(12) << r_value << endl;
			cout << "   S -- Start: " << setw(7) << rising[i] << " END:" << setw(7) << falling[i] << "  S Loc:" << setw(7) << s_loc << "  S:" << setw(12) << s_value << endl;
			cout << "   Q -- Start: " << setw(7) << rising[i] << " END:" << setw(7) << r_loc << "  Q Loc:" << setw(7) << q_loc << "  Q:" << setw(12) << q_value << endl << endl;
		}
		else {
			r_location[size_trim] = r_loc;
			s_location[size_trim] = s_loc;
			q_location[size_trim] = q_loc;

			cout << "R -- Start: " << setw(7) << rising[i] << " END:" << setw(7) << falling[i] << "  R Loc:" << setw(7) << r_loc << "  R:" << setw(12) << r_value << endl;
			cout << "S -- Start: " << setw(7) << rising[i] << " END:" << setw(7) << falling[i] << "  S Loc:" << setw(7) << s_loc << "  S:" << setw(12) << s_value << endl;
			cout << "Q -- Start: " << setw(7) << rising[i] << " END:" << setw(7) << r_loc << "  Q Loc:" << setw(7) << q_loc << "  Q:" << setw(12) << q_value << endl << endl;

			size_trim++;
		}
	}

	// find Q-IPI R-IPI S-IPI
	for (int i = 0; i < (size_trim - 1); i++) {
		double q_IPI = (((double)q_location[i + 1] - (double)q_location[i]) / fs);
		double r_IPI = (((double)r_location[i + 1] - (double)r_location[i]) / fs);
		double s_IPI = (((double)s_location[i + 1] - (double)s_location[i]) / fs);

		cout << "Q-IPI: " << setw(10) << q_IPI << "  R-IPI:" << setw(10) << r_IPI << "  S-IPI:" << setw(10) << s_IPI << endl;
	}

	int b;
	cin >> b;

	return 0;
}
