#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 800

typedef int (*funcp) ();
void libera (void *p);

static void error (const char *msg, int line) {
  fprintf(stderr, "erro %s na linha %d\n", msg, line);
  exit(EXIT_FAILURE);
}

// Passando o codigo de v2[0]-v2[tam] para v1[pos] sendo pos a posicao
// atual de v1, onde devem ser inseridos os codigos e tam a quantidade de condigo
int juntar_codigo(int pos, int tam,unsigned char*v1, unsigned char*v2){
	int i = 0;
	while (i < tam) {
		v1[pos] = v2[i];
		i++;
		pos++;
	}
	return pos;
}

void checkVar(char var, int idx, int line) {
  switch (var) {
    case 'v':
      if ((idx < 0) || (idx > 19))
       error("operando invalido", line);
      break;
    default:
       error("operando invalido", line);
   }
}
     
void checkVarP(char var, int idx, int line) {
  switch (var) {
    case 'v':
      if ((idx < 0) || (idx > 19))
       error("operando invalido", line);
      break;
    case 'p':
      if ((idx < 0) || (idx > 2))
       error("operando invalido", line);
      break;
    default:
       error("operando invalido", line);
   }
}
funcp compila (FILE *f)
{
	//alocar o array codigo
	 unsigned char *codigo = (unsigned char*) malloc (800 * sizeof(unsigned char));
	 //variaveis
	 int posicao_no_codigo = 0;
	 int valor = 0;
	 char c, varpc;
	 // vetores que contem a formatação típica de um arquivo assembly 
	 unsigned char start[] = {0x55,0x48,0x89,0xe5,0x48,0x83,0xec,0x50};
	 unsigned char end[] = {0xc9,0xc3};
	 unsigned char moveax[] = {0xb8};
	 unsigned char movedieax[] = {0x89,0xf8};
 	 unsigned char movesieax[] = {0x89,0xf0};
	 unsigned char movedxeax[] = {0x89,0xd0};

	 posicao_no_codigo = juntar_codigo(posicao_no_codigo,8,codigo,start);
	 

	 while( (c = fgetc(f)) != EOF ){
		
		if( c == 'r'){ // retornar constante
			fscanf(f,"et %c%d",&varpc,&valor);
			if(varpc == '$'){
				posicao_no_codigo = juntar_codigo(posicao_no_codigo,1,codigo,moveax);
				 *( (int *) &codigo[posicao_no_codigo] ) = valor; 	
				posicao_no_codigo = posicao_no_codigo+4;
			}
			else if( varpc == 'p'){ // retornar um dos tres parametros
				if(valor == 0)
					posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,movedieax);
				else if(valor == 1)
					posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,movesieax);
				else // valor == 2
					posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,movedxeax);
			}
			else if( varpc == 'v'){ // retornar uma variavel local
				if(valor > 20)
					exit(1);
				unsigned char alocar[] = {0x8b,0x45,0xfc};
				alocar[2] = alocar[2]-(4*valor); 
				posicao_no_codigo = juntar_codigo(posicao_no_codigo,3,codigo,alocar);
			}
		}
		else if( c == 'v'){  // Atribuicao
			//printf("Quantas vezes\n");
			int valor1 = 0;
			int valor2 = 0;
			int valor = 0;
			char op,varpc1;
			/*Vamos primeiro tentar rodar com constantes*/
			//por exemplo: v0 = p1 + p2
			fscanf(f,"%d = %c%d %c %c%d",&valor, &varpc, &valor1, &op, &varpc1, &valor2);
			//fscanf(f,"%d = %c%d",&valor,&varpc,&valor1);

			if(varpc == '$'){ //constante 
				//agora devemos saber qual vai ser o operador a ser usado
				if(op == '+'){
				//colocar no codigo a variavel local correto
	 			unsigned char atribavar[] = {0xc7,0x45,0xfc};
				atribavar[2] = atribavar[2]- (4*valor); 
				posicao_no_codigo = juntar_codigo(posicao_no_codigo,3,codigo,atribavar);
				//coolocar no codigo agora o valor a ser atribuido a variavel local
				*( (int *) &codigo[posicao_no_codigo] ) = valor1; 	
				posicao_no_codigo = posicao_no_codigo+4;
					if( varpc1 == '$'){ //constante
						// vamos adicionar a constante na variavel local
						unsigned char addvar[] = {0x81,0x45,0xfc};
						addvar[2] = addvar[2]-(4*valor);
						posicao_no_codigo = juntar_codigo(posicao_no_codigo,3,codigo,addvar);
						*( (int *) &codigo[posicao_no_codigo] ) = valor2; 
						posicao_no_codigo = posicao_no_codigo+4;	
					}
					else if( varpc1 == 'v'){ //var local
					}
					else if( varpc1 == 'p'){ //constante + parametro
						if (valor2 == 0) {
							unsigned char addvaredi[] = {0x81,0xc7,valor1};
							posicao_no_codigo = juntar_codigo(posicao_no_codigo,3,codigo,addvaredi);
							unsigned char movedivar[] = {0x67, 0x89, 0x7d, 0xfc};
							movedivar [3] = movedivar [3] - (4*valor);
							posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movedivar);
						}
						else if (valor2 == 1) {
							unsigned char addvaresi[] = {0x81,0xc6,valor1};
							posicao_no_codigo = juntar_codigo(posicao_no_codigo,3,codigo,addvaresi);
							unsigned char movesivar[] = {0x67, 0x89, 0x75, 0xfc};
							movesivar [3] = movesivar [3] - (4*valor);
							posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movesivar);
						}

						else if(valor2 ==2) { //constante + edx
							unsigned char addvaredx[] = {0x81,0xc2,valor1};
							posicao_no_codigo = juntar_codigo(posicao_no_codigo,3,codigo,addvaredx);
							unsigned char movedxvar[] = {0x67, 0x89, 0x55, 0xfc};
							movedxvar [3] = movedxvar [3] - (4*valor);
							posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movedxvar);
						}					
					}//fim constante + parametro		
				}
				if(op == '-'){
				//colocar no codigo a variavel local correto
	 			unsigned char atribavar[] = {0xc7,0x45,0xfc};
				atribavar[2] = atribavar[2]- (4*valor); 
				posicao_no_codigo = juntar_codigo(posicao_no_codigo,3,codigo,atribavar);
				//coolocar no codigo agora o valor a ser atribuido a variavel local
				*( (int *) &codigo[posicao_no_codigo] ) = valor1; 	
				posicao_no_codigo = posicao_no_codigo+4;
					if(varpc1 == '$'){ //constante
					     // vamos subtrair a constante a variavel local
						unsigned char subvar[] = {0x81,0x6d,0xfc};
						subvar[2] = subvar[2]-(4*valor);
						posicao_no_codigo = juntar_codigo(posicao_no_codigo,3,codigo,subvar);
						*( (int *) &codigo[posicao_no_codigo] ) = valor2; 
						posicao_no_codigo = posicao_no_codigo+4;
					}
				}
				if(op == '*'){
				unsigned char atribvarmult[] = {0xb9}; // mover para %ecx
				posicao_no_codigo = juntar_codigo(posicao_no_codigo,1,codigo,atribvarmult);
				//coolocar no codigo agora o valor a ser atribuido a variavel local
				*( (int *) &codigo[posicao_no_codigo] ) = valor1; 	
				posicao_no_codigo = posicao_no_codigo+4;
					if(varpc1 == '$'){ //constante
					// vamos multiplicar o r10d pela constante
						unsigned char imulvar[] = {0x69,0xc9};
						posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,imulvar);
						*( (int *) &codigo[posicao_no_codigo] ) = valor2; 
						posicao_no_codigo = posicao_no_codigo+4;
					// agora vamos fazer o mov de rd10 para a variavel local
						unsigned char imulatribvar[] = {0x89,0x4d,0xfc};
						imulatribvar[2] = imulatribvar[2]-(4*valor);   
						posicao_no_codigo = juntar_codigo(posicao_no_codigo,3,codigo,imulatribvar);
					}
				}
			}
			else if (varpc == 'p') {
				if (op == '+') {
					if (varpc1 == 'p') { //parametro + parametro
						if (valor1 == 0) {
							if (valor2 == 0) { // addl %edi, %edi
								unsigned char addediedi[] = {0x01,0xff};
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,addediedi);
								unsigned char movvar[] = {0x67, 0x89, 0x7d, 0xfc};
								movvar [3] = movvar[3] - (4*valor);
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movvar);
							}

							else if (valor2 == 1) { // addl %edi, %esi
								unsigned char addediesi[] = { 0x01, 0xfe };
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,addediesi);
								unsigned char movvar[] = { 0x67, 0x89, 0x75, 0xfc };
								movvar [3] = movvar [3] - (4*valor);
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movvar);
							}
					
							else if (valor2 == 2) { //addl %edi, %edx
								unsigned char addediedx[] = { 0x01, 0xfa };
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,addediedx);
								unsigned char movvar[] = { 0x67, 0x89, 0x55, 0xfc };
								movvar [3] = movvar [3] - (4*valor);
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movvar);
							}
						} // fim if valor1 == 0
						else if (valor1 == 1) {
							if (valor2 == 0) { // addl %esi, %edi
								unsigned char addesiedi[] = {0x01,0xf7};
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,addesiedi);
								unsigned char movvar[] = {0x67, 0x89, 0x7d, 0xfc};
								movvar [3] = movvar[3] - (4*valor);
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movvar);
							}

								else if (valor2 == 1) { // addl %esi, %esi
								unsigned char addesiesi[] = { 0x01, 0xf6 };
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,addesiesi);
								unsigned char movvar[] = { 0x67, 0x89, 0x75, 0xfc };
								movvar [3] = movvar [3] - (4*valor);
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movvar);
							}
					
								else if (valor2 == 2) { //addl %esi, %edx
								unsigned char addesiedx[] = { 0x01, 0xf2 };
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,addesiedx);
								unsigned char movvar[] = { 0x67, 0x89, 0x55, 0xfc };
								movvar [3] = movvar [3] - (4*valor);
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movvar);
							}
						}// fim if valor1 == 1
						
						else if (valor1 == 2) {
							if (valor2 == 0) { // addl %edx, %edi
								unsigned char addedxedi[] = {0x01,0xd7};
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,addedxedi);
								unsigned char movvar[] = {0x67, 0x89, 0x7d, 0xfc};
								movvar [3] = movvar[3] - (4*valor);
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movvar);
							}

								else if (valor2 == 1) { // addl %edx, %esi
								unsigned char addedxesi[] = { 0x01, 0xd6 };
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,addedxesi);
								unsigned char movvar[] = { 0x67, 0x89, 0x75, 0xfc };
								movvar [3] = movvar [3] - (4*valor);
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movvar);
							}
					
								else if (valor2 == 2) { //addl %edx, %edx
								unsigned char addedxedx[] = { 0x01, 0xd2 };
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,addedxedx);
								unsigned char movvar[] = { 0x67, 0x89, 0x55, 0xfc };
								movvar [3] = movvar [3] - (4*valor);
								posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movvar);
							}
						}// fim if valor1 == 2
					}//fim parametro + parametro

					else if (varpc1 == '$') {
						if (valor2 == 0) {
							unsigned char addvaredi[] = {0x81,0xc7,valor1};
							posicao_no_codigo = juntar_codigo(posicao_no_codigo,3,codigo,addvaredi);
							unsigned char movedivar[] = {0x67, 0x89, 0x7d, 0xfc};
							movedivar [3] = movedivar [3] - (4*valor);
							posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movedivar);
						}
						else if (valor2 == 1) {
							unsigned char addvaresi[] = {0x81,0xc6,valor1};
							posicao_no_codigo = juntar_codigo(posicao_no_codigo,3,codigo,addvaresi);
							unsigned char movesivar[] = {0x67, 0x89, 0x75, 0xfc};
							movesivar [3] = movesivar [3] - (4*valor);
							posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movesivar);
						}

						else if(valor2 ==2) { //constante + edx
							unsigned char addvaredx[] = {0x81,0xc2,valor1};
							posicao_no_codigo = juntar_codigo(posicao_no_codigo,3,codigo,addvaredx);
							unsigned char movedxvar[] = {0x67, 0x89, 0x55, 0xfc};
							movedxvar [3] = movedxvar [3] - (4*valor);
							posicao_no_codigo = juntar_codigo(posicao_no_codigo,4,codigo,movedxvar);
						}
					}//fim parametro + constante
					else if (varpc1 == 'v') {
					}//fim parametro + var local
				}// fim if op == +
			}
		}
		/*else if( c == 'i'){ // if

			char var;
 			int cnd1,cnd2,cnd3;
 
 			fscanf(f,"f %c%d %d %d %d",varpc,valor, &cnd1,&cnd2,&cnd3);
 	
 			if (var != '$') 
 				checkVar(var,valor,posicao_no_codigo);
		}*/ 
	}

	posicao_no_codigo = juntar_codigo(posicao_no_codigo,2,codigo,end);
	for(int i=0;i<posicao_no_codigo;i++)
		printf("0x%x -- %d\n",codigo[i],i);
	 return (funcp)codigo;
}

