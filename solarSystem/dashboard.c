#include "dashboard.h"
#include "planetas.h"
#include "simulador.h"
#include <stdio.h>

void show_dashboard(sqlite3* db) {
    int opcion;
    do {
        printf("\n--- PANEL SISTEMA SOLAR ---\n");
        printf("[1] Agregar planeta\n");
        printf("[2] Editar planeta\n");
        printf("[3] Eliminar planeta\n");
        printf("[4] Listar planetas\n");
        printf("[5] Iniciar simulacion\n");
        printf("[0] Salir\n");
        printf("Opcion: ");
        scanf("%d", &opcion);
        switch(opcion) {
            case 1: agregar_planeta(db); break;
            case 2: editar_planeta(db); break;
            case 3: eliminar_planeta(db); break;
            case 4: listar_planetas(db); break;
            case 5: iniciar_simulacion(db); break;
        }
    } while(opcion != 0);
}