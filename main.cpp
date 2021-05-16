/*
 Alunos responsáveis:
 Marcos Felipe Belisário Costa          11811BCC020
 Gabriel José Bueno Otsuka              11721BCC018
 Pedro Henrique Bufullin de Almieda     11711BCC028
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <map>
#include <iostream>

using namespace std;

// remove pontuacao de uma palavra
void removePontuacao(char *palavra)
{
    int length = strlen(palavra);
    if (
        (palavra[length - 1] == '.') || (palavra[length - 1] == ',') || (palavra[length - 1] == ';') ||
        (palavra[length - 1] == ':') || (palavra[length - 1] == '?') || (palavra[length - 1] == '!'))
        palavra[length - 1] = '\0';
}

// imprime linha do arquivo com base no offset da palavra
void imprimeLinha(int offset, FILE *f)
{
    int pos = ftell(f);
    char linha[2048];
    while (pos < offset)
    {
        fgets(linha, 2047, f);
        pos = ftell(f);
    }
    printf("%s", linha);
}

struct registro
{
    int valor;
    int proximo;
};

// classe que implementa a lista invertida
class listaInvertida
{
public:
    map<string, int> secondaryIndexes;
    FILE *indexFile;
    // construtor
    listaInvertida()
    {
        secondaryIndexes.clear();
        indexFile = fopen("indexes.idx", "wb+");
        if (indexFile == NULL)
        {
            printf("\n\n Nao consegui abrir arquivo de indices. Sinto muito.\n\n\n\n");
            return;
        }
    }
    // destrutor
    ~listaInvertida()
    {
        fclose(indexFile);
    }

    // adiciona palavra na estrutura
    void adiciona(char *palavra, int offset)
    {
        string aux(palavra);
        if (!secondaryIndexes.count(aux))
            secondaryIndexes[aux] = -1;
        int offsetStart = secondaryIndexes[aux];

        fseek(indexFile, 0, SEEK_END);
        int newStart = ftell(indexFile);
        struct registro reg;
        reg.valor = offset;
        reg.proximo = offsetStart;
        fwrite(&reg, sizeof(struct registro), 1, indexFile);

        secondaryIndexes[aux] = newStart;
    }

    // realiza busca, retornando vetor de offsets que referenciam a palavra
    int *busca(char *palavra, int *quantidade)
    {
        string aux(palavra);
        if (!secondaryIndexes.count(aux))
            secondaryIndexes[aux] = -1;
        int offsetStart = secondaryIndexes[aux];

        *quantidade = 0;
        while (offsetStart != -1)
        {
            fseek(indexFile, offsetStart, SEEK_SET);
            struct registro reg;
            fread(&reg, sizeof(struct registro), 1, indexFile);

            *quantidade += 1;

            offsetStart = reg.proximo;
        }

        offsetStart = secondaryIndexes[aux];

        int *offsets = new int[*quantidade];
        int cnt = 0;
        while (offsetStart != -1)
        {
            fseek(indexFile, offsetStart, SEEK_SET);
            struct registro reg;
            fread(&reg, sizeof(struct registro), 1, indexFile);

            offsets[cnt++] = reg.valor;

            offsetStart = reg.proximo;
        }
        return offsets;
    }

private:
};

// programa principal
int main(int argc, char **argv)
{
    // abrir arquivo
    ifstream in("biblia.txt");
    if (!in.is_open())
    {
        printf("\n\n Nao consegui abrir arquivo biblia.txt. Sinto muito.\n\n\n\n");
    }
    else
    {
        // vamos ler o arquivo e criar a lista invertida com as palavras do arquivo
        char *palavra = new char[100];
        int offset, contadorDePalavras = 0;
        listaInvertida lista;
        // ler palavras
        while (!in.eof())
        {
            // ler palavra
            in >> palavra;
            // pegar offset
            offset = in.tellg();
            // remover pontuacao
            removePontuacao(palavra);
            // desconsiderar palavras que sao marcadores do arquivo
            if (!((palavra[0] == '#') || (palavra[0] == '[') || ((palavra[0] >= '0') && (palavra[0] <= '9'))))
            {
                //printf("%d %s\n", offset,palavra); fflush(stdout); // debug :-)
                lista.adiciona(palavra, offset);
                contadorDePalavras++;
                if (contadorDePalavras % 1000 == 0)
                {
                    printf(".");
                    fflush(stdout);
                }
            }
        }
        in.close();

        // agora que ja construimos o indice, podemos realizar buscas
        do
        {
            printf("\nDigite a palavra desejada ou \"SAIR\" para sair: ");
            scanf("%s", palavra);
            if (strcmp(palavra, "SAIR") != 0)
            {
                int quantidade;
                // busca na lista invertida
                int *offsets = lista.busca(palavra, &quantidade);
                // com vetor de offsets, recuperar as linhas que contem a palavra desejada
                printf("quantidade -> %d\n", quantidade);
                if (quantidade > 0)
                {
                    FILE *f = fopen("biblia.txt", "rt");
                    for (int i = quantidade - 1; i >= 0; i--)
                        imprimeLinha(offsets[i], f);
                    fclose(f);
                }
                else
                    printf("nao encontrou %s\n", palavra);
            }
        } while (strcmp(palavra, "SAIR") != 0);

        printf("\n\nAte mais!\n\n");
    }

    return (EXIT_SUCCESS);
}
