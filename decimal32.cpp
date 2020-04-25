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

#define EXPOENTE_OFFSET 95  // offset para calculo do expoente 
#define COEFICIENTE_MAX 9999999 // (10^7) - 1 
#define DIGITOS_MAX 7 //  

#define ISDIGIT(x) (((x) >= '0') && ((x) <= '9'))

// definição da classe
class decimal32 {

    private:
        // constantes
        
        // variáveis       
        struct myDecimal {
            bitset<1> sign;
            bitset<5> comb;
            bitset<6> exp;
            bitset<20> coef;  
        } md;

        // funções
        bool parser(string, bool);  // converte a string para o decimal32, truncando, se permitir
        int getExpoente();
        void setExpoente(int e);
        unsigned int getCoeficiente();
        void setCoeficiente(unsigned int e);
        string chkInfNaN();
        void setInf();
        void setNaN();
        
    public: 
        // construtor
        decimal32() {};
        decimal32(string, bool);  // construtor

        
        // funções
        string to_stringBinarioIEE(void); // retorna o decimal32 em formato binário em string
        string to_stringDecimalIEE(void); // retorna o decimal32 em formato decimal em string
        string to_stringDecimal(void); // retorna o decimal32 em formato string
        float to_Float(void); // retorna o valor em formato float
        double to_Double(void); // retorna o valor em formato float
        
        // sobrecarga de operadores
        decimal32 operator+(decimal32 d);
        decimal32 operator-(decimal32 d);
        decimal32 operator*(decimal32 d);
        decimal32 operator/(decimal32 d);
        
};

/************
Construtur da classe:
Entrada:
    valor -> string com o valor a ser guardado como decimal no formato decimal 32.
    truncate -> se vai ignorar dígitos além do 7o dígito, se for após a casa decimal. Não trunca valor inteiro
Saída:
    nenuma
************/
decimal32::decimal32(string valor, bool truncate=true){

    // valida a string recebida e separa as partes inteira e decimal
    if (!parser(valor, truncate)){
        throw "Formato inválido";
    }
}

 

/************
Funcção para retornar o exponete já ajustado pelo offset.
Entrada:
    não há
Saída:
    int -> valor o expoente
************/
int decimal32::getExpoente(){
    string exp_str = "";

    // cria um string formando o binário od expoente
    if (md.comb[4] == 1 && md.comb[3] == 1){
        exp_str += (md.comb[2] + '0');
        exp_str += (md.comb[1] + '0'); 
    }
    else {
        exp_str += (md.comb[4] + '0');
        exp_str += (md.comb[3] + '0');
    }
    for (int i = 5; i >= 0; i--){
        exp_str += (md.exp[i] + '0');
    }

    //converte para decimal e subtrei o offset
    // aplica o offset e converte para binário
    mpz_t exp;
    mpz_init(exp);
    mpz_set_str(exp, exp_str.c_str(), 2);
    int e = mpz_get_ui(exp) - EXPOENTE_OFFSET;
    
    return e;

}

/************
Funcção para atribuir o valor do expoente, mantendo o sinal e ajustando o offset
Entrada:
    int -> novo valor do expoente
Saída:
    não há
************/

void decimal32::setExpoente(int e){
    
    // se já for inf ou NaN, não precisa guardar expoente
    if (chkInfNaN() != "") return;

    // valida faixa
    if (e < -95 || e > 96){
        setInf();
        return;
    }

    // aplica o offset e converte para binário
    mpz_t exp;
    mpz_init(exp);
    //int x = e+ EXPOENTE_OFFSET;
    mpz_set_ui(exp, e+ EXPOENTE_OFFSET);
    //mpz_set_str(exp, e + EXPOENTE_OFFSET, 10);
    string exp_bin = mpz_get_str(NULL, 2, exp);

    // garante o tamanho de 8 bits para facilitar o laço
    exp_bin = "00000000" + exp_bin;
    exp_bin = exp_bin.substr(exp_bin.length() - 8);

    // guarda os dois bits da esquerda
    if (md.comb[4] == 1 && md.comb[3] == 1){
        md.comb[2] = exp_bin[0] - '0';
        md.comb[1] = exp_bin[1] - '0';
    }
    else {
        md.comb[4] = exp_bin[0] - '0';
        md.comb[3] = exp_bin[1] - '0';
    }
    // guarda os bits restantes no expoente
    int pos = 5;
    for (int i = 2; i < exp_bin.length(); i++){
        md.exp[pos] = exp_bin[i] - '0';
        pos--;
    }
     
}

/************
Funcção para retornar o exponete já ajustado pelo offset.
Entrada:
    não há
Saída:
    int -> valor o expoente
************/
unsigned int decimal32::getCoeficiente(){
    
    // cria um string formando o binário do coeficiente
    string coef_str = "";

    // verifica a parte do combination
    if (md.comb[4] == 1 && md.comb[3] == 1){
        coef_str += "100";
        coef_str += (md.comb[0] + '0');
    }
    else {
        coef_str += '0';
        coef_str += (md.comb[2] + '0');
        coef_str += (md.comb[1] + '0');
        coef_str += (md.comb[0] + '0');
    }
    
    // pega o restante do oeficiente
    for (int i = 19; i >= 0; i--){
        coef_str += (md.coef[i] + '0');
    }

    //converte para decimal e subtrei o offset
    // aplica o offset e converte para binário
    mpz_t coef;
    mpz_init(coef);
    mpz_set_str(coef, coef_str.c_str(), 2);
    unsigned int c = mpz_get_ui(coef);
    
    return c;

}

/************
Funcção para atribuir o valor do expoente, mantendo o sinal e ajustando o offset
Entrada:
    int -> novo valor do expoente
Saída:
    não há
************/

void decimal32::setCoeficiente(unsigned int c){
    
    // valida infinity
    if (c > COEFICIENTE_MAX){
        setInf();
        return;
    }

    // converte para binário
    mpz_t coef;
    mpz_init(coef);
    mpz_set_ui(coef, c);
    //mpz_set_str(coef, c, 10);
    string coef_bin = mpz_get_str(NULL, 2, coef);
    // garante o tamanho de 24 bits para facilitar o laço
    coef_bin = "000000000000000000000000" + coef_bin;
    coef_bin = coef_bin.substr(coef_bin.length() - 24);

    // seo bit da esquerda é 1, move o expoente para comodar o flag
    if (coef_bin[0] == '1'){
        if (md.comb[4] != 1 || md.comb[3] != 1){
            md.comb[2] = md.comb[4];
            md.comb[1] = md.comb[3];
            md.comb[4] = md.comb[3] = 1;
        }
        md.comb[0] = (coef_bin[3] - '0');    
    }
    else
    {
        md.comb[2] = (coef_bin[1] - '0');
        md.comb[1] = (coef_bin[2] - '0');
        md.comb[0] = (coef_bin[3] - '0');
    }
    
    // guarda o resto do coeficiente
    int pos = 19;
    for (int i = 4; i < 24; i++){
        md.coef[pos] = (coef_bin[i] - '0');
        pos--;
    }
}

/************
Funcção para marcar este decimal32 como im Infinity.
Entrada:
    não há
Saída:
    não há
************/
void decimal32::setInf(){
    md.comb = bitset<5>(30);
}

/************
Funcção para marcar este decimal32 como im NaN.
Entrada:
    não há
Saída:
    não há
************/
void decimal32::setNaN(){
    md.comb = bitset<5>(31);
}

/************
Funcção para verificar se o número armazenado é um Inf ou um NaN.
Entrada:
    não há
Saída:
    string -> "Inf", "NaN" ou ""
************/
string decimal32::chkInfNaN(){
    
    // cria uma string para retorno
    string ret = "";

    // se é Inf Ou NaN
    int comb = (int)md.comb.to_ulong();

    switch (comb)
    {
    case 30:
        ret = "Inf";
        break;
    
    case 31:
        ret = "NaN";
        break;
    
    default:
        break;
    }
    
    return ret;

}

/************
Funcção par validar se a string recebida pode ser convertida para float.
Também divide o valor entre as partes inteira e decimal
Entrada:
    valor -> string com o valor a ser validado quanto à possibildiade de converter para fomato booleano
Saída:
    Bool-> indica se o valor pode ser convertido (true) ou não (false)
************/
bool decimal32::parser(string valor, bool truncate){
    
    
    int i;

    // Etapa 1 - guarda o sinal e separa as partes inteira e decimal
    string valor_inteiro = "", valor_decimal = ""; // inicaliza em branco para validar se não existe mais de um separador de decimal
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
                    md.sign[0] = 1;
                    break;
                
                case '+':
                    break;  // o bit de sinal já está setado em zero, então não há ação

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
    
    // remove os zeros não significativos da parte decimal.
    i = valor_decimal.length() - 1;
    while (valor_decimal[i] == '0'){
        valor_decimal = valor_decimal.substr(0, i);
        i--;
    }
    //cout << "valor inteiro: " << valor_inteiro << " e valor decimal: " << valor_decimal << endl;

    // remove os zeros não significativos da parte inteira.
    while (valor_inteiro.length() > 0 && valor_inteiro[0] == '0'){
        if (valor_inteiro.length() > 1)
            valor_inteiro = valor_inteiro.substr(1);
        else
            valor_inteiro = "";
        
    }
    //cout << "valor inteiro: " << valor_inteiro << " e valor decimal: " << valor_decimal << endl;

    // passa todos os dígitos para parte inteira, ajustando o expoente
    int expoente  = 0;
    if (valor_decimal.length() > 0){
        valor_inteiro += valor_decimal;
        expoente -= valor_decimal.length();
    }

    // se ainda tiver zeros á direita, retira aumentando o expoente
    i = valor_inteiro.length() - 1;
    while (valor_inteiro[i] == '0'){
        valor_inteiro = valor_inteiro.substr(0, i);
        expoente += 1;
        i--;
    }

    // tiver mais dígitos do que pode guardar 
    if (valor_inteiro.length() > 7){
        if (truncate){ // e se for para truncar
            // trunca, ajustando o expoente
            expoente += (valor_inteiro.length() - 7);
            valor_inteiro = valor_inteiro.substr(0, 7);
        }
        else { // senão, retorna o erro
            return false;
        }
    }

    // valida a faixa do expoente
    if (expoente > 96 || expoente < -95) return false;

    // guarda expoente
    setExpoente(expoente);
 
    // guarda o coeficiente
    setCoeficiente(stoul(valor_inteiro,nullptr,10)); 

    return true;
}

/************
Funcção para representar o float guardado em formato binário, separando em sinal, combination, exponete combination e coeficient.
Entrada:
    nenhuma
Saída:
    String-> representação do decimal32 armazenado
************/

string decimal32::to_stringBinarioIEE(void){
    string myDecimal = "";

    myDecimal = "Signal / Combination / Expoent Combination/ Coeficient Combination -> \n ";
    
    myDecimal += md.sign.to_string() + "/";
    myDecimal += md.comb.to_string() + "/";
    myDecimal += md.exp.to_string() + "/";
    myDecimal += md.coef.to_string();
    
    return myDecimal;
}

/************
Funcção para representar o float guardado em formato binário, separando em sinal, combination, exponete combination e coeficient.
Entrada:
    nenhuma
Saída:
    String-> representação do decimal32 armazenado
************/

string decimal32::to_stringDecimalIEE(void){
    string myDecimal = "";

    myDecimal = "[Signal, Coeficient, Expoent] -> [";
    
    if (md.sign == 1){
        myDecimal += "-, ";
    }
    else {
        myDecimal += "+, ";
    }
    myDecimal += to_string(getCoeficiente()) + ", ";
    myDecimal += to_string(getExpoente()) + "]";
    
    return myDecimal;
}

/************
Funcção para representar o float guardado em formato string
Entrada:
    nenhuma
Saída:
    String-> representação do float armazenado
************/
string decimal32::to_stringDecimal(void){
    string myDecimal = "";

    // se for int ou NaN, já retorna
    myDecimal = chkInfNaN();
    if (myDecimal != "") {
        return myDecimal;
    }

    // obtem o coeficiente
    myDecimal = to_string(getCoeficiente());

    // pega o expoente para as casas decimais
    int exp = getExpoente();

    if (exp != 0) {
        // se expoente é positivo, acrescenta os zeros à direita
        if (exp > 0){
            myDecimal += string(exp, '0');
        }
        else { // senão, inclui o ponto decimal
            if (fabs(exp) < myDecimal.length()) {  // se o ponto decimal cabe no meio do coeficiente
                myDecimal = myDecimal.substr(0, myDecimal.length() + exp) + "." + myDecimal.substr(myDecimal.length() + exp);
            }
            else { // se não, inclui os zeros necessário e o ponto
                myDecimal = "0." + string(fabs(exp) - myDecimal.length(), '0') + myDecimal;
            }
        }
    }

    // ajusta o sinal
    if (md.sign[0] == 1) {
        myDecimal = "-" + myDecimal;
    }

    return myDecimal;

}

/************
Funcção para representar o valor armazenado em um tipo float, caso o valor caiba em 8 bytes, senão retorna "inf"
Entrada:
    nenhuma
Saída:
    Float: valor armazenado em formato float
************/

float decimal32::to_Float(void){
    
    return stof(to_stringDecimal());
}


/************
Funcção para representar o valor armazenado em um tipo double
Entrada:
    nenhuma
Saída:
    Double: valor armazenado em formato double
************/

double decimal32::to_Double(void){
    return stod(to_stringDecimal());
}



/************
Sobrecarga do operador adição, que permite somar dois números do tipo decimal32
Parâmetros:
    decimal32 = decimal32 + decimal32
************/

decimal32 decimal32::operator+(decimal32 d){
    
    
    // cria uma cópia do valor atual para calcular o retorno
    decimal32 retorno;
    retorno.md = md;
    
    // verifica nan
    if (chkInfNaN() == "NaN" || d.chkInfNaN() == "NaN") {
        retorno.setNaN();
        return retorno;
    }

    // veriica infinity
    if (chkInfNaN() == "Inf" || d.chkInfNaN() == "Inf") {
        retorno.setInf();
        return retorno;
    }

    // obtem os valores do coeficientes para fazer a conta
    unsigned int c1 = getCoeficiente();
    unsigned int c2 = d.getCoeficiente();
    
    // iguala os expoentes, já ajustando o expoente do retorno
    int e1 = getExpoente();
    int e2 = d.getExpoente();
    //cout << "m1 " << m1 << "expoente " << e1 << endl;
    //cout << "m2 " << m2 << "expoente " << e2 << endl;
    int dif;
    if (e2 < e1){
        dif = fabs(e2 - e1);
        c1 *= pow(10, dif);
        e1 += (e2 - e1);
        retorno.setExpoente(e1);
    } 
    else {
        dif = fabs(e1 - e2);
        c2 *= pow(10, dif);
        e2 += (e1 - e2);
        retorno.setExpoente(e2);
    }
    //cout << "m1 norm " << m1 << "expoente " << retorno.getExpoente() << endl;
    //cout << "m2 norm " << m2 << "expoente " << retorno.getExpoente() << endl;
    
    // verifica o sinal dos  dois números, para ver se se é soma ou subtração
    bool opSoma = (md.sign == d.md.sign);
    
    // calcula o novo coeficiente
    if (opSoma){  // se os sianis são iguais

        retorno.setCoeficiente(c1 + c2);   //soma
        retorno.md.sign = md.sign; // repete o sinal
        //cout << "fez soma m1 " << m1 << "expoente " << retorno.getExpoente() << endl;
    }
    else {  
        
        // subtrai o maior do menor e usa o sinal do maior
        if (c1 > c2){
            retorno.setCoeficiente(c1 - c2);
            retorno.md.sign = md.sign;    
        }
        else {
            retorno.setCoeficiente(c2 - c1);
            retorno.md.sign = d.md.sign;
        }
        //cout << "fez subtração m1 " << m1 << "expoente " << retorno.getExpoente() << endl;
    }

    return retorno;
}

/************
Sobrecarga do operador subtração, que permite subtrair dois números do tipo decimal32
Parâmetros:
    decimal32 = decimal32 - decimal32
************/

decimal32 decimal32::operator-(decimal32 d){
    
    // Troca o sinal do valor recebido para usar a soma
    
    d.md.sign = (d.md.sign == 0 ? 1 : 0);
    
    return (*this) + d;

}

/************
Sobrecarga do operador multiplicação, que permite multiplicar dois números do tipo decimal32
Parâmetros:
    decimal32 = decimal32 * decimal32
************/

decimal32 decimal32::operator*(decimal32 d){
    
    // cria uma cópia do valor atual para calcular o retorno
    decimal32 retorno;
    
    // verifica nan
    if (chkInfNaN() == "NaN" || d.chkInfNaN() == "NaN") {
        retorno.setNaN();
        return retorno;
    }

    // veriica infinity
    if (chkInfNaN() == "Inf" || d.chkInfNaN() == "Inf") {
        retorno.setInf();
        return retorno;
    }

    
    // Multiplica os coeficientes
    // obtem os valores do coeficientes para fazer a conta
    unsigned int c1 = getCoeficiente();
    unsigned int c2 = d.getCoeficiente();
    retorno.setCoeficiente(c1*c2);

    // soma os expoentes
    int e1 = getExpoente();
    int e2 = d.getExpoente();
    retorno.setExpoente(e1 + e2);

    // sinal (ou exclusivo dos bits)
    retorno.md.sign = md.sign ^ d.md.sign;
    
    return retorno;
}

/************
Sobrecarga do operador divisão, que permite divdir o numero de referência pelo valor em b
Parâmetros:
    decimal32 = decimal32 / decimal32
************/

decimal32 decimal32::operator/(decimal32 d){
    
    // cria uma cópia do valor atual para calcular o retorno
    decimal32 retorno;
    
    // verifica nan
    if (chkInfNaN() == "NaN" || d.chkInfNaN() == "NaN") {
        retorno.setNaN();
        return retorno;
    }

    // veriica infinity
    if (chkInfNaN() == "Inf" || d.chkInfNaN() == "Inf") {
        retorno.setInf();
        return retorno;
    }
    
    unsigned int c1 = getCoeficiente();
    unsigned int c2 = d.getCoeficiente();
    
    // verifica se o divisor é zero (divisão por zero)
    if (c2 == 0){
        if (c1 == 0){
            retorno.setNaN();
        }
        else {
            retorno.setInf();
        }
        return retorno;
    }

    // se o dividendo é zero, já retorna zero
    if (c1 == 0){
        return  decimal32(0);
    }

    // realiza passo a passo a divisão dos coeficientes
    unsigned int c3 = 0;
    int adj = 0;
    // ajusta os coeficientes do dividendo e divisor para serem:
    // dividendo  >= divisor e dividendo < (10*divisor)
    while (c1 < c2){
        c1 *= 10;
        adj++;
    }
    while (c1 >= (10*c2)) {
        c2 *= 10;
        adj--;
    }

    // executa a divisão
    bool completo = false;
    while (!completo)
    {
        while (c2 <= c1) {
            c1 -= c2;
            c3++;
        }
        if ((c1 == 0 && adj >= 0) || to_string(c3).length() == DIGITOS_MAX){
            completo = true;
        }
        else {
            c3 *= 10;
            c1 *= 10;
            adj++;
        }
    }

    // guarda o coeficiente calculado
    retorno.setCoeficiente(c3);

    // guarda o expoente
    int e1 = getExpoente();
    int e2 = d.getExpoente();
    retorno.setExpoente(e1 - (e2 + adj));
    
    // sinal (ou exclusivo dos bits)
    retorno.md.sign = md.sign ^ d.md.sign;
    
    return retorno;
}



#include <cstdlib>
#include <ctime>
#include <iostream>
int numeroAleatorio(int menor, int maior) {
       return rand()%(maior-menor+1) + menor;
}



int main() {

    /*
    string s1, s2;
    decimal32 x, y, z;
    srand((unsigned)time(0)); //para gerar números aleatórios reais.
    for (int i = 0; i < 10; i++){
        s1 = to_string(numeroAleatorio(1, 1000)) +"."+to_string(numeroAleatorio(1, 1000));
        s2 = to_string(numeroAleatorio(1, 1000)) +"."+to_string(numeroAleatorio(1, 1000));
        x = decimal32(s1);
        y = decimal32(s2);
        z = x/y;
        cout << s1+" / "+s2+ " = " <<  fixed << setprecision(7) << (float)atof(s1.c_str())/atof(s2.c_str()) << " <==> " << z.to_Float() << endl;
   
        //z = x*y;
        //cout << s1+" * "+s2+ " = " <<  fixed << setprecision(10) << (double)(stod(s1.c_str())*stod(s2.c_str())) << " <==> " << z.to_Double() << endl;
        //z = x-y;
        //cout << s1+" - "+s2+ " = " <<  fixed << setprecision(10) << (double)(atof(s1.c_str())-atof(s2.c_str())) << " <==> " << z.to_Double() << endl;
        //z = x+y;
        //cout << s1+" + "+s2+ " = " <<  fixed << setprecision(10) << (double)(stod(s1.c_str())+stod(s2.c_str())) << " <==> " << z.to_Double() << endl;
        
        //cout << " tostring " << x.to_stringDecimal() << " <==> " << s1 << endl;
    
    }
    */
    
    string s1, s2;
    decimal32 x, y, z;
    s1 = "10";
    s2 = "3";
    x = decimal32(s1);
    y = decimal32(s2);
    //cout << "x / y" << x.to_Double() << " / " << y.to_Double() << endl;
    z = x/y;
    cout << s1+" * "+s2+ " = " <<  fixed << setprecision(10) << (double)(stod(s1.c_str())/stod(s2.c_str())) << " <==> " << z.to_Double() << endl;
    


    /*
    string s1, s2, s3, s4, s5;
    s1 = "12340";
    s2 = "0.1234";
    s3 = "0.001234";
    s4 = "-1234.03";
    s5 = "-123400000";
    
    decimal32 md;
    
    md = decimal32(s1);
    cout << s1 << " em string decimal32 " << md.to_stringDecimal() << " em float " << md.to_Float() << " em double " << md.to_Double() << endl; 

    md = decimal32(s2);
    cout << s2 << " em string decimal32 " << md.to_stringDecimal() << " em float " << md.to_Float() << " em double " << md.to_Double() << endl; 

    md = decimal32(s3);
    cout << s3 << " em string decimal32 " << md.to_stringDecimal() << " em float " << md.to_Float() << " em double " << md.to_Double() << endl; 

    md = decimal32(s4);
    cout << s4 << " em string decimal32 " << md.to_stringDecimal() << " em float " << md.to_Float() << " em double " << md.to_Double() << endl; 

    md = decimal32(s5);
    cout << s5 << " em string decimal32 " << md.to_stringDecimal() << " em float " << md.to_Float() << " em double " << md.to_Double() << endl; 
    
    */


    
}
    


