#include <stdio.h> // For input/output
#include <stdlib.h> // For general utilities (malloc, free)
#include <math.h> // For matemathical functions (sqrt, pow)

// --- Structure Definitions (To be developed, starting with 1 of them).

// Structures for a conductor
typedef struct{
		int sc_gauge_awg_kcmil; // Conductor gauge in AWG/kcmil
		float sc_ampacity_at_75c_amps; // Ampacity at 75°C conductor temp
} Conductor;

// --- Function Prootypes --- (To be developed, i will include the function in Main)
float calculate_load_current_amps(float arg_power_watts, float arg_voltage_volts, float arg_power_factor, int arg_phase_count); // Initial current calculation
float calculate_adjusted_current_amps(float arg_load_current_amps, float arg_temp_correction_factor, float arg_num_cond_adjustment_factor); // 

// --- Main Function ---
int main() {
	// Local variable for user input
	int local_power_watts; // Watts imput
	int local_voltage_volts;
	float local_power_factor; // Decimal from 0 to 1
	int local_phase_count; // 1, 2 or 3 phases.
	float local_circuit_length_meters;

	// Local variables calculated
	float local_load_current_amps;
	float local_temp_correction_factor;
	float local_num_cond_adjustment_factor;
	float local_adjusted_current_amps;
	int local_suggested_gauge_awg_kcmil;

	// --- A little presentation ---
	printf("\n\nElectrical Conductor Selection Program (NOM-001-SEDE-2012)\n");
	printf("----------------CS50 PROJECT by @Horfezyn----------------\n\n");

	// --- User Input ---
	printf("--- Enter Circuit Parameters ---\n");
    do{
        printf("Enter power (Watts, e.g., 10000): ");
        scanf("%f", &local_power_watts);
    }
    while (local_power_watts <= 0); //Validate that the power is a positive number
    do{
        printf("Enter system voltage (Volts, e.g., 220): ");
        scanf("%f", &local_voltage_volts);
    }
    while (local_voltage_volts <= 0); //Validate that the voltage is a positive number
    do{
        printf("Enter power factor (e.g., 0.85): ");
        scanf("%f", &local_power_factor);
    }
    while (local_power_factor <= 0); //Validate that power factor is a positive number
    do{
        printf("Enter number of phases (1 or 3): ");
        scanf("%d", &local_phase_count);
    }
    while (local_phase_count <= 0); //Validate that phase count is a positive number
    do{
        printf("Enter circuit length (meters, e.g., 50): ");
        scanf("%f", &local_circuit_length_meters);
    }
    while (local_circuit_length_meters <= 0); //Validate that phase count is a positive number

	// --- Calculations ---
	printf("\n--- Performing Calculations ---\n");
    // Current of the load
    local_load_current_amps = calculate_load_current_amps(local_power_watts, local_voltage_volts, local_power_factor, local_phase_count);
    if (local_load_current_amps < 0) { // Check for calculation errors
        printf("Error calculating load current. Exiting.\n");
        return 1;
    }
    printf("Calculated Load Current (Ib): %.2f Amps\n", local_load_current_amps);

    // Correction factors
    local_temp_correction_factor = 1.0; // Assuming 30C ambient, no corrction
    local_num_cond_adjustment_factor = 0.8; // Assuming 3 conductors, 80% adjustment (from NOM 310-15(B)(3)(a))

    local_adjusted_current_amps = calculate_adjusted_current_amps(local_load_current_amps, local_temp_correction_factor, local_num_cond_adjustment_factor);
    printf("Adjusted Design Current (Iz): %.2f Amps\n", local_adjusted_current_amps);
	return 0;
}

// --- Function Implementations (The hard part) ---

// Current calculation
float calculate_load_current_amps(float arg_power_watts, float arg_voltage_volts, float arg_power_factor, int arg_phase_count){
    if ( arg_voltage_volts == 0 || arg_power_factor == 0) {
        printf("Error: Voltage or Power Factor cannot be zero for current calculation.\n");
        return 0;
    }
    if (arg_phase_count == 1) { // Single-phase case
        return arg_power_watts / (arg_voltage_volts * arg_power_factor);
    } else if (arg_phase_count == 3) { // Three-phase case
        return arg_power_watts / (sqrt(3.0f) * arg_voltage_volts * arg_power_factor);
    }
        printf("Error: Unsupported number of phases (%d) for current calculation.\n",arg_phase_count);
        return -1;
}

// Current using correction factors
float calculate_adjusted_current_amps(float arg_load_current_amps, float arg_temp_correction_factor, float arg_num_cond_adjustment_factor){
    if (arg_temp_correction_factor == 0 || arg_num_cond_adjustment_factor == 0) {
        printf("Error: Correction factors cannot be zero for adjusted current calculation.\n");
        return -1;
    }
    return arg_load_current_amps / (arg_temp_correction_factor * arg_num_cond_adjustment_factor);
}