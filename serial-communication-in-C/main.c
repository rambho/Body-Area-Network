// gcc main.c sha256.c -o main -lwiringPi -lm

#include <stdio.h>		/* printf */
#include <math.h>       /* sqrt */
#include <memory.h>

#include "sha256.h"

#include <wiringPi.h>
#include <wiringSerial.h>

void print_hash(unsigned char hash[])
{
   int idx;
   for (idx=0; idx < 32; idx++)
      printf("%02x",hash[idx]);
   printf("\n");
}

int main ()
{
	int fd, i;
	const int input_pin = 0;
	const int IPI_size = 6;
	SHA256_CTX ctx;

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
				double Q_IPI_self[] = {.965,.967,.901,.964,.984,.913};
				int Q_IPI_terancate_self[IPI_size];
				for (i = 0; i < IPI_size; i++)
				{
					Q_IPI_terancate_self[i] = Q_IPI_self[i] * 128;
				}
				
				
				// Wait for generated Nounce from partner
				printf ("Wating for Nounce\n");
				while (!serialDataAvail (fd)) {}
				char nounce_partner[4];
				for (i = 0; i < 4; i++) {
					nounce_partner[i] = (char) serialGetchar(fd);
				}
				printf ("Partner's Nounce: ");
				for (i = 0; i < 4; i++) {
					printf("%d", nounce_partner[i]);
				}
				printf("\n");
				
				
				// Generate and send Nounce
				printf ("Gerenrate and Sending Nounce\n");
				char nounce_self[4];
				for (i = 0; i < 4; i++) {
					nounce_self[i] = (char) (rand() % 256);
					fflush (stdout);
					serialPutchar (fd, nounce_self[i]);
				}
				printf("Self Nounce: ");
				for (i = 0; i < 4; i++) {
					printf("%d", nounce_self[i]);
				}
				printf("\n");
				
				
				// Reciving the encryption
				printf( "Reciving the encryption\n");
				while (!serialDataAvail (fd)) {}
				char encrypt_partner[32];
				for (i = 0; i < 32; i++) {
					encrypt_partner[i] = (char)(serialGetchar(fd));
				}
				encrypt_partner[32] = '\0';
				printf("Partner Encryption: ");
				print_hash(encrypt_partner);
				
				
				// Generate The encryption
				printf("Generate The encryption\n");
				char encrypt_self_input[(4+IPI_size)];
				for (i = 0; i < 4; i++)
					encrypt_self_input[i] = nounce_partner[i];
				for (i = 0; i < IPI_size; i++)
					encrypt_self_input[i+4] = (char) Q_IPI_terancate_self[i];
				encrypt_self_input[(4+IPI_size)] = '\0';
				printf("Self Encryption Input: ");
				for (i = 0; i < (4+IPI_size); i++) {
					printf("%d", encrypt_self_input[i]);
				}
				printf("\n");
				
				
				char encrypt_self[32]; // change
				sha256_init(&ctx);
				sha256_update(&ctx, encrypt_self_input, (4+IPI_size));
				sha256_final(&ctx,encrypt_self);
				printf("Self Encryption: ");
				print_hash(encrypt_self); // change
				encrypt_self[32] = '\0';
				
				
				// Send the encryption
				printf("Send the encryption\n");
				serialPuts(fd, encrypt_self); // change
				
				
				// Reciving The IPI
				printf("Reciving The IPI\n");
				int Q_IPI_terancate_partner[IPI_size];
				printf("Partner IPI: ");
				for (i = 0; i < IPI_size; i++) {
					Q_IPI_terancate_partner[i] = (int)(serialGetchar(fd));
					printf("%d, ", Q_IPI_terancate_partner[i]);
				}
				printf("\n");
				
				
				// Send The IPI
				printf("Send The IPI\n");
				for (i = 0; i < IPI_size; i++)
				{
					serialPutchar(fd, (char)Q_IPI_terancate_self[i]);
				}
				
				
				// Generate and compare Encryption
				printf("Generate and compare the partner Encryption\n");
				char encrypt_partner_input[(4+IPI_size)];
				for (i = 0; i < 4; i++)
					encrypt_partner_input[i] = nounce_self[i];
				for (i = 0; i < IPI_size; i++)
					encrypt_partner_input[i+4] = (char) Q_IPI_terancate_partner[i];
				encrypt_partner_input[(4+IPI_size)] = '\0';
				printf("Partner Encryption Input: ");
				for (i = 0; i < (4+IPI_size); i++) {
					printf("%d", encrypt_partner_input[i]);
				}
				printf("\n");
				
				char encrypt_partner_calculated[32]; // change
				sha256_init(&ctx);
				sha256_update(&ctx, encrypt_partner_input, (4+IPI_size));
				sha256_final(&ctx,encrypt_partner_calculated);
				printf("Calculated Partner Encryption: "); // change
				print_hash(encrypt_partner_calculated);
				encrypt_partner_calculated[32] = '\0';
				
				
				char compare = 1;
				for (i = 1; i < 32; i++) {
					if (encrypt_partner_calculated[i] != encrypt_partner[i]) { // change
						compare = 0;
					}
				}
				
				if (compare == 1)
				{
					printf("Data Comparison and analysis\n");
					
					int sum = 0;
					float standard_deviation = 0.0;
					
					for (i = 0; i < IPI_size; i++)
					{
						sum += Q_IPI_terancate_partner[i] - Q_IPI_terancate_self[i];
						standard_deviation += ((Q_IPI_terancate_partner[i] - Q_IPI_terancate_self[i]) * (Q_IPI_terancate_partner[i] - Q_IPI_terancate_self[i]));
					}
					
					float avg = sum / (float)IPI_size;
					printf("Average: %f\n", avg);
					
					standard_deviation = sqrt(standard_deviation / (float)IPI_size);
					printf("Standard Deviation: %f\n", standard_deviation);
					
				}
				else
				{
					printf("Hash was not confirmed\n");
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
				double Q_IPI_self[] = {.965,.969,.905,.960,.986,.910};
				int Q_IPI_terancate_self[IPI_size];
				for (i = 0; i < IPI_size; i++)
				{
					Q_IPI_terancate_self[i] = Q_IPI_self[i] * 128;
				}
				
				
				// Generate and send Nounce
				printf ("Gerenrate and Sending Nounce\n");
				char nounce_self[4];
				for (i = 0; i < 4; i++) {
					nounce_self[i] = (char) (rand() % 256);
					fflush (stdout);
					serialPutchar (fd, nounce_self[i]);
				}
				printf("Self Nounce: ");
				for (i = 0; i < 4; i++) {
					printf("%d", nounce_self[i]);
				}
				printf("\n");
				
				
				// Wait for generated Nounce from partner
				printf ("Wating for Nounce\n");
				while (!serialDataAvail (fd)) {}
				char nounce_partner[4];
				for (i = 0; i < 4; i++) {
					nounce_partner[i] = (char) serialGetchar(fd);
				}
				printf ("Partner's Nounce: ");
				for (i = 0; i < 4; i++) {
					printf("%d", nounce_partner[i]);
				}
				printf("\n");

				
				// Generate The encryption
				printf("Generate The encryption\n");
				char encrypt_self_input[(4+IPI_size)];
				for (i = 0; i < 4; i++)
					encrypt_self_input[i] = nounce_partner[i];
				for (i = 0; i < IPI_size; i++)
					encrypt_self_input[i+4] = (char) Q_IPI_terancate_self[i];
				encrypt_self_input[(4+IPI_size)] = '\0';
				printf("Self Encryption Input: ");
				for (i = 0; i < (4+IPI_size); i++) {
					printf("%d", encrypt_self_input[i]);
				}
				printf("\n");
				
				
				char encrypt_self[32]; // change
				sha256_init(&ctx);
				sha256_update(&ctx, encrypt_self_input, (4+IPI_size));
				sha256_final(&ctx,encrypt_self);
				printf("Self Encryption: ");
				print_hash(encrypt_self); // change
				encrypt_self[32] = '\0';
				
				
				// Send the encryption
				printf("Send the encryption\n");
				serialPuts(fd, encrypt_self); // change
				
				
				// Reciving the encryption
				printf( "Reciving the encryption\n");
				while (!serialDataAvail (fd)) {}
				char encrypt_partner[32];
				for (i = 0; i < 32; i++) {
					encrypt_partner[i] = (char)(serialGetchar(fd));
				}
				encrypt_partner[32] = '\0';
				printf("Partner Encryption: ");
				print_hash(encrypt_partner);
				
				
				// Send The IPI
				printf("Send The IPI\n");
				for (i = 0; i < IPI_size; i++)
				{
					serialPutchar(fd, (char)Q_IPI_terancate_self[i]);
				}
				
				
				// Reciving The IPI
				printf("Reciving The IPI\n");
				int Q_IPI_terancate_partner[IPI_size];
				printf("Partner IPI: ");
				for (i = 0; i < IPI_size; i++) {
					Q_IPI_terancate_partner[i] = (serialGetchar(fd));
					printf("%d, ", Q_IPI_terancate_partner[i]);
				}
				printf("\n");
				
				
				// Generate and compare Encryption
				printf("Generate and compare the partner Encryption\n");
				char encrypt_partner_input[(4+IPI_size)];
				for (i = 0; i < 4; i++)
					encrypt_partner_input[i] = nounce_self[i];
				for (i = 0; i < IPI_size; i++)
					encrypt_partner_input[i+4] = (char) Q_IPI_terancate_partner[i];
				encrypt_partner_input[(4+IPI_size)] = '\0';
				printf("Partner Encryption Input: ");
				for (i = 0; i < (4+IPI_size); i++) {
					printf("%d", encrypt_partner_input[i]);
				}
				printf("\n");
				
				char encrypt_partner_calculated[32]; // change
				sha256_init(&ctx);
				sha256_update(&ctx, encrypt_partner_input, (4+IPI_size));
				sha256_final(&ctx,encrypt_partner_calculated);
				printf("Calculated Partner Encryption: "); // change
				print_hash(encrypt_partner_calculated);
				encrypt_partner_calculated[32] = '\0';
				
				
				char compare = 1;
				for (i = 1; i < 32; i++) {
					if (encrypt_partner_calculated[i] != encrypt_partner[i]) { // change
						compare = 0;
					}
				}
				
				if (compare == 1)
				{
					printf("Data Comparison and analysis\n");
					
					int sum = 0;
					float standard_deviation = 0.0;
					
					for (i = 0; i < IPI_size; i++)
					{
						sum += Q_IPI_terancate_partner[i] - Q_IPI_terancate_self[i];
						standard_deviation += ((Q_IPI_terancate_partner[i] - Q_IPI_terancate_self[i]) * (Q_IPI_terancate_partner[i] - Q_IPI_terancate_self[i]));
					}
					
					float avg = sum / (float)IPI_size;
					printf("Average: %f\n", avg);
					
					standard_deviation = sqrt(standard_deviation / (float)IPI_size);
					printf("Standard Deviation: %f\n", standard_deviation);
					
				}
				else
				{
					printf("Hash was not confirmed\n");
				}
			}
		}
	}

	printf ("\n") ;
	return 0 ;
}
