/* ATIDAQ F/T C Library
 * v1.0.2
 * Copyright (c) 2001 ATI Industrial Automation
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

 /* ftconvert.c - demonstrates configuring a DAQ F/T system and performing force/torque
 calculations; converts a sample set of voltages into forces and torques.

 Sam Skuce - v.1.0.2 - February.19.2004 - added some comments just before the declaration
	 of SampleBias explaining the difference	between hardware and software temp comp
  */

#include <stdio.h>
#include <stdlib.h>
#include "ATIDAQ/ftconfig.h"

#include <NIDAQmx.h>

#define DEVICE_NAME "POC-Sensor"  // Nome del dispositivo
#define NUM_CHANNELS 6      // Numero di canali (6 in questo caso)
#define NUM_SAMPLES 1000    // Numero di campioni da leggere per ogni canale

int main() {
	char* calfilepath;      // name of calibration file
	unsigned short index;   // index of calibration in file (second parameter; default = 1)
	Calibration* cal;		// struct containing calibration information
	unsigned short i;       // loop variable used to print results
	short sts;              // return value from functions

	// In this sample application, readings have been hard-coded for demonstration.
	// A working application would retrieve these vectors from a data acquisition system.
	// PLEASE NOTE:  There are 7 elements in the bias and reading arrays.  The first 6 are
	//	the gage values you would retrieve from the transducer.  The seventh represents the 
	//	thermistor gage, which is only meaningful if your sensor uses software temperature
	//	compensation.  If your sensor uses hardware temperature compensation (all sensors
	//	sold after mid-2003 do), the last value is meaningless, and you can just use a 6 element
	//	array.
	float SampleBias[7] = { 0.2651,-0.0177,-0.0384,-0.0427,-0.1891,0.1373,-3.2423 };
	float SampleReading[7] = { -3.2863,0.3875,-3.4877,0.4043,-3.9341,0.5474,-3.2106 };

	// This sample transform includes a translation along the Z-axis and a rotation about the X-axis.
	float SampleTT[6] = { 0,0,20,45,0,0 };

	float FT[6];            // This array will hold the resultant force/torque vector.

	
	calfilepath = "C:\\Users\\Admin\\Downloads\\FT53075\\FT53075.cal";
	index = 1;

	// create Calibration struct
	cal = createCalibration(calfilepath, index);
	if (cal == NULL) {
		printf("\nSpecified calibration could not be loaded.\n");
		scanf(".");
		return 0;
	}

	// Set force units.
	// This step is optional; by default, the units are inherited from the calibration file.
	sts = SetForceUnits(cal, (char*)"N");
	switch (sts) {
	case 0: break;	// successful completion
	case 1: printf("Invalid Calibration struct"); return 0;
	case 2: printf("Invalid force units"); return 0;
	default: printf("Unknown error"); return 0;
	}


	// Set torque units.
	// This step is optional; by default, the units are inherited from the calibration file.
	sts = SetTorqueUnits(cal, (char*)"N-m");
	switch (sts) {
	case 0: break;	// successful completion
	case 1: printf("Invalid Calibration struct"); return 0;
	case 2: printf("Invalid torque units"); return 0;
	default: printf("Unknown error"); return 0;
	}


	// Set tool transform.
	// This line is only required if you want to move or rotate the sensor's coordinate system.
	// This example tool transform translates the coordinate system 20 mm along the Z-axis 
	// and rotates it 45 degrees about the X-axis.
	sts = SetToolTransform(cal, SampleTT, (char*)"mm", (char*)"degrees");
	switch (sts) {
	case 0: break;	// successful completion
	case 1: printf("Invalid Calibration struct"); return 0;
	case 2: printf("Invalid distance units"); return 0;
	case 3: printf("Invalid angle units"); return 0;
	default: printf("Unknown error"); return 0;
	}


	// Temperature compensation is on by default if it is available.
	// To explicitly disable temperature compensation, uncomment the following code
	/*SetTempComp(cal,FALSE);                   // disable temperature compensation
	switch (sts) {
		case 0: break;	// successful completion
		case 1: printf("Invalid Calibration struct"); return 0;
		case 2: printf("Temperature Compensation not available on this transducer"); return 0;
		default: printf("Unknown error"); return 0;
	}*/


	// Variabili per errori e valori letti
	int32   error = 0;
	float64 data[NUM_CHANNELS][NUM_SAMPLES];  // Array per i dati letti (6 canali)
	int32   numRead;                           // Numero di campioni letti

	// Creazione del task di acquisizione
	TaskHandle taskHandle = 0;

	// Configurazione del task di lettura per 6 canali
	error = DAQmxCreateTask("", &taskHandle);
	if (error != 0) {
		printf("Errore nella creazione del task: %ld\n", error);
		return -1;
	}


	// Creazione dei 6 canali analogici
	for (int i = 0; i < NUM_CHANNELS; i++) {
		char channelName[20];
		snprintf(channelName, sizeof(channelName), "%s/ai%d", DEVICE_NAME, i);  // Nome del canale (ai0, ai1, ..., ai5)

		error = DAQmxCreateAIVoltageChan(taskHandle, channelName, "", DAQmx_Val_Cfg_Default, -10.0, 10.0, DAQmx_Val_Volts, NULL);
		if (error != 0) {
			printf("Errore nella creazione del canale %s: %ld\n", channelName, error);
			DAQmxClearTask(taskHandle);
			return -1;
		}
	}

	// Avvio della lettura
	error = DAQmxStartTask(taskHandle);
	if (error != 0) {
		printf("Errore nell'avvio del task: %ld\n", error);
		DAQmxClearTask(taskHandle);
		return -1;
	}

	// Leggere i dati da tutti i canali (1000 campioni per canale)
	error = DAQmxReadAnalogF64(taskHandle, NUM_SAMPLES, 10.0, DAQmx_Val_GroupByChannel, &data[0][0], NUM_SAMPLES * NUM_CHANNELS, &numRead, NULL);
	if (error != 0) {
		printf("Errore nella lettura dei dati: %ld\n", error);
		DAQmxClearTask(taskHandle);
		return -1;
	}

	// Visualizzazione dei dati letti
	printf("Valori letti dai 6 canali:\n");
	for (int i = 0; i < NUM_SAMPLES; i++) {
		for (int j = 0; j < NUM_CHANNELS; j++) {
			printf("Canale ai%d, Campione %d: %f\n", j, i, data[j][i]);
		}
	}

	// Pulizia del task
	DAQmxStopTask(taskHandle);
	DAQmxClearTask(taskHandle);

	// convert a loaded measurement into forces and torques
		// store an unloaded measurement; this removes the effect of tooling weight
	for (int i = 0; i < 6; i++)
		SampleBias[i] = data[i][0];
	Bias(cal,SampleBias);
	for (int j = 0; j < 1000; j++)
	{
		float temp[6];
		for (int k = 0; k < 6; k++)
			temp[k] = data[k][j];
		ConvertToFT(cal, temp, FT);
		
		for (int l = 0; l < 6; l++)
			printf("%9.6f ", FT[l]);
		printf("\n");
	}
	


	// print results
	/*/printf("Bias reading:\n");
	for (i = 0; i < 7; i++)
		printf("%9.6f ", SampleBias[i]);
	printf("\nMeasurement:\n");
	for (i = 0; i < 7; i++)
		printf("%9.6f ", data[0][i]);
	printf("\nResult:\n");*/
	

	// free memory allocated to Calibration structure
	destroyCalibration(cal);

	//wait for a keypress
	scanf(".");

	return 0;
}
