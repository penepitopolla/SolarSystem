#ifndef PLANETAS_H
#define PLANETAS_H

#include <sqlite3.h>

#define MAX_COLORES 3
#define MAX_LUNAS 8

#define MAX_COLORES 3

typedef struct {
    char nombre[32];
    float radio_km;
    float velocidad_orbital;
    float distancia_sol;
    char colores[MAX_COLORES][16]; // Ahora guarda el nombre del color
    int num_lunas;
} Planeta;

void agregar_planeta(sqlite3* db);
void editar_planeta(sqlite3* db);
void eliminar_planeta(sqlite3* db);
void listar_planetas(sqlite3* db);

#endif