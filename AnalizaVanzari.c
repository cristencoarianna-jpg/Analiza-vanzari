#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char data[11];
    int id;
    char nume[50];
    char categorie[30];
    char subcategorie[30];
    float pret;
    int cantitate;
    char tara[30];
    char oras[30];
    float venit;
} Tranzactie;

// Funcție să extragem luna din data (YYYY-MM-DD sau DD/MM/YYYY)
int get_luna(const char* data) {
    // Formatul e YYYY-MM-DD în fișier
    if (strlen(data) < 10) return 0;
    
    // Pentru format YYYY-MM-DD (ex: 2001-02-10)
    if (data[4] == '-') {
        return (data[5] - '0') * 10 + (data[6] - '0');
    }
    // Pentru format DD/MM/YYYY (ex: 01/02/2001)
    else if (data[2] == '/') {
        return (data[3] - '0') * 10 + (data[4] - '0');
    }
    return 0;
}

int main() {
    FILE* f = fopen("sales.csv", "r");
    if (!f) {
        printf("Nu gasesc sales.csv\n");
        return 1;
    }

    // Citim header-ul
    char buffer[1024];
    fgets(buffer, sizeof(buffer), f);

    // Alocare dinamica
    Tranzactie* t = NULL;
    int capacitate = 1000; // Începem cu 1000
    int n = 0;
    t = (Tranzactie*)malloc(capacitate * sizeof(Tranzactie));

    while (fgets(buffer, sizeof(buffer), f)) {
        // Parsare simpla - separam prin virgula
        char* p = buffer;
        char* campuri[9];
        int idx = 0;
        
        campuri[idx++] = p;
        while (*p) {
            if (*p == ',') {
                *p = '\0';
                campuri[idx++] = p + 1;
            }
            p++;
        }

        if (idx < 9) continue;

        // Salvam datele
        strcpy(t[n].data, campuri[0]);
        t[n].id = atoi(campuri[1]);
        strcpy(t[n].nume, campuri[2]);
        strcpy(t[n].categorie, campuri[3]);
        strcpy(t[n].subcategorie, campuri[4]);
        t[n].pret = atof(campuri[5]);
        t[n].cantitate = atoi(campuri[6]);
        strcpy(t[n].tara, campuri[7]);
        strcpy(t[n].oras, campuri[8]);
        t[n].venit = t[n].pret * t[n].cantitate;
        n++;

        // Redimensionare
        if (n >= capacitate) {
            capacitate *= 2;
            t = (Tranzactie*)realloc(t, capacitate * sizeof(Tranzactie));
        }
    }
    fclose(f);

    printf("S-au citit %d tranzactii\n\n", n);

    // 1. VENITUL PE LUNI
    printf("1. VENITUL TOTAL PE LUNI:\n");
    float venit_luna[12] = {0};
    int luna_curenta;
    
    for (int i = 0; i < n; i++) {
        luna_curenta = get_luna(t[i].data);
        if (luna_curenta >= 1 && luna_curenta <= 12) {
            venit_luna[luna_curenta - 1] += t[i].venit;
        }
    }
    
    char* nume_luni[] = {"Ian", "Feb", "Mar", "Apr", "Mai", "Iun", 
                          "Iul", "Aug", "Sep", "Oct", "Noi", "Dec"};
    for (int i = 0; i < 12; i++) {
        if (venit_luna[i] > 0)
            printf("%s: %.2f\n", nume_luni[i], venit_luna[i]);
    }
    printf("\n");

// 2. TOP 5 PRODUSE
printf("2. TOP 5 PRODUSE (dupa venit):\n");

// Alocare pentru produse
printf("Alocam memorie...\n");
int* id_unic = (int*)malloc(n * sizeof(int));
float* venit_produs = (float*)malloc(n * sizeof(float));
char (*nume_produs)[50] = (char(*)[50])malloc(n * 50 * sizeof(char));

if (!id_unic || !venit_produs || !nume_produs) {
    printf("Eroare la alocare!\n");
    return 1;
}
printf("Alocare reusita\n");

int n_produse = 0;
printf("Procesam %d tranzactii...\n", n);

for (int i = 0; i < n; i++) {
    if (i % 10000 == 0) printf("Procesat %d...\n", i);
    
    int gasit = 0;
    for (int j = 0; j < n_produse; j++) {
        if (id_unic[j] == t[i].id) {
            venit_produs[j] += t[i].venit;
            gasit = 1;
            break;
        }
    }
    if (!gasit) {
        id_unic[n_produse] = t[i].id;
        strcpy(nume_produs[n_produse], t[i].nume);
        venit_produs[n_produse] = t[i].venit;
        n_produse++;
    }
}

printf("Total produse unice: %d\n", n_produse);
printf("Sortam...\n");

// Sortare
for (int i = 0; i < n_produse - 1; i++) {
    for (int j = i + 1; j < n_produse; j++) {
        if (venit_produs[i] < venit_produs[j]) {
            float temp_f = venit_produs[i];
            venit_produs[i] = venit_produs[j];
            venit_produs[j] = temp_f;
            
            int temp_id = id_unic[i];
            id_unic[i] = id_unic[j];
            id_unic[j] = temp_id;
            
            char temp_n[50];
            strcpy(temp_n, nume_produs[i]);
            strcpy(nume_produs[i], nume_produs[j]);
            strcpy(nume_produs[j], temp_n);
        }
    }
}

printf("Afisam rezultatele:\n");
for (int i = 0; i < 5 && i < n_produse; i++) {
    printf("%d. %s - %.2f\n", i+1, nume_produs[i], venit_produs[i]);
}
printf("\n");

free(id_unic);
free(venit_produs);
free(nume_produs);
printf("Eliberat memoria pentru produse\n");

    // 3. DISTRIBUȚIA PE CATEGORII
    printf("3. VANZARI PE CATEGORII:\n");
    char categorii[100][30];
    float venit_cat[100] = {0};
    int n_categ = 0;
    
    for (int i = 0; i < n; i++) {
        int gasit = 0;
        for (int j = 0; j < n_categ; j++) {
            if (strcmp(categorii[j], t[i].categorie) == 0) {
                venit_cat[j] += t[i].venit;
                gasit = 1;
                break;
            }
        }
        if (!gasit && n_categ < 100) {
            strcpy(categorii[n_categ], t[i].categorie);
            venit_cat[n_categ] = t[i].venit;
            n_categ++;
        }
    }
    
    float total = 0;
    for (int i = 0; i < n_categ; i++) total += venit_cat[i];
    for (int i = 0; i < n_categ; i++) {
        printf("%s: %.2f (%.1f%%)\n", categorii[i], venit_cat[i], 
               (venit_cat[i] / total) * 100);
    }
    printf("\n");

 // 4. ORAȘELE CU CELE MAI MARI VÂNZĂRI
printf("4. ORASUL CU CELE MAI MARI VANZARI DIN FIECARE TARA:\n");
printf("========================================================\n\n");

int max_tari = 5000;
char (*tari)[100] = (char(*)[100])malloc(max_tari * 100 * sizeof(char));
char (*oras_best)[100] = (char(*)[100])malloc(max_tari * 100 * sizeof(char));
float* venit_best = (float*)malloc(max_tari * sizeof(float));

int n_tari = 0;

for (int i = 0; i < n; i++) {
    int gasit_tara = -1;
    for (int j = 0; j < n_tari; j++) {
        if (strcmp(tari[j], t[i].tara) == 0) {
            gasit_tara = j;
            break;
        }
    }
    
    if (gasit_tara == -1 && n_tari < max_tari) {
        strcpy(tari[n_tari], t[i].tara);
        strcpy(oras_best[n_tari], t[i].oras);
        venit_best[n_tari] = t[i].venit;
        n_tari++;
    } else if (gasit_tara != -1) {
        if (t[i].venit > venit_best[gasit_tara]) {
            strcpy(oras_best[gasit_tara], t[i].oras);
            venit_best[gasit_tara] = t[i].venit;
        }
    }
}

// Sortare
for (int i = 0; i < n_tari - 1; i++) {
    for (int j = i + 1; j < n_tari; j++) {
        if (venit_best[i] < venit_best[j]) {
            float temp_v = venit_best[i];
            venit_best[i] = venit_best[j];
            venit_best[j] = temp_v;
            
            char temp_t[100];
            strcpy(temp_t, tari[i]);
            strcpy(tari[i], tari[j]);
            strcpy(tari[j], temp_t);
            
            strcpy(temp_t, oras_best[i]);
            strcpy(oras_best[i], oras_best[j]);
            strcpy(oras_best[j], temp_t);
        }
    }
}

// Afișare simplă și curată
printf("Primele 10 tari cu cele mai mari vanzari:\n\n");

int afisate = (n_tari < 10) ? n_tari : 10;
for (int i = 0; i < afisate; i++) {
    printf("%d. %s\n", i+1, tari[i]);
    printf("   Oras: %s\n", oras_best[i]);
    printf("   Venit: %.2f lei\n", venit_best[i]);
    printf("   ------------------------\n");
}
printf("\n");

free(tari);
free(oras_best);
free(venit_best);
// 5. TENDINȚE PE SUBCATEGORII
printf("5. TENDINTE PE SUBCATEGORII (primele 3 luni):\n");

// Alocare pentru subcategorii
int max_sub = 5000;
char (*subc)[30] = (char(*)[30])malloc(max_sub * 30 * sizeof(char));
float (*venit_sub_luna)[12] = (float(*)[12])malloc(max_sub * 12 * sizeof(float));
int n_sub = 0;

if (!subc || !venit_sub_luna) {
    printf("Eroare la alocare pentru subcategorii!\n");
    return 1;
}

// Inițializare
for (int i = 0; i < max_sub; i++)
    for (int j = 0; j < 12; j++)
        venit_sub_luna[i][j] = 0;

for (int i = 0; i < n; i++) {
    int luna = get_luna(t[i].data) - 1;
    if (luna < 0 || luna >= 12) continue;
    
    int gasit = 0;
    for (int j = 0; j < n_sub; j++) {
        if (strcmp(subc[j], t[i].subcategorie) == 0) {
            venit_sub_luna[j][luna] += t[i].venit;
            gasit = 1;
            break;
        }
    }
    if (!gasit && n_sub < max_sub) {
        strcpy(subc[n_sub], t[i].subcategorie);
        venit_sub_luna[n_sub][luna] = t[i].venit;
        n_sub++;
    }
}

printf("Total subcategorii: %d\n", n_sub);

// Calculam totalul pe subcategorie pentru a sorta
float* total_sub = (float*)malloc(n_sub * sizeof(float));
for (int i = 0; i < n_sub; i++) {
    total_sub[i] = 0;
    for (int j = 0; j < 12; j++)
        total_sub[i] += venit_sub_luna[i][j];
}

// Sortam subcategoriile dupa total (cele mai mari)
for (int i = 0; i < n_sub - 1; i++) {
    for (int j = i + 1; j < n_sub; j++) {
        if (total_sub[i] < total_sub[j]) {
            float temp_t = total_sub[i];
            total_sub[i] = total_sub[j];
            total_sub[j] = temp_t;
            
            char temp_s[30];
            strcpy(temp_s, subc[i]);
            strcpy(subc[i], subc[j]);
            strcpy(subc[j], temp_s);
            
            for (int k = 0; k < 12; k++) {
                float temp_v = venit_sub_luna[i][k];
                venit_sub_luna[i][k] = venit_sub_luna[j][k];
                venit_sub_luna[j][k] = temp_v;
            }
        }
    }
}

// Afișăm pentru primele 5 subcategorii
for (int i = 0; i < 5 && i < n_sub; i++) {
    printf("%s: ", subc[i]);
    for (int l = 0; l < 3; l++) {  // primele 3 luni
        if (venit_sub_luna[i][l] > 0)
            printf("Luna %d: %.0f  ", l+1, venit_sub_luna[i][l]);
    }
    printf("\n");
}
printf("\n");

free(total_sub);
free(subc);
free(venit_sub_luna);
    free(t);

    printf("Program terminat cu succes!\n");
    return 0;
}