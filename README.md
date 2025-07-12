# Electrical Conductor Selection Tool

## Overview

This C program is a prototype tool designed to assist in the proper selection of electrical conductors, adhering to the guidelines set forth in **NOM-001-SEDE-2012** (Mexican Official Standard for Electrical Installations). The project aims to simplify the electrical design process, ensuring that selected conductors comply with current regulatory requirements in Mexico.

---

## Features

* **NOM Data Loading:** Reads crucial electrical data (ampacity, temperature correction factors, number of conductors adjustment factors, and conduit fill data) from CSV files.
* **Load Current Calculation (Ib):** Computes the load current based on user-provided power, voltage, power factor, and number of phases (1 or 3).
* **Adjusted Design Current Calculation (Iz):** Calculates the adjusted current by applying temperature correction and number of conductors adjustment factors to the load current.
* **Suggested Conductor Gauge:** Determines and suggests the appropriate conductor gauge (AWG or kcmil) based on the calculated adjusted current and loaded ampacity data.
* **Robust Error Handling:** Provides informative error messages for issues like file loading failures, invalid inputs, or data not found.
* **User-Friendly Interaction:** Guides the user through input prompts and pauses the console window at the end of execution (or upon error) to allow time for result review.

---

## How to Compile and Run

### Prerequisites

Ensure you have a C compiler installed on your system. **GCC** is recommended for Linux/macOS/WSL, and **MinGW** for Windows. You can check your GCC version by running:

```bash
gcc --version
```

### Required Files
Make sure the following CSV files are located in the same directory as your main.c source code file (and, consequently, your compiled executable):

* ampacity_data.csv

* temp_correction_data.csv

* num_cond_adj_data.csv

*conduit_fill_data.csv

### Compilation Steps
Use gcc (or your chosen C compiler) to compile the code. The -lm flag is essential for linking the math library (needed for functions like sqrt()):

```bash
gcc main.c -o conductor_selector.exe -lm
```
* main.c: Your main C source code file.

* -o conductor_selector.exe: Specifies the output executable file name. You can change conductor_selector.exe to anything you prefer (e.g., electrical_selector.exe).

* -lm: Links the math library.

### Running the Program
After successful compilation, run the executable from your terminal:

```bash
./conductor_selector.exe
```
The program will prompt you for input and display the results. It will pause at the end, waiting for you to press Enter before closing the console window.

Sample Input Prompts
The program will guide you step-by-step to enter the following parameters:

1. Power in watts (e.g., 10000)

2. System Voltage in volts (e.g., 220)

3. Power Factor (e.g., 0.85)

4. Number of Phases (1 or 3)

5. Circuit Length in meters (e.g., 50)

6. Ambient Temperature in °C (e.g., 30)

7. Number of Current-Carrying Conductors in conduit (e.g., 3)

8. Insulation Type (e.g., THHN, THW)

9. Conduit Type (e.g., EMT, PVC)

10. Conduit Nominal Diameter (e.g., 0.5 for 1/2 inch, 0.75 for 3/4 inch)

## Code Breakdown
### Key Functions
1. load_ampacity_table_data(const char *arg_file_name_ptr)

* Loads conductor ampacity data from a CSV file.

2. load_temperature_correction(const char *arg_file_name_ptr)

* Loads temperature correction factors from a CSV file.

3. load_nconductor_factor(const char *arg_file_name_ptr)

* Loads conductor count adjustment factors from a CSV file.

4. load_conduit_fill_data(const char *arg_file_name_ptr)

* Loads conduit fill data (types and areas) from a CSV file.

5. calculate_load_current_amps(float arg_power_watts, float arg_voltage_volts, float arg_power_factor, int arg_phase_count)

* Calculates the load current (Ib) based on power, voltage, power factor, and number of phases.

6. calculate_adjusted_current_amps(float arg_load_current_amps, float arg_temp_correction_factor, float arg_num_cond_adjustment_factor)

* Calculates the adjusted design current (Iz) using the load current and correction factors.

7. get_temp_correction_factor(int arg_ambient_temp)

* Retrieves the temperature correction factor for a given ambient temperature.

8. get_ncond_adj_factor(int arg_conductor_count)

* Retrieves the adjustment factor for the given number of conductors.

9. get_suggested_gauge_awg_kcmil(float arg_adjusted_current_amps)

* Determines and returns the suggested conductor gauge (AWG or kcmil) that meets the adjusted current.

10. get_conduit_area(const char *arg_conduit_type_ptr, float arg_conduit_diameter_nominal_inches)

* (Future implementation) Will retrieve the internal area of a specific conduit.

11. Input/Output Handling:

* The main loop in main() manages user input requests and result display. It includes basic input validation and a mechanism to pause execution at the end (getchar()).

### Error Codes (Defined in #define)
The program uses return codes to indicate success or specific types of errors:

* SUCCESS (0)

* ERROR_FILE_OPEN (-1)

* ERROR_INVALID_INPUT (-2)

* ERROR_DATA_NOT_FOUND (-3)

* ERROR_PHASE_COUNT (-4)

* ERROR_DIVIDE_BYZERO (-5)

## Example Output
### Program Start and Data Loading
```bash
Electrical Conductor Selection Program (NOM-001-SEDE-2012)
----------------CS50 PROJECT by @Horfezyn----------------

--- Loading NOM Data ---
Action: Loaded X ampacity data entries from ampacity_data.csv.
Action: Loaded X temperature correction factors from temp_correction_data.csv.
Action: Loaded X number of conductors adjustment factors from num_cond_adj_data.csv.
Action: Loaded X conduit fill data entries from conduit_fill_data.csv. 
--- Data Loading Complete ---

--- Enter Circuit Parameters ---
Enter power (Watts, e.g., 10000): 
```
### Successful Calculation Scenario
```bash
... (user inputs) ...
------------------------------

--- Performing Calculations ---
Calculated Load Current (Ib): 45.45 Amps
Action: Getting suggested gauge for 53.47 Amps with insulation type THHW.
Adjusted Design Current (Iz): 53.47 Amps
Suggested Conductor Gauge: 8 AWG

Press Enter to exit...
```
### Error Scenario (File Not Found)
```bash
--- Loading NOM Data ---
Error: Failed to open ampacity_data.csv
Error loading ampacity data. Exiting program...
Press Enter to exit...
```
### Known Limitations
* The get_conductor_resistance_km, get_conductor_reactance_km, and get_conductor_mm2 functions are declared but not yet implemented to retrieve actual data.

* User input validation is basic and could be enhanced to handle more complex cases.

* Currently, insulation type and conduit type/diameter are requested but do not directly influence conductor gauge selection in the current logic (though conduit data is loaded for future implementation).

### Future Enhancements
* Integrate voltage drop calculations using conductor resistance and reactance.

* Implement logic for conductor gauge selection based on insulation type and conduit fill capacity.

* Improve the robustness of user input validation.

* Consider implementing a Graphical User Interface (GUI) for a better user experience.

* Allow dynamic selection of insulation type for ampacity (currently fixed at 75°C).

## License
This project is licensed under the MIT License.