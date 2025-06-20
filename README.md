# Electrical Conductor Selection
## Project Overview
This C program is a prototype tool designed to assist in the proper selection of electrical conductors according to the guidelines of NOM-001-SEDE-2012
The project seeks to simplify the electrical design process, ensuring that the selected conductors comply with current regulatory requeriments in Mexico.
## Main features
* I WILL INCLUDE A BRIEF DESCRIPTION FOR EACH FUNCTION OR SOMETINGH LIKE THAT.

## Technology used
* Programming Language: C
* Design Standards: NOM-001-SEDE-2012

## How to compile and run
* INCLUDE THE STEPS FOR COMPILATION AND EXECUTION.
1. Ensure you have a C compiler installed. I recommend GCC for LINUX/macOS/WSL. or MinGW for Windows.
2. Required files: Place the following CSV files in the same directory as your compiled executable or source code: 
    2.1 ampacity_data.csv
    2.2 temp_correction_data.csv
    2.3 num_cond_adj_data.csv
3. Compilation
Use gcc (or another C compiler) to compile the code
gcc -o wiring.c -lm
4. Running the program
After compilation, run the program like this:
./wiring or wiring.exe 
5. Sample input
The program will prompt you for:
* Power in watts (e.g., 10000)
* Voltage in volts (e.g., 220)
* Power factor (e.g., 0.85)
* Number of phases (1 or 3)
* Circuit length in meters (e.g., 50)
* Ambient temperature in °C (e.g., 30)
* Number of current-carrying conductors (e.g., 3)

## License
This project is licensed under the MIT License. 