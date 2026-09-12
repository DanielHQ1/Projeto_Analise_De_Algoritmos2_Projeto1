/*
 * Projeto: Backup offline
 * Integrantes: substitua pelos nomes do seu grupo antes da entrega.
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ARQUIVOS 50
#define TAMANHO_CAMINHO 1024

typedef struct {
    long long tamanho;
    int original;
} Arquivo;

typedef struct {
    Arquivo arquivo[MAX_ARQUIVOS];
    int quantidade;
    long long capacidade;
    long long restante[MAX_ARQUIVOS + 1];
    unsigned char atual[MAX_ARQUIVOS];
    unsigned char melhor[MAX_ARQUIVOS];
    long long melhor_diferenca;
    int encontrou;
} Busca;

static long long absoluto(long long a, long long b) {
    return a >= b ? a - b : b - a;
}

/* Ordenacao propria, sem usar bibliotecas prontas de ordenacao. */
static void ordenar(Arquivo v[], int n) {
    int i, j, maior;
    for (i = 0; i < n - 1; i++) {
        maior = i;
        for (j = i + 1; j < n; j++)
            if (v[j].tamanho > v[maior].tamanho)
                maior = j;
        if (maior != i) {
            Arquivo aux = v[i];
            v[i] = v[maior];
            v[maior] = aux;
        }
    }
}

/* Backtracking: cada arquivo e colocado no pendrive A ou no B. */
static void distribuir(Busca *b, int i, long long usado_a, long long usado_b) {
    long long diferenca, limite, tamanho;

    if (usado_a > b->capacidade || usado_b > b->capacidade ||
        b->melhor_diferenca == 0)
        return;

    diferenca = absoluto(usado_a, usado_b);
    limite = diferenca > b->restante[i] ? diferenca - b->restante[i] : 0;
    if (limite >= b->melhor_diferenca)
        return;

    if (i == b->quantidade) {
        b->melhor_diferenca = diferenca;
        b->encontrou = 1;
        memcpy(b->melhor, b->atual, (size_t)b->quantidade);
        return;
    }

    tamanho = b->arquivo[i].tamanho;
    /* O menos ocupado e tentado primeiro para aumentar as podas. */
    if (usado_a < usado_b) {
        b->atual[i] = 1;
        distribuir(b, i + 1, usado_a + tamanho, usado_b);
        b->atual[i] = 0;
        distribuir(b, i + 1, usado_a, usado_b + tamanho);
    } else {
        b->atual[i] = 0;
        distribuir(b, i + 1, usado_a, usado_b + tamanho);
        b->atual[i] = 1;
        distribuir(b, i + 1, usado_a + tamanho, usado_b);
    }
    b->atual[i] = 0;
}

static void imprimir_grupo(FILE *saida, const long long tamanho[],
                           const unsigned char no_a[], int n, int grupo_a) {
    int i;
    for (i = 0; i < n; i++)
        if (no_a[i] == (unsigned char)grupo_a)
            fprintf(saida, "%lld GB\n", tamanho[i]);
}

static int processar(FILE *entrada, FILE *saida) {
    long long testes, teste;

    if (fscanf(entrada, "%lld", &testes) != 1 || testes <= 0)
        return 0;

    for (teste = 0; teste < testes; teste++) {
        Busca busca = {0};
        long long total, quantidade_lida, tamanhos[MAX_ARQUIVOS], soma = 0;
        unsigned char grupo_a[MAX_ARQUIVOS] = {0};
        int valido = 1, i, j;

        busca.melhor_diferenca = LLONG_MAX;
        if (fscanf(entrada, "%lld%lld", &total, &quantidade_lida) != 2 ||
            total <= 0 || total % 2 != 0 || quantidade_lida < 1 ||
            quantidade_lida > MAX_ARQUIVOS)
            return 0;

        busca.quantidade = (int)quantidade_lida;
        busca.capacidade = total / 2;
        for (i = 0; i < busca.quantidade; i++) {
            if (fscanf(entrada, "%lld", &tamanhos[i]) != 1 ||
                tamanhos[i] <= 0 || tamanhos[i] > LLONG_MAX - soma) {
                valido = 0;
                break;
            }
            for (j = 0; j < i; j++)
                if (tamanhos[i] == tamanhos[j])
                    valido = 0;
            if (!valido)
                break;
            soma += tamanhos[i];
            busca.arquivo[i].tamanho = tamanhos[i];
            busca.arquivo[i].original = i;
        }
        if (!valido)
            return 0;

        ordenar(busca.arquivo, busca.quantidade);
        for (i = busca.quantidade - 1; i >= 0; i--)
            busca.restante[i] = busca.restante[i + 1] + busca.arquivo[i].tamanho;
        if (soma <= total)
            distribuir(&busca, 0, 0, 0);

        fprintf(saida, "%lld GB\n", total);
        if (!busca.encontrou) {
            fprintf(saida, "Impossível gravar todos os arquivos nos pendrives.\n");
        } else {
            for (i = 0; i < busca.quantidade; i++)
                if (busca.melhor[i])
                    grupo_a[busca.arquivo[i].original] = 1;
            fprintf(saida, "Pendrive A (%lld GB)\n", busca.capacidade);
            imprimir_grupo(saida, tamanhos, grupo_a, busca.quantidade, 1);
            fprintf(saida, "\nPendrive B (%lld GB)\n", busca.capacidade);
            imprimir_grupo(saida, tamanhos, grupo_a, busca.quantidade, 0);
        }
        if (teste + 1 < testes)
            fputc('\n', saida);
    }
    return 1;
}

static int ler_caminho_entrada(char caminho[], size_t tamanho) {
    size_t comprimento;

    printf("Digite o caminho do arquivo de entrada: ");
    if (fgets(caminho, (int)tamanho, stdin) == NULL) {
        return 0;
    }

    caminho[strcspn(caminho, "\r\n")] = '\0';
    comprimento = strlen(caminho);

    /* Aceita um caminho copiado entre aspas pelo Windows. */
    if (comprimento >= 2 && caminho[0] == '"' &&
        caminho[comprimento - 1] == '"') {
        memmove(caminho, caminho + 1, comprimento - 2);
        caminho[comprimento - 2] = '\0';
    }

    return caminho[0] != '\0';
}

/*
 * Mantem a pasta do arquivo de entrada e troca sua extensao por .out.
 * Exemplo: C:\\Trabalho\\backup.in gera C:\\Trabalho\\backup.out.
 */
static int gerar_caminho_saida(const char entrada[], char saida[], size_t tamanho) {
    const char *ultima_barra = strrchr(entrada, '/');
    const char *ultima_contrabarra = strrchr(entrada, '\\');
    const char *nome;
    const char *ponto;
    size_t base;
    int escritos;

    if (ultima_contrabarra != NULL &&
        (ultima_barra == NULL || ultima_contrabarra > ultima_barra)) {
        ultima_barra = ultima_contrabarra;
    }

    nome = ultima_barra == NULL ? entrada : ultima_barra + 1;
    ponto = strrchr(nome, '.');
    base = (ponto != NULL && ponto != nome) ? (size_t)(ponto - entrada)
                                             : strlen(entrada);

    escritos = snprintf(saida, tamanho, "%.*s.out", (int)base, entrada);
    if (escritos < 0 || (size_t)escritos >= tamanho) {
        return 0;
    }

    /* Evita sobrescrever a entrada caso ela ja tenha extensao .out. */
    if (strcmp(entrada, saida) == 0) {
        escritos = snprintf(saida, tamanho, "%.*s_resultado.out", (int)base,
                             entrada);
        if (escritos < 0 || (size_t)escritos >= tamanho) {
            return 0;
        }
    }

    return 1;
}

int main(void) {
    FILE *entrada, *saida = stdout;
    char caminho_entrada[TAMANHO_CAMINHO];
    char caminho_saida[TAMANHO_CAMINHO];
    int resultado;

    if (!ler_caminho_entrada(caminho_entrada, sizeof(caminho_entrada))) {
        fprintf(stderr, "Erro: caminho de entrada vazio ou nao informado.\n");
        return EXIT_FAILURE;
    }
    if (!gerar_caminho_saida(caminho_entrada, caminho_saida,
                             sizeof(caminho_saida))) {
        fprintf(stderr, "Erro: o caminho informado e muito grande.\n");
        return EXIT_FAILURE;
    }

    entrada = fopen(caminho_entrada, "r");
    if (entrada == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo de entrada '%s'.\n",
                caminho_entrada);
        return EXIT_FAILURE;
    }

    saida = fopen(caminho_saida, "w");
    if (saida == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo de saida '%s'.\n",
                caminho_saida);
        fclose(entrada);
        return EXIT_FAILURE;
    }

    resultado = processar(entrada, saida);
    fclose(entrada);
    fclose(saida);
    if (!resultado) {
        fprintf(stderr, "Erro: arquivo de entrada invalido.\n");
        return EXIT_FAILURE;
    }

    printf("Combinacao realizada no arquivo %s.\n", caminho_saida);
    return EXIT_SUCCESS;
}
