#include <sqlite3.h>
#include "dashboard.h"
#include <stdio.h>

int main() {
    sqlite3 *db;
    if (sqlite3_open("planetas.db", &db) != SQLITE_OK) {
        printf("No se pudo abrir la base de datos.\n");
        return 1;
    }

    // Crear tabla si no existe
    const char *sql = "CREATE TABLE IF NOT EXISTS planetas("
        "nombre TEXT PRIMARY KEY,"
        "radio_km REAL,"
        "velocidad_orbital REAL,"
        "distancia_sol REAL,"
        "color1 TEXT,"
        "color2 TEXT,"
        "color3 TEXT,"
        "num_lunas INTEGER);";
    sqlite3_exec(db, sql, 0, 0, 0);

    // Mostrar dashboard en terminal
    show_dashboard(db);

    sqlite3_close(db);
    return 0;
}