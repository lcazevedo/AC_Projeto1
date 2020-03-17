#include <iostream>

using namespace std;

// efinição da classe
class binary128 {

    private:
        // constantes
        static const int MANTISSA = 1;

        // variáveis
        string valor_inteiro;
        string valor_decimal;

        // funções
        bool parser(string);  // valida o valor recebi no construtor

    public:
        // funções
        binary128(string);  // construtor
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
    
    valor_inteiro = valor_decimal = ""; // inicaliza em branco par validar se não existe mais de um separador de decimal
    bool partedecimal = false;

    for (int pos = 0; pos < valor.size(); pos++){  // percorrre os caracerres da string

        char c = valor[pos];  // identifica o caracter da string que será avaliado
        
        if (!isdigit(c)) {  // se não for dígoti (0..9) avalia se é ponto ou se é erro
            if (c == '.') {
                partedecimal = true;  // se for ponto, passa para avaliar a parte decimal
            }
            else {
                return false;  // se não for dígito (0..9) nem ponto, não é aceito no formato de entrada estabelecido
            }
        }
        else {
            if (partedecimal) {

            }
            else {

            }
        }
    }
    return true;
}

int main() {
    binary128 x = binary128("123.4234");
    cout << "asd" << endl; 
    return 0;
}
    
//http://krashan.ppa.pl/articles/stringtofloat/

