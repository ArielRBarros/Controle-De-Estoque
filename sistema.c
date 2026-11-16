/*
 * ============================================================================
 * TRABALHO DE ALGORITMOS - SISTEMA DE CADASTRO COM ARVORE BINARIA DE BUSCA
 * Tema: controle de estoque de uma loja de ferragens.
 * Arquivo unico: sistema.c
 *
 * JUSTIFICATIVA DO PERCURSO DE GRAVACAO (PRE-ORDEM):
 * O arquivo e lido e os registros sao inseridos na ordem em que aparecem nas
 * linhas. A gravacao ao sair usa o percurso PRE-ORDEM (raiz, esquerda,
 * direita). Em uma arvore binaria de busca, todos os nos da subarvore
 * esquerda possuem chave menor que a raiz, e todos os da direita possuem
 * chave maior. Ao reinserir a sequencia pre-ordem, a raiz e inserida
 * primeiro; depois sao inseridos todos os nos da subarvore esquerda (antes de
 * qualquer no da direita) e, por fim, os da subarvore direita. Esse mesmo
 * raciocinio se aplica recursivamente a cada subarvore. Portanto, a sequencia
 * pre-ordem determina univocamente a forma da arvore: reinserir os registros
 * nessa ordem reconstroi exatamente a mesma estrutura (mesmos pais, filhos e
 * alturas), sem necessidade de armazenar a forma separadamente.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TAM_DESCRICAO 50
#define ARQUIVO_ESTOQUE "estoque.csv"

/* Um registro de estoque. A descricao tem 50 bytes, incluindo o '\0'. */
typedef struct {
    int codigo;
    char descricao[TAM_DESCRICAO];
    int quantidade;
    double preco_unitario;
} Registro;

/* No da arvore: contem o registro completo e os ponteiros dos filhos. */
typedef struct No {
    Registro registro;
    struct No *esquerda;
    struct No *direita;
} No;

/* Resultado das rotinas de leitura do teclado. */
typedef enum {
    LEITURA_OK = 0,
    LEITURA_INVALIDA = 1,
    LEITURA_FIM = 2
} ResultadoLeitura;

/* ------------------------------------------------------------------------- */
/* Prototipos                                                                 */
/* ------------------------------------------------------------------------- */
static void limparBufferEntrada(void);
static ResultadoLeitura lerInteiro(const char *rotulo, int *destino);
static ResultadoLeitura lerDouble(const char *rotulo, double *destino);
static ResultadoLeitura lerTexto(const char *rotulo, char *destino);
static void copiarTexto(char *destino, const char *origem, size_t tamanho);

static No *buscarNo(No *raiz, int codigo);
static int inserir(No **raiz, Registro registro);
static int remover(No **raiz, int codigo);
static No *menorNo(No *raiz);
static int altura(const No *raiz);
static void liberarArvore(No **raiz);

static void listarEmOrdemRec(const No *raiz);
static void exibirArvoreRec(const No *raiz, int nivel, const char *prefixo);
static void salvarPreOrdem(FILE *arquivo, const No *raiz, int *erro);

static int carregarArquivo(No **raiz);
static void salvarArquivo(const No *raiz);

static void exibirMenu(void);
static int operacaoCadastrar(No **raiz);
static int operacaoBuscar(No *raiz);
static int operacaoRemover(No **raiz);
static void operacaoListar(const No *raiz);
static void operacaoExibirArvore(const No *raiz);
static void operacaoAltura(const No *raiz);

/* ------------------------------------------------------------------------- */
/* Entrada de dados                                                           */
/* ------------------------------------------------------------------------- */

/*
 * Descarta todos os caracteres restantes da linha atual (ou ate o fim do
 * arquivo). Nao entra em loop infinito quando getchar() retorna EOF.
 */
static void limparBufferEntrada(void)
{
    int caractere;

    while ((caractere = getchar()) != '\n' && caractere != EOF) {
        /* descarta */
    }
}

/*
 * Le um inteiro. Sempre limpa o restante da linha, tanto em caso de sucesso
 * quanto de erro. Retorna LEITURA_FIM se o stdin terminou.
 */
static ResultadoLeitura lerInteiro(const char *rotulo, int *destino)
{
    int lidos;

    printf("%s", rotulo);
    lidos = scanf("%d", destino);
    limparBufferEntrada();

    if (lidos == EOF) {
        return LEITURA_FIM;
    }
    if (lidos != 1) {
        return LEITURA_INVALIDA;
    }
    return LEITURA_OK;
}

/* Le um double com as mesmas garantias de limpeza e deteccao de EOF. */
static ResultadoLeitura lerDouble(const char *rotulo, double *destino)
{
    int lidos;

    printf("%s", rotulo);
    lidos = scanf("%lf", destino);
    limparBufferEntrada();

    if (lidos == EOF) {
        return LEITURA_FIM;
    }
    if (lidos != 1) {
        return LEITURA_INVALIDA;
    }
    return LEITURA_OK;
}

/*
 * Le uma linha de texto com o formato exigido: scanf(" %49[^\n]", ...).
 * O limite 49 corresponde a TAM_DESCRICAO - 1, deixando espaco para '\0'.
 * O buffer de entrada e limpo depois da leitura.
 *
 * O espaco no inicio do formato faria o scanf pular uma linha vazia e ler a
 * linha seguinte. Para que uma descricao vazia seja detectada corretamente,
 * verificamos antes se a linha atual so contem espacos. Se estiver vazia,
 * retornamos string vazia sem consumir a proxima linha. Se houver texto,
 * devolvemos o primeiro caractere ao buffer e usamos o scanf exigido.
 */
static ResultadoLeitura lerTexto(const char *rotulo, char *destino)
{
    int caractere;
    int lidos;

    printf("%s", rotulo);

    do {
        caractere = getchar();
    } while (caractere != EOF && caractere != '\n' &&
             isspace((unsigned char)caractere));

    if (caractere == EOF) {
        destino[0] = '\0';
        return LEITURA_FIM;
    }
    if (caractere == '\n') {
        destino[0] = '\0';
        return LEITURA_OK;
    }

    ungetc(caractere, stdin);
    lidos = scanf(" %49[^\n]", destino);
    limparBufferEntrada();

    if (lidos == EOF) {
        destino[0] = '\0';
        return LEITURA_FIM;
    }
    if (lidos != 1) {
        destino[0] = '\0';
        return LEITURA_INVALIDA;
    }
    return LEITURA_OK;
}

/* Copia texto com limite de tamanho, sem depender de snprintf/strncpy. */
static void copiarTexto(char *destino, const char *origem, size_t tamanho)
{
    size_t i = 0;

    if (tamanho == 0) {
        return;
    }
    while ((i + 1) < tamanho && origem[i] != '\0') {
        destino[i] = origem[i];
        i++;
    }
    destino[i] = '\0';
}

/* ------------------------------------------------------------------------- */
/* Arvore binaria de busca                                                    */
/* ------------------------------------------------------------------------- */

/* Busca iterativa: devolve o no com a chave ou NULL. */
static No *buscarNo(No *raiz, int codigo)
{
    while (raiz != NULL) {
        if (codigo < raiz->registro.codigo) {
            raiz = raiz->esquerda;
        } else if (codigo > raiz->registro.codigo) {
            raiz = raiz->direita;
        } else {
            return raiz;
        }
    }
    return NULL;
}

/*
 * Insere um registro. A raiz e passada por referencia (No **), inclusive nas
 * chamadas recursivas, conforme exigido. Retornos:
 *   1  = inserido
 *   0  = chave ja existente (nada foi alterado)
 *  -1  = falha de alocacao (nada foi alterado)
 */
static int inserir(No **raiz, Registro registro)
{
    No *novo;

    if (*raiz == NULL) {
        novo = (No *)malloc(sizeof(No));
        if (novo == NULL) {
            return -1;
        }
        novo->registro = registro;
        novo->esquerda = NULL;
        novo->direita = NULL;
        *raiz = novo;
        return 1;
    }

    if (registro.codigo < (*raiz)->registro.codigo) {
        return inserir(&(*raiz)->esquerda, registro);
    }
    if (registro.codigo > (*raiz)->registro.codigo) {
        return inserir(&(*raiz)->direita, registro);
    }
    return 0;
}

/* Menor chave de uma subarvore (usada para achar o sucessor em ordem). */
static No *menorNo(No *raiz)
{
    while (raiz != NULL && raiz->esquerda != NULL) {
        raiz = raiz->esquerda;
    }
    return raiz;
}

/*
 * Remocao padrao de ABB. A raiz e passada por referencia (No **).
 * Casos: no folha, no com um filho e no com dois filhos. No ultimo caso,
 * copia-se o registro do sucessor em ordem (menor chave da subarvore direita)
 * para o no atual e remove-se o sucessor da subarvore direita.
 * Retorna 1 se removeu e 0 se a chave nao existe.
 */
static int remover(No **raiz, int codigo)
{
    No *atual;
    No *sucessor;
    int codigoSucessor;

    if (*raiz == NULL) {
        return 0;
    }

    if (codigo < (*raiz)->registro.codigo) {
        return remover(&(*raiz)->esquerda, codigo);
    }
    if (codigo > (*raiz)->registro.codigo) {
        return remover(&(*raiz)->direita, codigo);
    }

    atual = *raiz;
    if (atual->esquerda == NULL) {
        /* Folha ou apenas filho direito. */
        *raiz = atual->direita;
        free(atual);
    } else if (atual->direita == NULL) {
        /* Apenas filho esquerdo. */
        *raiz = atual->esquerda;
        free(atual);
    } else {
        /* Dois filhos: sucessor em ordem = menor chave da direita. */
        sucessor = menorNo(atual->direita);
        codigoSucessor = sucessor->registro.codigo;
        atual->registro = sucessor->registro;
        remover(&atual->direita, codigoSucessor);
    }
    return 1;
}

/* Altura: arvore vazia = 0; no folha = 1. */
static int altura(const No *raiz)
{
    int alturaEsquerda;
    int alturaDireita;

    if (raiz == NULL) {
        return 0;
    }
    alturaEsquerda = altura(raiz->esquerda);
    alturaDireita = altura(raiz->direita);
    return 1 + (alturaEsquerda > alturaDireita ? alturaEsquerda : alturaDireita);
}

/* Libera todos os nos em pos-ordem e deixa a raiz do chamador em NULL. */
static void liberarArvore(No **raiz)
{
    if (*raiz == NULL) {
        return;
    }
    liberarArvore(&(*raiz)->esquerda);
    liberarArvore(&(*raiz)->direita);
    free(*raiz);
    *raiz = NULL;
}

/* ------------------------------------------------------------------------- */
/* Exibicao                                                                   */
/* ------------------------------------------------------------------------- */

/* Percurso em ordem: chaves em ordem crescente. */
static void listarEmOrdemRec(const No *raiz)
{
    if (raiz == NULL) {
        return;
    }
    listarEmOrdemRec(raiz->esquerda);
    printf("Codigo: %d | Descricao: %s | Quantidade: %d | Preco unitario: %.2f\n",
           raiz->registro.codigo,
           raiz->registro.descricao,
           raiz->registro.quantidade,
           raiz->registro.preco_unitario);
    listarEmOrdemRec(raiz->direita);
}

/*
 * Percurso pre-ordem com recuo de 4 espacos por nivel. A raiz nao recebe
 * prefixo; os filhos recebem "E: " ou "D: ".
 */
static void exibirArvoreRec(const No *raiz, int nivel, const char *prefixo)
{
    int i;

    if (raiz == NULL) {
        return;
    }
    for (i = 0; i < nivel; i++) {
        printf("    ");
    }
    if (prefixo != NULL) {
        printf("%s", prefixo);
    }
    printf("%d - %s (qtd: %d, preco: %.2f)\n",
           raiz->registro.codigo,
           raiz->registro.descricao,
           raiz->registro.quantidade,
           raiz->registro.preco_unitario);

    exibirArvoreRec(raiz->esquerda, nivel + 1, "E: ");
    exibirArvoreRec(raiz->direita, nivel + 1, "D: ");
}

/* ------------------------------------------------------------------------- */
/* Arquivo                                                                    */
/* ------------------------------------------------------------------------- */

/*
 * Carrega estoque.csv. Se o arquivo nao existir, informa e mantem a arvore
 * vazia. Linha com chave duplicada e ignorada com aviso. Qualquer linha
 * malformada interrompe a leitura, mas o programa continua normalmente.
 */
static int carregarArquivo(No **raiz)
{
    FILE *arquivo;
    int carregados = 0;

    arquivo = fopen(ARQUIVO_ESTOQUE, "r");
    if (arquivo == NULL) {
        printf("Aviso: arquivo %s nao encontrado. A arvore iniciara vazia.\n",
               ARQUIVO_ESTOQUE);
        return 0;
    }

    printf("Carregando dados de %s...\n", ARQUIVO_ESTOQUE);

    for (;;) {
        Registro registro;
        char descricao[TAM_DESCRICAO];
        int codigo = 0;
        int quantidade = 0;
        double preco = 0.0;
        int lidos;
        int caractere;
        int lixo = 0;

        descricao[0] = '\0';
        lidos = fscanf(arquivo, "%d;%49[^;];%d;%lf",
                       &codigo, descricao, &quantidade, &preco);

        if (lidos == EOF) {
            if (ferror(arquivo)) {
                printf("Aviso: erro de leitura no arquivo %s.\n", ARQUIVO_ESTOQUE);
            }
            break;
        }
        if (lidos != 4) {
            printf("Aviso: linha malformada no arquivo %s. Leitura interrompida.\n",
                   ARQUIVO_ESTOQUE);
            break;
        }

        /* Rejeita lixo depois do preco antes de aceitar o registro. */
        while ((caractere = fgetc(arquivo)) != '\n' && caractere != EOF) {
            if (!isspace((unsigned char)caractere)) {
                lixo = 1;
            }
        }
        if (lixo) {
            printf("Aviso: linha malformada no arquivo %s. Leitura interrompida.\n",
                   ARQUIVO_ESTOQUE);
            break;
        }

        if (codigo <= 0 || descricao[0] == '\0' ||
            quantidade < 0 || !(preco >= 0.0)) {
            printf("Aviso: registro invalido no arquivo %s. Leitura interrompida.\n",
                   ARQUIVO_ESTOQUE);
            break;
        }

        /*
         * O formato %49[^;] poderia atravessar uma quebra de linha se a linha
         * estivesse malformada (sem o ';' esperado). Nesse caso a descricao
         * conteria '\n' ou '\r'; rejeitamos como linha malformada.
         */
        if (strchr(descricao, '\n') != NULL || strchr(descricao, '\r') != NULL) {
            printf("Aviso: linha malformada no arquivo %s. Leitura interrompida.\n",
                   ARQUIVO_ESTOQUE);
            break;
        }

        if (buscarNo(*raiz, codigo) != NULL) {
            printf("Aviso: chave duplicada no arquivo (%d); linha ignorada.\n",
                   codigo);
            continue;
        }

        registro.codigo = codigo;
        copiarTexto(registro.descricao, descricao, TAM_DESCRICAO);
        registro.quantidade = quantidade;
        registro.preco_unitario = preco;

        if (inserir(raiz, registro) != 1) {
            printf("Aviso: memoria insuficiente ao carregar o arquivo. Leitura interrompida.\n");
            break;
        }
        carregados++;
    }

    if (fclose(arquivo) != 0) {
        printf("Aviso: falha ao fechar o arquivo %s apos a leitura.\n",
               ARQUIVO_ESTOQUE);
    }

    printf("Registros carregados: %d\n", carregados);
    return carregados;
}

/* Grava um no e suas subarvores em pre-ordem, verificando fprintf. */
static void salvarPreOrdem(FILE *arquivo, const No *raiz, int *erro)
{
    if (raiz == NULL || *erro) {
        return;
    }

    if (fprintf(arquivo, "%d;%s;%d;%.2f\n",
                raiz->registro.codigo,
                raiz->registro.descricao,
                raiz->registro.quantidade,
                raiz->registro.preco_unitario) < 0) {
        *erro = 1;
        return;
    }

    salvarPreOrdem(arquivo, raiz->esquerda, erro);
    salvarPreOrdem(arquivo, raiz->direita, erro);
}

/*
 * Abre estoque.csv para escrita e grava em pre-ordem. Confere fopen, o
 * retorno de cada fprintf, ferror e fclose.
 */
static void salvarArquivo(const No *raiz)
{
    FILE *arquivo;
    int erro = 0;

    arquivo = fopen(ARQUIVO_ESTOQUE, "w");
    if (arquivo == NULL) {
        printf("Erro: nao foi possivel abrir %s para gravacao.\n", ARQUIVO_ESTOQUE);
        return;
    }

    salvarPreOrdem(arquivo, raiz, &erro);

    if (erro || ferror(arquivo)) {
        printf("Erro durante a gravacao do arquivo %s.\n", ARQUIVO_ESTOQUE);
        erro = 1;
    }

    if (fclose(arquivo) != 0) {
        printf("Erro ao fechar o arquivo %s apos a gravacao.\n", ARQUIVO_ESTOQUE);
        erro = 1;
    }

    if (!erro) {
        printf("Dados gravados em %s (percurso pre-ordem).\n", ARQUIVO_ESTOQUE);
    }
}

/* ------------------------------------------------------------------------- */
/* Menu e operacoes                                                           */
/* ------------------------------------------------------------------------- */

static void exibirMenu(void)
{
    printf("\n===== CONTROLE DE ESTOQUE - LOJA DE FERRAGENS =====\n");
    printf("1 - Cadastrar\n");
    printf("2 - Buscar\n");
    printf("3 - Listar\n");
    printf("4 - Exibir arvore\n");
    printf("5 - Remover\n");
    printf("6 - Altura\n");
    printf("0 - Sair\n");
}

/*
 * Cadastra um registro. Retorna -1 se o stdin terminar durante a entrada
 * (nesse caso o programa deve salvar e encerrar) ou 0 para voltar ao menu.
 */
static int operacaoCadastrar(No **raiz)
{
    Registro registro;
    char descricao[TAM_DESCRICAO];
    int codigo;
    int quantidade;
    double preco;
    ResultadoLeitura leitura;
    int resultado;

    leitura = lerInteiro("Codigo: ", &codigo);
    if (leitura == LEITURA_FIM) {
        return -1;
    }
    if (leitura == LEITURA_INVALIDA) {
        printf("Entrada invalida para codigo. Cadastro cancelado.\n");
        return 0;
    }
    if (codigo <= 0) {
        printf("Codigo invalido: deve ser maior que zero. Cadastro cancelado.\n");
        return 0;
    }
    if (buscarNo(*raiz, codigo) != NULL) {
        printf("Chave %d ja cadastrada. Cadastro recusado.\n", codigo);
        return 0;
    }

    leitura = lerTexto("Descricao: ", descricao);
    if (leitura == LEITURA_FIM) {
        return -1;
    }
    if (leitura == LEITURA_INVALIDA) {
        printf("Entrada invalida para descricao. Cadastro cancelado.\n");
        return 0;
    }
    if (descricao[0] == '\0') {
        printf("Descricao nao pode ser vazia. Cadastro cancelado.\n");
        return 0;
    }
    if (strchr(descricao, ';') != NULL) {
        printf("Descricao nao pode conter ';'. Cadastro cancelado.\n");
        return 0;
    }

    leitura = lerInteiro("Quantidade: ", &quantidade);
    if (leitura == LEITURA_FIM) {
        return -1;
    }
    if (leitura == LEITURA_INVALIDA) {
        printf("Entrada invalida para quantidade. Cadastro cancelado.\n");
        return 0;
    }
    if (quantidade < 0) {
        printf("Quantidade invalida: deve ser maior ou igual a zero. Cadastro cancelado.\n");
        return 0;
    }

    leitura = lerDouble("Preco unitario: ", &preco);
    if (leitura == LEITURA_FIM) {
        return -1;
    }
    if (leitura == LEITURA_INVALIDA) {
        printf("Entrada invalida para preco. Cadastro cancelado.\n");
        return 0;
    }
    if (!(preco >= 0.0)) {
        printf("Preco invalido: deve ser maior ou igual a zero. Cadastro cancelado.\n");
        return 0;
    }

    registro.codigo = codigo;
    copiarTexto(registro.descricao, descricao, TAM_DESCRICAO);
    registro.quantidade = quantidade;
    registro.preco_unitario = preco;

    resultado = inserir(raiz, registro);
    if (resultado == 1) {
        printf("Registro %d cadastrado com sucesso.\n", codigo);
    } else if (resultado == 0) {
        printf("Chave %d ja cadastrada. Cadastro recusado.\n", codigo);
    } else {
        printf("Erro: memoria insuficiente. Registro %d nao cadastrado.\n", codigo);
    }
    return 0;
}

/* Busca uma chave e mostra o registro completo, ou a mensagem exigida. */
static int operacaoBuscar(No *raiz)
{
    int codigo;
    No *encontrado;
    ResultadoLeitura leitura;

    leitura = lerInteiro("Codigo para busca: ", &codigo);
    if (leitura == LEITURA_FIM) {
        return -1;
    }
    if (leitura == LEITURA_INVALIDA) {
        printf("Entrada invalida para codigo.\n");
        return 0;
    }
    if (codigo <= 0) {
        printf("Codigo invalido: deve ser maior que zero.\n");
        return 0;
    }

    encontrado = buscarNo(raiz, codigo);
    if (encontrado == NULL) {
        printf("chave nao encontrada\n");
        return 0;
    }

    printf("Codigo.........: %d\n", encontrado->registro.codigo);
    printf("Descricao......: %s\n", encontrado->registro.descricao);
    printf("Quantidade.....: %d\n", encontrado->registro.quantidade);
    printf("Preco unitario.: %.2f\n", encontrado->registro.preco_unitario);
    return 0;
}

/* Remove uma chave usando a remocao padrao de ABB. */
static int operacaoRemover(No **raiz)
{
    int codigo;
    ResultadoLeitura leitura;

    leitura = lerInteiro("Codigo para remover: ", &codigo);
    if (leitura == LEITURA_FIM) {
        return -1;
    }
    if (leitura == LEITURA_INVALIDA) {
        printf("Entrada invalida para codigo.\n");
        return 0;
    }
    if (codigo <= 0) {
        printf("Codigo invalido: deve ser maior que zero.\n");
        return 0;
    }

    if (remover(raiz, codigo) == 1) {
        printf("Registro %d removido com sucesso.\n", codigo);
    } else {
        printf("Chave %d nao encontrada. Nada foi removido.\n", codigo);
    }
    return 0;
}

static void operacaoListar(const No *raiz)
{
    if (raiz == NULL) {
        printf("Arvore vazia.\n");
        return;
    }
    printf("Registros em ordem crescente de codigo:\n");
    listarEmOrdemRec(raiz);
}

static void operacaoExibirArvore(const No *raiz)
{
    if (raiz == NULL) {
        printf("Arvore vazia.\n");
        return;
    }
    printf("Arvore em pre-ordem (E: filho esquerdo, D: filho direito):\n");
    exibirArvoreRec(raiz, 0, NULL);
}

static void operacaoAltura(const No *raiz)
{
    printf("Altura da arvore: %d\n", altura(raiz));
}

/* ------------------------------------------------------------------------- */
/* Programa principal                                                         */
/* ------------------------------------------------------------------------- */

int main(void)
{
    No *raiz = NULL;
    int executar = 1;
    int eofDetectado = 0;

    carregarArquivo(&raiz);

    while (executar) {
        int opcao = -1;
        ResultadoLeitura leitura;

        exibirMenu();
        leitura = lerInteiro("Opcao: ", &opcao);

        if (leitura == LEITURA_FIM) {
            eofDetectado = 1;
            break;
        }
        if (leitura == LEITURA_INVALIDA) {
            printf("Opcao invalida. Digite um numero entre 0 e 6.\n");
            continue;
        }

        switch (opcao) {
        case 1:
            if (operacaoCadastrar(&raiz) == -1) {
                eofDetectado = 1;
                executar = 0;
            }
            break;
        case 2:
            if (operacaoBuscar(raiz) == -1) {
                eofDetectado = 1;
                executar = 0;
            }
            break;
        case 3:
            operacaoListar(raiz);
            break;
        case 4:
            operacaoExibirArvore(raiz);
            break;
        case 5:
            if (operacaoRemover(&raiz) == -1) {
                eofDetectado = 1;
                executar = 0;
            }
            break;
        case 6:
            operacaoAltura(raiz);
            break;
        case 0:
            executar = 0;
            break;
        default:
            printf("Opcao invalida. Digite um numero entre 0 e 6.\n");
            break;
        }
    }

    if (eofDetectado) {
        printf("\nEntrada encerrada (EOF). Salvando dados...\n");
    }

    salvarArquivo(raiz);
    liberarArvore(&raiz);
    return 0;
}
