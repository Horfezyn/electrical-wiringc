# Electrical Conductor Selection

## Project Overview

This C program is a prototype tool designed to assist in the proper selection of electrical conductors according to the guidelines of **NOM-001-SEDE-2012**. The project seeks to simplify the electrical design process, ensuring that the selected conductors comply with current regulatory requirements in Mexico.

---

## Main Features

* **Load NOM-001-SEDE-2012 Data:** Reads critical electrical data (ampacity, temperature correction factors, number of conductors adjustment factors, and conduit fill data) from CSV files.
* **Load Current Calculation (Ib):** Computes the load current based on user-provided power, voltage, power factor, and number of phases (1 or 3).
* **Adjusted Design Current Calculation (Iz):** Calculates the adjusted current by applying temperature correction and number of conductors adjustment factors to the load current.
* **Suggested Conductor Gauge:** Determines and suggests the appropriate conductor gauge (AWG or kcmil) based on the calculated adjusted current and loaded ampacity data.
* **Error Handling & User Feedback:** Provides informative error messages for issues like file loading failures, invalid inputs, or data not found.
* **User-Friendly Interaction:** Guides the user through input prompts and pauses the console window at the end of execution (or upon error) to allow time to review results.

---

## Technology Used

* **Programming Language:** C
* **Design Standards:** NOM-001-SEDE-2012

---

## How to Compile and Run

### Requirements

Ensure you have a C compiler installed. I recommend **GCC** for Linux/macOS/WSL or **MinGW** for Windows. You can check if GCC is installed by running:

```bash
gcc --version
Required Files
Place the following CSV files in the same directory as your main.c source code file (and your compiled executable):

ampacity_data.csv

temp_correction_data.csv

num_cond_adj_data.csv

conduit_fill_data.csv

Compilation
Use gcc (o tu compilador C elegido) para compilar el código. El flag -lm es esencial para vincular la biblioteca matemática (usada para sqrt()):

Bash

gcc main.c -o conductor_selector.exe -lm
main.c: Tu archivo de código fuente C principal.

-o conductor_selector.exe: Especifica el nombre del ejecutable de salida. Puedes cambiar conductor_selector.exe a lo que prefieras (por ejemplo, wiring.exe).

-lm: Vincula la biblioteca matemática.

Running the Program
After successful compilation, run the executable from your terminal:

Bash

./conductor_selector.exe
The program will prompt you for inputs and display the results. It will pause at the end, waiting for you to press Enter before closing the console window.

Sample Input Prompts
The program will guide you through entering the following parameters:

Power in watts (e.g., 10000)

Voltage in volts (e.g., 220)

Power factor (e.g., 0.85)

Number of phases (1 or 3)

Circuit length in meters (e.g., 50)

Ambient temperature in °C (e.g., 30)

Number of current-carrying conductors in conduit (e.g., 3)

Insulation type (e.g., THHN, THW)

Conduit type (e.g., EMT, PVC)

Conduit nominal diameter (e.g., 0.5 for 1/2 inch, 0.75 for 3/4 inch)

License
This project is licensed under the MIT License.