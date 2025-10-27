#include <stdio.h> // For input/output
#include <stdlib.h> // For general utilities (malloc, free)
#include <math.h> // For mathematical functions (sqrtf, fabsf)
#include <string.h> // For string manipulation (strcpy, strtok, strcmp, strlen)
#include <ctype.h>  // For tolower
#include <stddef.h> // For size_t

// Include for Windows-specific console functions (gotoxy)
#ifdef _WIN32
#include <windows.h>
#endif

// --- Error Codes --- Corrected definitions
#define SUCCESS                 (0)
#define ERROR_FILE_OPEN         (-1)
#define ERROR_INVALID_INPUT     (-2)
#define ERROR_DATA_NOT_FOUND    (-3)
#define ERROR_PHASE_COUNT       (-4)
#define ERROR_DIVIDE_BYZERO     (-5)

// --- Macro for print the error ---
#define REPORT_ERROR(message) fprintf(stderr, "Error: %s\n",message)

// --- Structure Definitions ---

typedef struct sc_conductor{
    int sc_gauge_awg_kcmil;
    char sc_insulation_type[32];
    float sc_ampacity_at_75c_amps;
    float sc_ampacity_at_90c_amps;
    float sc_area_mm2;
    float sc_resistance_km;
    float sc_reactance_km;
} Conductor;

typedef struct s_temp_correction{
    int stc_ambient_temp;
    float stc_correction_factor;
} TempCorrectionFactor;

typedef struct s_numc_adjustment{
    int snca_conductor_count;
    float snca_adjustment_factor;
}NumCondFactor;

typedef struct s_conduit{
    char sc_conduit_type[50];
    float sc_diameter_inches;
    float sc_internal_area_mm2;
}Conduit;

// --- Global variables ---
Conductor g_conductor_data_g_list[20];
int g_conductor_count = 0;

TempCorrectionFactor g_temp_factors_g_list[20];
int g_temp_correction_count = 0;

NumCondFactor g_ncond_adj_g_list[20];
int g_ncond_adj_count = 0;

Conduit g_conduit_data_g_list[30];
int g_conduit_count = 0;

// --- Function Prototypes ---
// TUI Helpers
void gotoxy(int x, int y);
void draw_ascii_box(int x1, int y1, int x2, int y2);
void display_calculation_results_ascii(
    float local_voltage_volts,
    float local_load_current_amps,
    float local_adjusted_current_amps,
    int local_suggested_gauge_awg_kcmil,
    Conductor *suggested_conductor,
    float local_voltage_drop_volts, // <<< *** CORRECTION IS HERE ***
    float local_conduit_area,
    float local_total_conductor_area
);
int portable_strcasecmp(const char *s1, const char *s2); // Portable case-insensitive compare

// For data loading
int load_ampacity_table_data(const char *arg_file_name_ptr);
int load_temperature_correction(const char *arg_file_name_ptr);
int load_nconductor_factor(const char *arg_file_name_ptr);
int load_conduit_fill_data(const char *arg_file_name_ptr);

// Calculation base
float calculate_load_current_amps(float arg_power_watts, float arg_voltage_volts, float arg_power_factor, int arg_phase_count);
float calculate_adjusted_current_amps(float arg_load_current_amps, float arg_temp_correction_factor, float arg_num_cond_adjustment_factor);
float calculate_voltage_drop_volts(float arg_load_Current_amps, float arg_circuit_lenght_meters, float arg_resistance_per_km, float arg_reactance_per_km, float arg_power_factor, int arg_phase_count);

// Data retrieval (REFAC.: Centralized conductor search)
float get_temp_correction_factor(int arg_ambient_temp);
float get_ncond_adj_factor(int arg_conductor_count);
Conductor *find_conductor_by_gauge(int arg_gauge_awg_kcmil); // NEW
float get_conduit_area(const char *arg_conduit_type_ptr, float arg_conduit_diameter_nominal_inches);

// Selection and validation
int get_suggested_gauge_awg_kcmil(float arg_adjusted_current_amps, const char *arg_insulation_type_ptr, int arg_temp_rating);

// Helper to remove trailing whitespace/newlines
void trim_trailing_whitespace(char *str);


// --- Main Function ---
int main() {
    // Local variable for user input
    float local_power_watts;
    float local_voltage_volts;
    float local_power_factor;
    int local_phase_count;
    float local_circuit_length_meters;
    int local_ambient_temperature;
    int local_conductor_count;
    char local_insulation_type[32];
    char local_conduit_type[32];
    float local_conduit_diameter;
    int local_insulation_temperature_rating;

    // REFAC.: Results variables initialized
    float local_load_current_amps = (float)ERROR_DATA_NOT_FOUND;
    float local_adjusted_current_amps = (float)ERROR_DATA_NOT_FOUND;
    int local_suggested_gauge_awg_kcmil = ERROR_DATA_NOT_FOUND;
    Conductor *suggested_conductor = NULL; // Pointer to the found conductor struct

    float local_voltage_drop_volts = (float)ERROR_DATA_NOT_FOUND;
    float local_conduit_area = (float)ERROR_DATA_NOT_FOUND;
    float local_total_conductor_area = (float)ERROR_DATA_NOT_FOUND;

    float local_temp_correction_factor;
    float local_num_cond_adjustment_factor;
    int return_code;

    // --- Initial Load and Input (Remains standard output) ---
    printf("\n\nElectrical Conductor Selection Program (NOM-001-SEDE-2012)\n");
    printf("----------------CS50 PROJECT by @Horfezyn----------------\n\n");
    printf("--- Loading NOM Data ---\n");

    return_code = load_ampacity_table_data("ampacity_data.csv");
    if (return_code != SUCCESS){ printf("Error loading ampacity data. Exiting program...\n"); return return_code; }

    return_code = load_temperature_correction("temp_correction_data.csv");
    if (return_code != SUCCESS){ printf("Error loading temperature correction data. Exiting program...\n"); return return_code; }

    return_code = load_nconductor_factor("num_cond_adj_data.csv");
    if (return_code != SUCCESS){ printf("Error loading number of conductor adjustment data. Exiting program...\n"); return return_code; }

    return_code = load_conduit_fill_data("conduit_fill_data.csv");
    if (return_code != SUCCESS){ printf("Error loading conduit fill data. Exiting program...\n"); return return_code; }

    printf("--- Data Loading Complete ---\n\n");

    // --- User Input ---
    printf("--- Enter Circuit Parameters ---\n");
    do{
        printf("Enter power (Watts, e.g., 10000): ");
        if (scanf("%f", &local_power_watts) != 1) {
            REPORT_ERROR("Invalid input for power. Please enter a number.");
            while (getchar() != '\n');
            local_power_watts = 0;
        }
    } while (local_power_watts <= 0);
    do{
        printf("Enter system voltage (Volts, e.g., 220): ");
        if (scanf("%f", &local_voltage_volts) != 1) {
            REPORT_ERROR("Invalid input for voltage. Please enter a number.");
            while (getchar() != '\n');
            local_voltage_volts = 0;
        }
    } while (local_voltage_volts <= 0);
     do{
        printf("Enter power factor (e.g., 0.85): ");
        if (scanf("%f", &local_power_factor) != 1 || local_power_factor <= 0 || local_power_factor > 1.0) {
            REPORT_ERROR("Invalid input for power factor. Please enter a number between 0 and 1.");
            while (getchar() != '\n');
            local_power_factor = 0;
        }
    } while (local_power_factor <= 0 || local_power_factor > 1.0);
    do{
        printf("Enter number of phases (1 or 3): ");
        if (scanf("%d", &local_phase_count) != 1 || (local_phase_count != 1 && local_phase_count != 3) ) { // Validate 1 or 3
            REPORT_ERROR("Invalid input for phases. Please enter 1 or 3.");
            while (getchar() != '\n');
            local_phase_count = 0;
        }
    } while (local_phase_count != 1 && local_phase_count != 3);
    do{
        printf("Enter circuit length (meters, e.g., 50): ");
        if (scanf("%f", &local_circuit_length_meters) != 1) {
            REPORT_ERROR("Invalid input for circuit length. Please enter a number.");
            while (getchar() != '\n');
            local_circuit_length_meters = 0;
        }
    } while (local_circuit_length_meters <= 0);
    do{
        printf("Enter ambient temperature (celsius, e.g., 30): ");
        if (scanf("%d", &local_ambient_temperature) != 1) {
            REPORT_ERROR("Invalid input for ambient temperature. Please enter an integer.");
            while (getchar() != '\n');
            local_ambient_temperature = -1000; // Use an unlikely value to re-prompt
        }
    } while (local_ambient_temperature < -273); // Allow reasonable temps
    do{
        printf("Enter number of current-carrying conductors in conduit (e.g., 3): ");
        if (scanf("%d", &local_conductor_count) != 1) {
            REPORT_ERROR("Invalid input for conductor count. Please enter an integer.");
            while (getchar() != '\n');
            local_conductor_count = 0;
        }
    } while (local_conductor_count <= 0);

    printf("Enter insulation type (e.g., THHN, THW): ");
    scanf("%s", local_insulation_type);
    // Clear buffer after reading string
    while (getchar() != '\n');

    do {
        printf("Enter insulation temperature rating (75 or 90): ");
        if (scanf("%d", &local_insulation_temperature_rating) != 1 || (local_insulation_temperature_rating != 75 && local_insulation_temperature_rating != 90)) {
            REPORT_ERROR("Invalid input for insulation temperature rating. Please enter 75 or 90.");
            while (getchar() != '\n');
            local_insulation_temperature_rating = 0;
        }
    } while (local_insulation_temperature_rating != 75 && local_insulation_temperature_rating != 90);

    printf("Enter conduit type (e.g., EMT, PVC): ");
    scanf("%s", local_conduit_type);
     // Clear buffer after reading string
    while (getchar() != '\n');

    do{
        printf("Enter conduit nominal diameter (e.g., 0.5 for 1/2, 0.75 for 3/4): ");
        if (scanf("%f", &local_conduit_diameter) != 1) {
            REPORT_ERROR("Invalid input for conduit diameter. Please enter a number.");
            while (getchar() != '\n');
            local_conduit_diameter = 0;
        }
    } while (local_conduit_diameter <= 0);
    // Clear final buffer after last scanf
    while (getchar() != '\n');

    printf("\n------------------------------\n");
    // Calculation section does not print intermediate results anymore

    // --- Calculations ---

    // 1. Current of the load
    local_load_current_amps = calculate_load_current_amps(local_power_watts, local_voltage_volts, local_power_factor, local_phase_count);
    if (local_load_current_amps < 0) return (int)local_load_current_amps;

    // 2. Correction factors
    local_temp_correction_factor = get_temp_correction_factor(local_ambient_temperature);
    if (local_temp_correction_factor < 0) return (int) local_temp_correction_factor;

    local_num_cond_adjustment_factor = get_ncond_adj_factor(local_conductor_count);
    if (local_num_cond_adjustment_factor < 0) return (int) local_num_cond_adjustment_factor;

    // 3. Adjusted current
    local_adjusted_current_amps = calculate_adjusted_current_amps(local_load_current_amps, local_temp_correction_factor, local_num_cond_adjustment_factor);
    if (local_adjusted_current_amps < 0) return (int)local_adjusted_current_amps;

    // 4. Suggested gauge
    local_suggested_gauge_awg_kcmil = get_suggested_gauge_awg_kcmil(local_adjusted_current_amps, local_insulation_type, local_insulation_temperature_rating);

    // 5. Conductor properties and checks (REFAC.: Use find_conductor_by_gauge)
    if (local_suggested_gauge_awg_kcmil > 0) {
        suggested_conductor = find_conductor_by_gauge(local_suggested_gauge_awg_kcmil);
    }

    if (suggested_conductor != NULL) {
        // Voltage drop section
        local_voltage_drop_volts = calculate_voltage_drop_volts(
            local_load_current_amps,
            local_circuit_length_meters,
            suggested_conductor->sc_resistance_km,
            suggested_conductor->sc_reactance_km,
            local_power_factor,
            local_phase_count
        );

        // Conduit area for fill check
        local_conduit_area = get_conduit_area(local_conduit_type, local_conduit_diameter);
        if (local_conduit_area > 0) { // Only calculate if conduit area is valid
             local_total_conductor_area = suggested_conductor->sc_area_mm2 * local_conductor_count;
        } else {
            local_total_conductor_area = (float)ERROR_DATA_NOT_FOUND; // Mark as error if conduit not found
        }


    } else {
        REPORT_ERROR("No valid conductor gauge was suggested or properties not found.");
        // Set values to indicate errors if conductor not found
        local_voltage_drop_volts = (float)ERROR_DATA_NOT_FOUND;
        local_conduit_area = (float)ERROR_DATA_NOT_FOUND;
        local_total_conductor_area = (float)ERROR_DATA_NOT_FOUND;
    }

    // --- Display TUI results ---
    #ifdef _WIN32
        system("cls");
    #else
        printf("\033[2J\033[H"); // ANSI clear screen
    #endif

    display_calculation_results_ascii(
        local_voltage_volts,
        local_load_current_amps,
        local_adjusted_current_amps,
        local_suggested_gauge_awg_kcmil,
        suggested_conductor, // Pass the pointer
        local_voltage_drop_volts, // Pass the calculated VD
        local_conduit_area,
        local_total_conductor_area
    );

    // --- Final Prompt ---
    printf("\n\n"); // Add space before final messages
    printf("--- Calculations Complete ---\n");
    printf("Thank you for using the Electrical Conductor Selection Program. Goodbye!\n");
    printf("\nPress Enter to exit...");

    // Clear potential leftover input buffer before final pause
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // Consume remaining characters until newline or EOF

    getchar(); // Wait for Enter key
    return SUCCESS;
}

// --- Function Implementations ---

// Auxiliar function for cross-platform gotoxy
void gotoxy(int x, int y) {
    #ifdef _WIN32
        COORD coord;
        coord.X = x - 1; // Windows COORD is 0-based
        coord.Y = y - 1; // Windows COORD is 0-based
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    #else
        // ANSI escape code for Unix-like terminals (1-based index)
        printf("\033[%d;%dH", y, x);
        fflush(stdout); // Ensure the command is sent immediately
    #endif
}

// Function to Draw an ASCII Box
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

// Function to display the calculation results using ASCII formatting
void display_calculation_results_ascii(
    float local_voltage_volts,
    float local_load_current_amps,
    float local_adjusted_current_amps,
    int local_suggested_gauge_awg_kcmil,
    Conductor *suggested_conductor, // Accept pointer
    float local_voltage_drop_volts, // <<< *** CORRECTION IS HERE ***
    float local_conduit_area,
    float local_total_conductor_area
) {
    // Console area dimensions
    int box_x1 = 5;
    int box_y1 = 2;
    int box_x2 = 90; // Adjusted width for better fit
    int box_y2 = 23;

    draw_ascii_box(box_x1, box_y1, box_x2, box_y2);
    gotoxy(box_x1 + 2, box_y1); printf("--- ELECTRICAL CALCULATION RESULTS ---");

    int current_y = box_y1 + 2;

    // --- 1. CURRENT CALCULATIONS ---
    gotoxy(box_x1 + 2, current_y); printf("LOAD CURRENT (Ib):             %.2f Amps", local_load_current_amps);
    current_y++;
    gotoxy(box_x1 + 2, current_y); printf("ADJUSTED DESIGN CURRENT (Iz):  %.2f Amps", local_adjusted_current_amps);
    current_y += 2;

    // --- 2. CONDUCTOR SELECTION ---
    char gauge_label[40] = "N/A - Calculation Error or No Match"; // Increased buffer size
    if (local_suggested_gauge_awg_kcmil == ERROR_DATA_NOT_FOUND) {
        // Keep default message
        if(suggested_conductor == NULL) strcat(gauge_label, " (Conductor not found)");
    } else if (local_suggested_gauge_awg_kcmil >= 110 && local_suggested_gauge_awg_kcmil <= 140) {
        int awg_number = local_suggested_gauge_awg_kcmil - 100;
        sprintf(gauge_label, "%d/0 AWG", awg_number);
    } else if (local_suggested_gauge_awg_kcmil >= 250) {
        sprintf(gauge_label, "%d kcmil", local_suggested_gauge_awg_kcmil);
    } else if (local_suggested_gauge_awg_kcmil > 0) {
        sprintf(gauge_label, "%d AWG", local_suggested_gauge_awg_kcmil);
    }

    gotoxy(box_x1 + 2, current_y); printf("SUGGESTED GAUGE:               %s", gauge_label);
    current_y++;

    if (suggested_conductor != NULL) { // Use the pointer
        gotoxy(box_x1 + 2, current_y);
        printf("  - Area: %.2f mm^2 / Res: %.4f Ohm/km / React: %.4f Ohm/km", // Shortened labels
            suggested_conductor->sc_area_mm2, suggested_conductor->sc_resistance_km, suggested_conductor->sc_reactance_km);
    } else {
        gotoxy(box_x1 + 2, current_y);
        printf("  - Conductor Properties: Not Available");
    }
    current_y += 2;

    // --- 3. VOLTAGE DROP CHECK ---
    float max_allowed_vd = local_voltage_volts * 0.03f; // 3% voltage drop

    gotoxy(box_x1 + 2, current_y);
    // Use the passed parameter 'local_voltage_drop_volts'
    if(local_voltage_drop_volts != (float)ERROR_DATA_NOT_FOUND && local_voltage_drop_volts >= 0) {
        printf("VOLTAGE DROP (VD):             %.2f Volts (Max Allowed: %.2fV)", local_voltage_drop_volts, max_allowed_vd);
    } else {
        printf("VOLTAGE DROP (VD):             N/A (Calculation Error or Cond. Not Found)");
    }
    current_y++;

    gotoxy(box_x1 + 2, current_y);
    if (local_voltage_drop_volts > max_allowed_vd) {
        printf("  >> WARNING: VD exceeds recommended 3%% limit.");
    } else if (local_voltage_drop_volts >= 0) {
        printf("  >> RESULT: Voltage Drop is acceptable.");
    } else {
        printf("  >> STATUS: Voltage Drop calculation not performed or failed.");
    }
    current_y += 2;

    // --- 4. CONDUIT FILL CHECK ---
    gotoxy(box_x1 + 2, current_y); printf("CONDUIT FILL CHECK:");
    current_y++;

    if (local_conduit_area > 0 && local_total_conductor_area >= 0 && suggested_conductor != NULL) {
        float local_fill_percentage = (local_total_conductor_area / local_conduit_area) * 100.0f;

        gotoxy(box_x1 + 2, current_y);
        printf("  - Total Cond. Area: %.2f mm^2 / Conduit Area: %.2f mm^2",
               local_total_conductor_area, local_conduit_area);
        current_y++;
        gotoxy(box_x1 + 2, current_y);
        printf("  - Fill Percentage: %.2f %% (Limit: 40.00 %%)", local_fill_percentage);
        current_y++;
        gotoxy(box_x1 + 2, current_y);

        if (local_fill_percentage > 40.0f) {
            printf("  >> WARNING: Conduit fill exceeds 40%% limit. A larger conduit is required.");
        } else {
            printf("  >> RESULT: Conduit fill is within acceptable limits.");
        }
    } else {
        gotoxy(box_x1 + 2, current_y);
        if(local_conduit_area == (float)ERROR_DATA_NOT_FOUND)
             printf("  >> ERROR: Conduit Area not found for the given type/diameter.");
        else
            printf("  >> STATUS: Cannot calculate fill (Conductor data missing).");
    }

    // Set cursor below the box for the final prompt
    gotoxy(1, box_y2 + 2);
}


// Auxiliar function for portable case-insensitive string comparison
int portable_strcasecmp(const char *s1, const char *s2) {
    if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1); // Handle NULL pointers
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
            return (tolower((unsigned char)*s1) - tolower((unsigned char)*s2));
        }
        s1++;
        s2++;
    }
    return (tolower((unsigned char)*s1) - tolower((unsigned char)*s2));
}

// REFAC.: New centralized conductor search function
Conductor *find_conductor_by_gauge(int arg_gauge_awg_kcmil) {
    for (int i = 0; i < g_conductor_count; i++) {
        if (g_conductor_data_g_list[i].sc_gauge_awg_kcmil == arg_gauge_awg_kcmil) {
            return &g_conductor_data_g_list[i];
        }
    }
    return NULL; // Return NULL if not found
}

// --- CSV Loading Functions (Robust Parsing) ---

// Helper to remove trailing newline/CR/space characters
void trim_trailing_whitespace(char *str) {
    if (str == NULL) return;
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) { // Use isspace for generality
        str[len - 1] = '\0';
        len--;
    }
}


int load_ampacity_table_data(const char *arg_file_name_ptr) {
    FILE *file_ptr = fopen(arg_file_name_ptr,"r");
    if (!file_ptr){ REPORT_ERROR("Failed to open ampacity_data.csv"); return ERROR_FILE_OPEN; }

    char line[256]; // Increased buffer
    if(fgets(line, sizeof(line),file_ptr) == NULL) { // Skip header or handle empty file
         fclose(file_ptr); REPORT_ERROR("Ampacity file is empty or header missing."); return ERROR_FILE_OPEN;
    }

    g_conductor_count = 0;
    int line_num = 1; // Track line number for error reporting
    while (fgets(line, sizeof(line), file_ptr) != NULL && g_conductor_count < 20){
        line_num++;
        // Check for empty line or line starting with comment char (e.g., #)
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0' || line[0] == '#') continue;

        char *token;
        // Using strtok - ensure robustness
        token = strtok(line, ","); if (token) g_conductor_data_g_list[g_conductor_count].sc_gauge_awg_kcmil = atoi(token); else { fprintf(stderr, "Error loading ampacity line %d: Invalid Gauge\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }
        token = strtok(NULL, ","); if (token) strcpy(g_conductor_data_g_list[g_conductor_count].sc_insulation_type, token); else { fprintf(stderr, "Error loading ampacity line %d: Invalid Insulation\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }
        token = strtok(NULL, ","); if (token) g_conductor_data_g_list[g_conductor_count].sc_ampacity_at_75c_amps = atof(token); else { fprintf(stderr, "Error loading ampacity line %d: Invalid Amp 75C\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }
        token = strtok(NULL, ","); if (token) g_conductor_data_g_list[g_conductor_count].sc_ampacity_at_90c_amps = atof(token); else { fprintf(stderr, "Error loading ampacity line %d: Invalid Amp 90C\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }
        token = strtok(NULL, ","); if (token) g_conductor_data_g_list[g_conductor_count].sc_area_mm2 = atof(token); else { fprintf(stderr, "Error loading ampacity line %d: Invalid Area\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }
        token = strtok(NULL, ","); if (token) g_conductor_data_g_list[g_conductor_count].sc_resistance_km = atof(token); else { fprintf(stderr, "Error loading ampacity line %d: Invalid Resistance\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }
        token = strtok(NULL, ",\n\r"); if (token) g_conductor_data_g_list[g_conductor_count].sc_reactance_km = atof(token); else { fprintf(stderr, "Error loading ampacity line %d: Invalid Reactance\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }

        // Clean trailing whitespace from insulation type
        trim_trailing_whitespace(g_conductor_data_g_list[g_conductor_count].sc_insulation_type);

        g_conductor_count++;
    }

    fclose(file_ptr);
    if (g_conductor_count == 0) { REPORT_ERROR("No valid data loaded from ampacity file."); return ERROR_DATA_NOT_FOUND;}
    printf("Action: Loaded %d ampacity data entries from %s.\n", g_conductor_count, arg_file_name_ptr);
    return SUCCESS;
}

int load_temperature_correction(const char *arg_file_name_ptr){
    FILE *file_ptr = fopen(arg_file_name_ptr, "r");
    if (!file_ptr){ REPORT_ERROR("Error opening temp_correction_data.csv"); return ERROR_FILE_OPEN; }

    char line[128];
    if(fgets(line, sizeof(line), file_ptr) == NULL) { fclose(file_ptr); REPORT_ERROR("Temp correction file empty or header missing."); return ERROR_FILE_OPEN; }

    g_temp_correction_count = 0;
    int line_num = 1;
    while(fgets(line, sizeof(line), file_ptr) && g_temp_correction_count < 20){
        line_num++;
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0' || line[0] == '#') continue;
        char *token;
        token = strtok(line, ","); if (token) g_temp_factors_g_list[g_temp_correction_count].stc_ambient_temp = atoi(token); else { fprintf(stderr, "Error loading temp line %d: Invalid Temp\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }
        token = strtok(NULL, ",\n\r"); if (token) g_temp_factors_g_list[g_temp_correction_count].stc_correction_factor = atof(token); else { fprintf(stderr, "Error loading temp line %d: Invalid Factor\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }
        g_temp_correction_count++;
    }
    fclose(file_ptr);
    if (g_temp_correction_count == 0) { REPORT_ERROR("No valid data loaded from temp correction file."); return ERROR_DATA_NOT_FOUND;}
    printf("Action: Loaded %d temperature correction factors from %s.\n", g_temp_correction_count, arg_file_name_ptr);
    return SUCCESS;
}

int load_conduit_fill_data(const char *arg_file_name_ptr){
    FILE *file_ptr = fopen(arg_file_name_ptr, "r");
    if(!file_ptr){ REPORT_ERROR("Error loading conduit_fill_data.csv"); return ERROR_FILE_OPEN; }

    char line[128];
    if(fgets(line, sizeof(line), file_ptr) == NULL) { fclose(file_ptr); REPORT_ERROR("Conduit file empty or header missing."); return ERROR_FILE_OPEN; }

    g_conduit_count = 0;
    int line_num = 1;
    while(fgets(line, sizeof(line), file_ptr) && g_conduit_count < 30){
         line_num++;
         if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0' || line[0] == '#') continue;
        char *token;
        token = strtok(line, ","); if(token) strcpy(g_conduit_data_g_list[g_conduit_count].sc_conduit_type, token); else { fprintf(stderr, "Error loading conduit line %d: Invalid Type\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }
        token = strtok(NULL, ","); if(token) g_conduit_data_g_list[g_conduit_count].sc_diameter_inches = atof(token); else { fprintf(stderr, "Error loading conduit line %d: Invalid Diameter\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }
        token = strtok(NULL, ",\n\r"); if(token) g_conduit_data_g_list[g_conduit_count].sc_internal_area_mm2 = atof(token); else { fprintf(stderr, "Error loading conduit line %d: Invalid Area\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }

        trim_trailing_whitespace(g_conduit_data_g_list[g_conduit_count].sc_conduit_type);

        g_conduit_count++;
    }
    fclose(file_ptr);
    if (g_conduit_count == 0) { REPORT_ERROR("No valid data loaded from conduit file."); return ERROR_DATA_NOT_FOUND;}
    printf("Action: Loaded %d conduit fill data entries from %s. \n", g_conduit_count,arg_file_name_ptr);
    return SUCCESS;
}

int load_nconductor_factor(const char *arg_file_name_ptr){
    FILE *file_ptr = fopen(arg_file_name_ptr, "r");
    if (!file_ptr) { REPORT_ERROR("Error opening num_cond_adj_data.csv"); return ERROR_FILE_OPEN; }

    char line[100];
    if(fgets(line, sizeof(line), file_ptr) == NULL) { fclose(file_ptr); REPORT_ERROR("Num conductor file empty or header missing."); return ERROR_FILE_OPEN; }

    g_ncond_adj_count = 0;
    int line_num = 1;
    while (fgets(line, sizeof(line), file_ptr) && g_ncond_adj_count < 20) {
         line_num++;
         if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0' || line[0] == '#') continue;
        char *token;
        token = strtok(line, ","); if (token) g_ncond_adj_g_list[g_ncond_adj_count].snca_conductor_count = atoi(token); else { fprintf(stderr, "Error loading num_cond line %d: Invalid Count\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }
        token = strtok(NULL, ",\n\r"); if (token) g_ncond_adj_g_list[g_ncond_adj_count].snca_adjustment_factor = atof(token); else { fprintf(stderr, "Error loading num_cond line %d: Invalid Factor\n", line_num); fclose(file_ptr); return ERROR_INVALID_INPUT; }
        g_ncond_adj_count++;
    }
    fclose(file_ptr);
    if (g_ncond_adj_count == 0) { REPORT_ERROR("No valid data loaded from num conductor file."); return ERROR_DATA_NOT_FOUND;}
    printf("Action: Loaded %d number of conductors adjustment factors from %s.\n", g_ncond_adj_count, arg_file_name_ptr);
    return SUCCESS;
}

// --- Other Calculation and Retrieval Functions ---

float calculate_load_current_amps(float arg_power_watts, float arg_voltage_volts, float arg_power_factor, int arg_phase_count){
    if ( arg_voltage_volts <= 0 || arg_power_factor <= 0) { REPORT_ERROR("Voltage or Power Factor cannot be zero or negative."); return (float)ERROR_DIVIDE_BYZERO; }
    if (arg_phase_count == 1) return arg_power_watts / (arg_voltage_volts * arg_power_factor);
    if (arg_phase_count == 3) return arg_power_watts / (sqrtf(3.0f) * arg_voltage_volts * arg_power_factor); // Use sqrtf for float
    REPORT_ERROR("Unsupported number of phases."); return (float)ERROR_PHASE_COUNT;
}

float calculate_adjusted_current_amps(float arg_load_current_amps, float arg_temp_correction_factor, float arg_num_cond_adjustment_factor){
    if (arg_temp_correction_factor <= 0 || arg_num_cond_adjustment_factor <= 0) { // Check <= 0
         REPORT_ERROR("Correction factors must be positive.");
         return (float)ERROR_DIVIDE_BYZERO; // Or ERROR_INVALID_INPUT depending on desired handling
    }
    return arg_load_current_amps / (arg_temp_correction_factor * arg_num_cond_adjustment_factor);
}

// Suggested gauge now includes insulation type check
int get_suggested_gauge_awg_kcmil(float arg_adjusted_current_amps, const char *arg_insulation_type_ptr, int arg_temp_rating) {
    if (arg_adjusted_current_amps <= 0) { REPORT_ERROR("Adjusted current must be positive."); return ERROR_INVALID_INPUT; }
    if (arg_insulation_type_ptr == NULL || arg_insulation_type_ptr[0] == '\0') { REPORT_ERROR("Insulation type cannot be empty."); return ERROR_INVALID_INPUT;}


    for (int i = 0; i < g_conductor_count; i++) {
        float ampacity_to_check;
        // Select correct ampacity based on rating
        if (arg_temp_rating == 90) ampacity_to_check = g_conductor_data_g_list[i].sc_ampacity_at_90c_amps;
        else if (arg_temp_rating == 75) ampacity_to_check = g_conductor_data_g_list[i].sc_ampacity_at_75c_amps;
        else continue; // Should not happen due to input validation, but good practice

        // Check ampacity AND insulation type (case-insensitive)
        if (ampacity_to_check >= arg_adjusted_current_amps &&
            portable_strcasecmp(g_conductor_data_g_list[i].sc_insulation_type, arg_insulation_type_ptr) == 0) {
            return g_conductor_data_g_list[i].sc_gauge_awg_kcmil; // Found the smallest suitable gauge
        }
    }
    // If loop finishes without finding a match
    fprintf(stderr,"Error: No conductor gauge found matching %.2fA current, insulation '%s', and %dC rating.\n",
            arg_adjusted_current_amps, arg_insulation_type_ptr, arg_temp_rating);
    return ERROR_DATA_NOT_FOUND;
}


float get_temp_correction_factor(int arg_ambient_temp){
    for (int i = 0; i < g_temp_correction_count; i++){
        // Allow a small range in case data isn't exact? Or require exact match? Sticking to exact for now.
        if (g_temp_factors_g_list[i].stc_ambient_temp == arg_ambient_temp) {
            return g_temp_factors_g_list[i].stc_correction_factor;
        }
    }
    fprintf(stderr,"Error: Temperature correction factor not found for %dC.\n", arg_ambient_temp);
    return (float)ERROR_DATA_NOT_FOUND;
}

float get_ncond_adj_factor(int arg_conductor_count){
     // Handle cases where the exact number isn't in the table but falls within a range
     // Find the closest count <= user input count. Assumes CSV is sorted by count ascending.
    int best_match_index = -1;
    for (int i = 0; i < g_ncond_adj_count; i++){
        if (g_ncond_adj_g_list[i].snca_conductor_count == arg_conductor_count) {
             return g_ncond_adj_g_list[i].snca_adjustment_factor; // Exact match found
        }
        // Keep track of the highest count that is less than or equal to the target
        if (g_ncond_adj_g_list[i].snca_conductor_count <= arg_conductor_count) {
             if (best_match_index == -1 || g_ncond_adj_g_list[i].snca_conductor_count > g_ncond_adj_g_list[best_match_index].snca_conductor_count) {
                 best_match_index = i;
             }
        }
    }
     // If no exact match, use the factor for the highest count below the requested count
    if (best_match_index != -1) {
        // Optional: Warn user that an approximation is used?
        // printf("Warning: Using adjustment factor for %d conductors (closest lower match found).\n", g_ncond_adj_g_list[best_match_index].snca_conductor_count);
        return g_ncond_adj_g_list[best_match_index].snca_adjustment_factor;
    }

    // If even no lower match is found (e.g., user entered 0 or less than smallest table value)
    fprintf(stderr,"Error: Number of conductors adjustment factor not found or applicable for %d conductors.\n", arg_conductor_count);
    return (float)ERROR_DATA_NOT_FOUND;
}


float calculate_voltage_drop_volts(float arg_load_current_amps, float arg_circuit_length_meters, float arg_resistance_per_km, float arg_reactance_per_km, float arg_power_factor, int arg_phase_count) {
    if (arg_resistance_per_km < 0 || arg_reactance_per_km < 0 || arg_circuit_length_meters < 0 || arg_power_factor < 0 || arg_power_factor > 1) {
        REPORT_ERROR("Invalid input for voltage drop calculation."); return (float)ERROR_INVALID_INPUT;
    }
    if (arg_load_current_amps < 0) { REPORT_ERROR("Load current cannot be negative for VD calc."); return (float)ERROR_INVALID_INPUT;}

    float local_circuit_length_km = arg_circuit_length_meters / 1000.0f;
    // Ensure power factor is not exactly 1 to avoid sqrt of negative
    float pf_adjusted = (arg_power_factor > 0.99999f) ? 0.99999f : arg_power_factor;
     // Handle potential domain error if pf_adjusted > 1 due to floating point inaccuracies
    if (pf_adjusted * pf_adjusted > 1.0f) pf_adjusted = 1.0f;
    float local_sin_phi = sqrtf(1.0f - (pf_adjusted * pf_adjusted)); // Use sqrtf
    float local_effective_resistance = (arg_resistance_per_km * pf_adjusted) + (arg_reactance_per_km * local_sin_phi);

    float local_voltage_drop;
    if (arg_phase_count == 1) local_voltage_drop = 2 * arg_load_current_amps * local_circuit_length_km * local_effective_resistance;
    else if (arg_phase_count == 3) local_voltage_drop = sqrtf(3.0f) * arg_load_current_amps * local_circuit_length_km * local_effective_resistance; // Use sqrtf
    else { REPORT_ERROR("Unsupported phases for VD calculation."); return (float)ERROR_PHASE_COUNT; }

    return local_voltage_drop;
}

float get_conduit_area(const char *arg_conduit_type_ptr, float arg_conduit_diameter_nominal_inches) {
     if (arg_conduit_type_ptr == NULL || arg_conduit_type_ptr[0] == '\0') { REPORT_ERROR("Conduit type cannot be empty."); return (float)ERROR_INVALID_INPUT;}
     if (arg_conduit_diameter_nominal_inches <= 0) { REPORT_ERROR("Conduit diameter must be positive."); return (float)ERROR_INVALID_INPUT;}

    for (int i = 0; i < g_conduit_count; i++) {
        if (portable_strcasecmp(g_conduit_data_g_list[i].sc_conduit_type, arg_conduit_type_ptr) == 0 &&
            fabsf(g_conduit_data_g_list[i].sc_diameter_inches - arg_conduit_diameter_nominal_inches) < 0.001f) { // Use fabsf for float
            return g_conduit_data_g_list[i].sc_internal_area_mm2;
        }
    }
    fprintf(stderr,"Error: Conduit type '%s' and diameter %.2f not found in data.\n", arg_conduit_type_ptr, arg_conduit_diameter_nominal_inches);
    return (float)ERROR_DATA_NOT_FOUND;
}