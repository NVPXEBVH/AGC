
#include <iostream>
#include "hackrf.h"
#include "Windows.h"
using namespace std;

//основные переменные
hackrf_device* device = NULL;
int lna_gain = 8;                            // Текущее усиление lna /*0-40db шаг 8db*/
int vga_gain = 36;                           // Текущее усиление vga /*0-62db шаг 2db*/
const uint8_t TARGET_PEAK_LOW = 20;          // Желаемый нижний порог пика
const uint8_t TARGET_PEAK_HIGH = 120;        // Желаемый верхний порог пика
const float TARGET_RMS_LOW = 10;             // Нижний порог RMS (среднеквадратического значения)
const float TARGET_RMS_HIGH = 50;            // Верхний порог RMS
const uint8_t TARGET_PEAK_NOISE = 7;        // Порог шума
void AGC(signed char* hackrf_iq,int valid_length,int* lna_gain,int* vga_gain)
{
    double max = 0, rms=0, sum = 0;
    //расчет среднего пикового значения (защита от резких скачков усиления)
    for (int i = 0; i < valid_length; i+=10000)
    {
        for (int j = 0; j < 10000; j++)
        {
            if ((i + j) == 260000) break;
            if (abs(hackrf_iq[i + j]) > max)
            {
                max = abs(hackrf_iq[i+j]);
            }
            rms += hackrf_iq[i + j]* hackrf_iq[i + j];
        }
        sum += max;
        rms = sqrt(rms/260000);
        max = 0;
    }
    max = sum / 26.;
    cout << "max= " << max << endl;
    if (max < TARGET_PEAK_NOISE)
{
    //Если уровень сигнала близок к уровню шума то ничего не делаем
}
// Если уровень сигнала слишком низкий - увеличиваем усиление
else if (max < TARGET_PEAK_LOW || rms< TARGET_RMS_LOW) {
        // Сначала увеличиваем LNA (меньше шума)
        if (*lna_gain < 40) {
            *lna_gain = min(*lna_gain + 8, 40);
            std::cout << " hackrf_set_lna_gain=			" << hackrf_error_name((hackrf_error)hackrf_set_lna_gain(device, *lna_gain)) << std::endl;	
        }
        // Если LNA на максимуме, увеличиваем VGA
        else if (*vga_gain < 62) {
            *vga_gain = min(*vga_gain + 2, 62);
            std::cout << "hackrf_set_vga_gain=			" << hackrf_error_name((hackrf_error)hackrf_set_vga_gain(device, *vga_gain)) << std::endl;        
        }
    }
    // Если уровень сигнала слишком высокий - уменьшаем усиление
    else if (max > TARGET_PEAK_HIGH || rms>TARGET_RMS_HIGH) {
        // Сначала уменьшаем VGA (чтобы избежать перегрузки)
        if (*vga_gain > 0) {
            *vga_gain = max(*vga_gain - 2, 0);
           std::cout << "hackrf_set_vga_gain=			" << hackrf_error_name((hackrf_error)hackrf_set_vga_gain(device, *vga_gain)) << std::endl; 
        }
        // Если VGA на минимуме, уменьшаем LNA
        else if (*lna_gain > 0) {
            *lna_gain = max(*lna_gain - 8, 0);
            std::cout << " hackrf_set_lna_gain=			" << hackrf_error_name((hackrf_error)hackrf_set_lna_gain(device, *lna_gain)) << std::endl;	
        }
    }
    // Если уровень в допустимом диапазоне - ничего не делаем
    }

