#include <iostream>
#include <stdlib.h>
#include <iomanip>
#include <math.h>
#include <string>
#include <bits/stdc++.h> 
#include <gmp.h>
#include <stdio.h>
#include <assert.h>

using namespace std;

#define EXPOENTE_OFFSET 262143  // offset para calculo do expoente (posivo e negativo) com 15 bits
#define EXPOENTE_OFFSET_NEGATIVO -262143  // offset negativo para usar em comparações
#define EXPOENTE_MAXIMO 524287  // máximo unsigned int do expoente, para usar em comparações
#define MANTISSA_BITS 236 // número de bits para a mantissa 
#define EXPOENTE_BITS 19 // bit do unsigned int que guarda o sinal. Bit mais significativo
#define BIT_SINAL 19 // bit do unsigned int que guarda o sinal. Bit mais significativo
// tem um 224 em mutiplicabtsets !!!


#define ISDIGIT(x) (((x) >= '0') && ((x) <= '9'))
#define SETBIT(n, p) (n |= 1UL << p)
#define CLEARBIT(n, p) (n &= ~(1UL << p))
#define CHKBIT(n, p) ((n >> p) & 1U)

// efinição da classe
class binary256 {

    private:
        // constantes
        static const int MANTISSA = 1;
        static const bitset<MANTISSA_BITS + 1> ZERO;  // para comparação

        // variáveis       
        struct myFloat {
            unsigned int s_exp = 0;  
            bitset<MANTISSA_BITS> m;  
        } mf;

        // funções
        bool parser(string);  // converte a string para o meu float
        bitset<MANTISSA_BITS + 1> somaBitsets(bitset<MANTISSA_BITS + 1> v1, bitset<MANTISSA_BITS + 1> v2, bool &overflow);
        bitset<MANTISSA_BITS + 1> subtraiBitsets(bitset<MANTISSA_BITS + 1> v1, bitset<MANTISSA_BITS + 1> v2, bool &underflow);
        bitset<MANTISSA_BITS + 1> multiplicaBitsets(bitset<MANTISSA_BITS + 1> v1, bitset<MANTISSA_BITS + 1> v2, int &indice);
        bitset<MANTISSA_BITS + 1> divideBitsets(bitset<MANTISSA_BITS + 1> v1, bitset<MANTISSA_BITS + 1> v2, int &indice);
        
        int getExpoente();
        void setExpoente(int e);
        int getSinal() { return CHKBIT(mf.s_exp, BIT_SINAL);};
        void setSinal(int v) {if (v == 0) CLEARBIT(mf.s_exp, BIT_SINAL); else SETBIT(mf.s_exp, BIT_SINAL);};

    public: 
        // construtor
        binary256() {};
        binary256(string);  // construtor

        
        // funções
        string to_stringBinarioIEE(void); // retorna o meu float em formato string
        string to_stringDecimal(void); // retorna o meu float em formato string
        float to_Float(void); // retorna o valor em formato float, caso caiba em 8 bytes, senão retorna "inf"
        double to_Double(void); // retorna o valor em formato float, caso caiba em 8 bytes, senão retorna "inf"
        
        // sobrecarga de operadores
        binary256 operator+(binary256 b);
        binary256 operator-(binary256 b);
        binary256 operator*(binary256 b);
        binary256 operator/(binary256 b);
        
};

/************
Construtur da classe:
Entrada:
    valor -> string com o valor a ser guardado como float no forma binary256
Saída:
    nenuma
************/
binary256::binary256(string valor){

    // valida a string recebida e separa as partes inteira e decimal
    if (!parser(valor)){
        throw "Formato inválido";
    }
}

/************
Funcção para somar duas mantissas com operação por bit.
Entrada:
    v1 -> mantissa no formato bitset<112>
    v2 -> mantissa no formato bitset<112>
Saída:
    resultado da soma por bits de v1 e v2
************/
bitset<MANTISSA_BITS + 1> binary256::somaBitsets(bitset<MANTISSA_BITS + 1> v1, bitset<MANTISSA_BITS + 1> v2, bool &overflow){

    bitset<MANTISSA_BITS + 1> carry, retorno = v1;
	while (v2 != 0) 
	{ 
		carry = retorno & v2;   
        retorno = retorno ^ v2;  

		v2 = carry << 1; 
	} 
    overflow = (retorno.to_string() < v1.to_string() || retorno.to_string() < v2.to_string());
	return retorno;
}

/************
Funcção para sobtrair duas mantissas com operação por bit.
Entrada:
    v1 -> mantissa no formato bitset<112>
    v2 -> mantissa no formato bitset<112>
Saída:
    resultado da subtração de v1 por v2
************/
bitset<MANTISSA_BITS + 1> binary256::subtraiBitsets(bitset<MANTISSA_BITS + 1> v1, bitset<MANTISSA_BITS + 1> v2, bool  &underflow){
    bitset<MANTISSA_BITS + 2> empresta, n1(v1.to_string()), n2(v2.to_string());
    while (n2 != 0) 
	{ 
		empresta = (~n1 & n2); 
		n1 = n1 ^ n2; 
		n2 = empresta << 1; 
	} 
    underflow = (empresta.to_ulong() > 0); // significa a divisão de um número menor por um maior
    bitset<MANTISSA_BITS + 1> retorno(n1.to_string().substr(1)); // copia eliminado o bit mais significativo
    
    return retorno;
}

/************
Funcção para multiplicar duas mantissas com operação por bit.
Entrada:
    v1 -> mantissa no formato bitset<112>
    v2 -> mantissa no formato bitset<112>
Saída:
    resultado da multiplicação por bits de v1 e v2
************/
bitset<MANTISSA_BITS + 1> binary256::multiplicaBitsets(bitset<MANTISSA_BITS + 1> v1, bitset<MANTISSA_BITS + 1> v2, int &indice) {
    mpz_t n1, n2;
    string retornoStr;
    char *x;
    
    // cria os inteiros de precisão arbitrária
    mpz_init(n1);
    mpz_init(n2);
    if (mpz_set_str(n1, v1.to_string().c_str(), 2)) {throw "Error!";};
    if (mpz_set_str(n2, v2.to_string().c_str(), 2)) {throw "Error!";};
    
    // faz a multiplicação e retorna a string em binário
    // ao fazer a conta considerando todos os bits como parte de um inteiro, 
    // sendo que o valor da mantissa foi acrescido apenas de um "1" inteiro, 
    // considero como um exponete -236 (número de bits da mantissa)
    mpz_mul(n1, n1, n2);  // n1 = n1 * n2
    retornoStr = mpz_get_str(NULL ,2, n1);  // converte n1 para string com base 2 (binario)
    
    // como considerei acima que cada número tinha expoente -236, devo "retornar a vírgula" 472 casas para esquerda
    indice = retornoStr.length() - 472 - 1; // menos 1 pois tem o 1 que fica como parte inteira
    
    // guarda apenas os 113 dígitos da esquerda
    bitset<MANTISSA_BITS + 1> resultado(retornoStr.substr(0, MANTISSA_BITS + 1));

    // se o bit 114 for 1 e o 113 for 0, somo 1 como forma de arredondamento
    if (resultado[MANTISSA_BITS] == 0 && retornoStr[MANTISSA_BITS + 1] == '1')
        resultado[MANTISSA_BITS] == 1;
    
    return resultado;
	
}

/************
Funcção para dividir a mantissa em v1 pela mantissa em v2.
Entrada:
    v1 -> mantissa no formato "1" + bitset<112>
    v2 -> mantissa no formato "1" + bitset<112>
Saída:
    resultado da divisão por bits de v1 por v2
************/
bitset<MANTISSA_BITS + 1> binary256::divideBitsets(bitset<MANTISSA_BITS + 1> v1, bitset<MANTISSA_BITS + 1> v2, int &indice) {
    mpz_t n1, n2, resto, resultado;
    string retornoStr, resultadoStr;
    char *x;
    indice = 0;
    
    // cria os inteiros de precisão arbitrária
    mpz_init(n1);
    mpz_init(n2);
    mpz_init(resultado);
    mpz_init(resto);

    // atribui n1 e n2 removendo os zeros da direita, pois como são teoricamente casas decimais, não são significativas
    retornoStr = v1.to_string();
    //cout << "v1 " << temp << endl;
    //while(temp[temp.length() - 1] == '0') temp = temp.substr(0, temp.length() - 1);
    if (mpz_set_str(n1, retornoStr.c_str(), 2)) {throw "Error!";};
    retornoStr = v2.to_string();
    //cout << "v1 " << temp << endl;
    //while(temp[temp.length() - 1] == '0') temp = temp.substr(0, temp.length() - 1);
    if (mpz_set_str(n2, retornoStr.c_str(), 2)) {throw "Error!";};
    
    // faz a multiplicação e retorna para string em binário
    // ao fazer a conta considerando todos os bits como parte de um inteiro, 
    // sendo que o valor da mantissa foi acrescido apenas de um "1" inteiro, 
    // considero como um exponete -112
    //cout << "n1 " << mpz_get_str(NULL, 2, n1) <<  " - "  << mpz_get_str(NULL, 10, n1) << endl;
    //cout << "n2 " << mpz_get_str(NULL, 2, n2) <<  " - "  << mpz_get_str(NULL, 10, n2) << endl;
    mpz_divmod(resultado, resto, n1, n2);  // n1 = n1 * n2
    retornoStr = mpz_get_str(NULL ,2, resultado);  // converte n1 (resultado) para string com base 2 (binario)
    //cout << "retorno da divisão " << mpz_get_str(NULL ,10, n1) <<  " - "  << retornoStr << endl;
    //cout << "resto da fivisão " << mpz_get_str(NULL ,10, resto) <<  " - "  << mpz_get_str(NULL ,2, resto) << endl;

    // como considerei acima que cada número tinha expoente -112, devo "retornar a vírgula" a quantidade de casas do resultado - 1
    //indiceAjustado = retornoStr.length() - 1; // menos 1 pois tem o 1 que fica como parte inteira
    //cout << "indice ajustado " << indiceAjustado << endl;

    // fator de ajuste do expoente
    if (mpz_cmp_ui(resto, 0) != 0 && mpz_cmp(n1, n2) < 0){
        indice = -1;
    }

    
    // continua a divisão do resto, ajustando o índice
    //string xx;
    int i = 1;
    while (mpz_cmp_ui(resto, 0) != 0 && i < (MANTISSA_BITS + 1))
    {
        //xx = mpz_get_str(NULL ,2, resto);
        //cout << "resto antes " << mpz_get_str(NULL ,2, resto) << "tamanho " << xx.length() << endl;
        //xx = (string(mpz_get_str(NULL ,2, resto)) + "0");
        //cout << "resto com zero " << (char*)(string(mpz_get_str(NULL ,2, resto)) + "0").c_str() << "tamanho " << xx.length() << endl;
        mpz_set_str(resto,  (char*)(string(mpz_get_str(NULL ,2, resto)) + "0").c_str(), 2);
        //cout << "n2 " << mpz_get_str(NULL ,2, n2) << endl;
        mpz_divmod(resultado, resto, resto, n2);  // n1 = n1 * n2
        //cout << "controle " << i << endl;
        resultadoStr = mpz_get_str(NULL ,2, resultado);
        //cout << "controle " << i << endl;
        retornoStr += resultadoStr;
        //cout << "controle " << i << endl;
        //cout << "resto depois " << mpz_get_str(NULL ,2, resto) << endl;
        //cout << "resultado depois " << resultadoStr << endl;
        //cout << "incideajustado " << indiceAjustado << endl;
        i++;
    }
    //cout << "controle saida laço" << endl;

    //cout << "retorno parcial " << retornoStr << endl;

    

    // retira os zeros da esquerda, reajustando o índice
    while (retornoStr[0] == '0'){
    //    indiceAjustado--;
        retornoStr = retornoStr.substr(1); 
    }
    
    // adiciona zeros à direita para completar o tamanho, se precisar
    if (retornoStr.length() < (MANTISSA_BITS + 1))
        retornoStr += string((MANTISSA_BITS + 1) - retornoStr.length(),'0');
    
    // guarda apenas os 113 dígitos da esquerda
    bitset<MANTISSA_BITS + 1> retorno(retornoStr);
    
    return retorno;
	
}

/************
Funcção para retornar o exponete já ajustado pelo offset.
Entrada:
    não há
Saída:
    int -> valor o expoente
************/
int binary256::getExpoente(){
    int expoente = 0;

    // faz uma cópia para calcular
    expoente = mf.s_exp;

    // garante que o bit de sinal não estará ativo
    CLEARBIT(expoente, BIT_SINAL);
    
    // reduz o oofset
    expoente -= EXPOENTE_OFFSET;
    
    return expoente;
}

/************
Funcção para atribuir o valor do expoente, mantendo o sinal e ajustando o offset
Entrada:
    int -> novo valor do expoente
Saída:
    não há
************/
void binary256::setExpoente(int e){
    
    // guarda o sinal atual
    int sinal = getSinal();


    // verficia overflow w undeflow
    if (e > EXPOENTE_OFFSET){
        mf.s_exp = EXPOENTE_MAXIMO;
    } 
    else if (e < EXPOENTE_OFFSET_NEGATIVO)
    {
        mf.s_exp = 0;
    } 
    else{
        // guarda o novo expoente ajustado pelo offset
        mf.s_exp = e + EXPOENTE_OFFSET;
    }
     

    
    // retorna o sinal
    setSinal(sinal);
    
}


/************
Funcção par validar se a string recebida pode ser convertida para float.
Também divide o valor entre as partes inteira e decimal
Entrada:
    valor -> string com o valor a ser validado quanto à possibildiade de converter para fomato booleano
Saída:
    Bool-> indica se o valor pode ser convertido (true) ou não (false)
************/
bool binary256::parser(string valor){
    
    // em etapas:
    // 1 - guarda o sinal e separa as pateste inteira e decimal
    // 2 - converte para binário
    // 3 - normaliza a mantissa, ajustando o expoente 
    // 4 - guarda a mantissa

    int i;

    // Etapa 1 - guarda o sinal e separa as pateste inteira e decimal
    string valor_inteiro = "", valor_decimal = ""; // inicaliza em branco par validar se não existe mais de um separador de decimal
    bool partedecimal = false;
    int sinal = 0;

    for (int pos = 0; pos < valor.size(); pos++){  // percorrre os caracerres da string

        char c = valor[pos];  // identifica o caracter da string que será avaliado
        
        if (!ISDIGIT(c)) {  // se não for dígoti (0..9) avalia se é ponto ou se é erro
            switch (c) { // se for ponto, passa para avaliar a parte decimal
                case '.':
                    partedecimal = true;  
                    break;
                
                case '-':
                    sinal = 1;
                    break;
                
                case '+':
                    break;  // i bit inical já está setado em zero, então não há ação

                default:
                    return false;  // se não for dígito (0..9) nem ponto, nem sinal, não é aceito no formato de entrada estabelecido
            }
            
        }
        else {
            // separa as strings das partes inteira e decimal
            if (partedecimal){ 
                valor_decimal += c;
            }
            else{
                valor_inteiro += c;
            }
        }
    }
    // reove os zeros não significativos da parte decimal. É necessário na hora de converter para binário
    i = valor_decimal.length() - 1;
    while (valor_decimal[i] == '0'){
        valor_decimal = valor_decimal.substr(0, i);
        i--;
    }
    //cout << "valor inteiro: " << valor_inteiro << " e valor decimal: " << valor_decimal << endl;

    // Etapa 2 - converte para binário
    string binario = "";
    // parte 1: converte a parte inteira
    mpz_t v_transf, quociente, resto;
    // cria um inteiro de precisão arbitrária para guardar cada uma das partes para conversão
    mpz_init(v_transf);
    mpz_init(resto);
    mpz_init(quociente);
    if (mpz_set_str(v_transf, valor_inteiro.c_str(), 10)) {throw "Error!";};
    while (mpz_cmp_ui(v_transf, 0) > 0) {
        mpz_divmod_ui(quociente, resto, v_transf, 2);
        binario = to_string(mpz_sgn(resto)) + binario;
        mpz_set(v_transf, quociente);
        
    }

    // trasição para a parte decimal  
    binario = (binario == "" ? "0" : binario);  // garante que tenha pelo menos uma casa antes do ponto
    binario += ".";
    //cout << "binario do inteiro: " <<  binario << endl;
    
    // parte 2: converte a parte decimal
    string v_string;
    if (valor_decimal == "") valor_decimal = "0";
    if (mpz_set_str(v_transf, valor_decimal.c_str(), 10)) {throw "Error!";};
    int tamanho_base = strlen(mpz_get_str(NULL, 10, v_transf));
    for (i = 1; i <= MANTISSA_BITS; i++){ // gera os 112 digitos binarios da mantissa
        //cout << "v antes de multiplicar " << mpz_get_str(NULL, 10, v_transf) << endl;
        mpz_mul_ui(v_transf, v_transf, 2);
        //cout << "v depois de multiplicar " << mpz_get_str(NULL, 10, v_transf) << endl;
        if (strlen(mpz_get_str(NULL, 10, v_transf)) > tamanho_base){  // se aumentou de tamnho, ou seja, passou uma casa
            //cout << "bit " << i << " deu 1" << endl;
            binario += "1";
            v_string = mpz_get_str(NULL ,10, v_transf);
            if (mpz_set_str(v_transf, v_string.substr(1).c_str(), 10)) {throw "Error!";};  // remove o dígito que passou e guarda o nov valor
        }
        else {
            //cout << "bit " << i << " deu 0" << endl;
            binario += "0";
        }
    }

    // 3 - normaliza a mantissa, ajustando o expoente 
    i = 0;
    int expoente = 0;
    //cout << "parte 0: " << i << " - " << expoente << " - " << binario << endl; 
    while (binario[i] != '.'){ // percorre a parte inteira para ajustar o expoente
        expoente ++;
        i++;
    }
    //cout << "parte 1: " << i << " - "  << expoente << " - " << binario << endl; 
    binario = binario.substr(0, i) + binario.substr(i+1); // remove o ponto
    //cout << "parte 1.5: " << i << " - "  << expoente << " - " << binario << endl; 
    i = 0;
    while (binario[i] != '1'){ // remove os zeros iniciais reajustando o expoente
        expoente --;
        i++;
    }
    //cout << "parte 2: " << i << " - "  << expoente << " - " << binario << endl; 
    // para não guardar o 1 inciial, dá mais um salto no expoente e remove ele
    expoente--;
    binario = binario.substr(i+1); 
    //cout << "parte 3: " << i << " - "  << expoente << " - " << binario << endl; 
    
    
    // 4 - guarda a sinal, expoente e mantissa
    //cout << mf.s_exp << " - " << expoente << " - " << EXPOENT_OFFSET << endl;
    mf.s_exp = expoente + EXPOENTE_OFFSET;  // expoente
    if (sinal)
        SETBIT(mf.s_exp, EXPOENTE_BITS); // seta o 16 bit como 1 para indicar negativo (os outros 15 bits são o expoente) 
    for (int i = 0; i < binario.length() && i < MANTISSA_BITS; i++){
        //cout << "i: " << i << " binario[i] " << binario[i] << endl;
        mf.m[MANTISSA_BITS - 1 - i] = binario[i] - '0';  // guarda com 111 - i para ser da esquera para direita
    }
    
    //cout << "s_exp: " << mf.s_exp << " mantissa: " << mf.m << endl;

    return true;
}

/************
Funcção para representar o float guardado em formato binário, separando em sinal, exponete e mantissa.
Entrada:
    nenhuma
Saída:
    String-> representação do float armazenado
************/
string binary256::to_stringBinarioIEE(void){
    string myFloat = "";

    // pega o sinal e limpa o inteiro para calcular o expoente
    bitset<EXPOENTE_BITS + 1> ex;
    ex = mf.s_exp;
    string sinal = (ex[EXPOENTE_BITS] ? "1" : "0");
    
    myFloat = "Sinal / Expoente / mantissa -> \n " + sinal + " / " ;
    for (int i = (EXPOENTE_BITS - 1); i >= 0; i--) {
        myFloat += (ex[i] ? "1" : "0");
        if ((i % 8) == 0 && i > 0)
            myFloat += "-";
    }
    myFloat += " / ";
    for (int i = (MANTISSA_BITS - 1); i >= 0; i--) {
        myFloat += (mf.m[i] ? "1" : "0");
        if ((i % 8) == 0 && i > 0)
            myFloat += "-";
    }

    return myFloat;
}

/************
Funcção para representar o float guardado em formato string
Entrada:
    nenhuma
Saída:
    String-> representação do float armazenado
************/
string binary256::to_stringDecimal(void){
    string myFloat = "";

    // verfica exibição de overflow e undeflow
    if (mf.s_exp == EXPOENTE_MAXIMO) return "inf";
    if (mf.s_exp == 0) return "NaN";

    // pega o sinal
    int s = getSinal();

    // pega o expoente
    int e = getExpoente();

    
    // calcula a mantissa
    mpz_t v2, v36, v200, mantissa, modulo, temp;
    mpz_init(temp);
    mpz_init(v2);
    mpz_set_str(v2, "2", 10); // cria um valor "2" para facilitar o cáculo da exponenciação
    mpz_init(v36);
    mpz_set_str(v36, "1000000000000000000000000000000000000", 10); // cria um multiplicator com 36 zeros, pois e a expectativa máxima de casas decimais => log(2^113) aprox 34
    mpz_init(v200); // cria um multiplicator com 200 zeros. Não consegui calcular o log(2^16383), que seia o número máximo de casas decimais para o 2^(-expoente)
    mpz_set_str(v200, "100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", 10); 
    mpz_init(mantissa); // mantissa iniciada com zero

    // inicia a mantissa com o v36, pois é o "1" que fica oculto na representação
    mpz_set(mantissa, v36);

    for (int i = (MANTISSA_BITS - 1); i >= 0; i--) { // percorre os bits da mantissa calculando o valor, com expectativa de até 100 casas decimais
        if (mf.m[i] == 1){
            mpz_pow_ui(temp, v2, (MANTISSA_BITS-i));
            mpz_div(temp, v36, temp);
            mpz_add(mantissa, mantissa, temp); // mantissa = mantissa + (10^36 / (2 ^ (112 - i)))  -> isto permite montar a mantissa sem usar casas decimais, supondo que até 36 casas existiriam
        }
    }
    //cout << "mantissa " << mpz_get_str(NULL, 10, mantissa) << endl;

    // mantissa * (2^expoente)
    if (e >= 0){
        //cout << "e positivo " << to_string(e) << endl;
        mpz_set_str(temp, to_string(e).c_str(), 10);
        //cout << "converteu positivo v2 = " << mpz_get_str(NULL, 10, v2) << " temp = " << mpz_get_str(NULL, 10, temp) << " modulo = " << mpz_get_str(NULL, 10, modulo) << endl;
        //cout << " e " << mpz_get_str(NULL, 10, temp) <<  endl;
        mpz_powm(temp, v2, temp, v36);
        //cout << " 2^e " << mpz_get_str(NULL, 10, temp) <<  endl;
    }
    else{
        //cout << "e negativo " << to_string(e * (-1)) << endl;
        mpz_set_str(temp, to_string(e * (-1)).c_str(), 10);
        //cout << "converteu positivo " << endl;
        mpz_powm(temp, v2, temp, v36);
        //cout << "calculou potencia " << endl;
        mpz_div(temp, v200, temp);
    }
    //cout << " 2 ^ expoente " << mpz_get_str(NULL, 10, temp) <<  endl;
    
    mpz_mul(temp, temp, mantissa);

    //cout << "controle 6" << endl;
    
    // converte para string e ajusta as casas decimais
    myFloat = mpz_get_str(NULL, 10, temp);
    int decimais = 36;
    if (e < 0) decimais += 200; // se tinha expoente negativo, adiciona 200 casas decimais
    
    // completa com zeros à esquerda, se necessário (para formar o decimal)
    while(decimais > myFloat.length()) myFloat = "0" + myFloat;

    // coloca o sinal
    if (s) myFloat = "-" + myFloat;

    //cout << "mfloat leght " << myFloat.length() << endl;
    //int x = myFloat.length();
    myFloat = myFloat.substr(0, myFloat.length() - decimais) + "." + myFloat.substr(myFloat.length() - decimais);
    
    // elimina os zeros não significativos da direita
    while (myFloat[myFloat.length() - 1] == '0') myFloat = myFloat.substr(0, myFloat.length() - 1);


    return myFloat;

}

/************
Funcção para representar o valor armazenado em um tipo float, caso o valor caiba em 8 bytes, senão retorna "inf"
Entrada:
    nenhuma
Saída:
    Float: valor armazenado em formato float
************/
float binary256::to_Float(void){
    float myFloat = 1;

    //cout << "myfloat = " << myFloat << endl;
    //cout << "m = " << mf.m.to_string() << endl;
        
    // converte a mantissa
    for (int i = (MANTISSA_BITS - 1); i >= 0; i--){
        //cout << "---------------- " << endl;
        //cout << "i = " << i << endl;
        //cout << "m[i] = " << mf.m[i] << endl;
        if (mf.m[i] == 1) {
            //cout << "i-112 = " << i-112 << endl;
            
            myFloat += pow( 2, i-MANTISSA_BITS);
            //cout << "pow = " << pow( 2, i-112) << " gerou myfloat = " << myFloat << endl;
        
        }
    }

    //cout << "2 ^ expoente = " << pow( 2, getExpoente()) << endl;

    myFloat *= pow(-1, getSinal());
    myFloat *= pow(2, getExpoente()); 
    return myFloat;
}

/************
Funcção para representar o valor armazenado em um tipo double
Entrada:
    nenhuma
Saída:
    Double: valor armazenado em formato double
************/
double binary256::to_Double(void){
    double myDouble = 1;

    //cout << "myfloat = " << myFloat << endl;
    //cout << "m = " << mf.m.to_string() << endl;
        
    // converte a mantissa
    for (int i = (MANTISSA_BITS - 1); i >= 0; i--){
        //cout << "---------------- " << endl;
        //cout << "i = " << i << endl;
        //cout << "m[i] = " << mf.m[i] << endl;
        if (mf.m[i] == 1) {
            //cout << "i-112 = " << i-112 << endl;
            
            myDouble += pow( 2, i-MANTISSA_BITS);
            //cout << "pow = " << pow( 2, i-112) << " gerou myfloat = " << myFloat << endl;
        
        }
    }

    //cout << "2 ^ expoente = " << pow( 2, getExpoente()) << endl;

    myDouble *= pow(-1, getSinal());
    myDouble *= pow(2, getExpoente()); 
    return myDouble;
}

/************
Sobrecarga do operador adição, que permite somar dois números do tipo binary256
Parâmetros:
    binary256 = binary256 + biary128
************/
binary256 binary256::operator+(binary256 b){
    
    // cria uma cópia do valor atual para calcular o retorno
    binary256 retorno;
    retorno.mf = mf;
    
    // faz uma cópia das mantissas para poder adicionar o 1 oculto e realizar as operações
    bitset<MANTISSA_BITS + 1> m1(mf.m.to_string());
    bitset<MANTISSA_BITS + 1> m2(b.mf.m.to_string());
    m1[MANTISSA_BITS] = 1;
    m2[MANTISSA_BITS] = 1;
    bitset<MANTISSA_BITS + 1> zero(string("0"));
    
    //cout << "v1 " << to_stringDecimal() << endl;
    //cout << "v2 " << b.to_stringDecimal() << endl;
    
    // iguala os expoentes, já ajustando o expoente do retorno
    int e1 = getExpoente();
    int e2 = b.getExpoente();
    //cout << "m1 " << m1 << "expoente " << e1 << endl;
    //cout << "m2 " << m2 << "expoente " << e2 << endl;
    if (e2 < e1){
        while(e2 < e1){
            m2  = m2 >> 1;
            e2++;
        }        
        retorno.setExpoente(e2);
    } 
    else {
        while(e1 < e2){
            m1  = m1 >> 1;
            e1++;
        }
        retorno.setExpoente(e1);
    }
    //cout << "m1 norm " << m1 << "expoente " << retorno.getExpoente() << endl;
    //cout << "m2 norm " << m2 << "expoente " << retorno.getExpoente() << endl;
    
    // verifica o sinal dos  dois números, para ver se se é soma ou subtração
    int s1 = getSinal();
    int s2 = b.getSinal();
    bool opSoma = (s1 == s2);
    
    // calcula a nova mantissa
    bool carry;
    if (opSoma){
        retorno.mf.m = b.mf.m;   // o sinal mantem, pois são iguais. O expoente mantém pois foi ajustado
        //cout << "retorno na entrada da soma  \n" << retorno.to_stringBinarioIEE() << endl;
        m1 = somaBitsets(m1, m2, carry);
        //cout << "m1 saiu da soma " << m1 << "expoente " << retorno.getExpoente() << endl;
        if (carry){
            m1 = m1 >> 1;
            m1[MANTISSA_BITS] = 1;
            //cout << "retorno antes  \n" << retorno.to_stringBinarioIEE() << endl;
            //cout << "expoente antes " << retorno.getExpoente() << endl;
            //cout << "expoente que vai gravar " << (retorno.getExpoente() + 1) << endl;
            retorno.setExpoente(retorno.getExpoente() + 1);
            //cout << "expoente depois " << retorno.getExpoente() << endl;
            //cout << "retorno depois  \n" << retorno.to_stringBinarioIEE() << endl;
        }
        //cout << "fez soma m1 " << m1 << "expoente " << retorno.getExpoente() << endl;
    }
    else{
        
        // faz a subtração maior - menor (em módulo) e utiliza o sinal do maior (em módulo)
        // ao fazer sempre o menor - maio, evita o carry (underflow) da subtração
        if (m1.to_string() > m2.to_string()) 
        {
            m1 = subtraiBitsets(m1, m2, carry);
            retorno.setSinal(getSinal());
        }
        else{
            m1 = subtraiBitsets(m2, m1, carry);
            retorno.setSinal(b.getSinal());
        }
        //cout << "fez subtração m1 " << m1 << "expoente " << retorno.getExpoente() << endl;
    }

        // Normaliza o resultado, retirando os zeros da esquerda da mantissa e ajusta o expoente. 
    // Isto permite retirar o "1" que ficará a esquerda na mantissa e não é guardado
    if (m1 != zero){
        while (m1[MANTISSA_BITS] == 0){  
            m1 = m1 << 1; 
            retorno.setExpoente(retorno.getExpoente() - 1); 
        }
    }
    //cout << "m1 normalizado" << m1 << "expoente " << retorno.getExpoente() << endl;
    
    // guarda a mantissa calculada em retorno, removendo o 1 que não guarda 
    for (int i = 0; i < MANTISSA_BITS; i++) retorno.mf.m[i] = m1[i];   
    //cout << "m de retorno " << retorno.mf.m << "expoente " << retorno.getExpoente() << endl;
    
    // arredonda e renormaliza, se necessário

    return retorno;
}

/************
Sobrecarga do operador subtração, que permite subtrair dois números do tipo binary256
Parâmetros:
    binary256 = binary256 - biary128
************/
binary256 binary256::operator-(binary256 b){
    
    // Troca o sinal do valor recebido para usar a soma
    int sinal = b.getSinal();
    sinal = (sinal == 0 ? sinal = 1 : sinal = 0);
    b.setSinal(sinal);

    return (*this) + b;

}

/************
Sobrecarga do operador multiplicação, que permite multiplicar dois números do tipo binary256
Parâmetros:
    binary256 = binary256 * biary128
************/
binary256 binary256::operator*(binary256 b){
    binary256 retorno;

    // Multiplica as mantissas
    bitset<MANTISSA_BITS + 1> m1(mf.m.to_string());
    bitset<MANTISSA_BITS + 1> m2(b.mf.m.to_string());
    m1[MANTISSA_BITS] = 1;
    m2[MANTISSA_BITS] = 1;
    int indiceOverflow;
    //cout << "m1 " << m1 << endl;
    //cout << "m2 " << m2 << endl;
    bitset<MANTISSA_BITS + 1> m = multiplicaBitsets(m1, m2, indiceOverflow);
    //cout << "m " << m << endl;
    //cout << "overflow " << indiceOverflow << endl;
    
    
    // soma os expoentes dos número mltiplicados com o da pantissa (P1+P2+Pm)
    int e = getExpoente() + b.getExpoente() + indiceOverflow;

    // ajusta o expoente
    retorno.setExpoente(e);

    // sinal
    int s1 = getSinal();
    int s2 = b.getSinal();

    // ajusta o sina
    if (s1 == s2){  // '+' * '+' = '+' e '-' * '-' = '+'
        retorno.setSinal(0); 
    }
    else  // outros casos o sinal é '-'
    {
        retorno.setSinal(1);
    }

    // ajusta a mantissa
    for (int i = (MANTISSA_BITS - 1); i >= 0; i--){
        retorno.mf.m[i] = m[i];
    }
    
    return retorno;
}

/************
Sobrecarga do operador divisão, que permite divdir o numero de referência pelo valor em b
Parâmetros:
    binary256 = binary256 / biary128
************/
binary256 binary256::operator/(binary256 b){
    binary256 retorno;

    // Divide as mantissas
    bitset<MANTISSA_BITS + 1> m1(mf.m.to_string());
    bitset<MANTISSA_BITS + 1> m2(b.mf.m.to_string());
    m1[MANTISSA_BITS] = 1;
    m2[MANTISSA_BITS] = 1;
    int indiceAjustado;
    //cout << "m1 " << m1 << endl;
    //cout << "m2 " << m2 << endl;
    bitset<MANTISSA_BITS + 1> m = divideBitsets(m1, m2, indiceAjustado);
    //cout << "m " << m << endl;
    //cout << "indiceAjustado " << indiceAjustado << endl;
    
    // soma os expoentes dos número mltiplicados com o da pantissa (P1+P2+Pm)
    //cout << "e1 " << getExpoente() << endl;
    //cout << "e2 " << b.getExpoente() << endl;
    int e = getExpoente() - b.getExpoente() + indiceAjustado;
    //cout << "e final " << e << endl;
    
    // sinal
    int s1 = getSinal();
    int s2 = b.getSinal();

    // ajusta o sina
    if (s1 == s2){  // '+' * '+' = '+' e '-' * '-' = '+'
        retorno.setSinal(0); 
    }
    else  // outros casos o sinal é '-'
    {
        retorno.setSinal(1);
    }

    // ajusta o expoente
    retorno.setExpoente(e);

    // ajusta a mantissa
    for (int i = (MANTISSA_BITS - 1); i >= 0; i--){
        retorno.mf.m[i] = m[i];
    }
    
    return retorno;
}



#include <cstdlib>
#include <ctime>
#include <iostream>
int numeroAleatorio(int menor, int maior) {
       return rand()%(maior-menor+1) + menor;
}



int main() {

    
    string s1, s2;
    binary256 x, y, z;
    srand((unsigned)time(0)); //para gerar números aleatórios reais.
    for (int i = 0; i < 10; i++){
        s1 = to_string(numeroAleatorio(1, 1000))+"."+to_string(numeroAleatorio(1, 100000000))+to_string(numeroAleatorio(1, 100000000));
        s2 = to_string(numeroAleatorio(1, 1000))+"."+to_string(numeroAleatorio(1, 100000000));
        x = binary256(s1);
        y = binary256(s2);
        
        z = x/y;
        cout << s1+" / "+s2+ " = " <<  fixed << setprecision(10) << (double)stod(s1.c_str())/stod(s2.c_str()) << " <==> " << z.to_stringDecimal() << endl;
   
        //z = x*y;
        //cout << s1+" * "+s2+ " = " <<  fixed << setprecision(10) << (double)(stod(s1.c_str())*stod(s2.c_str())) << " <==> " << z.to_Double() << endl;
        //z = x-y;
        //cout << s1+" - "+s2+ " = " <<  fixed << setprecision(10) << (double)(atof(s1.c_str())-atof(s2.c_str())) << " <==> " << z.to_Double() << endl;
        //z = x+y;
        //cout << s1+" + "+s2+ " = " <<  fixed << setprecision(10) << (double)(stod(s1.c_str())+stod(s2.c_str())) << " <==> " << z.to_stringDecimal() << endl;
        
        //cout << " tostring " << x.to_stringDecimal() << " <==> " << s1 << endl;
    
    }
    


/*
   string s1, s2;
    s1 = "13.1828626045730441";
    s2 = "210.77826774";

    // certo
    //s1 = "75.79";
    //s2 = "53.86";

    //cout << "criando binary 1" << endl;
    binary256 x = binary256(s1);
    //cout << "criando binary 2" << endl;
    binary256 y = binary256(s2);
    //cout << "somando" << endl;
    binary256 z = x / y;
    cout << "x: " << x.to_stringBinarioIEE() << endl;
    cout << "y: " << y.to_stringBinarioIEE() << endl;
    cout << "z: " << z.to_stringBinarioIEE() << endl;
    cout << "z: " << z.to_stringDecimal() << endl;
    cout << "resultado " << fixed << setprecision(7) << (float)(atof(s1.c_str())/atof(s2.c_str())) << endl;
*/    


/*
    string s1 = "0.5005";
    string s2 = "0.75";
    binary256 x, y, z;
    x = binary256(s1);
    y = binary256(s2);
        //z = x/y;
        //cout << s1+" / "+s2+ " = " <<  fixed << setprecision(7) << (float)atof(s1.c_str())/atof(s2.c_str()) << " <==> " << z.to_Float() << endl;
   
        //z = x*y;
        //cout << s1+" * "+s2+ " = " <<  fixed << setprecision(10) << (double)(stod(s1.c_str())*stod(s2.c_str())) << " <==> " << z.to_Double() << endl;
        //z = x-y;
        //cout << s1+" - "+s2+ " = " <<  fixed << setprecision(10) << (double)(stod(s1.c_str())-stod(s2.c_str())) << " <==> " << z.to_Double() << endl;
        z = x+y;
        cout << s1+" + "+s2+ " = " <<  fixed << setprecision(10) << (double)(stod(s1.c_str())+stod(s2.c_str())) << " <==> " << z.to_Double() << endl;
        //cout << " tostring " << x.to_stringDecimal() << " <==> " << s1 << endl;
    
        cout << "em string " << z.to_stringDecimal() << endl; 
     */   
    
}
    


