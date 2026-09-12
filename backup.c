/* Projeto: Backup offline. Preencha os integrantes antes da entrega. */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ARQUIVOS 50
#define ARQUIVO_ENTRADA "arquivo de entrada.txt"
#define ARQUIVO_SAIDA "arquivo de saida.txt"

/* Vetores compartilhados pelo backtracking e dados do teste atual. */
int arquivos_ordenados[MAX_ARQUIVOS];
int posicao_original[MAX_ARQUIVOS];
int restante[MAX_ARQUIVOS + 1];
int escolha_atual[MAX_ARQUIVOS];
int melhor_escolha[MAX_ARQUIVOS];
int quantidade_arquivos;
int capacidade_pendrive;
int melhor_diferenca;
int encontrou_solucao;

/* Calcula uma diferenca sempre positiva. */
int absoluto(int a, int b) {
    return a >= b ? a - b : b - a;
}

/* Ordenacao por selecao: arquivos maiores primeiro ajudam as podas. */
void ordenar(int tamanhos[], int posicoes[], int quantidade) {
    int i, j, maior, auxiliar;

    for (i = 0; i < quantidade - 1; i++) {
        maior = i;
        for (j = i + 1; j < quantidade; j++) {
            if (tamanhos[j] > tamanhos[maior]) {
                maior = j;
            }
        }
        if (maior != i) {
            auxiliar = tamanhos[i];
            tamanhos[i] = tamanhos[maior];
            tamanhos[maior] = auxiliar;

            auxiliar = posicoes[i];
            posicoes[i] = posicoes[maior];
            posicoes[maior] = auxiliar;
        }
    }
}

/* Copia a distribuicao atual quando ela melhora a resposta encontrada. */
void salvar_melhor_escolha(void) {
    int i;

    for (i = 0; i < quantidade_arquivos; i++) {
        melhor_escolha[i] = escolha_atual[i];
    }
}

/*
 * Backtracking: cada chamada testa o proximo arquivo em A e em B. Ramo que
 * excede a capacidade ou nao pode melhorar a resposta atual e interrompido.
 */
void distribuir(int indice, int usado_a, int usado_b) {
    int diferenca, limite, tamanho;

    if (encontrou_solucao && melhor_diferenca == 0) {
        return;
    }
    if (usado_a > capacidade_pendrive || usado_b > capacidade_pendrive) {
        return;
    }

    if (indice == quantidade_arquivos) {
        diferenca = absoluto(usado_a, usado_b);
        if (!encontrou_solucao || diferenca < melhor_diferenca) {
            melhor_diferenca = diferenca;
            encontrou_solucao = 1;
            salvar_melhor_escolha();
        }
        return;
    }

    diferenca = absoluto(usado_a, usado_b);
    limite = diferenca > restante[indice] ? diferenca - restante[indice] : 0;
    if (encontrou_solucao && limite >= melhor_diferenca) {
        return;
    }

    tamanho = arquivos_ordenados[indice];
    if (usado_a <= usado_b) {
        escolha_atual[indice] = 1;
        distribuir(indice + 1, usado_a + tamanho, usado_b);
        escolha_atual[indice] = 0;
        distribuir(indice + 1, usado_a, usado_b + tamanho);
    } else {
        escolha_atual[indice] = 0;
        distribuir(indice + 1, usado_a, usado_b + tamanho);
        escolha_atual[indice] = 1;
        distribuir(indice + 1, usado_a + tamanho, usado_b);
    }
    escolha_atual[indice] = 0;
}

/* Escreve os tamanhos associados ao Pendrive A ou B na ordem original. */
void imprimir_pendrive(FILE *saida, const int tamanhos[],
                       const int no_pendrive_a[], int pendrive_a) {
    int i;

    for (i = 0; i < quantidade_arquivos; i++) {
        if (no_pendrive_a[i] == pendrive_a) {
            fprintf(saida, "%d GB\n", tamanhos[i]);
        }
    }
}

/* Le cada teste, prepara a busca e grava a distribuicao ou a mensagem de erro. */
int processar(FILE *entrada, FILE *saida) {
    int testes, teste;

    if (fscanf(entrada, "%d", &testes) != 1 || testes <= 0) {
        return 0;
    }

    for (teste = 0; teste < testes; teste++) {
        int total, quantidade_lida, tamanhos[MAX_ARQUIVOS];
        int no_pendrive_a[MAX_ARQUIVOS] = {0};
        int soma = 0, excedeu_total = 0;
        int i;

        if (fscanf(entrada, "%d%d", &total, &quantidade_lida) != 2 ||
            total <= 0 || total % 2 != 0 || quantidade_lida < 1 ||
            quantidade_lida > MAX_ARQUIVOS) {
            return 0;
        }

        quantidade_arquivos = quantidade_lida;
        capacidade_pendrive = total / 2;

        /* Copia os dados para os vetores da busca e detecta excesso de espaco. */
        for (i = 0; i < quantidade_arquivos; i++) {
            if (fscanf(entrada, "%d", &tamanhos[i]) != 1 || tamanhos[i] <= 0) {
                return 0;
            }
            arquivos_ordenados[i] = tamanhos[i];
            posicao_original[i] = i;
            if (!excedeu_total) {
                if (tamanhos[i] > total - soma) {
                    excedeu_total = 1;
                } else {
                    soma += tamanhos[i];
                }
            }
        }

        encontrou_solucao = 0;
        melhor_diferenca = total;
        for (i = 0; i < quantidade_arquivos; i++) {
            escolha_atual[i] = 0;
            melhor_escolha[i] = 0;
        }

        /* restante permite estimar se um ramo ainda pode melhorar a resposta. */
        if (!excedeu_total) {
            ordenar(arquivos_ordenados, posicao_original, quantidade_arquivos);
            restante[quantidade_arquivos] = 0;
            for (i = quantidade_arquivos - 1; i >= 0; i--) {
                restante[i] = restante[i + 1] + arquivos_ordenados[i];
            }
            distribuir(0, 0, 0);
        }

        fprintf(saida, "%d GB\n", total);
        if (!encontrou_solucao) {
            fprintf(saida, "Impossivel gravar todos os arquivos nos pendrives.\n");
        } else {
            /* Reconstroi a resposta na mesma ordem em que os tamanhos foram lidos. */
            for (i = 0; i < quantidade_arquivos; i++) {
                if (melhor_escolha[i]) {
                    no_pendrive_a[posicao_original[i]] = 1;
                }
            }
            fprintf(saida, "Pendrive A (%d GB)\n", capacidade_pendrive);
            imprimir_pendrive(saida, tamanhos, no_pendrive_a, 1);
            fprintf(saida, "\nPendrive B (%d GB)\n", capacidade_pendrive);
            imprimir_pendrive(saida, tamanhos, no_pendrive_a, 0);
        }
        if (teste + 1 < testes) {
            fputc('\n', saida);
        }
    }
    return 1;
}

/* Abre os arquivos fixos, processa os testes e fecha os arquivos. */
int main(void) {
    FILE *entrada, *saida;
    int resultado;

    entrada = fopen(ARQUIVO_ENTRADA, "r");
    if (entrada == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s.\n", ARQUIVO_ENTRADA);
        return EXIT_FAILURE;
    }

    saida = fopen(ARQUIVO_SAIDA, "w");
    if (saida == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar %s.\n", ARQUIVO_SAIDA);
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

    printf("Combinacao realizada no arquivo %s.\n", ARQUIVO_SAIDA);
    return EXIT_SUCCESS;
}
