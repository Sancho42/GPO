#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <fstream>
#include <iterator>

#include "../spl_c/spl_c.h"

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "27015"


const char* scale = "../data/scale-model-freq.bin";
const char* spectrum = "../data/spectrum.bin";
const char* mask = "../data/mask.bin";
const char* pitch = "../data/pitch.bin";
const char* bit_mask = "../data/bit_mask.bit";
const char* vocal = "../data/vocal.bin";

enum CommCode {
    DONE = 111,
    SCALE_GENERATE_STANDART = 211,
    SCALE_GENERATE_CUSTOM = 212,
    SPECTRUM = 311,
    SPECTRUM_GRAF = 312,
    MASK = 411,
    MUSK_GRAF = 412,
    VOCAL = 511,
    VOCAL_GRAF = 512
};

void scale_generate(freq_t* Fr, int K, freq_t F1 = 50.0, freq_t F2 = 4000.0)
{
    spl_freq_scale_generate(K, Fr, freq_scale_model, F1, F2);

    spl_freq_scale_save(scale, Fr, K);
}

void spectrum_calc(freq_t* Fr, int K, const char* signal_wav)
{
    size_t spec = spl_spectrum_calc_wav_file(K, Fr, signal_wav, spectrum, 0.001);

    spl_freq_scale_save(scale, Fr, K);
}

void mask_calc(freq_t* Fr, int K)
{
    spl_freq_scale_load(scale, &Fr, &K);

    spl_freq_mask_calc_bin_file(K, Fr, spectrum, mask, 0.001);
}

void vocal_calc(freq_t* Fr, int K)
{
    spl_freq_scale_load(scale, &Fr, &K);

    spl_pitch_calc_bin_file(K, Fr, mask, pitch, 70, 400, 0.001);

    spl_vocal_calc_bin_file(K, Fr, pitch, vocal, 0.030, 0.030, 12000);
}

int sendMes(std::string mes, SOCKET ClientSocket)
{
    int iSendResult;
    std::vector<char> bytes(mes.begin(), mes.end());
    bytes.push_back('\0');
    char* ans = &bytes[0];

    iSendResult = send(ClientSocket, ans, std::strlen(ans), 0);
    if (iSendResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    printf("Bytes sent: %d\n", iSendResult);
    delete[] ans;
    return 0;
}

int sendMes(std::vector<char> bytes, SOCKET ClientSocket)
{
    int iSendResult;
    bytes.push_back('\0');
    char* ans = &bytes[0];

    iSendResult = send(ClientSocket, ans, std::strlen(ans), 0);
    if (iSendResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    printf("Bytes sent: %d\n", iSendResult);
    delete[] ans;
    return 0;
}

std::string command(std::string mes, SOCKET ClientSocket)
{
    if(sendMes("111", ClientSocket) == 1) return "errMes";

    int comm = atoi(mes.substr(0, 3).c_str());

    switch (comm)
    {
    case 211:
        {
            int K = 256;
            freq_t* Fr = new freq_t[K];
            scale_generate(Fr, K);
            break;
        }
    case 212:
        {
            int K = 256;
            freq_t* Fr = new freq_t[K];
            freq_t F1 = atoi(mes.substr(3, 3).c_str());
            freq_t F2 = atoi(mes.substr(6, 4).c_str());
            scale_generate(Fr, K, F1, F2);
            break;
        }
    case 311:
        {
            const char* signal_wav = mes.substr(3).c_str();
            int K = 256;
            freq_t* Fr = new freq_t[K];
            spectrum_calc(Fr, K, signal_wav);
            std::ifstream file(spectrum, std::ios::binary);
            std::vector<char> buff(std::istreambuf_iterator<char>(file), {});
            std::vector<char> subBuff;
            for(int i=0;i<buff.size();i+=1024)
            {
                subBuff = std::vector<char>(buff.begin()+i, buff.begin()+i+1024);
                sendMes(subBuff, ClientSocket);
            }
            sendMes("111", ClientSocket);
            break;
        }
    case 312:
        {
            break;
        }
    case 411:
        {
            break;
        }
    case 412:
        {
            break;
        }
    case 511:
        {
            break;
        }
    case 512:
        {
            break;
        }
    default:
        {
            return "errMes";
        }
    }

    return "0";
}

int __cdecl main(void)
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char* recvbuf = new char[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    closesocket(ListenSocket);

    std::string mes;
    do {
        memset(recvbuf, 0, recvbuflen);
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 1) {
            printf("Bytes received: %d\n", iResult);
            
            mes = (const char*)recvbuf;
            printf("Message - %s\n", mes);

            std::string err = command(mes, ClientSocket);
        }
        else if (iResult == 1)
            printf("Connection closing...\n");
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 2);

    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}