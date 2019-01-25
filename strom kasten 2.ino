// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       strom kasten 2.ino
    Created:	24/01/2019 20:17:56
    Author:     LAPTOP-CELLS40J\v_sam
*/


#define HEATING_PIN A0
#define APPLIANCE_PIN A1

int number_samples = 100;
int inter_sample_delay = 1;
int number_samples_read = 0;
long last_sample_time;

int heating_sensor;
int appliance_sensor;

int min_appliance_sensor = 1000;
int max_appliance_sensor = 0;
long appliance_running_total;
int appliance_mean;

int min_heating_sensor = 1000;
int max_heating_sensor = 0;
long heating_running_total;
int heating_mean;

int calibration_min_heating = 1000;
int calibration

// Define User Types below here or use a .h file
//


// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//

// The setup() function runs once each time the micro-controller starts
void setup()
{


}

// Add the main program code into the continuous loop() function
void loop()
{


}

bool request_sample(long last_sample_time, int inter_sample_delay)
{
	if (millis() >= last_sample_time + inter_sample_delay)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool valid_sample(int min, int max, int mean)
{
	int range = abs(max - min);
	int limit = 20;
	if ((mean / range) < limit)
	{
		return false;
	}
	else
	{
		return true;
	}
}

int sample_mean(int number_samples, long running_total)
{
	int mean = running_total / number_samples;
	return mean;
}

void update_sensor_parameters()
{
	heating_sensor = analogRead(HEATING_PIN);
	appliance_sensor = analogRead(APPLIANCE_PIN);
	number_samples_read++;
	heating_running_total += heating_sensor;
	appliance_running_total += appliance_sensor;

	if (heating_sensor < min_heating_sensor)
	{
		min_heating_sensor = heating_sensor;
	}

	if (heating_sensor > max_heating_sensor)
	{
		max_heating_sensor = heating_sensor;
	}

	if (appliance_sensor < min_appliance_sensor)
	{
		min_appliance_sensor = appliance_sensor;
	}

	if (appliance_sensor > max_appliance_sensor)
	{
		max_appliance_sensor = appliance_sensor;
	}
}

void initial_calibration()
{


	if (request_sample(last_sample_time, inter_sample_delay))
	{
		update_sensor_parameters();

		if (number_samples_read == number_samples)
		{
			heating_mean = heating_running_total / number_samples;
			appliance_mean = appliance_running_total / number_samples;

			if (valid_sample(min_heating_sensor, max_heating_sensor, heating_mean))
			{

			}
		}
	}
}