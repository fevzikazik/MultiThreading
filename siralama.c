#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>

#define ELEMAN_SAYISI 90
#define URETIM_SINIRI 1000


pthread_mutex_t mutex;

unsigned char kullanilan_sayilar[URETIM_SINIRI] = { 0 };

typedef struct
{
    int baslangic;
    int bitis;
    short ip_no;
} parametre;

int* ilkDizi;
int* sonDizi;

void* sirala(void* parametreler);               /* sıralama iş parçası fonksiyonu */
void* birlestirvesirala(void* parametreler);    /* sonuçları birleştiren iş parçası fonksiyonu */

int main()
{

    pthread_mutex_init(&mutex,NULL);

    ilkDizi = (int*) malloc(ELEMAN_SAYISI * sizeof(*ilkDizi));
    sonDizi = (int*) malloc(ELEMAN_SAYISI * sizeof(*sonDizi));

    srand((unsigned)time(0));
    printf("\nURETILEN ELEMANLAR:\n");

    /* Floyd Algoritması ile rastgele benzersiz sayı üretimi. O(ELEMAN_SAYISI) */
    int in, im = 0;

    for (in = URETIM_SINIRI - ELEMAN_SAYISI; in < URETIM_SINIRI && im < ELEMAN_SAYISI; ++in)
    {
        int s = rand() % (in + 1);      /* rastgele bir sayı üret */

        if (kullanilan_sayilar[s])      /* üretilen sayıyı zaten kullandık */
            s = in;                     /* üretilen sayı yerine in'yi kullan */

        assert(!kullanilan_sayilar[s]); /* kullanılacak sayı kullanılmamış olmalı */

        ilkDizi[im++] = s;              /* sayıyı kullan */

        kullanilan_sayilar[s] = 1;      /* kullanılan sayılara indis olarak ekle */
    }

    assert(im == ELEMAN_SAYISI);        /* im = ELEMAN_SAYISI olmalı */
    /* Floyd Algoritması */

    /* Elemanların yazdırılması */
    printf("\n");

    for (unsigned int i = 0; i < ELEMAN_SAYISI; ++i)
    {
        printf("%d\t", ilkDizi[i]);

        if ((i + 1) % 10 == 0)
            printf("\n");
    }

    printf("\n");
    /* Elemanların yazdırılması */

    pthread_t isleyiciler[4];

    /* Birinci sıralama iş parçasının oluşturulması */
    parametre* p = (parametre*) malloc(sizeof(parametre));
    p->baslangic = 0;
    p->bitis = 30;
    p->ip_no = 1;
    pthread_create(&isleyiciler[0], 0, sirala, p);
    /* Birinci sıralama iş parçasının oluşturulması */


    /* İkinci sıralama iş parçasının oluşturulması */
    p = (parametre*) malloc(sizeof(parametre));
    p->baslangic = 30;
    p->bitis = 60;
    p->ip_no = 2;
    pthread_create(&isleyiciler[1], 0, sirala, p);
    /* İkinci sıralama iş parçasının oluşturulması */


    /* Üçüncü sıralama iş parçasının oluşturulması */
    p = (parametre*) malloc(sizeof(parametre));
    p->baslangic = 60;
    p->bitis = 90;
    p->ip_no = 3;
    pthread_create(&isleyiciler[2], 0, sirala, p);
    /* Üçüncü sıralama iş parçasının oluşturulması */

    /* Birleştirmek için ilk üç iş parçasının bitmesini bekle */
    for (int i = 0; i < 3; i++)
    {
        pthread_join(isleyiciler[i], NULL);
    }

    /* Birleştirmek için kullanılacak iş parçasının oluşturulması */
    p = (parametre*) malloc(sizeof(parametre));
    p->baslangic = 0;
    p->bitis = ELEMAN_SAYISI;
    p->ip_no = 4;
    pthread_create(&isleyiciler[3], 0, birlestirvesirala, p);
    /* Birleştirmek için kullanılacak iş parçasının oluşturulması */

    pthread_join(isleyiciler[3], NULL); /* Dördüncü iş parçasını bekle */

    free(ilkDizi); /* baz diziyi bellekten temizle */

    /* Sıralanmış dizi elemanlarının dosyaya yazdırılması */
    FILE* d = fopen("son.txt", "w");

    if (d == NULL)
    {
        printf("Dosya hatası oluştu.\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < ELEMAN_SAYISI; i++)
    {
        fprintf(d, "%d. Eleman: %d\n", i + 1, sonDizi[i]);
    }

    fclose(d);

    free(sonDizi); /* son diziyi bellekten temizle */

    return EXIT_SUCCESS;
}

void* sirala(void* parametreler)
{
    pthread_mutex_lock(&mutex);

    printf("\n------------------------------------------------------------\n");
    parametre* p = (parametre*) parametreler;

    int baslangic = p->baslangic;
    int bitis = p->bitis;

    /* Alinan dizinin gosterilmesi */
    printf("\n(Thread: %d) SIRALAMA IS PARCASI ICIN ALINAN DIZI:\n", p->ip_no);
    for (int i = baslangic; i < bitis; i++)
    {
        printf("%d-", ilkDizi[i]);
    }

    /* Alinan dizinin siralanmasi BubbleSort Algoritması*/
    int temp = 0;
    for (int i = baslangic; i < bitis; i++)
    {
        for (int j = baslangic; j < bitis - 1; j++)
        {
            if (ilkDizi[j] > ilkDizi[j + 1])
            {
                temp = ilkDizi[j];
                ilkDizi[j] = ilkDizi[j + 1];
                ilkDizi[j + 1] = temp;
            }
        }
    }
    printf("\n");
    /* Siralanmis dizinin yazdirilmasi */
    printf("\n(Thread: %d) SIRALAMA IS PARCASI SONRASI SIRALANAN DIZI:\n", p->ip_no);
    for (int i = baslangic; i < bitis; i++)
    {
        printf("%d-", ilkDizi[i]);
    }
    printf("\n");


    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
}

void* birlestirvesirala(void* parametreler)
{
    printf("\n------------------------------------------------------------\n\n");
    parametre* p = (parametre*) parametreler;

    int baslangic = p->baslangic;
    int bitis = p->bitis;

    /* Dizinin sıralanarak birleştirilmesi */
    int i = 0, j = 0, k = 30, m = 60;



    for (i = baslangic; i < bitis; i++)
    {
        if (j == 30)
        {
            if (ilkDizi[k] < ilkDizi[m] && k<60)
                sonDizi[i] = ilkDizi[k++];
            else
                sonDizi[i] = ilkDizi[m++];
        }
        else if (k == 60)
        {
            if (ilkDizi[j] < ilkDizi[m] && j<30)
                sonDizi[i] = ilkDizi[j++];
            else
                sonDizi[i] = ilkDizi[m++];
        }
        else if (m == 90)
        {
            if (ilkDizi[k] < ilkDizi[j] && k<60)
                sonDizi[i] = ilkDizi[k++];
            else
                sonDizi[i] = ilkDizi[j++];
        }
        else
        {
            if (ilkDizi[j] < ilkDizi[k])
            {
                if (ilkDizi[j] < ilkDizi[m])
                    sonDizi[i] = ilkDizi[j++];
                else
                  sonDizi[i] = ilkDizi[m++];

            }
            else
            {
                if (ilkDizi[k] < ilkDizi[m])
                    sonDizi[i] = ilkDizi[k++];
                else
                  sonDizi[i] = ilkDizi[m++];
            }
        }
    }
    /* Sıralanmış dizinin yazdırılması */
    printf("(Thread: %d) BIRLESTIRME VE SIRALAMA SONRASI DIZI:\n", p->ip_no);
    for (int i = baslangic; i < bitis; i++)
    {
        printf("%d\t", sonDizi[i]);

        if (i != 0 && (i + 1) % 10 == 0)
            printf("\n");
    }
    printf("\n");

    pthread_exit(0);
}
