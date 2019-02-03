// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
  Name:       strom kasten 2.ino
  Created:  24/01/2019 20:17:56
  Author:     LAPTOP-CELLS40J\v_sam
*/


#define HEATING_PIN A0
#define APPLIANCE_PIN A1
#define HEATING_MONITOR_PIN 3
#define APPLIANCE_MONITOR_PIN 4

#define HEATING_LOWER_HYSTERESIS 4
#define HEATING_UPPER_HYSTERESIS 4
#define APPLIANCE_LOWER_HYSTERESIS 4
#define APPLIANCE_UPPER_HYSTERESIS 4
#define NUMBER_SAMPLES 10
#define CALIBRATION_INTERVAL 300000
#define CONSISTENCY_SAMPLES 5
#define CONSISTENCY_TOLERANCE 20
#define INTER_SAMPLE_DELAY 1

#define dateTimeMsgLength 13
#define dateTimeMsgHeader 'X'

#define perform_dump true

int number_samples_read = 0;
long last_sample_time = 0;

int heating_sensor;
int appliance_sensor;

int min_appliance_sensor = 10000;
int max_appliance_sensor = 0;
long appliance_running_total;
int appliance_mean;

int min_heating_sensor = 10000;
int max_heating_sensor = 0;
long heating_running_total;
int heating_mean;

int calibration_min_heating = 10000;
int calibration_max_heating = 0;
int calibration_min_appliance = 10000;
int calibration_max_appliance = 0;

long last_calibration_time = 0;

int threshold_lower_heating = 400;
int threshold_upper_heating = 600;
int threshold_lower_appliance = 100;
int threshold_upper_appliance = 600;

int consistency_min_heating = 10000;
int consistency_max_heating = 0;
long consistency_running_total_heating = 0;
int consistency_heating_sample_count = 0;

int consistency_min_appliance = 10000;
int consistency_max_appliance = 0;
long consistency_running_total_appliance = 0;
int consistency_appliance_sample_count = 0;
int consistency_sample_count = 0;

bool heating = false;
bool appliance = false;
bool sample_complete = false;

String heating_data_buffer = "";

// Define User Types below here or use a .h file
//
#include <SPI.h>
#include <SD.h>
File myFile;
File fullDump;


// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//
bool inside_short_term_sampling_period(long last_sample_time)
{
	if (millis() >= (last_sample_time + INTER_SAMPLE_DELAY))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool valid_sample(int min, int max, int mean, int limit)
{
	int range = abs(max - min);
	{
		if (range < limit)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

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

void add_to_heating_consistency_set()
{
	if (consistency_min_heating > heating_mean)
	{
		consistency_min_heating = heating_mean;
	}
	if (consistency_max_heating < heating_mean)
	{
		consistency_max_heating = heating_mean;
	}
	consistency_running_total_heating += heating_mean;
	consistency_heating_sample_count++;
}

void add_to_appliance_consistency_set()
{
	if (consistency_min_appliance > appliance_mean)
	{
		consistency_min_appliance = appliance_mean;
	}
	if (consistency_max_appliance < appliance_mean)
	{
		consistency_max_appliance = appliance_mean;
	}
	consistency_running_total_appliance += appliance_mean;
	consistency_appliance_sample_count++;
}

void add_to_heating_calibration_set()
{
	if (calibration_min_heating > heating_mean)
	{
		calibration_min_heating = heating_mean;
	}
	if (calibration_max_heating < heating_mean)
	{
		calibration_max_heating = heating_mean;
	}
}

void add_to_appliance_calibration_set()
{
	if (calibration_min_appliance > appliance_mean)
	{
		calibration_min_appliance = appliance_mean;
	}
	if (calibration_max_appliance < appliance_mean)
	{
		calibration_max_appliance = appliance_mean;
	}
}

bool recalibrate(long last_calibration_time)
{
	if ((millis() - last_calibration_time) > CALIBRATION_INTERVAL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void perform_recalibration()
{
	int range_heating = calibration_max_heating - calibration_min_heating;
	int range_appliance = calibration_max_appliance - calibration_min_appliance;

	threshold_lower_heating = calibration_min_heating + range_heating / HEATING_LOWER_HYSTERESIS;
	threshold_upper_heating = calibration_max_heating - range_heating / HEATING_UPPER_HYSTERESIS;
	threshold_lower_appliance = calibration_min_appliance + range_appliance / APPLIANCE_LOWER_HYSTERESIS;
	threshold_upper_appliance = calibration_max_appliance - range_appliance / APPLIANCE_UPPER_HYSTERESIS;

	Serial.println();
	Serial.println("Performing Recalibration");
	send_min_max_values_to_monitor();
	send_heating_values_to_monitor();
	send_appliance_values_to_monitor();
	Serial.println();

	last_calibration_time = millis();
}

void reset_short_term_accumulators()
{
	heating_running_total = 0;
	appliance_running_total = 0;
	min_heating_sensor = 10000;
	max_heating_sensor = 0;
	min_appliance_sensor = 10000;
	max_appliance_sensor = 0;
	number_samples_read = 0;
}

void reset_consistency_accumulators()
{
	consistency_running_total_heating = 0;
	consistency_running_total_appliance = 0;
	consistency_min_heating = 10000;
	consistency_max_heating = 0;
	consistency_min_appliance = 10000;
	consistency_max_appliance = 0;
	consistency_heating_sample_count = 0;
	consistency_appliance_sample_count = 0;
	consistency_sample_count = 0;
}

void reset_calibration_accumulators()
{
	calibration_min_heating = 10000;
	calibration_max_heating = 0;
	calibration_min_appliance = 10000;
	calibration_max_appliance = 0;
}

void send_min_max_values_to_monitor()
{
	Serial.print("Min and Max heating: ");
	Serial.print(calibration_min_heating);
	Serial.print(", ");
	Serial.println(calibration_max_heating);
	Serial.print("Min and Max appliance: ");
	Serial.print(calibration_min_appliance);
	Serial.print(", ");
	Serial.println(calibration_max_appliance);
}

void send_heating_values_to_monitor()
{
	Serial.print("Heating new value: ");
	Serial.print(heating);
	Serial.print(", ");
	Serial.print(heating_mean);
	Serial.print(", ");
	Serial.print(threshold_lower_heating);
	Serial.print(", ");
	Serial.println(threshold_upper_heating);
}

void send_appliance_values_to_monitor()
{
	Serial.print("Appliance new value: ");
	Serial.print(appliance);
	Serial.print(", ");
	Serial.print(appliance_mean);
	Serial.print(", ");
	Serial.print(threshold_lower_appliance);
	Serial.print(", ");
	Serial.println(threshold_upper_appliance);
}

void log_entry(char entry_type, bool value) {
	String text_to_print;
	text_to_print = millis() + String(",") + String(entry_type) + "," + String(value) + "\r\n";

	if (entry_type == 'H') {
		heating_data_buffer += text_to_print;
	}
	else {
		myFile = SD.open("test.txt", FILE_WRITE);
		if (heating_data_buffer != "") {
			myFile.print(heating_data_buffer);
			heating_data_buffer = "";
		}
		myFile.print(text_to_print);
		myFile.close();
	}
}

void dump(int value) {
	String text_to_print;
	text_to_print = millis() + String(",") + String(value) + "\r\n";
	fullDump = SD.open("dump.txt", FILE_WRITE);
	fullDump.print(text_to_print);
	fullDump.close();
}

void serial_sync_time() {
	String newDateTime;
	String text_to_print;
	char d;

	while (Serial.available() >= dateTimeMsgLength) {
		if (Serial.read() == dateTimeMsgHeader) {
			for (int i = 0; i < dateTimeMsgLength; i++) {
				char c = Serial.read();
				newDateTime += c;
			}
			newDateTime.remove(newDateTime.length() - 1);
			text_to_print = millis() + String(",S,") + newDateTime;
			Serial.println(text_to_print);

			myFile = SD.open("test.txt", FILE_WRITE);
			myFile.println(text_to_print);
			myFile.close();
		}
	}
}

void binarize_heating_sensor_signal()
{
	if (heating && (heating_mean < threshold_lower_heating))
	{
		heating = false;
		send_heating_values_to_monitor();
		digitalWrite(HEATING_MONITOR_PIN, LOW);
		log_entry('H', heating);
	}
	else if (!heating && (heating_mean > threshold_upper_heating))
	{
		heating = true;
		send_heating_values_to_monitor();
		digitalWrite(HEATING_MONITOR_PIN, HIGH);
		log_entry('H', heating);
	}

}

void binarize_appliance_sensor_signal()
{
	if (appliance && (appliance_mean < threshold_lower_appliance))
	{
		appliance = false;
		send_appliance_values_to_monitor();
		digitalWrite(APPLIANCE_MONITOR_PIN, LOW);
		log_entry('A', appliance);
	}
	else if (!appliance && (appliance_mean > threshold_upper_appliance))
	{
		appliance = true;
		send_appliance_values_to_monitor();
		digitalWrite(APPLIANCE_MONITOR_PIN, HIGH);
		log_entry('A', appliance);
	}
}

void sampling_cycle()
{
	update_sensor_parameters();
	sample_complete = inside_short_term_sampling_period(last_sample_time);
	// Serial.println(sample_complete);
	if (sample_complete)
	{
		heating_mean = heating_running_total / number_samples_read;
		appliance_mean = appliance_running_total / number_samples_read;
		binarize_appliance_sensor_signal();
		if (perform_dump)
		{
			dump(heating_mean);
		}
		if (valid_sample(min_heating_sensor, max_heating_sensor, heating_mean, CONSISTENCY_TOLERANCE))
		{
			add_to_heating_consistency_set();
		}
		if (valid_sample(min_appliance_sensor, max_appliance_sensor, appliance_mean, CONSISTENCY_TOLERANCE))
		{
			add_to_appliance_consistency_set();
		}
		reset_short_term_accumulators();

		consistency_sample_count++;
		if (consistency_sample_count == CONSISTENCY_SAMPLES)
		{
			heating_mean = consistency_running_total_heating / CONSISTENCY_SAMPLES;
			appliance_mean = consistency_running_total_appliance / CONSISTENCY_SAMPLES;
			if (consistency_heating_sample_count == CONSISTENCY_SAMPLES)
			{
				if (valid_sample(consistency_min_heating, consistency_max_heating, heating_mean, CONSISTENCY_TOLERANCE))
				{
					binarize_heating_sensor_signal();
					add_to_heating_calibration_set();
				}
			}
			if (consistency_appliance_sample_count == CONSISTENCY_SAMPLES)
			{
				if (valid_sample(consistency_min_appliance, consistency_max_appliance, appliance_mean, CONSISTENCY_TOLERANCE))
				{
					add_to_appliance_calibration_set();
				}
			}
			reset_consistency_accumulators();
		}

		if (recalibrate(last_calibration_time))
		{
			perform_recalibration();
			reset_calibration_accumulators();
		}
		last_sample_time = millis();
	}
}

// The setup() function runs once each time the micro-controller starts
void setup()
{
	Serial.begin(115200);
	Serial.flush();

	if (!SD.begin(4))
	{
		Serial.println("initialization failed!");
		while (1);
	}

	Serial.println("Starting...");
	pinMode(HEATING_MONITOR_PIN, OUTPUT);
	pinMode(APPLIANCE_MONITOR_PIN, OUTPUT);
}

// Add the main program code into the continuous loop() function
void loop()
{
	sampling_cycle();
}

