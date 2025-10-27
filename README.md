# Electrical Conductor Selection
#### Video Demo: <URL>
-
## Project Overview
This program is a prototype tool designed to assist in the proper selection of electrical conductors according to the guidelines of NOM-001-SEDE-2012
The project seeks to simplify the electrical design process, ensuring that the selected conductors comply with current regulatory requeriments in Mexico. It automates key calculations based on user input and standard data tables, presenting the results in a clear Text-based User Interface (TUI) within the console.
-
## Main features
* **NOM Data Loading:** Reads crucial electrical data (ampacity, temperature correction, conductor adjustment, conduit propierties) from CSV files with robust parsing.
* **Load Current Calculation (Ib):** Calculates the adjusted current applying temperatura and grouping factors.
* **Adjusted Design Current Calculation (Iz):** Calculates the adjusted current applying temperature and grouping factors.
* **Conductor Gauge Selection:** Determines the appropriate conductor gauge (AWG/kcmil) meeting the adjusted current, **required insulation type**, and temperature rating (75°C or 90°C).
* **Conductor Property Retrieval:** Efficiently fetches area, resistance, and reactance for the suggested conductor.
* **Voltage Drop Calculation:** Computes the voltage drop based on conductor properties, load current, and circuit length, comparing it against the 3% NOM limit.
* **Conduit Fill Check:** Calculates the percentage fill of the specified conduit and checks against the 40% limit.
* **Text-based User Interface (TUI):** Clears the screen after input and presents calculation results within an ASCII-drawn box for enhanced readability using platform-specific console functions (`gotoxy` via `windows.h` or ANSI escape codes).
* **Portable Code:** Uses standard C libraries and includes portable functions for case-insensitive string comparison and whitespace trimming.
-

## Technology used
* Programming Language: 100% C
* Design Standards: NOM-001-SEDE-2012
-

### Required Files
Make sure the following CSV files are located in /data directory:

* ampacity_data.csv

* temp_correction_data.csv

* num_cond_adj_data.csv

* conduit_fill_data.csv

Verify compiler:
```bash
gcc --version
```

## How to compile and run
* A C compiler (GCC recommended via MinGW/MSYS2 on Windows, or native on Linux/macOS).
* **On Windows:** The `windows.h` header is required for console manipulation (`gotoxy`, `cls`). MinGW typically includes this.
* **On Linux/macOS:** A terminal that supports ANSI escape codes (most modern terminals do).

Use gcc (or your chosen C compiler) to compile the code. The -lm flag is essential for linking the math library (needed for functions like sqrt()):

```bash
gcc wiring.c -o wiring.exe -lm
```
* wiring.c: The main C source code file.

* -o wiring.exe: Specifies the output executable file name. You can change wiring.exe to anything you prefer (e.g., electrical_selector.exe).

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
1. load_..._data() Functions: Load data from the four CSV files with error checking and whitespace trimming.

2. calculate_load_current_amps(...): Calculates initial load current (Ib).

3. calculate_adjusted_current_amps(...): Calculates adjusted design current (Iz).

4. get_temp_correction_factor(...): Retrieves temperature correction factor.

5. get_ncond_adj_factor(...): Retrieves conductor grouping adjustment factor, handling ranges.

6. get_suggested_gauge_awg_kcmil(...): Selects the smallest gauge meeting Iz, insulation type, and temp rating.

7. find_conductor_by_gauge(...): Efficiently returns a pointer (Conductor*) to all properties for a given gauge.

8. calculate_voltage_drop_volts(...): Calculates voltage drop using conductor R/X values.

9. get_conduit_area(...): Retrieves internal area for a specific conduit type/size.

10. display_calculation_results_ascii(...): Draws the TUI box and displays formatted results using gotoxy.

11. gotoxy(...), draw_ascii_box(...): Helper functions for TUI rendering.

12. portable_strcasecmp(...), trim_trailing_whitespace(...): Utility functions for string handling.

13. main(): Orchestrates loading, input, calculations, and calls the TUI display function.

### Error Codes (#Defines in #define)
The program uses return codes to indicate success or specific types of erros:

* SUCCESS (0): Operation successful.

* ERROR_FILE_OPEN (-1): Failed to open CSV file.

* ERROR_INVALID_INPUT (-2): User input invalid or calculated value out of bounds.

* ERROR_DATA_NOT_FOUND (-3): Required data not found in CSVs.

* ERROR_PHASE_COUNT (-4): Invalid phase count.

* ERROR_DIVIDE_BYZERO (-5): Division by zero.

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
```

## Example of TUI Output
After entering all the parameters, the console will clear and display something similar to this:
```bash
+-----------------------------------------------------------------------------------------+
| --- ELECTRICAL CALCULATION RESULTS ---                                                  |
|                                                                                         |
| LOAD CURRENT (Ib):             35.11 Amps                                               |
| ADJUSTED DESIGN CURRENT (Iz):  46.70 Amps                                               |
|                                                                                         |
| SUGGESTED GAUGE:               8 AWG                                                    |
|   - Area: 8.37 mm^2 / Res: 2.1600 Ohm/km / React: 0.1200 Ohm/km                         |
|                                                                                         |
| VOLTAGE DROP (VD):             2.55 Volts (Max Allowed: 6.60V)                          |
|   >> RESULT: Voltage Drop is acceptable.                                                |
|                                                                                         |
| CONDUIT FILL CHECK:                                                                     |
|   - Total Cond. Area: 33.48 mm^2 / Conduit Area: 412.90 mm^2                            |
|   - Fill Percentage: 8.11 % (Limit: 40.00 %)                                            |
|   >> RESULT: Conduit fill is within acceptable limits.                                  |
|                                                                                         |
+-----------------------------------------------------------------------------------------+


--- Calculations Complete ---
Thank you for using the Electrical Conductor Selection Program. Goodbye!

Press Enter to exit...
```
(Note: Specific values depend on user input and CSV data accuracy)

### Known Limitations and Future Enhancements
#### Limitations

* **Data Accuracy:** Results depend entirely on the correctness of data in CSV files per NOM-001-SEDE-2012.

* **Input Validation:** While basic checks exist, more advanced validation (e.g., ensuring entered insulation/conduit types exist in the data) could be added.

* **strtok:** CSV parsing uses strtok. For complex or potentially malformed CSVs, more robust methods might be needed.

* **Cross-Platform TUI:** Relies on #ifdef _WIN32 for gotoxy and cls. ANSI codes are used otherwise, assuming terminal compatibility.

#### Future Enhancements

* **Enhanced Input:** Allow selection from lists of available options (insulation, conduit) derived from loaded data.

* **More Complex Scenarios:** Handle parallel conductors, different load types, or motor starting currents.

* **Unit Options:** Offer input/output in different units (feet, °F).

* **Configuration File:** Use a config file for settings (filenames, limits).

## License
This project is licensed under the MIT License.