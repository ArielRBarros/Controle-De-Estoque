# Sistema de Controle de Estoque - Loja de Ferragens

Este projeto implementa um sistema de cadastro e gerenciamento de estoque para uma loja de ferragens em linguagem C, utilizando uma **Árvore Binária de Busca (ABB)** como estrutura de dados principal em memória e persistência em arquivo CSV.

## Funcionalidades

O sistema oferece um menu interativo via terminal com as seguintes operações:

1. **Cadastrar Produto (`1`):**
   - Insere um novo item na árvore ordenado pelo `codigo` (chave única).
   - **Validações:** Impede códigos menores ou iguais a zero, chaves duplicadas, descrições vazias ou contendo ponto e vírgula (`;`), quantidades negativas e preços negativos.
2. **Buscar Produto (`2`):**
   - Realiza uma busca iterativa pelo `codigo` informado.
   - Exibe todos os dados do produto (código, descrição, quantidade e preço unitário) ou informa `"chave nao encontrada"`.
3. **Listar Produtos (`3`):**
   - Percorre a árvore **Em-Ordem (In-Order)**, exibindo todos os itens cadastrados em ordem crescente de código.
4. **Exibir Estrutura da Árvore (`4`):**
   - Percorre a árvore em **Pré-Ordem (Pre-Order)** com recuo visual por nível hierárquico, identificando claramente os filhos à esquerda (`E:`) e à direita (`D:`).
5. **Remover Produto (`5`):**
   - Remove um item a partir de seu `codigo`, aplicando a remoção padrão de ABB para os três casos possíveis: nó folha, nó com um filho e nó com dois filhos (substituição pelo sucessor em ordem).
6. **Consultar Altura (`6`):**
   - Calcula e exibe a altura atual da árvore (considerando árvore vazia com altura `0` e nó folha com altura `1`).
7. **Sair e Salvar (`0`):**
   - Grava os dados atuais no arquivo `estoque.csv`, libera toda a memória alocada dinamicamente (percurso pós-ordem) e encerra o programa.

## Persistência de Dados (`estoque.csv`)

- **Carregamento Automático:** Ao iniciar, o programa tenta ler o arquivo `estoque.csv` no formato `codigo;descricao;quantidade;preco_unitario`. Caso o arquivo não exista, o sistema inicia com a árvore vazia. Registros malformados ou duplicados são tratados com avisos sem interromper a execução.
- **Gravação em Pré-Ordem:** Ao encerrar (via opção `0` ou fim de entrada `EOF`), os dados são salvos em **Pré-Ordem (Raiz, Esquerda, Direita)**. Esse percurso garante que, ao reiniciar o programa e reinserir os registros linha a linha, a árvore seja reconstruída com exatamente a mesma topologia (mesmos pais, filhos e altura) da sessão anterior.

## Estrutura dos Dados

Cada registro armazenado nos nós da árvore segue a definição:

- **Código (`int`):** Identificador numérico único e positivo.
- **Descrição (`char[50]`):** Nome/descrição do produto (até 49 caracteres).
- **Quantidade (`int`):** Quantidade disponível em estoque ($\ge 0$).
- **Preço Unitário (`double`):** Valor unitário do item ($\ge 0.0$).

## Como Compilar e Executar

Utilize um compilador C padrão (como `gcc` ou `clang`) no terminal:

```bash
# Compilar o código
gcc -Wall -Wextra -std=c99 sistema.c -o sistema

# Executar o programa
./sistema
```

## Robustez de Entrada

O sistema conta com rotinas dedicadas (`lerInteiro`, `lerDouble` e `lerTexto`) que limpam o buffer de entrada (`stdin`) após cada leitura, evitando loops infinitos em caso de digitação inválida e tratando adequadamente o fim de arquivo (`EOF`) com salvamento automático antes de fechar.