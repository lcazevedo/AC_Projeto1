#include <iostream>
#include <stdlib.h>
#include <iomanip>
#include <math.h>

using namespace std;

#define DIGITS 34  // número máximo de dígitos que o meu float vai suportar 
#define EXPOENT_OFFSET 16383  // offset para calculo do expoente (posivo e negativo) com 15 bits
#define MAX_UINT 65535  // máximo valor inteiro a ser guardado em dois bytes sem sinal

#define ISDIGIT(x) (((x) >= '0') && ((x) <= '9'))
#define SETBIT(n, p) (n |= 1UL << p)
#define CLEARBIT(n, p) (n &= ~(1UL << p))
#define CHKBIT(n, p) ((n >> p) & 1U)

// efinição da classe
class binary128 {

    private:
        // constantes
        static const int MANTISSA = 1;

        // variáveis
        string valor_inteiro;
        string valor_decimal;
        
        class mantissa {
            private:
                unsigned short int n[7];  // 7 partes de 2 bytes (16 bits) = 112 bits

                void validaposicao(int posicao){
                    if (posicao < 0 || posicao > 111)
                        throw "Invalid value";
                }

                void partes (int posicao, int *bloco, int *bit){
                    *bloco = posicao / (8 * 2);  // divide pelo numero de bits por byte vezes o número de bytes por int
                    *bit = (posicao % (8 * 2));  // e o resto é a posicao
                }

            public:
                mantissa(){
                    for (int i =0 ; i < 8; i++)
                    n[i] = 0;
                };

                void setbit(int posicao){
                    int bloco = 0, bit = 0;
                    
                    validaposicao(posicao);
                    partes(posicao, &bloco, &bit);
                    //cout << "bloco " << bloco << endl; 
                    //cout << "bit " << bit << endl; 
                    SETBIT(n[bloco], bit);
                };
                void celarbit(int posicao){
                    int bloco = 0, bit = 0;
                    
                    validaposicao(posicao);
                    partes(posicao, &bloco, &bit);
                    CLEARBIT(n[bloco], bit);
                };

                // nesta função é necessário mutiplicar o número atual por 10 e somar o valor do caracter
                // para isso é necessário verficar se o numero ainda cabe no inteiro ou se é necessário
                // passar para o bloco anterior
                void somadigito(char c){
                    int op = 0;
                    int v = c - '0';
                    
                    // percorre os inteiros procurando os espaços para somar
                    for (int i = 0; i < 8; i++){
                        op = (n[i] * 10) + v; // faz o cálculo com 4 bytes 
                        n[i] = op; // guarda os dois bytes da direita no local de armazenamento
                        v = (op >> 16); // faz um shift para direita de 2 bytes, pegando os dois bytes da esquerda para guardar no próxio bloco
                    }
                }

                string toStr(){
                    string r = "";
                    for (int i = 6; i >= 0; i--)
                        r += to_string(n[i]) + " ";
                    return r;
                }

        };
        struct myFloat {
            int s_exp = EXPOENT_OFFSET;  // equivalente à zero, quanto aplicado o offset de 16383 (15 bits)
            mantissa m;  
        } mf;

        // funções
        bool parser(string);  // valida o valor recebi no construtor

    public:
        // funções
        binary128(string);  // construtor

        string to_strfloat(void); // retorna o meu float em formato string
};

/************
Construtur da classe:
Entrada:
    valor -> string com o valor a ser guardado como float no forma binary128
Saída:
    nenuma
************/
binary128::binary128(string valor){

    // valida a string recebida e separa as partes inteira e decimal
    if (!parser(valor)){
        throw "Formato inválido";
    }
}

/************
Funcção par validar se a string recebida pode ser convertida para float.
Também divide o valor entre as partes inteira e decimal
Entrada:
    valor -> string com o valor a ser validado quanto à possibildiade de converter para fomato booleano
Saída:
    Bool-> indica se o valor pode ser convertido (true) ou não (false)
************/
bool binary128::parser(string valor){
    
    //myFloat mf; // automaticamente inicializa com zeros a estrutura que vai guardar o float
    int digitos = 0; // conta os dígitos a medida que vão sendo guardados, para evitar um número maior do que o formato suporta

    valor_inteiro = valor_decimal = ""; // inicaliza em branco par validar se não existe mais de um separador de decimal
    bool partedecimal = false;

    for (int pos = 0; pos < valor.size(); pos++){  // percorrre os caracerres da string

        char c = valor[pos];  // identifica o caracter da string que será avaliado
        
        if (!ISDIGIT(c)) {  // se não for dígoti (0..9) avalia se é ponto ou se é erro
            switch (c) { // se for ponto, passa para avaliar a parte decimal
                case '.':
                    partedecimal = true;  
                    break;
                
                case '-':
                    SETBIT(mf.s_exp, 15); // seta o 16 bit como 1 para indicar negativo (os outros 15 bits são o expoente) 
                    break;
                
                case '+':
                    break;  // i bit inical já está setado em zero, então não há ação

                default:
                    return false;  // se não for dígito (0..9) nem ponto, nem sinal, não é aceito no formato de entrada estabelecido
            }
            
        }
        else {
            if (digitos <= DIGITS){ // se ainda cabe no meu float
                mf.m.somadigito(c); // adiciona o dígito na mantissa
                if (partedecimal) { // se está adicionando na parte decimal
                    if (mf.s_exp == 0)
                        throw "Quantidade de casas decimais excede o limite";
                    mf.s_exp--; // reduz o expoente em 1, significando um salto da vírgula para direita
                }
                digitos++;
            }
            else{
                throw "Número maior que o suportado";
            }
        }
    }
    return true;
}

/************
Funcção par representar o float guardado em formato string.
Entrada:
    nenhuma
Saída:
    String-> representação do float armazenado
************/
string binary128::to_strfloat(void){
    string myFloat = "";

    // representa a parte numérica
    //for (int i = 0; i < 8; i++){

    //} 

    myFloat = "Expoente: " + to_string(mf.s_exp - 16383) + " - mantissa: " + mf.m.toStr(); 
    return myFloat;
}

int main() {
    
/*
    string a[] = {"65535", "65536", "186580", "186586","186587","186588","2186589","34567890"};

    for (int i = 0; i < 8; i ++){
        binary128 x = binary128(a[i]);
        cout << a[i] << " = " << x.to_strfloat() << endl;
    
    }
 */

    unsigned __int128 z = 3;

    uint64_t x = pow(2, 66);

    cout << "exp 60: " << x << endl;

    cout << "exp 66: " << pow(2, -66) << endl;

    return 0;
}
    
//http://krashan.ppa.pl/articles/stringtofloat/

