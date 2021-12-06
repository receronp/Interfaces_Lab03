// StateMachine_Calculator_Serial.c : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>

#define TRANS_COUNT 9
#define EDO_COUNT 15

// Inicializaciones seriales
HANDLE *serial;
unsigned char TxDBuffer[10];
unsigned char RxDBuffer[10];
int aux;
int acum = 0;

DCB config;
COMMTIMEOUTS touts;

char chr = 0;
float acum1 = 0.0;
float acum2 = 0.0;
float multiplier = 1.0;
float res = 0.0;
int dec1 = 0;
int dec2 = 0;
enum Oper
{
    Suma,
    Resta,
    Mult,
    Div
};
enum Oper oper;

int edo = 0;
int edoAnt = 0;
int trans = 0;

// 0-->Inválida
// 6-->Digito
// 7-->Operador
int chrTrans[TRANS_COUNT] = {0, '(', ')', '=', 8, 27, 6, 7, '.'};
int mtzTrans[EDO_COUNT][TRANS_COUNT] = {
    {0, 1, 0, 0, 0, 0, 0, 0, 0},          // Wait for '('
    {1, 1, 1, 1, 99, 99, 2, 1, 1},        // Wait for first digit
    {2, 2, 2, 2, 99, 99, 3, 7, 4},        // Wait for a digit or operator
    {3, 2, 2, 2, 99, 99, 2, 2, 2},        // Wait for a digit
    {4, 4, 4, 4, 99, 99, 5, 4, 4},        // Wait for '.'
    {5, 5, 5, 5, 99, 99, 6, 7, 5},        // Wait for a digit or operator
    {6, 5, 5, 5, 99, 99, 5, 5, 5},        // Wait for a digit
    {7, 7, 7, 7, 99, 99, 8, 7, 7},        // Wait for operator
    {8, 8, 13, 8, 99, 99, 9, 8, 10},      // Wait for second digit term
    {9, 8, 8, 8, 99, 99, 8, 8, 8},        // Wait for second digit term
    {10, 10, 10, 10, 99, 99, 11, 10, 10}, // Wait for '.'
    {11, 11, 13, 11, 99, 99, 12, 7, 11},  // Wait for a digit or operator
    {12, 11, 11, 11, 99, 99, 11, 11, 11}, // Wait for a digit
    {13, 13, 13, 14, 99, 99, 13, 13, 0},  // Wait for '='
    {14, 0, 0, 0, 0, 0, 0, 0, 0}};        // End

int calcTrans(char chr)
{
    int trans = 0;
    if ((chr >= '0') && (chr <= '9')) // Digito
        return (6);
    switch (chr)
    {
    case '+':
    case '-':
    case '*':
    case '/':
        return (7);
    }
    if (chr == '.')
    {
        return (8);
    };
    for (trans = 5; trans > 0; trans--)
        if (chr == chrTrans[trans])
            break;
    return (trans);
}

int sigEdo(int edo, int trans)
{
    return (mtzTrans[edo][trans]);
}

int ejecutaEdo(int edo)
{
    switch (edo)
    {
    case 0:
        break;
    case 1:
        acum1 = 0;
        dec1 = 0;
        printf("%c", chr);
        break;
    case 2:
        printf("%c", chr);
        acum1 *= 10;
        acum1 += (chr - '0');
        break;
    case 3:
        printf("%c", chr);
        acum1 *= 10;
        acum1 += (chr - '0');
        return (2);
    case 4:
        multiplier = 1.0;
        printf("%c", chr);
        break;
    case 5:
        dec1++;
        printf("%c", chr);
        multiplier /= 10.0;
        acum1 += ((chr - '0') * multiplier);
        break;
    case 6:
        dec1++;
        printf("%c", chr);
        multiplier /= 10.0;
        acum1 += ((chr - '0') * multiplier);
        return (5);
    case 7:
        printf("%c", chr);
        switch (chr)
        {
        case '+':
            oper = Suma;
            break;
        case '-':
            oper = Resta;
            break;
        case '*':
            oper = Mult;
            break;
        case '/':
            oper = Div;
            break;
        }
        acum2 = 0; // Preparar la entrada al estado
        dec2 = 0;
        break;
    case 8:
        printf("%c", chr);
        acum2 *= 10;
        acum2 = (float)(chr - '0');
        break;
    case 9:
        printf("%c", chr);
        acum2 *= 10;
        acum2 += (chr - '0');
        return (8);
    case 10:
        multiplier = 1.0;
        printf("%c", chr);
        break;
    case 11:
        dec2++;
        printf("%c", chr);
        multiplier /= 10.0;
        acum2 += ((chr - '0') * multiplier);
        break;
    case 12:
        dec2++;
        printf("%c", chr);
        multiplier /= 10.0;
        acum2 += ((chr - '0') * multiplier);
        return (11);
    case 13:
        printf("%c", chr);
        break;
    case 14:
        printf("%c", chr);
        switch (oper)
        {
        case Suma:
            res = acum1 + acum2;
            break;
        case Resta:
            res = acum1 - acum2;
            break;
        case Mult:
            res = acum1 * acum2;
            break;
        case Div:
            if (acum2)
                res = acum1 / acum2;
            else
                res = -1;
            break;
        }
        printf("%f\n", res);
        sprintf(TxDBuffer, "%f", res);
        WriteFile(serial, &TxDBuffer, strlen(TxDBuffer), &aux, NULL);
        return (0);
    case 99:
        printf("\n<<<Captura cancelada>>>\n");
        return (0); // Estado aceptor, rompe la rutina y marca estado de salida
    }
    return (edo); // Para estados no aceptores regresar el estado ejecutado
}

void main()
{
    printf("Starting serial communication ...\n");
    serial = CreateFile("COM4", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (serial != INVALID_HANDLE_VALUE)
    {
        printf("... Serial OK ... \n");

        // Configurar protocolo y velocidad
        GetCommState(serial, &config);
        config.BaudRate = CBR_9600;
        config.fParity = 0;
        config.fBinary = 1;
        config.StopBits = ONESTOPBIT;
        config.ByteSize = 8;
        SetCommState(serial, &config);

        // Configurar "timeouts"
        touts.ReadTotalTimeoutConstant = 0;
        touts.ReadIntervalTimeout = 0;
        touts.ReadTotalTimeoutMultiplier = 0;
        SetCommTimeouts(serial, &touts);

        while (chr != 27)
        { // El caracter '.' termina la ejecución del programa
            ReadFile(serial, &chr, 1, &aux, NULL);
            if (chr == 27) break;
            trans = calcTrans(chr);             // Calcular la transición según la entrada del teclado
            if (trans)
            {                                   // Validar por transición valida (la transición 0 es inválida)
                edoAnt = edo;                   // Guardar el estado anterior
                edo = sigEdo(edoAnt, trans);    // Calcular el siguiente estado
                if (edoAnt != edo)              // Solo si hay cambio de estado hay que ...
                    edo = ejecutaEdo(edo);      // ... ejecutar el nuevo estado y asignar estado de continuidad
            };
        }
        printf("\nFin!!!\n");
        CloseHandle(serial);
    }
    else
    {
        printf("Error: COM4 inaccesible\n");
    }
}
