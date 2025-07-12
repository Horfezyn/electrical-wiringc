# Herramienta de Selección de Conductores Eléctricos

## Resumen
Esta es una herramienta prototipo desarrollada en C, diseñada para facilitar la selección adecuada de conductores eléctricos, siguiendo las directrices establecidas en la **NOM-001-SEDE-2012** (Norma Oficial Mexicana para Instalaciones Eléctricas). Su objetivo es simplificar el proceso de diseño eléctrico, asegurando que los conductores elegidos cumplan con la normativa vigente en México.

## Características
- **Carga de Datos NOM:** Importa datos cruciales (ampacidad, factores de corrección por temperatura, factores de ajuste por número de conductores y datos de llenado de tubería conduit) desde archivos CSV.
- **Cálculo de Corriente de Carga (Ib):** Determina la corriente de carga basándose en la potencia, voltaje, factor de potencia y número de fases (1 o 3) proporcionados por el usuario.
- **Cálculo de Corriente de Diseño Ajustada (Iz):** Calcula la corriente ajustada aplicando factores de corrección por temperatura y por número de conductores a la corriente de carga.
- **Sugerencia de Calibre de Conductor:** Propone el calibre de conductor apropiado (AWG o kcmil) en función de la corriente ajustada calculada y los datos de ampacidad cargados.
- **Manejo de Errores:** Proporciona mensajes de error informativos para problemas como fallos en la carga de archivos, entradas inválidas o datos no encontrados.
- **Interacción Amigable:** Guía al usuario a través de las entradas y mantiene la ventana de la consola abierta al finalizar la ejecución (o en caso de error) para revisar los resultados.

## Cómo Compilar y Ejecutar

### Requisitos previos
Asegúrate de tener un compilador de C instalado en tu sistema. Se recomienda **GCC** para Linux/macOS/WSL o **MinGW** para Windows. Puedes verificar tu versión de GCC ejecutando:
```bash
gcc --version
Archivos Necesarios
Asegúrate de que los siguientes archivos CSV se encuentren en la misma carpeta que tu archivo de código fuente main.c (y, por lo tanto, en la misma ubicación que tu ejecutable compilado):

ampacity_data.csv

temp_correction_data.csv

num_cond_adj_data.csv

conduit_fill_data.csv

Pasos para Compilar
Utiliza gcc (o tu compilador C elegido) para compilar el código. El flag -lm es esencial para vincular la biblioteca matemática (necesaria para funciones como sqrt()):

Bash

gcc main.c -o conductor_selector.exe -lm
main.c: Tu archivo de código fuente C principal.

-o conductor_selector.exe: Especifica el nombre del archivo ejecutable de salida. Puedes cambiar conductor_selector.exe a lo que prefieras (por ejemplo, selector_electrico.exe).

-lm: Vincula la biblioteca matemática.

Pasos para Ejecutar
Después de una compilación exitosa, ejecuta el programa desde tu terminal:

Bash

./conductor_selector.exe
El programa te pedirá los datos de entrada y mostrará los resultados. Se pausará al final, esperando que presiones Enter antes de cerrar la ventana de la consola.

Instrucciones de Uso
El programa te guiará paso a paso para ingresar los siguientes parámetros:

Potencia en watts (ejemplo: 10000)

Voltaje del sistema en volts (ejemplo: 220)

Factor de potencia (ejemplo: 0.85)

Número de fases (1 o 3)

Longitud del circuito en metros (ejemplo: 50)

Temperatura ambiente en °C (ejemplo: 30)

Número de conductores con corriente dentro de la tubería conduit (ejemplo: 3)

Tipo de aislamiento (ejemplo: THHN, THW)

Tipo de tubería conduit (ejemplo: EMT, PVC)

Diámetro nominal de la tubería conduit (ejemplo: 0.5 para 1/2 pulgada, 0.75 para 3/4 pulgada)

Desglose del Código
Funciones Clave
load_ampacity_table_data(const char *arg_file_name_ptr)

Carga los datos de ampacidad de los conductores desde un archivo CSV.

load_temperature_correction(const char *arg_file_name_ptr)

Carga los factores de corrección por temperatura desde un archivo CSV.

load_nconductor_factor(const char *arg_file_name_ptr)

Carga los factores de ajuste por el número de conductores desde un archivo CSV.

load_conduit_fill_data(const char *arg_file_name_ptr)

Carga los datos de llenado de tubería conduit (tipos y áreas) desde un archivo CSV.

calculate_load_current_amps(float arg_power_watts, float arg_voltage_volts, float arg_power_factor, int arg_phase_count)

Calcula la corriente de carga (Ib) según la potencia, voltaje, factor de potencia y número de fases.

calculate_adjusted_current_amps(float arg_load_current_amps, float arg_temp_correction_factor, float arg_num_cond_adjustment_factor)

Calcula la corriente de diseño ajustada (Iz) utilizando la corriente de carga y los factores de corrección.

get_temp_correction_factor(int arg_ambient_temp)

Busca y retorna el factor de corrección de temperatura para una temperatura ambiente dada.

get_ncond_adj_factor(int arg_conductor_count)

Busca y retorna el factor de ajuste por el número de conductores.

get_suggested_gauge_awg_kcmil(float arg_adjusted_current_amps)

Determina y retorna el calibre de conductor sugerido (AWG o kcmil) que cumple con la corriente ajustada.

get_conduit_area(const char *arg_conduit_type_ptr, float arg_conduit_diameter_nominal_inches)

(Implementación futura) Obtendrá el área interna de una tubería conduit específica.

Manejo de Entrada y Salida:

El bucle principal en main() maneja las solicitudes de entrada del usuario y la visualización de resultados. Incluye validación básica de entrada y un mecanismo para pausar la ejecución al finalizar (getchar()).

Códigos de Error (Definidos en #define)
El programa utiliza códigos de retorno para indicar el éxito o tipos específicos de errores:

SUCCESS (0)

ERROR_FILE_OPEN (-1)

ERROR_INVALID_INPUT (-2)

ERROR_DATA_NOT_FOUND (-3)

ERROR_PHASE_COUNT (-4)

ERROR_DIVIDE_BYZERO (-5)

Ejemplo de Salida
Inicio y carga de datos
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
Escenario de Cálculo Exitoso
... (entradas del usuario) ...
------------------------------

--- Performing Calculations ---
Calculated Load Current (Ib): 45.45 Amps
Action: Getting suggested gauge for 53.47 Amps with insulation type THHW.
Adjusted Design Current (Iz): 53.47 Amps
Suggested Conductor Gauge: 8 AWG

Press Enter to exit...
Escenario de Error (Archivo No Encontrado)
--- Loading NOM Data ---
Error: Failed to open ampacity_data.csv
Error loading ampacity data. Exiting program...
Press Enter to exit...
Limitaciones Conocidas
Las funciones get_conductor_resistance_km, get_conductor_reactance_km y get_conductor_mm2 están declaradas pero aún no implementadas para recuperar datos reales.

La validación de entrada del usuario es básica y podría mejorarse para manejar casos más complejos.

Actualmente, el tipo de aislamiento y el tipo/diámetro de la tubería conduit se solicitan, pero no influyen directamente en la selección del calibre del conductor en la lógica actual (aunque los datos de conduit se cargan para futura implementación).

Mejoras Futuras
Integrar el cálculo de caída de tensión utilizando la resistencia y reactancia de los conductores.

Implementar la lógica para la selección de calibre basada en el tipo de aislamiento y la capacidad de llenado de la tubería conduit.

Mejorar la robustez de la validación de entrada del usuario.

Considerar la implementación de una interfaz gráfica (GUI) para una mejor experiencia de usuario.

Permitir la selección dinámica del tipo de aislamiento para la ampacidad (actualmente fija a 75°C).

Licencia
Este proyecto está licenciado bajo la Licencia MIT.