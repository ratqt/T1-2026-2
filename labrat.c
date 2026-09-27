/* 
 * Jogo do labirinto; feito por Roger Alan Torquato
 * 
 * -->	ffmpeg é necessário para que o áudio funcione (usado na função da linha 49)
 * -->	O jogo deve ser compilado e executado em linux; Não funcionará corretamente no windows.
 *
 * No lugar de armadilhas, coloquei criaturas que andam aleatoriamente até que consigam ver o jogador.
 * Enquanto o jogador está sendo perseguido, poderá usar letras maiúsculas para pular dois espaços ao invés de 1.
 * Isso possibilita correr mais facilmente das criaturas, e também pular por cima/atravessar paredes finas.
 * As rochas no mapa indicam áreas por onde o jogador pode escolher pular, e não a única rota possível. As rochas também não indicam a melhor rota.
 * 
 */

#include <stdio.h>   // Necessário para printf e scanf
#include <stdlib.h>  // Necessário para system (limpar tela em alguns sistemas)
#include <ctype.h>   // Necessário para toupper (converter tecla para maiúscula)
#include <time.h>
#include <unistd.h>  //sleep

//constantes globais
#define CR 20 //quantidade máxima de criaturas
#define DEBUG 0	//toggle para visualização de mensagens DEBUG
#define N 33 //tamanho máximo do labirinto

//cores
//#define RED "\e[0;91m"
#define GREEN "\e[0;32m"
#define BOLD "\e[1m"
#define RED_BOLD "\e[1;91m"
#define RESET "\e[0m"

//Globais:
int passos = 0;
short vida = 5;
int inimigoAvistado[CR] = {DEBUG, DEBUG, DEBUG, DEBUG, DEBUG, DEBUG, DEBUG, DEBUG, DEBUG, DEBUG}; //10 possíveis inimigos/criatura, que podem ou não ser avistados.
int cx[CR] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; //coordenada x para cada possível criatura
int cy[CR] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; //coordenada y para cada possível criatura
int labirinto[N][N];
int mensagem[3] = {0, 0, 0}; //Usado para imprimir mensagens específicas apenas após a impressão do labirinto. Antes, a mensagem aparecia por uma fração de um segundo e logo sumia.

int numAleat(int max){
	return (rand() % (max)) ;
}

int blInimAvist(){
	for(int c = 0; c < CR; c++){
		if(inimigoAvistado[c]) return 1;
	}
	return 0;
}

void sleepms(int milissegundos){
	usleep(milissegundos * 1000);
}

void tocarSom(int tipoSom){

	switch(tipoSom){
	case 1: 
		system("{ ffplay -nodisp -autoexit -nostats -hide_banner ganhar-novo.wav & } 2>/dev/null ; disown"); break;
	case 2:
		system("{ ffplay -nodisp -autoexit -nostats -hide_banner perder-novo.wav & } 2>/dev/null ; disown"); break;
	case 3:
		system("{ ffplay -nodisp -autoexit -nostats -hide_banner passos.wav & } 2>/dev/null ; disown"); break;
	case 4:
		system("{ ffplay -nodisp -autoexit -nostats -hide_banner dano.wav & } 2>/dev/null ; disown"); break;
	case 5:
		system("{ ffplay -nodisp -autoexit -nostats -hide_banner incorreto.wav & } 2>/dev/null ; disown"); break;
	case 6:
		system("{ ffplay -nodisp -autoexit -nostats -hide_banner detectar.wav & } 2>/dev/null ; disown"); break;
	}
}

void printJogador(){
	switch(vida){
	case 5:
		printf("\U0001F61F"); //emoji que representa o jogador fica mais assustado conforme perde vida
		break;
	case 4:
		printf("\U0001F627");
		break;
	case 3:
		printf("\U0001F628");
		break;
	case 2:
		printf("\U0001F630");
		break;
	case 1:
		printf("\U0001F631");
		break;
	default:
		printf("\U0001F480"); //emoji de caveira, quando jogador perde toda a vida
		break;
	}	
}

int criaturaAqui(int x, int y){
	for(int i = 0; i < CR; i++){
		if(cx[i] == x && cy[i] == y && inimigoAvistado[i]){
			return 1;
		}
	}
	
	return 0;
}

void mostrarLab(int x, int y){
        //printf("Jogo do Labirinto por Roger Alan Torquato\n"); //informações movidas para o início do código
	//printf("Obs.: Caso n\U000000E3o h\U000000E1 audio, ffmpeg deve ser instalado. O comando 'ffplay' do ffmpeg \U000000E9 usado para tocar som.\n\n");
	printJogador();
	printf(" = Jogador (pontos de vida: %d); \U0001F9F1 = Paredes; \U0001FA9C = Sa\U000000EDda\n", vida);
	//códigos unicode foram usados para imprimir letras com acentos (esqueci como fazer da forma correta e fiquei com preguiça de ver)
	if(blInimAvist()) printf("\U0001F47F = Criatura");
	printf("\n");

	int i, j;
	for (i = 0; i < N; i++){
		for (j = 0; j < N; j++){
			if (i == x && j == y){
				printJogador();
			}
			else if (criaturaAqui(i, j))
				printf("\U0001F47F"); // Mostra criatura -> U+1F47F = emoji de demoninho
			else if (labirinto[i][j] == 1)
				printf("\U0001F9F1"); // Mostra parede -> U+1F9F1 = emoji de tijolo
			else if (labirinto[i][j] == -1)
				printf("\U0001FA9C"); // Mostra saída -> U+1FA9C = emoji de escada
			else if (labirinto[i][j] == 2)
				printf("\U0001FAA8"); // Mostra armadilha -> U+1F573 = emoji de buraco
			else
				printf("  "); // Mostra caminho livre
		}
		printf("\n");
	}
	printf("\n");
}

int verifPosAntiga(int ultimaPosicao[CR][10][2], int futx, int futy, int idCriatura){
	for(int i = 0; i < 10; i++){
		if(futx == ultimaPosicao[idCriatura][i][0] && futy == ultimaPosicao[idCriatura][i][1])
		return 1;
	}
	return 0;
}

void criatura(int px, int py, int idCriat){
	
	if(cx[idCriat] == -1 || cy[idCriat] == -1) return;

	static int ultimaPos[CR][10][2] = {
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{-1, -1}
	};
	static int direcao[CR] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //0 = norte = -y ; 1 = sul = +y ; 2 = leste = +x ; 3 = oeste = -x
	int direcaoPrinc = direcao[idCriat];
	int verpx = cx[idCriat], vernx = cx[idCriat];
	int verpy = cy[idCriat], verny = cy[idCriat];
	while(verpx != -1 || vernx != -1 || verpy != -1 || verny != -1){ //-1 = encontrou parede antes de encontrar o jogador
		if(verny != -1) verny--;
		if(verpy != -1) verpy++;
		if(vernx != -1) vernx--;
		if(verpx != -1) verpx++;


		if(verny >= 0 && verny < N){
			if(px == cx[idCriat] && py == verny){
				if(px == cx[idCriat] && py == cy[idCriat] - 1){
					vida--;
					tocarSom(4);
					mensagem[2] = 1;
				}else cy[idCriat]--;
				direcao[idCriat] = 0;
				break;
			}else if(labirinto[cx[idCriat]][verny] >= 1){
				verny = -1;
			}
		}else verny = -1;
		
		if(verpy >= 0 && verpy < N){
			if(px == cx[idCriat] && py == verpy){
				if(px == cx[idCriat] && py == cy[idCriat] + 1){
					vida--;
					tocarSom(4);
					mensagem[2] = 1;
				}else cy[idCriat]++;
				direcao[idCriat] = 1;
				break;
			}else if(labirinto[cx[idCriat]][verpy] >= 1){
				verpy = -1;
			}
		}else verpy = -1;

		if(vernx >= 0 && vernx < N){
			if(px == vernx && py == cy[idCriat]){
				if(px == cx[idCriat] - 1 && py == cy[idCriat]){
					vida--;
					tocarSom(4);
					mensagem[2] = 1;
				}else cx[idCriat]--;
				direcao[idCriat] = 3;
				break;
			}else if(labirinto[vernx][cy[idCriat]] >= 1){
				vernx = -1;
			}
		}else vernx = -1;
	
		if(verpx >= 0 && verpx < N){
			if(px == verpx && py == cy[idCriat]){
				if(px == cx[idCriat] + 1 && py == cy[idCriat]){
					vida--;
					tocarSom(4);
					mensagem[2] = 1;
				}else cx[idCriat]++;
				direcao[idCriat] = 2;
				break;
			}else if(labirinto[verpx][cy[idCriat]] >= 1){
				verpx = -1;
			}
		}else verpx = -1;
	}

	int limite = 10;
	while(verpx == -1 && vernx == -1 && verpy == -1 && verny == -1){
		if(direcao[idCriat] == 0){
			if(labirinto[cx[idCriat]][cy[idCriat] - 1] < 1 && !verifPosAntiga(ultimaPos, cx[idCriat], cy[idCriat] - 1, idCriat)){
				cy[idCriat]--;
				break;
			}
		}
		if(direcao[idCriat] == 1){
			if(labirinto[cx[idCriat]][cy[idCriat] + 1] < 1 && !verifPosAntiga(ultimaPos, cx[idCriat], cy[idCriat] + 1, idCriat)){
				cy[idCriat]++;
				break;
			}
		}
		if(direcao[idCriat] == 2){
			if(labirinto[cx[idCriat] + 1][cy[idCriat]] < 1 && !verifPosAntiga(ultimaPos, cx[idCriat] + 1, cy[idCriat], idCriat)){
				cx[idCriat]++;
				break;
			}
		}
		if(direcao[idCriat] == 3){
			if(labirinto[cx[idCriat] - 1][cy[idCriat]] < 1 && !verifPosAntiga(ultimaPos, cx[idCriat] - 1, cy[idCriat], idCriat)){
				cx[idCriat]--;
				break;
			}
		}
		
		//0 = oeste = -y ; 1 = leste = +y ; 2 = sul = +x ; 3 = norte = -x
		if(direcao[idCriat] == 0) direcao[idCriat] = 2;
		else if(direcao[idCriat] == 2) direcao[idCriat] = 1;
		else if(direcao[idCriat] == 1) direcao[idCriat] = 3;
		else if(direcao[idCriat] == 3) direcao[idCriat] = 0;
	
		if(limite == 0) break;
		else limite--;
	}
	
	//if(px == cx[idCriat] && py == cy[idCriat]){
	//	vida--;
	//	tocarSom(4);
	//}

	int encontrado = 0;
	for(int i = 0; i < 10; i++){
		if(ultimaPos[idCriat][i][0] == -1){
			ultimaPos[idCriat][i][0] = cx[idCriat];
			ultimaPos[idCriat][i][1] = cy[idCriat];
			encontrado = 1;
		}
	}

	if(!encontrado){
		for(int i = 9; i > 0; i--){
			ultimaPos[idCriat][i][0] = ultimaPos[idCriat][i - 1][0];
			ultimaPos[idCriat][i][1] = ultimaPos[idCriat][i - 1][1];
		}
		ultimaPos[idCriat][0][0] = cx[idCriat];
		ultimaPos[idCriat][0][1] = cy[idCriat];
	}
}

void avistarInimigo(int px, int py){
	for(int idCriat = 0; idCriat < CR; idCriat++){
		int verpx = cx[idCriat];
		int vernx = cx[idCriat];
		int verpy = cy[idCriat];
		int verny = cy[idCriat];

		int limite = N*2;
		while(verpx != -1 || vernx != -1 || verpy != -1 || verny != -1){ //-1 = encontrou parede antes de encontrar o jogador
			if(limite == 0) break;
			limite--;
			if(verny != -1) verny--;
			if(verpy != -1) verpy++;
			if(vernx != -1) vernx--;
			if(verpx != -1) verpx++;

			if(verny >= 0 && verny < N){
				if(px == cx[idCriat] && py == verny){
					inimigoAvistado[idCriat] = 2;
					tocarSom(6);
					break;
				}else if(labirinto[cx[idCriat]][verny] == 1){
					verny = -1;
				}
			}else verny = -1;
			
			if(verpy >= 0 && verpy < N){
				if(px == cx[idCriat] && py == verpy){
					inimigoAvistado[idCriat] = 2;
					tocarSom(6);
					break;
				}else if(labirinto[cx[idCriat]][verpy] == 1){
					verpy = -1;
				}
			}else verpy = -1;
			
			if(vernx >= 0 && vernx < N){
				if(px == vernx && py == cy[idCriat]){
					inimigoAvistado[idCriat] = 2;
					tocarSom(6);
					break;
				}else if(labirinto[vernx][cy[idCriat]] == 1){
					vernx = -1;
				}
			}else vernx = -1;
			
			if(verpx >= 0 && verpx < N){
				if(px == verpx && py == cy[idCriat]){
					inimigoAvistado[idCriat] = 2;
					tocarSom(6);
					break;
				}else if(labirinto[verpx][cy[idCriat]] == 1){
					verpx = -1;
				}
			}
		}
		
		if(verpx == -1 && vernx == -1 && verpy == -1 && verny == -1 && inimigoAvistado[idCriat] > 0){ //-1 = encontrou parede antes de encontrar o jogador
			inimigoAvistado[idCriat]--;
		}

		if(px == cx[idCriat] && py == cy[idCriat]){
			inimigoAvistado[idCriat] = 2;
			tocarSom(6);
		}
	}

}

void gerarLabirinto(){
	int labNovo[N][N] = {
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},
		{1,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
		{1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
		{1,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1},
		{1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1},
		{1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1},
		{1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1},
		{1,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,1},
		{1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1},
		{1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,0,0,1},
		{1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1},
		{1,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1},
		{1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1},
		{1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
		{1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1},
		{1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1},
		{1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1},
		{1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
		{1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1},
		{1,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1},
		{1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,1,1,1,1,1,0,0,0,1,1,1},
		{1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,0,1,0,1,1,1},
		{1,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,1,1,1,0,0,0,1,0,0,0,1},
		{1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},
		{1,0,0,0,1,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,0,0,0,0,0,1,0,1},
		{1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1},
		{1,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1},
		{1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,1,0,1,1},
		{1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,1,1,1,1,1,1,0,0,-1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
	};
	
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			labirinto[i][j] = labNovo[i][j];
		}
	}

	int qtInims = (numAleat(5) + 3) * 2;
	int naoDuplicar[22] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	while(qtInims > 0){
		int valor = numAleat(16);
		switch(valor){
		case 0:
			if(naoDuplicar[0]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[0] = 1;
			cx[qtInims] = 13;
			cy[qtInims] = 3;
			break;
		case 1:
			if(naoDuplicar[1]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[1] = 1;
			cx[qtInims] = 7;
			cy[qtInims] = 7;
			break;
		case 2:
			if(naoDuplicar[2]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[2] = 1;
			cx[qtInims] = 3;
			cy[qtInims] = 9;
			break;
		case 3:
			if(naoDuplicar[3]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[3] = 1;
			cx[qtInims] = 11;
			cy[qtInims] = 9;
			break;
		case 4:
			if(naoDuplicar[4]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[4] = 1;
			cx[qtInims] = 3;
			cy[qtInims] = 15;
			break;
		case 5:
			if(naoDuplicar[5]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[5] = 1;
			cx[qtInims] = 9;
			cy[qtInims] = 15;
			break;
		case 6:
			if(naoDuplicar[6]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[6] = 1;
			cx[qtInims] = 1;
			cy[qtInims] = 19;
			break;
		case 7:
			if(naoDuplicar[7]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[7] = 1;
			cx[qtInims] = 7;
			cy[qtInims] = 19;
			break;
		case 8:
			if(naoDuplicar[8]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[8] = 1;
			cx[qtInims] = 19;
			cy[qtInims] = 19;
			break;
		case 9:
			if(naoDuplicar[9]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[9] = 1;
			cx[qtInims] = 11;
			cy[qtInims] = 21;
			break;
		case 10:
			if(naoDuplicar[10]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[10] = 1;
			cx[qtInims] = 13;
			cy[qtInims] = 23;
			break;
		case 11:
			if(naoDuplicar[11]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[11] = 1;
			cx[qtInims] = 21;
			cy[qtInims] = 23;
			break;
		case 12:
			if(naoDuplicar[12]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[12] = 1;
			cx[qtInims] = 19;
			cy[qtInims] = 25;
			break;
		case 13:
			if(naoDuplicar[13]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[13] = 1;
			cx[qtInims] = 13;
			cy[qtInims] = 29;
			break;
		case 14:
			if(naoDuplicar[14]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[14] = 1;
			cx[qtInims] = 15;
			cy[qtInims] = 31;
			break;
		case 15:
			if(naoDuplicar[15]){
				valor = numAleat(16);
				continue;
			}
			naoDuplicar[15] = 1;
			cx[qtInims] = 21;
			cy[qtInims] = 31;
			break;
		}
		qtInims--;
		valor = numAleat(6);
		switch(valor){
		case 0://
			if(naoDuplicar[16]){
				valor = numAleat(6);
				continue;
			}
			naoDuplicar[16] = 1;
			cx[qtInims] = 25;
			cy[qtInims] = 1;
			break;
		case 1://
			if(naoDuplicar[17]){
				valor = numAleat(6);
				continue;
			}
			naoDuplicar[17] = 1;
			cx[qtInims] = 23;
			cy[qtInims] = 5;
			break;
		case 2://
			if(naoDuplicar[18]){
				valor = numAleat(6);
				continue;
			}
			naoDuplicar[18] = 1;
			cx[qtInims] = 29;
			cy[qtInims] = 7;
			break;
		case 3://
			if(naoDuplicar[19]){
				valor = numAleat(6);
				continue;
			}
			naoDuplicar[19] = 1;
			cx[qtInims] = 25;
			cy[qtInims] = 9;
			break;
		case 4://
			if(naoDuplicar[20]){
				valor = numAleat(6);
				continue;
			}
			naoDuplicar[20] = 1;
			cx[qtInims] = 23;
			cy[qtInims] = 13;
			break;
		case 5://
			if(naoDuplicar[21]){
				valor = numAleat(6);
				continue;
			}
			naoDuplicar[21] = 1;
			cx[qtInims] = 29;
			cy[qtInims] = 15;
			break;
		}
		qtInims--;
	}

}

int main(){

	srand(time(NULL)); //aleatorização usada na geração das posições e quantidade dos inimigos

	gerarLabirinto();

	int x = 1, y = 1;
	char comando[100]; //Transformado em string pois char causava múltiplos caracteres inseridos a contarem como múltiplos comandos
	int jogando = 1;

	// Loop principal do jogo
	while (jogando)
	{
        system("clear");

	for(int i = 0; i < CR; i++){ //executa movimentação de cada criatura no mapa
		criatura(x, y, i);
	}

	avistarInimigo(x, y); //função para verificar se o jogador vê algum inimigo (e qual está sendo visto)

	mostrarLab(x, y);

	if(mensagem[0]){ //mensagens de erro (U+00E1 => á)
		printf("Comando Inv\U000000E1lido!\n");
		mensagem[0] = 0;
	}
	if(mensagem[1] == 1){ //U+00EA -> ê
		printf("Voc\U000000EA encontrou uma parede!\n");
		mensagem[1] = 0;
	}else if(mensagem[1] == 2){
		printf("Voc\U000000EA encontrou uma rocha!\n");
		printf("Talvez consiga pular por cima com mais velocidade...\n");
		mensagem[1] = 0;
	}

	if(mensagem[2]){
		printf("Voc\U000000EA foi atacado por uma criatura.\n");
		mensagem[2] = 0;
	}
	
	if(blInimAvist()){
		printf("%sVoc\U000000EA est\U000000E1 sendo perseguido por uma criatura.%s\n", RED_BOLD, RESET); //RED -> vermelho; BOLD -> negrito; RESET -> formatação do texto é resetada
		
		printf("Enquanto est\U000000E1 sendo perseguido, voc\U000000EA pode usar %sletras mai\U000000FAsculas%s para %scorrer%s. ", RED_BOLD, RESET, RED_BOLD, RESET);
		printf("Talvez consiga at\U000000E9 mesmo %spular por cima de obst\U000000E1culos%s...\n", RED_BOLD, RESET);

		printf("Obs.: Voc\U000000EA ainda pode andar normalmente com letras min\U000000FAsculas.\n");
	}

	if(vida <= 0){
		//system("clear");				//Eu estava planejando mostrar uma imagem quando o jogador perde o jogo, mas achei que ficou muito longe do tema.
		//if(numAleat(2)) system("cat plo.ansi");
		//else system("cat anomalocaris.ansi");
		printf("\n%sVoc\U000000EA perdeu toda a sua vida!%s\n", RED_BOLD, RESET);
		tocarSom(2);
		break;
	}else if (labirinto[x][y] == -1){
		printf("\nVoc\U000000EA encontrou a sa\U000000EDda com um total de: ");
		printf("%s%d passos e %d pontos de vida%s. Sua pontua\U000000E7\U000000E3o foi %s%d%s!\n", GREEN, passos, vida, RESET, GREEN, 1000 * vida - passos, RESET);
		tocarSom(1);
		break;
	}

	printf("\nCoordenadas atuais: x = %d, y = %d.\n", x, y);
        // Solicita movimento do jogador
        printf("Qual dire\U000000E7\U000000E3o voc\U000000EA deseja ir? ( w (Norte) / a (Oeste) / s (Sul) / d (Leste) ):\n");
	printf("> ");
        scanf(" %s", comando);
        if(!blInimAvist()) comando[0] = tolower(comando[0]); // Converte para minúscula, a não ser que esteja vendo um inimigo
	
        int novoX = x;
        int novoY = y;
	
	int duplo = 0;

	switch(comando[0]){
	case 'w':
		novoX--; break;
	case 's':
		novoX++; break;
	case 'a':
		novoY--; break;
	case 'd':
		novoY++; break;

	case 'W':
		novoX -= 2;
		duplo = 1;
		break;
	case 'S':
		novoX += 2;
		duplo = 1;
		break;
	case 'A':
		novoY -= 2;
		duplo = 1;
		break;
	case 'D':
		novoY += 2;
		duplo = 1;
		break;
	
	default:
		mensagem[0] = 1; break; //printf("Comando invalido!\n"); break;
	}

        if (novoX >= 0 && novoX < N && novoY >= 0 && novoY < N && labirinto[novoX][novoY] < 1){
		x = novoX;
		y = novoY;
		passos++;
		if(!mensagem[0]) tocarSom(3); //tocar som de passos apenas se o jogador conseguiu andar
        }else if(duplo){
		switch(comando[0]){
		case 'W':
			novoX++;
			break;
		case 'S':
			novoX--;
			break;
		case 'A':
			novoY++;
			break;
		case 'D':
			novoY--;
			break;
		}
	        if (novoX >= 0 && novoX < N && novoY >= 0 && novoY < N && labirinto[novoX][novoY] != 1){
			x = novoX;
			y = novoY; // Atualiza posição do jogador
			passos++;
			tocarSom(3);
	        }else{
			mensagem[1] = 1; //printf("Você encontrou uma parede!\n");
			tocarSom(5);
	        }
        }else{
		if(novoX >= 0 && novoX < N && novoY >= 0 && novoY < N && labirinto[novoX][novoY] == 2) mensagem[1] = 2; //você encontrou uma rocha!
		else mensagem[1] = 1; //printf("Você encontrou uma parede!\n");
		tocarSom(5); 
	}

	sleepms(50); //pausa por 50 milissegundos entre turnos
    }

    return 0; // Fim do jogo
}
