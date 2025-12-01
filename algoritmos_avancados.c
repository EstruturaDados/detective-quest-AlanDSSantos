#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// =======================================================
// Detective Quest - Nível Mestre
// Arvore binaria (mansao) + BST de pistas + Tabela Hash
// =======================================================

// --------------------- Estruturas ----------------------

// Nó da árvore de cômodos (mansão)
typedef struct Sala {
    char nome[40];
    struct Sala *esq;
    struct Sala *dir;
    const char *pista;      // pista encontrada nesta sala (opcional)
    const char *suspeito;   // suspeito ligado a essa pista (opcional)
    int pistaColetada;      // 0 = ainda nao coletada, 1 = ja coletada
} Sala;

// Nó da BST de pistas (ordenadas por nome)
typedef struct NoPista {
    char pista[60];
    struct NoPista *esq;
    struct NoPista *dir;
} NoPista;

// Lista encadeada de pistas por suspeito (para a hash)
typedef struct PistaHash {
    char pista[60];
    struct PistaHash *prox;
} PistaHash;

// Entrada de suspeito na tabela hash
typedef struct SuspeitoHash {
    char nome[40];
    int contagem;              // quantas vezes apareceu ligado a pistas
    PistaHash *pistas;         // lista de pistas dessa pessoa
    struct SuspeitoHash *prox;
} SuspeitoHash;

#define TAM_HASH 11

typedef struct {
    SuspeitoHash *tabela[TAM_HASH];
} HashSuspeitos;

// ---------------------- Protótipos ----------------------

// Mansão
Sala   *criarSala(const char *nome, const char *pista, const char *suspeito);
Sala   *montarMansao(void);
void    explorarSalas(Sala *raiz, NoPista **raizPistas, HashSuspeitos *hash);

// BST de pistas
NoPista *inserirPista(NoPista *raiz, const char *pista);
void     listarPistasEmOrdem(const NoPista *raiz);
void     liberarArvorePistas(NoPista *raiz);

// Tabela Hash de suspeitos
void     inicializarHash(HashSuspeitos *h);
unsigned funcaoHash(const char *nome);
void     registrarPistaSuspeito(HashSuspeitos *h,
                                const char *suspeito,
                                const char *pista);
void     exibirSuspeitosEPistas(const HashSuspeitos *h);
const SuspeitoHash *encontrarSuspeitoMaisCitado(const HashSuspeitos *h);
void     liberarHash(HashSuspeitos *h);

// Util
void limparBuffer(void);
void liberarMansao(Sala *raiz);

// ------------------------ main --------------------------

int main(void) {
    Sala *mansao = montarMansao();
    NoPista *raizPistas = NULL;
    HashSuspeitos hash;

    inicializarHash(&hash);

    int opcao;

    do {
        printf("\n================= DETECTIVE QUEST =================\n");
        printf("1 - Explorar a mansao\n");
        printf("2 - Listar todas as pistas em ordem alfabetica\n");
        printf("3 - Exibir suspeitos, pistas associadas e mais citado\n");
        printf("0 - Sair\n");
        printf("===================================================\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1) {
            limparBuffer();
            printf("Entrada invalida.\n");
            continue;
        }
        limparBuffer();

        switch (opcao) {
            case 1:
                explorarSalas(mansao, &raizPistas, &hash);
                break;

            case 2:
                printf("\n----- PISTAS COLETADAS (em ordem alfabetica) -----\n");
                if (raizPistas == NULL) {
                    printf("Nenhuma pista coletada ainda.\n");
                } else {
                    listarPistasEmOrdem(raizPistas);
                }
                break;

            case 3: {
                printf("\n----------- SUSPEITOS E PISTAS LIGADAS -----------\n");
                exibirSuspeitosEPistas(&hash);

                const SuspeitoHash *maisCitado =
                    encontrarSuspeitoMaisCitado(&hash);
                if (maisCitado != NULL) {
                    printf("\n>> Suspeito mais citado: %s (menções: %d)\n",
                           maisCitado->nome, maisCitado->contagem);
                } else {
                    printf("\nNenhum suspeito registrado ainda.\n");
                }
                break;
            }

            case 0:
                printf("\nEncerrando Detective Quest...\n");
                break;

            default:
                printf("\nOpcao invalida.\n");
                break;
        }

        if (opcao != 0) {
            printf("\nPressione ENTER para continuar...");
            getchar();
        }

    } while (opcao != 0);

    // Limpeza de memoria (opcional, mas bonito :)
    liberarMansao(mansao);
    liberarArvorePistas(raizPistas);
    liberarHash(&hash);

    return 0;
}

// ======================= Mansão =========================

Sala *criarSala(const char *nome, const char *pista, const char *suspeito) {
    Sala *s = (Sala*)malloc(sizeof(Sala));
    if (!s) {
        fprintf(stderr, "Falha ao alocar memoria para sala.\n");
        exit(1);
    }
    strncpy(s->nome, nome, sizeof(s->nome) - 1);
    s->nome[sizeof(s->nome) - 1] = '\0';
    s->esq = NULL;
    s->dir = NULL;
    s->pista = pista;
    s->suspeito = suspeito;
    s->pistaColetada = 0;
    return s;
}

Sala *montarMansao(void) {
    // Você pode ajustar o "mapa" como quiser.
    // Aqui um exemplo simples de mansão:

    // Nivel 0
    Sala *hall = criarSala("Hall de Entrada", NULL, NULL);

    // Nivel 1
    Sala *biblioteca = criarSala("Biblioteca",
                                 "Luvas sujas de tinta",
                                 "Jardineiro");
    Sala *salaJantar = criarSala("Sala de Jantar",
                                 "Taca de cristal quebrada",
                                 "Chef");

    hall->esq = biblioteca;
    hall->dir = salaJantar;

    // Nivel 2 - Biblioteca
    Sala *escritorio = criarSala("Escritorio",
                                 "Contrato rasgado",
                                 "Advogado");
    Sala *arquivo = criarSala("Sala de Arquivos",
                              NULL,
                              NULL);

    biblioteca->esq = escritorio;
    biblioteca->dir = arquivo;

    // Nivel 2 - Sala de Jantar
    Sala *cozinha = criarSala("Cozinha",
                              "Faca ausente do conjunto",
                              "Chef");
    Sala *porao = criarSala("Porao",
                            "Pegadas enlameadas",
                            "Jardineiro");

    salaJantar->esq = cozinha;
    salaJantar->dir = porao;

    return hall;
}

void explorarSalas(Sala *raiz, NoPista **raizPistas, HashSuspeitos *hash) {
    if (raiz == NULL) {
        printf("A mansao ainda nao foi montada.\n");
        return;
    }

    Sala *atual = raiz;
    char opc;

    printf("\n=== Exploracao iniciada na mansao ===\n");

    while (atual != NULL) {
        printf("\nVoce esta em: %s\n", atual->nome);

        // Se a sala tem pista e ainda nao foi coletada, coleta agora
        if (atual->pista != NULL && !atual->pistaColetada) {
            printf("Voce encontrou uma pista: '%s'\n", atual->pista);
            if (atual->suspeito != NULL) {
                printf("Essa pista parece apontar para o suspeito: %s\n",
                       atual->suspeito);
            }

            // Insere na BST de pistas (evitando duplicatas simples)
            *raizPistas = inserirPista(*raizPistas, atual->pista);

            // Registra pista -> suspeito na hash
            if (atual->suspeito != NULL) {
                registrarPistaSuspeito(hash, atual->suspeito, atual->pista);
            }

            atual->pistaColetada = 1;
        }

        // Se for nó folha (sem esquerda e direita), fim do caminho
        if (atual->esq == NULL && atual->dir == NULL) {
            printf("\nVoce chegou ao fim deste caminho da mansao.\n");
            break;
        }

        printf("\nPara onde deseja ir?\n");
        if (atual->esq != NULL) {
            printf("  (e) Esquerda -> %s\n", atual->esq->nome);
        }
        if (atual->dir != NULL) {
            printf("  (d) Direita  -> %s\n", atual->dir->nome);
        }
        printf("  (s) Sair da exploracao\n");
        printf("Opcao: ");

        if (scanf(" %c", &opc) != 1) {
            limparBuffer();
            printf("Entrada invalida.\n");
            continue;
        }
        limparBuffer();

        if (opc == 's' || opc == 'S') {
            printf("Voce decidiu encerrar a exploracao por agora.\n");
            break;
        } else if (opc == 'e' || opc == 'E') {
            if (atual->esq != NULL) {
                atual = atual->esq;
            } else {
                printf("Nao ha sala a esquerda.\n");
            }
        } else if (opc == 'd' || opc == 'D') {
            if (atual->dir != NULL) {
                atual = atual->dir;
            } else {
                printf("Nao ha sala a direita.\n");
            }
        } else {
            printf("Opcao invalida.\n");
        }
    }
}

// ======================= BST de Pistas ====================

NoPista *inserirPista(NoPista *raiz, const char *pista) {
    if (raiz == NULL) {
        NoPista *novo = (NoPista*)malloc(sizeof(NoPista));
        if (!novo) {
            fprintf(stderr, "Falha ao alocar memoria para pista.\n");
            exit(1);
        }
        strncpy(novo->pista, pista, sizeof(novo->pista) - 1);
        novo->pista[sizeof(novo->pista) - 1] = '\0';
        novo->esq = novo->dir = NULL;
        return novo;
    }

    int cmp = strcmp(pista, raiz->pista);
    if (cmp < 0) {
        raiz->esq = inserirPista(raiz->esq, pista);
    } else if (cmp > 0) {
        raiz->dir = inserirPista(raiz->dir, pista);
    } else {
        // Pista igual (duplicada), nao insere repetida
        // Poderia contar repeticoes, se quisesse.
    }
    return raiz;
}

void listarPistasEmOrdem(const NoPista *raiz) {
    if (raiz == NULL) return;
    listarPistasEmOrdem(raiz->esq);
    printf(" - %s\n", raiz->pista);
    listarPistasEmOrdem(raiz->dir);
}

void liberarArvorePistas(NoPista *raiz) {
    if (raiz == NULL) return;
    liberarArvorePistas(raiz->esq);
    liberarArvorePistas(raiz->dir);
    free(raiz);
}

// ===================== Tabela Hash =========================

void inicializarHash(HashSuspeitos *h) {
    for (int i = 0; i < TAM_HASH; i++) {
        h->tabela[i] = NULL;
    }
}

unsigned funcaoHash(const char *nome) {
    unsigned soma = 0;
    for (int i = 0; nome[i] != '\0'; i++) {
        soma += (unsigned char)nome[i];
    }
    return soma % TAM_HASH;
}

void registrarPistaSuspeito(HashSuspeitos *h,
                            const char *suspeito,
                            const char *pista) {
    unsigned idx = funcaoHash(suspeito);
    SuspeitoHash *atual = h->tabela[idx];

    // Procura se o suspeito ja existe na lista
    while (atual != NULL) {
        if (strcmp(atual->nome, suspeito) == 0) {
            break;
        }
        atual = atual->prox;
    }

    if (atual == NULL) {
        // Nao existe ainda: cria novo nó de suspeito
        atual = (SuspeitoHash*)malloc(sizeof(SuspeitoHash));
        if (!atual) {
            fprintf(stderr, "Falha ao alocar memoria para suspeito.\n");
            exit(1);
        }
        strncpy(atual->nome, suspeito, sizeof(atual->nome) - 1);
        atual->nome[sizeof(atual->nome) - 1] = '\0';
        atual->contagem = 0;
        atual->pistas = NULL;
        atual->prox = h->tabela[idx];
        h->tabela[idx] = atual;
    }

    // Incrementa contagem de citacoes
    atual->contagem++;

    // Cria nó de pista ligada a esse suspeito
    PistaHash *novoP = (PistaHash*)malloc(sizeof(PistaHash));
    if (!novoP) {
        fprintf(stderr, "Falha ao alocar memoria para pistaHash.\n");
        exit(1);
    }
    strncpy(novoP->pista, pista, sizeof(novoP->pista) - 1);
    novoP->pista[sizeof(novoP->pista) - 1] = '\0';
    novoP->prox = atual->pistas;
    atual->pistas = novoP;
}

void exibirSuspeitosEPistas(const HashSuspeitos *h) {
    bool algum = false;

    for (int i = 0; i < TAM_HASH; i++) {
        SuspeitoHash *s = h->tabela[i];
        while (s != NULL) {
            algum = true;
            printf("\nSuspeito: %s (citacoes: %d)\n", s->nome, s->contagem);
            printf("Pistas associadas:\n");
            if (s->pistas == NULL) {
                printf("  (nenhuma pista registrada)\n");
            } else {
                PistaHash *p = s->pistas;
                while (p != NULL) {
                    printf("  - %s\n", p->pista);
                    p = p->prox;
                }
            }

            s = s->prox;
        }
    }

    if (!algum) {
        printf("Nenhum suspeito registrado ainda.\n");
    }
}

const SuspeitoHash *encontrarSuspeitoMaisCitado(const HashSuspeitos *h) {
    const SuspeitoHash *melhor = NULL;

    for (int i = 0; i < TAM_HASH; i++) {
        SuspeitoHash *s = h->tabela[i];
        while (s != NULL) {
            if (melhor == NULL || s->contagem > melhor->contagem) {
                melhor = s;
            }
            s = s->prox;
        }
    }
    return melhor;
}

void liberarHash(HashSuspeitos *h) {
    for (int i = 0; i < TAM_HASH; i++) {
        SuspeitoHash *s = h->tabela[i];
        while (s != NULL) {
            SuspeitoHash *proxS = s->prox;
            // libera lista de pistas
            PistaHash *p = s->pistas;
            while (p != NULL) {
                PistaHash *proxP = p->prox;
                free(p);
                p = proxP;
            }
            free(s);
            s = proxS;
        }
        h->tabela[i] = NULL;
    }
}

// ======================= Utilidades =======================

void limparBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // descarta
    }
}

void liberarMansao(Sala *raiz) {
    if (raiz == NULL) return;
    liberarMansao(raiz->esq);
    liberarMansao(raiz->dir);
    free(raiz);
}
