#include "../spl_c/spl_c.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>

using namespace std;

const char* signal_wav = "E:/testdata/test/170_60.wav";
const char* scale = "E:/testdata/test/test2-scale-model-freq.bin";
const char* spectrum = "E:/testdata/test/spectrum.bin";
const char* mask = "E:/testdata/test/mask.bin";
const char* pitch = "E:/testdata/test/pitch.bin";
const char* bit_mask = "E:/testdata/test/bit_mask.bit";
const char* vocal = "E:/testdata/test/vocal.bin";

int da()
{
    //int F;
    //cout << "0 - write, 1 - read, 2 - mask \n";
    //cin >> F;

    //if (F == 0)
    //{
    //
    //    int K = 256;
    //    freq_t* Fr = new freq_t[K];
    //    freq_t F1 = 50.0, F2 = 4000.0;


    //    spl_freq_scale_generate(K, Fr, freq_scale_model, F1, F2);

    //    int m = 0;

    //    for (int i = 0; i < 256; i++)
    //    {
    //        if (m == 10)
    //        {
    //            printf("%lf \n", Fr[i]);
    //            m = 0;
    //        }
    //        else
    //        {
    //            printf("%lf \t", Fr[i]);
    //            m++;
    //        }
    //    }

    //    spl_freq_scale_save(scale, Fr, K);

    //    size_t spec = spl_spectrum_calc_wav_file(K, Fr, signal_wav, spectrum, 0.001);
    //}
    //else if (F == 1)
    //{
    //    FILE* binfile;
    //    errno_t err;
    //    int n = 0;
    //    int co = 0;
    //    int cd = 0;
    //    err = fopen_s(&binfile, pitch, "rb");
    //    if (err)
    //    {
    //        printf_s("Failed open file, error %d", err);
    //        return 0;
    //    }

    //    /*cout << "enter start byte number - ";
    //    cin >> n;
    //    cout << "\n";

    //    cout << "enter byte count - ";
    //    cin >> co;
    //    cout << "\n";

    //    cout << "enter cycle count - ";
    //    cin >> cd;
    //    cout << "\n";*/

    //    double buf;



    //    for (int i = 0; i < cd; i++)
    //    {
    //        for (int j = 0; j < co; j++)
    //        {
    //            fseek(binfile, (n * i) * sizeof(double), 0);

    //            fread_s(&buf, sizeof(double), sizeof(double), 1, binfile);

    //            cout << "byte - " << buf;
    //            if (j == cd - 1)
    //            {
    //                cout << "\n";
    //            }
    //            else
    //            {               
    //                cout << "\t";
    //            }
    //        }

    //        n++;
    //    }
    //    fclose(binfile);

    //    cout << "\n \n";

    //    ifstream file(pitch, ios_base::binary);

    //    for (int i = 0; i < co; i++)
    //    {
    //        file.read(reinterpret_cast<char*>(&buf), sizeof(double));

    //        cout << "byte - " << buf << "\n";
    //    }

    //    file.close();
    //    return 0;
    //}
    //else if (F == 2)
    //{
    //    int K = 256;
    //    freq_t* Fr = new freq_t[K];

    //    spl_freq_scale_load(scale, &Fr, &K);

    //    spl_freq_mask_calc_bin_file(K, Fr, spectrum, mask, 0.001);

    //    spl_freq_mask_calc_bit_file(K, Fr, spectrum, bit_mask, 0.001);
    //}
    //else if (F == 3)
    //{
    //    int K = 256;
    //    freq_t* Fr = new freq_t[K];

    //    spl_freq_scale_load(scale, &Fr, &K);

    //    spl_pitch_calc_bin_file(K, Fr, mask, pitch, 70, 400, 0.001);

    //    spl_vocal_calc_bin_file(K, Fr, pitch, vocal, 0.030, 0.030, 12000);

    //}
    //else
    //{
    //    return 0;
    //}
    return 0;
}

