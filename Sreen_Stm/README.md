## Instrucciones para cambiar la hora y fecha en la pantalla

1. Ir a la línea 39 del archivo main.cpp y agregar lo siguiente: screen_set_rtc_time();
2. Ir a la línea 340 del archivo screen_driver.cpp donde se encuentra definida la función screen_set_rtc_time() y modificar los valores de arcuerdo a la hora que quiere programa. Por ejemplo, para programar que son las 19:11:00 del 01/09/2020 se escribiría:<br />

dataBuffer[1] = 0x1409; //año(offset a partir del 2000) - mes<br />
dataBuffer[2] = 0x0113; //día - hora<br />
dataBuffer[3] = 0x0B00; //minuto - segundo<br />

3. Compilar y programar el STM32. Esa función configurará la hora y fecha en la pantalla. 

4. Comentar la función. Volver a compilar y programar el STM32 esta vez con la función comentada.

5. Para programar otra pantalla, descomenta la función, cambia los valores del arreglo, y sigue los pasos de nuevo.
