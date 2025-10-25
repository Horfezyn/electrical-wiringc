#include <stdio.h> // For input/output
#include <stdlib.h> // For general utilities (malloc, free)
#include <math.h> // For matemathical functions (sqrt, pow)
#include <string.h> // For string manipulation (strcpy, strtok, strcmp)
#include <strings.h> // For strcasecmp
#include <windows.h> // For gotoxy functionality on Windows console

// --- Error Codes ---
#define SUCCESS                 (0)
#define ERROR_FILE_OPEN         (-1)
#define ERROR_INVALID_INPUT     (-2)
#define ERROR_DATA_NOT_FOUND    (-3)
#define ERROR_PHASE_COUNT       (-4)
#define ERROR_DIVIDE_BYZERO     (-5)

// --- Macro for print the error ---
#define REPORT_ERROR(message) fprintf(stderr, "Error: %s\n",message)

// --- Structure Definitions ---

// Structures for a conductor
typedef struct sc_conductor{
    int sc_gauge_awg_kcmil; // Conductor gauge in AWG/kcmil
    char sc_insulation_type[32]; // Insultation type
    float sc_ampacity_at_75c_amps; // Ampacity at 75°C conductor temp
    float sc_ampacity_at_90c_amps; // Ampacity at 90°C
    float sc_area_mm2;
    float sc_resistance_km;
    float sc_reactance_km;
} Conductor;

// Structures for temperature correction factors
typedef struct s_temp_correction{
    int stc_ambient_temp;
    float stc_correction_factor;
} TempCorrectionFactor;

// Structure for number of conductors adjusment factor
typedef struct s_numc_adjustment{
    int snca_conductor_count;
    float snca_adjustment_factor;
}NumCondFactor;

// Structure for the conduit
typedef struct s_conduit{
    char sc_conduit_type[50];
    float sc_diameter_inches;
    float sc_internal_area_mm2;
}Conduit;

// --- Global variables ---
Conductor g_conductor_data_g_list[20]; // General structure filled with DATA from CSV files, max. 20 types of conductor.
int g_conductor_count = 0; // For the number of conductors in the csv file.

TempCorrectionFactor g_temp_factors_g_list[20]; // General structure filled with DATA from CSV file, max 20 types of factors
int g_temp_correction_count = 0; // For the number of correction factors in the CSV file

NumCondFactor g_ncond_adj_g_list[20]; // General structure filled with DATA from CSV file, max 20 types of factors
int g_ncond_adj_count = 0; // For the number of correction factors in the CSV file.

Conduit g_conduit_data_g_list[20]; // General structure filled with DATA from CSV file, max 20 types of factors
int g_conduit_count = 0;

// --- Function Prototypes ---
// NEW: ASCII Interface Helpers
void gotoxy(int x, int y);
void draw_ascii_box(int x1, int y1, int x2, int y2);
void display_calculation_results_ascii(
    float local_voltage_volts,
    float local_load_current_amps,
    float local_adjusted_current_amps,
    int local_suggested_gauge_awg_kcmil,
    float conductor_area,
    float conductor_resistance,
    float conductor_reactance,
    float local_voltage_drop_volts,
    float local_conduit_area,
    float local_total_conductor_area
);


// For data loading
int load_ampacity_table_data(const char *arg_file_name_ptr); // Function to load the ampacity table
int load_temperature_correction(const char *arg_file_name_ptr); // Function to load the temperature correction factors.
int load_nconductor_factor(const char *arg_file_name_ptr); // Function to load the number of conductor, correction factor.
int load_conduit_fill_data(const char *arg_file_name_ptr); // function to load the properties of conduit.

// Calculation base
float calculate_load_current_amps(float arg_power_watts, float arg_voltage_volts, float arg_power_factor, int arg_phase_count); // Initial current calculation
float calculate_adjusted_current_amps(float arg_load_current_amps, float arg_temp_correction_factor, float arg_num_cond_adjustment_factor); //
float calculate_voltage_drop_volts(float arg_load_Current_amps, float arg_circuit_lenght_meters, float arg_resistance_per_km, float arg_reactance_per_km, float arg_power_factor, int arg_phase_count);

// Data retrieval
float get_temp_correction_factor(int arg_ambient_temp);
float get_ncond_adj_factor(int arg_conductor_count);
float get_conductor_resistance_km(int arg_gauge_awg_kcmil);
float get_conductor_reactance_km(int arg_gauge_awg_kcmil);
float get_conductor_mm2(int arg_gauge_awg_kcmil);
float get_conduit_area(const char *arg_conduit_type_ptr, float arg_conduit_diameter_nominal_inches);

// Selection and validation
int get_suggested_gauge_awg_kcmil(float arg_adjusted_current_amps, const char *arg_insulation_type_ptr, int arg_temp_rating);
int check_conduit_fill(float arg_conductor_area, int arg_conductor_count, const char *arg_conduit_type_ptr, float arg_conduit_diameter_nominal_inches);

// --- Main Function ---
int main() {
    // Local variable for user input
    float local_power_watts; // Watts imput
    float local_voltage_volts;
    float local_power_factor; // Decimal from 0 to 1
    int local_phase_count; // 1, 2 or 3 phases.
    float local_circuit_length_meters;
    int local_ambient_temperature;
    int local_conductor_count;
    char local_insulation_type[32];
    char local_conduit_type[32];
    float local_conduit_diameter;
    int local_insulation_temperature_rating;
    float local_voltage_drop_volts = (float)ERROR_DATA_NOT_FOUND; // Initialize with error
    float conductor_area = (float)ERROR_DATA_NOT_FOUND;
    float conductor_resistance = (float)ERROR_DATA_NOT_FOUND;
    float conductor_reactance = (float)ERROR_DATA_NOT_FOUND;
    float local_conduit_area = (float)ERROR_DATA_NOT_FOUND;
    float local_total_conductor_area = (float)ERROR_DATA_NOT_FOUND;
    
    // Local variables calculated
    float local_load_current_amps = (float)ERROR_DATA_NOT_FOUND;
    float local_temp_correction_factor;
    float local_num_cond_adjustment_factor;
    float local_adjusted_current_amps = (float)ERROR_DATA_NOT_FOUND;
    int local_suggested_gauge_awg_kcmil = ERROR_DATA_NOT_FOUND;
    int return_code;    // To return values from functions.
    int local_conduit_fill_check_result;

    // --- A little presentation ---
    printf("\n\nElectrical Conductor Selection Program (NOM-001-SEDE-2012)\n");
    printf("----------------CS50 PROJECT by @Horfezyn----------------\n\n");

    // --- Data loading ---
    printf("--- Loading NOM Data ---\n");

    return_code = load_ampacity_table_data("ampacity_data.csv");
    if (return_code != SUCCESS){
        printf("Error loading ampacity data. Exiting program...");
        return return_code;
    }

    return_code = load_temperature_correction("temp_correction_data.csv");
    if (return_code != SUCCESS){
        printf("Error loading temperature correction data. Exiting program...");
        return return_code;
    }
    return_code = load_nconductor_factor("num_cond_adj_data.csv");
    if (return_code != SUCCESS){
        printf("Error loading number of conductor adjustmen data. Exiting program...");
        return return_code;
    }
    return_code = load_conduit_fill_data("conduit_fill_data.csv");
    if (return_code != SUCCESS){
        printf("Error loading conduit fill data. Exiting program...");
        return return_code;
    }
    printf("--- Data Loading Complete ---\n\n");

    // --- User Input ---
    printf("--- Enter Circuit Parameters ---\n");
    do{
        printf("Enter power (Watts, e.g., 10000): ");
        if (scanf("%f", &local_power_watts) != 1) {
            REPORT_ERROR("Invalid input for power. Please enter a number.");
            while (getchar() != '\n'); // Clear input buffer
            local_power_watts = 0; // Set to 0 to re-enter loop
        }
    } while (local_power_watts <= 0); //Validate that the power is a positive number
    do{
        printf("Enter system voltage (Volts, e.g., 220): ");
        if (scanf("%f", &local_voltage_volts) != 1) {
            REPORT_ERROR("Invalid input for voltage. Please enter a number.");
            while (getchar() != '\n');
            local_voltage_volts = 0;
        }
    } while (local_voltage_volts <= 0); //Validate that the voltage is a positive number
    do{
        printf("Enter power factor (e.g., 0.85): ");
        if (scanf("%f", &local_power_factor) != 1) {
            REPORT_ERROR("Invalid input for power factor. Please enter a number.");
            while (getchar() != '\n');
            local_power_factor = 0;
        }
    } while (local_power_factor <= 0); //Validate that power factor is a positive number
    do{
        printf("Enter number of phases (1 or 3): ");
        if (scanf("%d", &local_phase_count) != 1) {
            REPORT_ERROR("Invalid input for phases. Please enter 1 or 3.");
            while (getchar() != '\n');
            local_phase_count = 0;
        }
    } while (local_phase_count <= 0); //Validate that phase count is a positive number
    do{
        printf("Enter circuit length (meters, e.g., 50): ");
        if (scanf("%f", &local_circuit_length_meters) != 1) {
            REPORT_ERROR("Invalid input for circuit length. Please enter a number.");
            while (getchar() != '\n');
            local_circuit_length_meters = 0;
        }
    } while (local_circuit_length_meters <= 0); //Validate that phase count is a positive number
    do{
        printf("Enter ambient temperature (celsius, e.g., 30): ");
        if (scanf("%d", &local_ambient_temperature) != 1) {
            REPORT_ERROR("Invalid input for ambient temperature. Please enter an integer.");
            while (getchar() != '\n');
            local_ambient_temperature = 0;
        }
    } while (local_ambient_temperature <= 0); //Validate that ambient temperature is a value
    do{
        printf("Enter number of current-carrying conductors in conduit (e.g., 3): ");
        if (scanf("%d", &local_conductor_count) != 1) {
            REPORT_ERROR("Invalid input for conductor count. Please enter an integer.");
            while (getchar() != '\n');
            local_conductor_count = 0;
        }
    } while (local_conductor_count <= 0);

    printf("Enter insulation type (e.g., THHN, THW): "); // Insultation type
    scanf("%s", local_insulation_type);
    do {
        printf("Enter insulation temperature rating (75 or 90): ");
        if (scanf("%d", &local_insulation_temperature_rating) != 1 || (local_insulation_temperature_rating != 75 && local_insulation_temperature_rating != 90)) {
            REPORT_ERROR("Invalid input for insulation temperature rating. Please enter 75 or 90.");
            while (getchar() != '\n');
            local_insulation_temperature_rating = 0; // Set to 0 to re-enter loop
        }
    } while (local_insulation_temperature_rating != 75 && local_insulation_temperature_rating != 90);

    printf("Enter conduit type (e.g., EMT, PVC): "); // Conduit type
    scanf("%s", local_conduit_type);

    do{
        printf("Enter conduit nominal diameter (e.g., 0.5 for 1/2, 0.75 for 3/4): ");
        if (scanf("%f", &local_conduit_diameter) != 1) {
            REPORT_ERROR("Invalid input for conduit diameter. Please enter a number.");
            while (getchar() != '\n');
            local_conduit_diameter = 0;
        }
    } while (local_conduit_diameter <= 0); //Validate that phase count is a positive number

    printf("------------------------------\n\n");

    // --- Calculations ---
    // Clear screen before showing detailed calculations
    system("cls");

    // Current of the load
    local_load_current_amps = calculate_load_current_amps(local_power_watts, local_voltage_volts, local_power_factor, local_phase_count);
    if (local_load_current_amps < 0) {
        printf("Error calculating load current. Error code: %d. Exiting.\n", (int)local_load_current_amps);
        return (int)local_load_current_amps;
    }

    // Correction factors
    local_temp_correction_factor = get_temp_correction_factor(local_ambient_temperature);
    if (local_temp_correction_factor < 0){
        printf("Error: Temperature correction factor not found for %dC. Error code: %d. Exiting\n", local_ambient_temperature, (int)local_temp_correction_factor);
        return (int) local_temp_correction_factor;
    }
    local_num_cond_adjustment_factor = get_ncond_adj_factor(local_conductor_count);
    if (local_num_cond_adjustment_factor < 0){
        printf("Error: Number of conductors adjustment factor not found for %d conductors. Error code: %d. Exiting \n", local_conductor_count, (int)local_num_cond_adjustment_factor);
        return (int) local_num_cond_adjustment_factor;
    }

    // Adjusted current
    local_adjusted_current_amps = calculate_adjusted_current_amps(local_load_current_amps, local_temp_correction_factor, local_num_cond_adjustment_factor);
    if (local_adjusted_current_amps < 0) { // Check for calculation errors
        printf("Error calculating adjusted current. Error code: %d. Exiting.\n", (int)local_adjusted_current_amps);
        return (int)local_adjusted_current_amps;
    }

    // Suggested gauge
    local_suggested_gauge_awg_kcmil = get_suggested_gauge_awg_kcmil(local_adjusted_current_amps, local_insulation_type, local_insulation_temperature_rating);
    
    // Conductor properties and checks
    if (local_suggested_gauge_awg_kcmil > 0 || local_suggested_gauge_awg_kcmil == 110 || local_suggested_gauge_awg_kcmil == 120 || local_suggested_gauge_awg_kcmil == 130 || local_suggested_gauge_awg_kcmil == 140) { // Check for valid gauge returned
        conductor_area = get_conductor_mm2(local_suggested_gauge_awg_kcmil);
        conductor_resistance = get_conductor_resistance_km(local_suggested_gauge_awg_kcmil);
        conductor_reactance = get_conductor_reactance_km(local_suggested_gauge_awg_kcmil);

        if (conductor_area != (float)ERROR_DATA_NOT_FOUND && conductor_resistance != (float)ERROR_DATA_NOT_FOUND && conductor_reactance != (float)ERROR_DATA_NOT_FOUND) {
            // Voltage drop section
            local_voltage_drop_volts = calculate_voltage_drop_volts(local_load_current_amps, local_circuit_length_meters, conductor_resistance, conductor_reactance, local_power_factor, local_phase_count);

            // Conduit area for fill check
            local_conduit_area = get_conduit_area(local_conduit_type, local_conduit_diameter);
            local_total_conductor_area = conductor_area * local_conductor_count;

            // NOTE: The conduit fill check printouts are now handled inside display_calculation_results_ascii
            // The return value of check_conduit_fill is discarded here to avoid duplicate printing.
            // local_conduit_fill_check_result = check_conduit_fill(...);
            
        } else {
            REPORT_ERROR("Could not retrieve all properties for the suggested conductor gauge.");
        }
    } else {
        REPORT_ERROR("No valid conductor gauge was suggested.");
    }
    
    // Display all results using ASCII formatting
    display_calculation_results_ascii(
        local_voltage_volts,
        local_load_current_amps,
        local_adjusted_current_amps,
        local_suggested_gauge_awg_kcmil,
        conductor_area,
        conductor_resistance,
        conductor_reactance,
        local_voltage_drop_volts,
        local_conduit_area,
        local_total_conductor_area
    );

    // Pausa antes de cerrar el .exe
    // Moved the final printouts to the end of the calculations for visual clarity
    gotoxy(1, 25);
    printf("\n--- Calculations Complete ---\n");
    printf("Thank you for using the Electrical Conductor Selection Program. Goodbye!\n");
    printf("\nPress Enter to exit...");
    while (getchar() != '\n');
    getchar();
    return SUCCESS;
}

// --- Function Implementations (The hard part) ---

// NEW: Gotoxy function for console cursor control (Windows specific)
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// NEW: Function to Draw an ASCII Box
void draw_ascii_box(int x1, int y1, int x2, int y2) {
    int i;
    
    // Corners
    gotoxy(x1, y1); printf("+");
    gotoxy(x2, y1); printf("+");
    gotoxy(x1, y2); printf("+");
    gotoxy(x2, y2); printf("+");

    // Top and Bottom Borders
    for (i = x1 + 1; i < x2; i++) {
        gotoxy(i, y1); printf("-");
        gotoxy(i, y2); printf("-");
    }

    // Side Borders
    for (i = y1 + 1; i < y2; i++) {
        gotoxy(x1, i); printf("|");
        gotoxy(x2, i); printf("|");
    }
}

// NEW: Function to display the calculation results using ASCII formatting
void display_calculation_results_ascii(
    float local_voltage_volts,
    float local_load_current_amps,
    float local_adjusted_current_amps,
    int local_suggested_gauge_awg_kcmil,
    float conductor_area,
    float conductor_resistance,
    float conductor_reactance,
    float local_voltage_drop_volts,
    float local_conduit_area,
    float local_total_conductor_area
) {
    // Console area dimensions
    int box_x1 = 5;
    int box_y1 = 2;
    int box_x2 = 90;
    int box_y2 = 23;
    
    draw_ascii_box(box_x1, box_y1, box_x2, box_y2);
    gotoxy(box_x1 + 2, box_y1); printf("--- ELECTRICAL CALCULATION RESULTS ---");
    
    int current_y = box_y1 + 2;

    // --- 1. CURRENT CALCULATIONS ---
    gotoxy(box_x1 + 2, current_y); printf("LOAD CURRENT (Ib): %.2f Amps", local_load_current_amps);
    current_y++;
    gotoxy(box_x1 + 2, current_y); printf("ADJUSTED DESIGN CURRENT (Iz): %.2f Amps", local_adjusted_current_amps);
    current_y += 2;

    // --- 2. CONDUCTOR SELECTION ---
    char gauge_label[20];
    if (local_suggested_gauge_awg_kcmil >= 110 && local_suggested_gauge_awg_kcmil <= 140) {
        int awg_number = local_suggested_gauge_awg_kcmil - 100;
        sprintf(gauge_label, "%d/0 AWG", awg_number);
    } else if (local_suggested_gauge_awg_kcmil >= 250) {
        sprintf(gauge_label, "%d kcmil", local_suggested_gauge_awg_kcmil);
    } else if (local_suggested_gauge_awg_kcmil > 0) {
        sprintf(gauge_label, "%d AWG", local_suggested_gauge_awg_kcmil);
    } else {
        sprintf(gauge_label, "N/A (Error: %d)", ERROR_DATA_NOT_FOUND);
    }

    gotoxy(box_x1 + 2, current_y); printf("SUGGESTED GAUGE: %s", gauge_label);
    current_y++;
    gotoxy(box_x1 + 2, current_y); printf("Area: %.2f mm^2 / Resistance: %.4f Ohm/km / Reactance: %.4f Ohm/km", 
                                          conductor_area, conductor_resistance, conductor_reactance);
    current_y += 2;
    
    // --- 3. VOLTAGE DROP CHECK ---
    float max_allowed_vd = local_voltage_volts * 0.03f; // 3% voltage drop
    
    gotoxy(box_x1 + 2, current_y); printf("VOLTAGE DROP (VD): %.2f Volts (Max Allowed: %.2fV)", local_voltage_drop_volts, max_allowed_vd);
    current_y++;
    gotoxy(box_x1 + 2, current_y); 
    if (local_voltage_drop_volts > max_allowed_vd) {
        printf(">> WARNING: VD exceeds recommended 3%% limit.");
    } else if (local_voltage_drop_volts >= 0) {
        printf(">> RESULT: Voltage Drop is acceptable.");
    } else {
        printf(">> ERROR: Voltage Drop calculation failed.");
    }
    current_y += 2;

    // --- 4. CONDUIT FILL CHECK ---
    gotoxy(box_x1 + 2, current_y); printf("CONDUIT FILL CHECK:");
    current_y++;
    
    if (local_conduit_area > 0) {
        float local_fill_percentage = (local_total_conductor_area / local_conduit_area) * 100.0f;
        
        gotoxy(box_x1 + 2, current_y); printf("Total Conductor Area: %.2f mm^2 / Conduit Area: %.2f mm^2", 
                                              local_total_conductor_area, local_conduit_area);
        current_y++;
        gotoxy(box_x1 + 2, current_y); printf("Fill Percentage: %.2f %% (Limit: 40.00 %%)", local_fill_percentage);
        current_y++;
        gotoxy(box_x1 + 2, current_y);
        
        if (local_fill_percentage > 40.0f) {
            printf(">> WARNING: Conduit fill exceeds 40%% limit. A larger conduit is required.");
        } else {
            printf(">> RESULT: Conduit fill is within acceptable limits.");
        }
    } else {
        gotoxy(box_x1 + 2, current_y); printf(">> ERROR: Conduit Area not found in data for the given type/diameter.");
    }
    
    // Set cursor below the box for the final prompt
    gotoxy(1, box_y2 + 2); 
}

// Auxiliar function to remove the possible '\r' (carriage return) from Windows CSV files
void remove_cr(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\r') {
        s[len - 1] = '\0';
    }
}

// Ampacity CSV File
int load_ampacity_table_data(const char *arg_file_name_ptr) {
    FILE *file_ptr = fopen(arg_file_name_ptr,"r");
    if (!file_ptr){
        REPORT_ERROR("Failed to open ampacity_data.csv");
        return ERROR_FILE_OPEN;
    }

    char line[128]; 
    fgets(line, sizeof(line),file_ptr); // Skip header

    g_conductor_count = 0;
    while (fgets(line, sizeof(line), file_ptr) != NULL && g_conductor_count < 20){
        char temp_line[256];
        strcpy(temp_line, line);
        char *token;

        // 1. Gauge_AWG_kcmil
        token = strtok(temp_line, ",");
        if (token) {
            g_conductor_data_g_list[g_conductor_count].sc_gauge_awg_kcmil = atoi(token);
        } else {
            REPORT_ERROR("Invalid format in ampacity_data.csv: Gauge not found.");
            fclose(file_ptr);
            return ERROR_INVALID_INPUT;
        }
        // 2. Insulation_Type
        token = strtok(NULL, ",");
        if (token) {
            size_t len = strlen(token);
            if (len > 0 && token[len - 1] == '\n') {
                token[len - 1] = '\0';
            }
            strcpy(g_conductor_data_g_list[g_conductor_count].sc_insulation_type, token);
        } else {
            REPORT_ERROR("Invalid format in ampacity_data.csv: Insulation Type not found.");
            fclose(file_ptr);
            return ERROR_INVALID_INPUT;
        }
        // 3. Ampacity_75C
        token = strtok(NULL, ",");
        if (token) {
            g_conductor_data_g_list[g_conductor_count].sc_ampacity_at_75c_amps = atof(token);
        } else {
            REPORT_ERROR("Invalid format in ampacity_data.csv: Ampacity 75C not found.");
            fclose(file_ptr);
            return ERROR_INVALID_INPUT;
        }
        // 4. Ampacity_90C
        token = strtok(NULL, ",");
        if (token) {
            g_conductor_data_g_list[g_conductor_count].sc_ampacity_at_90c_amps = atof(token);
        } else {
            REPORT_ERROR("Invalid format in ampacity_data.csv: Ampacity 90C not found.");
            fclose(file_ptr);
            return ERROR_INVALID_INPUT;
        }
        // 5. Area_mm2
        token = strtok(NULL, ",");
        if (token) {
            g_conductor_data_g_list[g_conductor_count].sc_area_mm2 = atof(token);
        } else {
            REPORT_ERROR("Invalid format in ampacity_data.csv: Area mm2 not found.");
            fclose(file_ptr);
            return ERROR_INVALID_INPUT;
        }
        // 6. Resistance_ohm_km
        token = strtok(NULL, ",");
        if (token) {
            g_conductor_data_g_list[g_conductor_count].sc_resistance_km = atof(token);
        } else {
            REPORT_ERROR("Invalid format in ampacity_data.csv: Resistance not found.");
            fclose(file_ptr);
            return ERROR_INVALID_INPUT;
        }
        // 7. Reactance_ohm_km
        token = strtok(NULL, "\n");
        if (token) {
            g_conductor_data_g_list[g_conductor_count].sc_reactance_km = atof(token);
        } else {
            REPORT_ERROR("Invalid format in ampacity_data.csv: Reactance not found.");
            fclose(file_ptr);
            return ERROR_INVALID_INPUT;
        }
        g_conductor_count++;
    }

    fclose(file_ptr);
    printf("Action: Loaded %d ampacity data entries from %s.\n", g_conductor_count, arg_file_name_ptr);
    return SUCCESS;
}

// Temperature Corection Factor CSV File
int load_temperature_correction(const char *arg_file_name_ptr){
    FILE *file_ptr = fopen(arg_file_name_ptr, "r");
    if (!file_ptr){
        REPORT_ERROR("Error opening temp_correction_data.csv\n");
        return ERROR_FILE_OPEN;
    }

    char line[128]; // Limit for each line.
    fgets(line, sizeof(line), file_ptr);

    g_temp_correction_count = 0;
    while(fgets(line, sizeof(line), file_ptr) && g_temp_correction_count < 20){ // 20 is the max number of correction factors
        char *token;
        token = strtok(line, ",");
        if (token){
            g_temp_factors_g_list[g_temp_correction_count].stc_ambient_temp = atoi(token);
        } else{
            REPORT_ERROR("Invalid formatin temp_correction_data.csv: Ambient temp not found.");
            fclose(file_ptr);
            return ERROR_INVALID_INPUT;
        }
        token = strtok(NULL, "\n");
        if (token){
            g_temp_factors_g_list[g_temp_correction_count].stc_correction_factor = atof(token);
        } else {
            REPORT_ERROR("Invalid format in temp_correction_data.csv: Correction factor not found.");
            return ERROR_INVALID_INPUT;
        }
        g_temp_correction_count++;
    }
    fclose(file_ptr);
    printf("Action: Loaded %d temperature correction factors from %s.\n", g_temp_correction_count, arg_file_name_ptr);
    return SUCCESS;
}

// Conduit fill data CSV
int load_conduit_fill_data(const char *arg_file_name_ptr){
    FILE *file_ptr = fopen(arg_file_name_ptr, "r");
    if(!file_ptr){
        REPORT_ERROR("Error loading conduit_fill_data.csv\n");
        return ERROR_FILE_OPEN;
    }
    char line[128];
    fgets(line, sizeof(line), file_ptr);

    g_conduit_count = 0;
    while(fgets(line, sizeof(line), file_ptr) && g_conduit_count < 20){ // Corrected the limit to 20 to match global array size
        char *token;
        token = strtok(line, ",");
        if(token){
            strcpy(g_conduit_data_g_list[g_conduit_count].sc_conduit_type, token);
        } else{
            REPORT_ERROR("Invalid format in conduit_fill_data.csv: Conduit type not found\n");
            fclose(file_ptr);
            return ERROR_INVALID_INPUT;
        }
        token = strtok(NULL, ",");
        if(token){
            g_conduit_data_g_list[g_conduit_count].sc_diameter_inches = atof(token);
        }else {
            REPORT_ERROR ("Invalid format in conduit_fill_data.csv: Diameter not found");
            fclose(file_ptr);
            return ERROR_INVALID_INPUT;
        }
        token = strtok(NULL, ",");
        if(token){
            g_conduit_data_g_list[g_conduit_count].sc_internal_area_mm2 = atof(token);
        }else{
            REPORT_ERROR("Invalid format in conduit_fill_data.csv: Area not found");
            fclose(file_ptr);
            return ERROR_INVALID_INPUT;
        }
        g_conduit_count++;
    }
    fclose(file_ptr);
    printf("Action: Loaded %d conduit fill data entries from %s. \n", g_conduit_count,arg_file_name_ptr);
    return SUCCESS;
}

// Number of conductors, correction factor
int load_nconductor_factor(const char *arg_file_name_ptr){
    FILE *file_ptr = fopen(arg_file_name_ptr, "r");
    if (!file_ptr) {
        REPORT_ERROR("Error opening num_cond_ad_data.csv\n");
        return ERROR_FILE_OPEN;
    }
    char line[100];
    fgets(line, sizeof(line), file_ptr); // Skip header
    g_ncond_adj_count = 0;
    while (fgets(line, sizeof(line), file_ptr) && g_ncond_adj_count < 20) {
        char *token;
        token = strtok(line, ",");
        if (token) {
            g_ncond_adj_g_list[g_ncond_adj_count].snca_conductor_count = atoi(token);
        } else{
            REPORT_ERROR("Invalid format in num_cond_adj_data.csv: Conductor count not found.");
            fclose(file_ptr);
            return ERROR_INVALID_INPUT;
        }
        token = strtok(NULL, "\n");
        if (token) {
            g_ncond_adj_g_list[g_ncond_adj_count].snca_adjustment_factor = atof(token);
        } else{
            REPORT_ERROR("Invalid format in num_cond_adj_data.csv: Adjustment factor not found.");
            fclose(file_ptr);
            return ERROR_INVALID_INPUT;
        }
        g_ncond_adj_count++;
    }
    fclose(file_ptr);
    printf("Action: Loaded %d number of conductors adjustment factors from %s.\n", g_ncond_adj_count, arg_file_name_ptr);
    return SUCCESS;
}

// Current calculation
float calculate_load_current_amps(float arg_power_watts, float arg_voltage_volts, float arg_power_factor, int arg_phase_count){
    if ( arg_voltage_volts == 0 || arg_power_factor == 0) {
        REPORT_ERROR("Voltage or Power Factor cannot be zero for current calculation.\n");
        return (float)ERROR_DIVIDE_BYZERO;
    }
    if (arg_phase_count == 1) { // Single-phase case
        return arg_power_watts / (arg_voltage_volts * arg_power_factor);
    } else if (arg_phase_count == 3) { // Three-phase case
        return arg_power_watts / (sqrt(3.0f) * arg_voltage_volts * arg_power_factor);
    }
        REPORT_ERROR("Unsupported number of phases for current calculation.");
        return (float)ERROR_PHASE_COUNT;
}

// Current using correction factors
float calculate_adjusted_current_amps(float arg_load_current_amps, float arg_temp_correction_factor, float arg_num_cond_adjustment_factor){
    if (arg_temp_correction_factor == 0 || arg_num_cond_adjustment_factor == 0) {
        REPORT_ERROR("Correction factors cannot be zero for adjusted current calculation.\n");
        return (float)ERROR_DIVIDE_BYZERO;
    }
    return arg_load_current_amps / (arg_temp_correction_factor * arg_num_cond_adjustment_factor);
}

// Suggested gauge only using the adjustmen current
int get_suggested_gauge_awg_kcmil(float arg_adjusted_current_amps,const char *arg_insulation_type_ptr, int arg_temp_rating){
    // printf("Action: Getting suggested gauge for %.2f Amps with insulation %s and temperature rating %dC.\n", arg_adjusted_current_amps, arg_insulation_type_ptr, arg_temp_rating); // Removed original printout
    
    if (arg_adjusted_current_amps <=0){
        REPORT_ERROR("Adjusted current must be positive to find a gauge.");
        return ERROR_INVALID_INPUT;
    }
    for (int i = 0; i < g_conductor_count; i++){
            float ampacity_to_check;
            if (arg_temp_rating == 90) {
                ampacity_to_check = g_conductor_data_g_list[i].sc_ampacity_at_90c_amps;
            } else { // Default to 75C if not 90C or if invalid input
                ampacity_to_check = g_conductor_data_g_list[i].sc_ampacity_at_75c_amps;
            }

            if (ampacity_to_check >= arg_adjusted_current_amps){
                return g_conductor_data_g_list[i].sc_gauge_awg_kcmil;
            }
    }
    REPORT_ERROR("No conductor gauge found for the adjusted current");
    return ERROR_DATA_NOT_FOUND;
}

// Temperature correction factors based on the ambient temp
float get_temp_correction_factor(int arg_ambient_temp){
    for (int i = 0; i < g_temp_correction_count; i++){
        if (g_temp_factors_g_list[i].stc_ambient_temp == arg_ambient_temp){
            return g_temp_factors_g_list[i].stc_correction_factor;
        }
    }
    REPORT_ERROR("Temperature correction factor not found for the ambient temperature.");
    return  (float)ERROR_DATA_NOT_FOUND;
}

// Number of conductors factors based on user´s input
float get_ncond_adj_factor(int arg_conductor_count){
    for (int i = 0; i < g_ncond_adj_count; i++){
        if (g_ncond_adj_g_list[i].snca_conductor_count == arg_conductor_count){
            return g_ncond_adj_g_list[i].snca_adjustment_factor;
        }
    }
    REPORT_ERROR("Number of conductors adjustemnt factor not found for the given count.");
    return (float)ERROR_DATA_NOT_FOUND;
}
// Resistance for the conductor calculated
float get_conductor_resistance_km(int arg_gauge_awg_kcmil) {
    for (int i = 0; i < g_conductor_count; i++) {
        if (g_conductor_data_g_list[i].sc_gauge_awg_kcmil == arg_gauge_awg_kcmil) {
            return g_conductor_data_g_list[i].sc_resistance_km;
        }
    }
    REPORT_ERROR("Conductor resistance not found for the given gauge.");
    return (float)ERROR_DATA_NOT_FOUND;
}
// Reactance for the conductor calculated
float get_conductor_reactance_km(int arg_gauge_awg_kcmil) {
    for (int i = 0; i < g_conductor_count; i++) {
        if (g_conductor_data_g_list[i].sc_gauge_awg_kcmil == arg_gauge_awg_kcmil) {
            return g_conductor_data_g_list[i].sc_reactance_km;
        }
    }
    REPORT_ERROR("Conductor reactance not found for the given gauge.");
    return (float)ERROR_DATA_NOT_FOUND;
}
// Area in mm2 for the conductor calculated
float get_conductor_mm2(int arg_gauge_awg_kcmil) {
    for (int i = 0; i < g_conductor_count; i++) {
        if (g_conductor_data_g_list[i].sc_gauge_awg_kcmil == arg_gauge_awg_kcmil) {
            return g_conductor_data_g_list[i].sc_area_mm2;
        }
    }
    REPORT_ERROR("Conductor area not found for the given gauge.");
    return (float)ERROR_DATA_NOT_FOUND;
}

// Voltage Drop Calculation
float calculate_voltage_drop_volts(float arg_load_current_amps, float arg_circuit_length_meters, float arg_resistance_per_km, float arg_reactance_per_km, float arg_power_factor, int arg_phase_count) {
    if (arg_resistance_per_km < 0 || arg_reactance_per_km < 0 || arg_circuit_length_meters < 0) {
        REPORT_ERROR("Resistance, reactance, or circuit length cannot be negative for voltage drop calculation.");
        return (float)ERROR_INVALID_INPUT;
    }

    float local_circuit_length_km = arg_circuit_length_meters / 1000.0f;
    float local_sin_phi = sqrt(1.0f - (arg_power_factor * arg_power_factor));
    float local_effective_resistance = (arg_resistance_per_km * arg_power_factor) + (arg_reactance_per_km * local_sin_phi);

    float local_voltage_drop;
    if (arg_phase_count == 1) { // Single-phase
        local_voltage_drop = 2 * arg_load_current_amps * local_circuit_length_km * local_effective_resistance;
    } else if (arg_phase_count == 3) { // Three-phase
        local_voltage_drop = sqrt(3.0f) * arg_load_current_amps * local_circuit_length_km * local_effective_resistance;
    } else {
        REPORT_ERROR("Unsupported number of phases for voltage drop calculation.");
        return (float)ERROR_PHASE_COUNT;
    }
    return local_voltage_drop;
}

// Conduit area based on user input
float get_conduit_area(const char *arg_conduit_type_ptr, float arg_conduit_diameter_nominal_inches) {
    for (int i = 0; i < g_conduit_count; i++) {
        if (strcasecmp(g_conduit_data_g_list[i].sc_conduit_type, arg_conduit_type_ptr) == 0 &&
            fabs(g_conduit_data_g_list[i].sc_diameter_inches - arg_conduit_diameter_nominal_inches) < 0.001) {
            return g_conduit_data_g_list[i].sc_internal_area_mm2;
        }
    }
    REPORT_ERROR("Conduit type and diameter not found in data.");
    return (float)ERROR_DATA_NOT_FOUND;
}

// Check conduit fill
int check_conduit_fill(float arg_conductor_area, int arg_conductor_count, const char *arg_conduit_type_ptr, float arg_conduit_diameter_nominal_inches){
    if (arg_conductor_area <= 0 || arg_conductor_count <= 0) {
        REPORT_ERROR("Conductor area and count must be positive for fill check.");
        return ERROR_INVALID_INPUT;
    }

    float local_total_conductor_area = arg_conductor_area * arg_conductor_count;
    float local_conduit_area = get_conduit_area(arg_conduit_type_ptr, arg_conduit_diameter_nominal_inches);

    if (local_conduit_area < 0) {
        return (int)local_conduit_area;
    }

    float local_fill_percentage = (local_total_conductor_area / local_conduit_area) * 100.0f;

    // The display logic is now in display_calculation_results_ascii, 
    // but the return code logic remains here for functional integrity.
    if (local_fill_percentage <= 40.0f) {
        return SUCCESS;
    } else {
        return ERROR_INVALID_INPUT;
    }
}