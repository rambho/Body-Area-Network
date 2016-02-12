/*********************************************************************
* Filename:   main.cpp
* Author:     Farshad Momtaz (Fdoktorm@uci.edu)
* Copyright:
* Details:    
* Compile: 	  g++ main.cpp sha256.cpp -o main -lwiringPi -lrt
*********************************************************************/

#include <iostream>
#include <string.h> 
#include <errno.h>
#include <stdlib.h>

#include <stdio.h>		/* printf */
#include <math.h>       /* sqrt */

#include <wiringPi.h>
#include <wiringSerial.h>

#include "sha256.h"

using namespace std;

int main ()
{
	int fd;
	const int input_pin = 0;
	const int IPI_size = 6;

	if ((fd = serialOpen ("/dev/ttyAMA0", 115200)) < 0)
	{
		printf ("Unable to open serial device\n") ;
		return 1 ;
	}

	if (wiringPiSetup () == -1)
	{
		printf ("Unable to start wiringPi") ;
		return 1 ;
	}
	
	// Setting up the pin for initializing pairing sequence
	pinMode(input_pin, INPUT);
	pullUpDnControl(input_pin, PUD_UP);

	while (1)
	{
		if (serialDataAvail (fd))
		{
			printf("\n\n\nRecieved Data\n");
			
			// start recieve protocal
			int seq_1 = serialGetchar (fd);
			int seq_2 = serialGetchar (fd);
			int seq_3 = serialGetchar (fd);
			fflush (stdout);
			
			if (seq_1 == 115 && seq_2 == 121 && seq_3 == 110) {
				printf ("Pairing Sequence Started\n");
				fflush (stdout) ;
				serialPuts (fd, "ack");
				
				
				// Record the data
				printf("Record Data\n");
				double Q_IPI_self[IPI_size] = {.965,.967,.901,.964,.984,.913};
				int Q_IPI_terancate_self[IPI_size];
				for (int i = 0; i < IPI_size; i++)
				{
					Q_IPI_terancate_self[i] = Q_IPI_self[i] * 128;
				}
				
				
				// Wait for generated Nounce from partner
				printf ("Wating for Nounce\n");
				while (!serialDataAvail (fd)) {}
				char nounce_partner[4];
				for (int i = 0; i < 4; i++) {
					nounce_partner[i] = (char) serialGetchar(fd);
				}
				printf ("Partner's Nounce: ");
				for (int i = 0; i < 4; i++) {
					printf("%d", nounce_partner[i]);
				}
				printf("\n");
				
				
				// Generate and send Nounce
				printf ("Gerenrate and Sending Nounce\n");
				char nounce_self[4];
				for (int i = 0; i < 4; i++) {
					nounce_self[i] = (char) (rand() % 256);
					fflush (stdout);
					serialPutchar (fd, nounce_self[i]);
				}
				printf("Self Nounce: ");
				for (int i = 0; i < 4; i++) {
					printf("%d", nounce_self[i]);
				}
				printf("\n");
				
				
				// Reciving the encryption
				printf( "Reciving the encryption\n");
				while (!serialDataAvail (fd)) {}
				char encrypt_partner[64];
				for (int i = 0; i < 64; i++) {
					encrypt_partner[i] = (char)(serialGetchar(fd));
				}
				encrypt_partner[64] = '\0';
				printf("Partner Encryption: %s\n", encrypt_partner);
				
				
				// Generate The encryption
				printf("Generate The encryption\n");
				char encrypt_self_input[(4+IPI_size)];
				for (int i = 0; i < 4; i++)
					encrypt_self_input[i] = nounce_partner[i];
				for (int i = 0; i < IPI_size; i++)
					encrypt_self_input[i+4] = (char) Q_IPI_terancate_self[i];
				encrypt_self_input[(4+IPI_size)] = '\0';
				printf("Self Encryption Input: ");
				for (int i = 0; i < (4+IPI_size); i++) {
					printf("%d", encrypt_self_input[i]);
				}
				printf("\n");
				
				string encrypt_self = sha256(encrypt_self_input); // change
				cout << "Self Encryption: " << encrypt_self << endl; // change
				
				
				// Send the encryption
				printf("Send the encryption\n");
				serialPuts(fd, encrypt_self.c_str()); // change
				
				
				// Reciving The IPI
				printf("Reciving The IPI\n");
				int Q_IPI_terancate_partner[IPI_size];
				printf("Partner IPI: ");
				for (int i = 0; i < IPI_size; i++) {
					Q_IPI_terancate_partner[i] = (serialGetchar(fd));
					printf("%d, ", Q_IPI_terancate_partner[i]);
				}
				printf("\n");
				
				// Send The IPI
				printf("Send The IPI\n");
				for (int i = 0; i < IPI_size; i++)
					serialPutchar(fd, (char)Q_IPI_terancate_self[i]);
				
				
				// Generate and compare Encryption
				printf("Generate and compare the partner Encryption\n");
				char encrypt_partner_input[(4+IPI_size)];
				for (int i = 0; i < 4; i++)
					encrypt_partner_input[i] = nounce_self[i];
				for (int i = 0; i < IPI_size; i++)
					encrypt_partner_input[i+4] = (char) Q_IPI_terancate_partner[i];
				encrypt_partner_input[(4+IPI_size)] = '\0';
				printf("Partner Encryption Input: ");
				for (int i = 0; i < (4+IPI_size); i++) {
					printf("%d", encrypt_partner_input[i]);
				}
				printf("\n");
				
				string encrypt_partner_calculated = sha256(encrypt_partner_input); // change
				cout << "Calculated Partner Encryption: " << encrypt_partner_calculated << endl; // change
				
				
				char compare = 1;
				for (int i = 0; i < 64; i++) {
					if (encrypt_partner_calculated.c_str()[i] != encrypt_partner[i]) { // change
						compare = 0;
					}
				}
				
				if (compare == 1)
				{
					printf("Data Comparison and analysis\n");
					
					int sum = 0;
					float standard_deviation = 0.0;
					
					for (int i = 0; i < IPI_size; i++)
					{
						sum += Q_IPI_terancate_partner[i] - Q_IPI_terancate_self[i];
						standard_deviation += ((Q_IPI_terancate_partner[i] - Q_IPI_terancate_self[i]) * (Q_IPI_terancate_partner[i] - Q_IPI_terancate_self[i]));
					}
					
					float avg = sum / (float)IPI_size;
					printf("Average: %f\n", avg);
					
					standard_deviation = sqrt(standard_deviation / (float)IPI_size);
					printf("Standard Deviation: %f\n", standard_deviation);
					
				}
			}
		}
		
		if (!digitalRead(input_pin))
		{
			printf("\n\n\nSend Command Recieved\n");
						
			// start send protocal			
			printf("Pairing Sequence Started\n");
			fflush (stdout);
			serialPuts (fd, "syn");
			
			while (!serialDataAvail (fd)) {}
			
			int seq_1 = serialGetchar (fd);
			int seq_2 = serialGetchar (fd);
			int seq_3 = serialGetchar (fd);
			fflush (stdout);
			
			if (seq_1 == 97 && seq_2 == 99 && seq_3 == 107) {
				// Record the data
				printf("Record Data\n");
				double Q_IPI_self[IPI_size] = {.965,.969,.905,.960,.986,.910};
				int Q_IPI_terancate_self[IPI_size];
				for (int i = 0; i < IPI_size; i++)
				{
					Q_IPI_terancate_self[i] = Q_IPI_self[i] * 128;
				}
				
				
				// Generate and send Nounce
				printf ("Gerenrate and Sending Nounce\n");
				char nounce_self[4];
				for (int i = 0; i < 4; i++) {
					nounce_self[i] = (char) (rand() % 256);
					fflush (stdout);
					serialPutchar (fd, nounce_self[i]);
				}
				printf("Self Nounce: ");
				for (int i = 0; i < 4; i++) {
					printf("%d", nounce_self[i]);
				}
				printf("\n");
				
				
				// Wait for generated Nounce from partner
				printf ("Wating for Nounce\n");
				while (!serialDataAvail (fd)) {}
				char nounce_partner[4];
				for (int i = 0; i < 4; i++) {
					nounce_partner[i] = (char) serialGetchar(fd);
				}
				printf ("Partner's Nounce: ");
				for (int i = 0; i < 4; i++) {
					printf("%d", nounce_partner[i]);
				}
				printf("\n");

				
				// Generate The encryption
				printf("Generate The encryption\n");
				char encrypt_self_input[(4+IPI_size)];
				for (int i = 0; i < 4; i++)
					encrypt_self_input[i] = nounce_partner[i];
				for (int i = 0; i < IPI_size; i++)
					encrypt_self_input[i+4] = (char) Q_IPI_terancate_self[i];
				encrypt_self_input[(4+IPI_size)] = '\0';
				printf("Self Encryption Input: ");
				for (int i = 0; i < (4+IPI_size); i++) {
					printf("%d", encrypt_self_input[i]);
				}
				printf("\n");
				
				
				string encrypt_self = sha256(encrypt_self_input); // change
				cout << "Self Encryption: " << encrypt_self << endl; // change
				
				
				// Send the encryption
				printf("Send the encryption\n");
				serialPuts(fd, encrypt_self.c_str());  // change
				
				
				// Reciving the encryption
				printf( "Reciving the encryption\n");
				while (!serialDataAvail (fd)) {}
				char encrypt_partner[64];
				for (int i = 0; i < 64; i++) {
					encrypt_partner[i] = (char)(serialGetchar(fd));
				}
				encrypt_partner[64] = '\0';
				printf("Partner Encryption: %s\n", encrypt_partner);
				
				
				// Send The IPI
				printf("Send The IPI\n");
				for (int i = 0; i < IPI_size; i++)
					serialPutchar(fd, (char)Q_IPI_terancate_self[i]);
				
				// Reciving The IPI
				printf("Reciving The IPI\n");
				int Q_IPI_terancate_partner[IPI_size];
				printf("Partner IPI: ");
				for (int i = 0; i < IPI_size; i++) {
					Q_IPI_terancate_partner[i] = (serialGetchar(fd));
					printf("%d, ", Q_IPI_terancate_partner[i]);
				}
				printf("\n");
				
				
				// Generate and compare Encryption
				printf("Generate and compare the partner Encryption\n");
				char encrypt_partner_input[(4+IPI_size)];
				for (int i = 0; i < 4; i++)
					encrypt_partner_input[i] = nounce_self[i];
				for (int i = 0; i < IPI_size; i++)
					encrypt_partner_input[i+4] = (char) Q_IPI_terancate_partner[i];
				encrypt_partner_input[(4+IPI_size)] = '\0';
				printf("Partner Encryption Input: ");
				for (int i = 0; i < (4+IPI_size); i++) {
					printf("%d", encrypt_partner_input[i]);
				}
				printf("\n");
				
				string encrypt_partner_calculated = sha256(encrypt_partner_input); // change
				cout << "Calculated Partner Encryption: " << encrypt_partner_calculated << endl; // change
				
				
				char compare = 1;
				for (int i = 0; i < 64; i++) {
					if (encrypt_partner_calculated.c_str()[i] != encrypt_partner[i]) { // change
						compare = 0;
					}
				}
				
				if (compare == 1)
				{
					printf("Data Comparison and analysis\n");
					
					int sum = 0;
					float standard_deviation = 0.0;
					
					for (int i = 0; i < IPI_size; i++)
					{
						sum += Q_IPI_terancate_partner[i] - Q_IPI_terancate_self[i];
						standard_deviation += ((Q_IPI_terancate_partner[i] - Q_IPI_terancate_self[i]) * (Q_IPI_terancate_partner[i] - Q_IPI_terancate_self[i]));
					}
					
					float avg = sum / (float)IPI_size;
					printf("Average: %f\n", avg);
					
					standard_deviation = sqrt(standard_deviation / (float)IPI_size);
					printf("Standard Deviation: %f\n", standard_deviation);
					
				}
			}
		}
	}

	printf ("\n") ;
	return 0 ;
}