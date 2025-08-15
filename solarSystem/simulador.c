#include "simulador.h"
#include <raylib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include "planetas.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>

#define MAX_PLANETAS 16

bool pausado = false;

Color nombre_a_color(const char *nombre) {
    if (strcmp(nombre, "RED") == 0) return RED;
    if (strcmp(nombre, "BLUE") == 0) return BLUE;
    if (strcmp(nombre, "GREEN") == 0) return GREEN;
    if (strcmp(nombre, "YELLOW") == 0) return YELLOW;
    if (strcmp(nombre, "ORANGE") == 0) return ORANGE;
    if (strcmp(nombre, "PURPLE") == 0) return PURPLE;
    if (strcmp(nombre, "BROWN") == 0) return BROWN;
    if (strcmp(nombre, "GRAY") == 0) return GRAY;
    if (strcmp(nombre, "WHITE") == 0) return WHITE;
    if (strcmp(nombre, "BLACK") == 0) return BLACK;
    return RAYWHITE; // Color por defecto
}

// cantidad de manchas
typedef struct {
    Vector3 secundarias[124];
    Vector3 terciarias[32];
} ManchasPlaneta;

static inline Vector3 Vector3Subtract(Vector3 v1, Vector3 v2) {
    return (Vector3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}
static inline Vector3 Vector3Scale(Vector3 v, float scale) {
    return (Vector3){v.x * scale, v.y * scale, v.z * scale};
}
static inline Vector3 Vector3Add(Vector3 v1, Vector3 v2) {
    return (Vector3){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

Vector3 rotar_horizontal(Vector3 pos, float ang) {
    // Rota el vector alrededor del eje Y (horizontal)
    float x = pos.x * cosf(ang) - pos.z * sinf(ang);
    float z = pos.x * sinf(ang) + pos.z * cosf(ang);
    return (Vector3){x, pos.y, z};
}

// Dibuja las órbitas de los planetas
void dibujar_orbita(Vector3 centro, float radio, Color color) {
    int segmentos = 64;
    for (int i = 0; i < segmentos; i++) {
        float ang1 = (2 * PI * i) / segmentos;
        float ang2 = (2 * PI * (i + 1)) / segmentos;
        Vector3 p1 = { centro.x + cosf(ang1) * radio, centro.y, centro.z + sinf(ang1) * radio };
        Vector3 p2 = { centro.x + cosf(ang2) * radio, centro.y, centro.z + sinf(ang2) * radio };
        DrawLine3D(p1, p2, color);
    }
}

void iniciar_simulacion(sqlite3* db) {
    Planeta planetas[MAX_PLANETAS];
    int n = 0;

    // Inicializar audio
    InitAudioDevice();
    Music musica = LoadMusicStream("soundspace.wav"); // Archivo corto
    PlayMusicStream(musica);
    SetMusicVolume(musica, 0.5f);
    if (musica.ctxData == NULL) {
    printf("No se pudo cargar el archivo de audio.\n");
}

    // Leer planetas de la base de datos
    sqlite3_stmt *stmt;
    const char *sql = "SELECT nombre, radio_km, velocidad_orbital, distancia_sol, color1, color2, color3, num_lunas FROM planetas;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW && n < MAX_PLANETAS) {
            strcpy(planetas[n].nombre, (const char*)sqlite3_column_text(stmt, 0));
            planetas[n].radio_km = (float)sqlite3_column_double(stmt, 1);
            planetas[n].velocidad_orbital = (float)sqlite3_column_double(stmt, 2);
            planetas[n].distancia_sol = (float)sqlite3_column_double(stmt, 3);
            strcpy(planetas[n].colores[0], (const char*)sqlite3_column_text(stmt, 4));
            strcpy(planetas[n].colores[1], (const char*)sqlite3_column_text(stmt, 5));
            strcpy(planetas[n].colores[2], (const char*)sqlite3_column_text(stmt, 6));
            planetas[n].num_lunas = sqlite3_column_int(stmt, 7);
            n++;
        }
        sqlite3_finalize(stmt);
    }
ManchasPlaneta manchas[MAX_PLANETAS];
srand((unsigned int)time(NULL)); // Inicializa la semilla aleatoria

for (int i = 0; i < n; i++) {
    float radio = 2.0f + planetas[i].radio_km * 0.001f;
    float x = 0, z = 0; // Las posiciones relativas, se suman en el dibujo

    for (int i = 0; i < n; i++) {
    float radio = 2.0f + planetas[i].radio_km * 0.001f;

    // Secundarias (más cantidad, más pegadas)
    for (int m = 0; m < 124; m++) {
        float theta = ((float)rand() / RAND_MAX) * 2 * PI;
        float phi = ((float)rand() / RAND_MAX) * PI;
        float factor = 0.93f + ((float)rand() / RAND_MAX) * 0.04f; // entre 0.93 y 0.97
        manchas[i].secundarias[m].x = cosf(theta) * sinf(phi) * radio * factor;
        manchas[i].secundarias[m].y = sinf(theta) * sinf(phi) * radio * factor;
        manchas[i].secundarias[m].z = cosf(phi) * radio * factor;
    }
    // Terciarias (menos cantidad, también más pegadas)
    for (int m = 0; m < 32; m++) {
        float theta = ((float)rand() / RAND_MAX) * 2 * PI;
        float phi = ((float)rand() / RAND_MAX) * PI;
        float factor = 0.93f + ((float)rand() / RAND_MAX) * 0.04f;
        manchas[i].terciarias[m].x = cosf(theta) * sinf(phi) * radio * factor;
        manchas[i].terciarias[m].y = sinf(theta) * sinf(phi) * radio * factor;
        manchas[i].terciarias[m].z = cosf(phi) * radio * factor;
    }
}


    // --- Simulación Raylib ---
    
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Sistema Solar 3D");
    ToggleFullscreen();
    SetExitKey(0); // Desactiva el cierre automático con ESC

   #define NUM_ESTRELLAS 750
float estrellas_x[NUM_ESTRELLAS];
int estrellas_y[NUM_ESTRELLAS];
srand(42); // Semilla fija para que siempre sean iguales

for (int s = 0; s < NUM_ESTRELLAS; s++) {
    estrellas_x[s] = rand() % 800;
    estrellas_y[s] = rand() % 600;
}

    SetTargetFPS(60);

    Camera camera = { 0 };
camera.position = (Vector3){ 0.0f, 50.0f, 120.0f };
camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
camera.fovy = 45.0f;
camera.projection = CAMERA_PERSPECTIVE;
    float tiempo = 0.0f;

float cam_angle_x = 0.0f;
float cam_angle_y = 15.0f * PI / 180.0f; // 15 grados en radianes
bool dragging = false;
int last_mouse_x = 0, last_mouse_y = 0;
float cam_distance = 500.0f; // Distancia de la cámara al Sol
    
int planeta_seleccionado = -1; // -1: ninguno seleccionado

float lunas_phi[MAX_PLANETAS][MAX_LUNAS];
float lunas_velocidad[MAX_PLANETAS][MAX_LUNAS];
for (int i = 0; i < n; i++) {
    for (int l = 0; l < planetas[i].num_lunas; l++) {
        lunas_phi[i][l] = ((float)rand() / RAND_MAX) * PI;
        lunas_velocidad[i][l] = 0.2f + ((float)rand() / RAND_MAX) * 0.8f;
}

void cargar_planetas(sqlite3* db, Planeta planetas[], int *n) {
    *n = 0;
    sqlite3_stmt *stmt;
    const char *sql = "SELECT nombre, radio_km, velocidad_orbital, distancia_sol, color1, color2, color3, num_lunas FROM planetas;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW && *n < MAX_PLANETAS) {
            strcpy(planetas[*n].nombre, (const char*)sqlite3_column_text(stmt, 0));
            planetas[*n].radio_km = (float)sqlite3_column_double(stmt, 1);
            planetas[*n].velocidad_orbital = (float)sqlite3_column_double(stmt, 2);
            planetas[*n].distancia_sol = (float)sqlite3_column_double(stmt, 3);
            strcpy(planetas[*n].colores[0], (const char*)sqlite3_column_text(stmt, 4));
            strcpy(planetas[*n].colores[1], (const char*)sqlite3_column_text(stmt, 5));
            strcpy(planetas[*n].colores[2], (const char*)sqlite3_column_text(stmt, 6));
            planetas[*n].num_lunas = sqlite3_column_int(stmt, 7);
            (*n)++;
        }
        sqlite3_finalize(stmt);
    }
}

float tiempo_actualizacion = 0.0f;

int ancho = GetScreenWidth();
    int alto = GetScreenHeight();

for (int s = 0; s < NUM_ESTRELLAS; s++) {
    estrellas_x[s] = rand() % ancho;
    estrellas_y[s] = rand() % alto;
}

// Bucle principal---------------------------------------------------------------

while (!WindowShouldClose()) {

    if (IsKeyPressed(KEY_Q)) {
    break; // Sale del bucle principal y termina la simulación
}
    UpdateMusicStream(musica);// Actualiza la música

    if (IsKeyPressed(KEY_SPACE)) { // Pausar/continuar
        pausado = !pausado;
    }

    if (!pausado) {
        tiempo += GetFrameTime(); // Actualiza el tiempo
    }

    // --- Control de cámara personalizado ---\\ 

    // Detecta inicio de arrastre
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        dragging = true;
        last_mouse_x = GetMouseX();
        last_mouse_y = GetMouseY();
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        dragging = false;
    }
    // Si está arrastrando, actualiza los ángulos
    if (dragging) {
        int dx = GetMouseX() - last_mouse_x;
        int dy = GetMouseY() - last_mouse_y;
        cam_angle_x -= dx * 0.004f; // Sensibilidad horizontal
        cam_angle_y += dy * 0.004f; // Sensibilidad vertical
        last_mouse_x = GetMouseX();
        last_mouse_y = GetMouseY();
    }
    // Zoom con la rueda
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        cam_distance -= wheel * 5.0f; // Sensibilidad de zoom
        if (cam_distance < 50.0f) cam_distance = 50.0f;
        if (cam_distance > 6000.0f) cam_distance = 6000.0f;

    }

    // Calcula posiciones y radios de los planetas
Vector3 posiciones[MAX_PLANETAS];
float radios[MAX_PLANETAS];

for (int i = 0; i < n; i++) {
    float ang = tiempo * planetas[i].velocidad_orbital * 0.01f;
    float x = cosf(ang) * planetas[i].distancia_sol * 0.1f;
    float z = sinf(ang) * planetas[i].distancia_sol * 0.1f;
    posiciones[i] = (Vector3){x, 0, z};
    radios[i] = 2.0f + planetas[i].radio_km * 0.001f;
}

    // Selección de planeta
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < n; i++) {
        Vector3 pos3d = posiciones[i];
        Vector2 pos2d = GetWorldToScreen(pos3d, camera);
        float radio_pantalla = radios[i] * 4.0f; // Ajusta el factor según escala visual
        if (CheckCollisionPointCircle(mouse, pos2d, radio_pantalla)) {
            planeta_seleccionado = i;
            cam_distance = 160.0f; // acercamiento al planeta
            break;
        }
    }
}

// Volver a la vista del Sol con ESC
if (IsKeyPressed(KEY_ESCAPE)) {
    planeta_seleccionado = -1;
    // Restablece la cámara a la vista original
    cam_angle_x = 0.0f;
    cam_angle_y = 30.0f * PI / 180.0f; // 30 grados en radianes
    cam_distance = 500.0f;
    camera.target = (Vector3){0, 0, 0};
}

// Determina el centro de rotación
Vector3 centro;
if (planeta_seleccionado >= 0) {
    centro = posiciones[planeta_seleccionado];
    camera.target = centro;
} else {
    centro = (Vector3){0, 0, 0}; // El Sol por defecto
    camera.target = centro;
}

// Calcula la posición de la cámara en base a los ángulos, usando el centro como eje
camera.position.x = centro.x + sinf(cam_angle_x) * cosf(cam_angle_y) * cam_distance;
camera.position.y = centro.y + sinf(cam_angle_y) * cam_distance;
camera.position.z = centro.z + cosf(cam_angle_x) * cosf(cam_angle_y) * cam_distance;

    //comienza el dibujo
    BeginDrawing();
    ClearBackground((Color){10, 10, 30, 255}); // Azul oscuro
    if (pausado) {
        DrawText("Simulacion pausada (SPACE para continuar)", 10, 90, 20, RED);
    }
      
    // Dibuja estrellas de fondo
        for (int s = 0; s < NUM_ESTRELLAS; s++) {
    estrellas_x[s] += 0.05f;
    if (estrellas_x[s] > ancho) {
        estrellas_x[s] = 0;
        estrellas_y[s] = rand() % alto;
    }
    DrawPixel((int)estrellas_x[s], estrellas_y[s], RAYWHITE);
}

    // Comienza el modo 3D
        BeginMode3D(camera);

      // Sol en el centro
      DrawSphere((Vector3){0,0,0}, 16.0f, YELLOW);
      Color brillo = (Color){255, 255, 100, 65}; // Amarillo claro, alfa bajo
      DrawSphereEx((Vector3){0,0,0}, 19.0f, 32, 32, brillo);
                
      // Planetas
for (int i = 0; i < n; i++) {
    float ang = tiempo * planetas[i].velocidad_orbital * 0.01f;
    float x = cosf(ang) * planetas[i].distancia_sol * 0.1f;
    float z = sinf(ang) * planetas[i].distancia_sol * 0.1f;
    float radio = 2.0f + planetas[i].radio_km * 0.001f;
    float rotacion_eje = tiempo * planetas[i].velocidad_orbital * 0.05f;

    // Dibuja la órbita (no para el Sol)
    if (planetas[i].distancia_sol > 0.0f) {
        dibujar_orbita((Vector3){0, 0, 0}, planetas[i].distancia_sol * 0.1f, GRAY);
    }

    // Dibuja la esfera base con el color principal
    Color color_principal = nombre_a_color(planetas[i].colores[0]);
    DrawSphereEx((Vector3){x, 0, z}, radio, 16, 16, color_principal);

   // Manchas secundarias (aleatorias y girando horizontalmente)
Color color_secundario = nombre_a_color(planetas[i].colores[1]);
for (int m = 0; m < 124; m++) {
    Vector3 rel = rotar_horizontal(manchas[i].secundarias[m], rotacion_eje);
    Vector3 pos = { x + rel.x, rel.y, z + rel.z };
    DrawSphere(pos, radio * 0.08f, color_secundario);
}

// Manchas terciarias (aleatorias y girando horizontalmente)
Color color_terciario = nombre_a_color(planetas[i].colores[2]);
for (int m = 0; m < 32; m++) {
    Vector3 rel = rotar_horizontal(manchas[i].terciarias[m], rotacion_eje);
    Vector3 pos = { x + rel.x, rel.y, z + rel.z };
    DrawSphere(pos, radio * 0.06f, color_terciario);
}

    // Lunas
     for (int l = 0; l < planetas[i].num_lunas; l++) {
    float velocidad_luna = lunas_velocidad[i][l]; // velocidad única para cada luna
    float theta = tiempo * velocidad_luna + l * (2 * PI / planetas[i].num_lunas);
    float phi = lunas_phi[i][l];
    float distancia_luna = radio * 2.0f + l * 1.0f;

    Vector3 luna_pos = {
        x + cosf(theta) * distancia_luna,
        sinf(phi) * radio * 1.f,
        z + sinf(theta) * distancia_luna
    };
    DrawSphere(luna_pos, radio * 0.1f, GRAY);
}
   }

        EndMode3D();
if (planeta_seleccionado >= 0) {
    int panel_ancho = 260;
    int panel_alto = 180;
    int panel_x = ancho - panel_ancho - 20; // 20 px de margen derecho
    int panel_y = 100;

    // Marco blanco
    DrawRectangle(panel_x - 2, panel_y - 2, panel_ancho + 4, panel_alto + 4, WHITE);

    // Panel negro semitransparente
    DrawRectangle(panel_x, panel_y, panel_ancho, panel_alto, Fade(BLACK, 0.7f));

    DrawText("[Presione ESC para salir de la visualización]", ancho - MeasureText("[Presione ESC para salir de la visualización]", 18) - 20, panel_y + panel_alto + 20, 18, RED);
    DrawText(TextFormat("Nombre: %s", planetas[planeta_seleccionado].nombre), panel_x + 10, panel_y + 10, 18, RAYWHITE);
    DrawText(TextFormat("Radio: %.0f km", planetas[planeta_seleccionado].radio_km), panel_x + 10, panel_y + 30, 18, RAYWHITE);
    DrawText(TextFormat("Distancia al Sol: %.0f Millones km", planetas[planeta_seleccionado].distancia_sol), panel_x + 10, panel_y + 50, 18, RAYWHITE);
    DrawText(TextFormat("Lunas: %d", planetas[planeta_seleccionado].num_lunas), panel_x + 10, panel_y + 70, 18, RAYWHITE);
    DrawText(TextFormat("Velocidad orbital: %.2f km/s", planetas[planeta_seleccionado].velocidad_orbital), panel_x + 10, panel_y + 90, 18, RAYWHITE);
}


        DrawText("Controles: Space-bar para pausar, Mouse para rotar, rueda para zoom", 10, 10, 20, RAYWHITE);
        DrawText("Presione Q para salir de la simulacion", 10, 30, 20, RAYWHITE);
        EndDrawing();
    }

     UnloadMusicStream(musica);
    CloseAudioDevice();
    CloseWindow();
    return;
}}}

//gcc main.c dashboard.c planetas.c simulador.c -o solar.exe -lsqlite3 -lraylib -lopengl32 -lgdi32 -lwinmm