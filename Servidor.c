#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <mysql.h>
#include <pthread.h>

typedef struct{
	int PosicionX;
	int PosicionY;
	int Vivo;
	int Asesino;
	int Tareas;
}Personaje; 
typedef struct{
	char Usuario[20];
	int Socket;
	int EnPartida; // 1 si está en una sala o partida/ 0 si no
	Personaje Personaje;
}Jugador;
typedef struct{
	Jugador jugadores[100];
	int num;
}ListaJugadores;
typedef struct{
	Jugador Jugador[8];
	int numJugadores;
	int ID_Lobby;
	char NombreLobby[100];
}Lobby;
typedef struct{
	Lobby Lobby[20];
	int num;
}ListaLobbies;
typedef struct{
	int id;
	char fecha[10];
	int duracion_MINUTOS;
	char ganador[20];
}Partida;
typedef struct{
	Partida partidas[100];
	int num;
}ListaPartidas;



//#################
//#== VARIABLES ==#
//#################

char peticion[8128];


int num_sockets;
int sockets[100];

ListaJugadores ListaJugador;
ListaLobbies ListaLobby;
ListaPartidas ListaPartida;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	//inicializar la exclusion

//##################
//#== FUNCTIONES ==#
//##################


void AddConectado(ListaJugadores *lista, char usuario[20], int socket){
	int i= 0;
	int encontrado = 0;
	while(i< lista->num&& encontrado == 0){
		if(strcmp(lista->jugadores[i].Usuario,usuario)==0){
			encontrado=1;
		}
		i = i+1;
	}
	if(encontrado==0 ){
		strcpy(lista->jugadores[lista->num].Usuario,usuario);
		lista->jugadores[lista->num].Socket = socket;
		lista->num = lista->num +1 ;
		printf("Flag:Añadido contactado: %s, Numero de usuarios conectados: %d \n", usuario, lista->num);
	}
}
int EliminaConectado(ListaJugadores *lista, char usuario[20]){
	int i = 0;
	while(i< lista->num){
		if(strcmp(lista->jugadores[i].Usuario,usuario) == 0){
			//printf("Flag: Parametros eliminar conectados : %s, %d. Num lista = %d \n	++ Jugador encontrado: %s %d\n",lista->jugadores[lista->num].Usuario,lista->jugadores[lista->num].Socket,lista->num,lista->jugadores[i].Usuario,lista->jugadores[i].Socket);
			strcpy(lista->jugadores[i].Usuario,lista->jugadores[lista->num-1].Usuario);
			lista->jugadores[i].Socket = lista->jugadores[lista->num-1].Socket;
			strcpy(lista->jugadores[lista->num-1].Usuario,"");
			lista->jugadores[lista->num-1].Socket =0 ;
			lista->num = lista->num - 1;
			printf("Flag:Usuario: %s eliminado\n",usuario);
		}
		i = i+1;
	}
	
}
void EnviarListaConectados(ListaJugadores *lista, char *respuesta){
	
	int i =0;	
	int j = 0 ;
	char conectados[1024];

	conectados[0] = '\0';
	strcat(conectados,"*99/");
	while(j<lista->num){
		if(strcmp(lista->jugadores[j].Usuario,"")!=0){
			strcat(conectados,lista->jugadores[j].Usuario);
			if(j!= lista->num-1){
				strcat(conectados,"-");
			}
		}
		j++;
	}
	strcat(conectados,"*");
	strcat(respuesta,conectados);

}
void EnviarInformacionLobby (ListaLobbies *ListaLobbies, char *respuesta){
	char LobbyInfo[1024];
	char LobbyInfo3[1024];
	
	LobbyInfo[0] = '\0';
	LobbyInfo3[0] = '\0';
	strcat(LobbyInfo,"*102/");
	for(int i=0; i< ListaLobbies->num; i++){
		sprintf(LobbyInfo3, "|^|%d|__|%s|_|",ListaLobbies->Lobby[i].ID_Lobby, ListaLobbies->Lobby[i].NombreLobby);
		strcat(LobbyInfo,LobbyInfo3);
		for(int k=0; k< ListaLobbies->Lobby[i].numJugadores; k++){
			sprintf(LobbyInfo3, "%s||", ListaLobbies->Lobby[i].Jugador[k].Usuario);
			strcat(LobbyInfo,LobbyInfo3);
		}
	}
	
	strcat(respuesta,LobbyInfo);
}

void EnviarMensaje(ListaLobbies *ListaLobbies,ListaJugadores *lista, int RoomID, int TipoMensaje2, char MensajeAEnviar[1024], char *respuesta){
    // El cliente enviará un mensaje a partir de una interfaz con un codigo asociado.
	// Ese mensaje se distribuirá a todos los clientes, y estos discerniran si pueden
	// recibir el mensaje o no segun si se encuentran en la ventana que tenga el codigo respectivo
	// Codigo1 0 = Menu principal (Chat general)
	// Codigo1 = 1 ; Codigo2 = X : Se manda el mensaje a todos los jugadores de la sala X
	// Codigo1 = 2 ; Codigo2 = X ; Se manda el mensaje a todos los jugadores en la partida X
	
	
	char mensaje[1024];
	mensaje[0] = '\0';
	if(RoomID == 0){
		sprintf(respuesta, "*6/0|%s*", MensajeAEnviar);	// Envio general
	}
	else{
		sprintf(respuesta, "*6/%d|%s*", RoomID ,MensajeAEnviar);
	}
	printf("Jugador ha enviado un mensaje por chat global\n");
	
}

int AnadirJugadorASala(ListaJugadores *lista, ListaLobbies *ListaLobbies, char usuario[128], int RoomID){ // Retrona 1 si se añadio correctamente, 0 si no
	int i= 0;
	while(i< lista->num){
		if(strcmp(lista->jugadores[i].Usuario,usuario) == 0 && lista->jugadores[i].EnPartida == 1){
			
			printf("Flag:Jugador ya en partida\n");
			return 0;
		}
		i = i+1;
	}
	i=0;
	int j = 0;
	while(i< ListaLobbies->num){
		printf("Flag:ID lobby : %d\n", ListaLobbies->Lobby[i].ID_Lobby);
		if(ListaLobbies->Lobby[i].ID_Lobby == RoomID){
			while(j< lista->num){
				printf("Flag:Buscando Comparacion %s  --  %s ", lista->jugadores[j].Usuario,usuario);
				if(strcmp(lista->jugadores[j].Usuario,usuario) == 0){
					ListaLobbies->Lobby[i].Jugador[ListaLobbies->Lobby[i].numJugadores] = lista->jugadores[j];
					printf("Flag:Jugador añadido a partida: %s\n", ListaLobbies->Lobby[i].Jugador[ListaLobbies->Lobby[i].numJugadores].Usuario );
					ListaLobbies->Lobby[i].numJugadores = ListaLobbies->Lobby[i].numJugadores + 1;
					return 1;
				}
				j = j+1;
			}
			//strcpy(ListaLobbies->Lobby[i].Jugador[ListaLobbies->Lobby[i].numJugadores].Usuario,usuario);
			//ListaLobbies->Lobby[i].Jugador[ListaLobbies->Lobby[i].numJugadores].Socket = socket;
			//ListaLobbies->Lobby[i].Jugador[ListaLobbies->Lobby[i].numJugadores].EnPartida = 1;
			
			
		}
		i = i+1;
	}
	return 0;
}
int EliminarJugadorSala(ListaLobbies *ListaLobbies, char usuario[128], int RoomID){ // Retrona 1 si se añadio correctamente, 0 si no
	int i= 0;
	int j = 0;
	while(i< ListaLobbies->num){
		if(ListaLobbies->Lobby[i].ID_Lobby == RoomID){
			while(j< ListaLobbies->Lobby[i].numJugadores){
				if(strcmp(ListaLobbies->Lobby[i].Jugador[j].Usuario,usuario) == 0){
					ListaLobbies->Lobby[i].Jugador[j] = ListaLobbies->Lobby[i].Jugador[ListaLobbies->Lobby[i].numJugadores];
					ListaLobbies->Lobby[i].numJugadores = ListaLobbies->Lobby[i].numJugadores - 1;
					printf("Flag:Jugador eliminado de la partida: %s\n", usuario );
					if(ListaLobbies->Lobby[i].numJugadores == 0){
						ListaLobbies->num -= 1;
					}
					return 1;
				}
				j = j+1;
			}
		}
		i = i+1;
	}
	return 0;
}
void EliminarJugadorSalaTodo(ListaLobbies *ListaLobbies, char usuario[128]){ 
	int i= 0;
	int j = 0;
	while(i< ListaLobbies->num){
		while(j< ListaLobbies->Lobby[i].numJugadores){
			printf("Flag: Eliminando jugador de todo Comparison: %s - %s  ", ListaLobbies->Lobby[i].Jugador[j].Usuario , usuario);
			if(strcmp(ListaLobbies->Lobby[i].Jugador[j].Usuario,usuario) == 0){
				ListaLobbies->Lobby[i].Jugador[j] = ListaLobbies->Lobby[i].Jugador[ListaLobbies->Lobby[i].numJugadores];
				ListaLobbies->Lobby[i].numJugadores = ListaLobbies->Lobby[i].numJugadores - 1;
				printf("Flag:Jugador eliminado de la partida: %s\n", usuario );
				if(ListaLobbies->Lobby[i].numJugadores == 0){
					ListaLobbies->Lobby[i] = ListaLobbies->Lobby[ListaLobbies->num];
					ListaLobbies->num -= 1;
				}
				
			}
			j = j+1;
		}
		i = i+1;
	}

}
int CrearSala(ListaLobbies *ListaLobbies, ListaJugadores *listaJ, char NombreLobby[100], char UsuarioQueEnvia[128], char *respuesta){ //Retorna 1 si se creo correctamente, 0 si no
	if(ListaLobbies->num < 19){
		strcpy(ListaLobbies->Lobby[ListaLobbies->num].NombreLobby,NombreLobby);
		ListaLobbies->Lobby[ListaLobbies->num].ID_Lobby = ListaLobbies->num+1;
		for(int i = 0; i < listaJ->num;i++){
			if(strcmp(listaJ->jugadores[i].Usuario,UsuarioQueEnvia) == 0){
				char LobbyInfo[4000];
				char RespuestaTemp[4000];
				LobbyInfo[0] = '\0';
				sprintf(RespuestaTemp, "101/%d|__|%s",ListaLobbies->Lobby[ListaLobbies->num].ID_Lobby, ListaLobbies->Lobby[ListaLobbies->num].NombreLobby);
				strcat(respuesta, RespuestaTemp);
				write (listaJ->jugadores[i].Socket,respuesta, strlen(respuesta));
			}
		}
		ListaLobbies->num = ListaLobbies->num + 1;
		printf("Flag: Sala %d ha sido creada por %s\n",ListaLobbies->Lobby[ListaLobbies->num-1].ID_Lobby, UsuarioQueEnvia);
		return ListaLobbies->Lobby[ListaLobbies->num-1].ID_Lobby;
	}
	else
	   return 0;
}
int ComprobarContrasena(MYSQL *conn, char password[40], char nombre_Usuario[40]){
	//Retorna 1 si la contraseña y el usuario pasados por parametro coinciden	
	int checked;
	MYSQL_RES *resultado;
	MYSQL_ROW fila;
	char consulta[500];
	sprintf(consulta,"SELECT jugador.username from (jugador) where jugador.username ='%s' AND jugador.password ='%s'", nombre_Usuario, password);
	
	int err;
	err = mysql_query(conn, consulta);
	if (err!=0){
		printf ("Error al consultar datos de la base %u %s\n", mysql_errno(conn),mysql_error(conn));
		exit(1);
	}
	
	resultado = mysql_store_result(conn);
	fila = mysql_fetch_row(resultado);
	
	if(fila == NULL)
		checked = 0;	//No se ha pasado ningun dato, esta vacio
	
	else{
		checked = 1;	//Si la contraseña y el usuario SÍ coinciden
	}
	return checked;
}

int Conquienhejugado(MYSQL *conn, char  nombre_Usuario[40]){
	//Retorna 1 si la contraseña y el usuario pasados por parametro coinciden	
	int checked;
	MYSQL_RES *resultado;
	MYSQL_ROW fila;
	char consulta[500];
	sprintf(consulta,"SELECT jugador.username from (jugador,participacion, partida) WHERE jugador.username ='%s' AND jugador.id  = participacion.id_jugador AND participacion.id_partida", nombre_Usuario);
	
	int err;
	err = mysql_query(conn, consulta);
	if (err!=0){
		printf ("Error al consultar datos de la base %u %s\n", mysql_errno(conn),mysql_error(conn));
		exit(1);
	}
	
	resultado = mysql_store_result(conn);
	fila = mysql_fetch_row(resultado);
	
	if(fila == NULL)
		checked = 0;	//No se ha pasado ningun dato, esta vacio
	
	else{
		checked = 1;	//Si la contraseña y el usuario SÍ coinciden
	}
	return checked;
}
void RegistrarUsuario(MYSQL *conn, char nombre_Usuario[20], char password[20], int id){
	
	MYSQL_RES *resultado;
	char consulta[500];
	consulta[0] = '\0';	//la primera posicion es para identificar el codigo
	sprintf(consulta, "INSERT into jugador VALUES ('%s', '%s', %d)", nombre_Usuario, password, id);
	
	int err;
	err = mysql_query(conn, consulta);
	if (err!=0){
		printf ("Error al consultar datos de la base %u %s\n", mysql_errno(conn),mysql_error(conn));
		exit(1);
	}
}
void DarDeBaja(MYSQL *conn, char nombre_Usuario[20]){
	
	MYSQL_RES *resultado;
	char consulta[500];
	consulta[0] = '\0';	//la primera posicion es para identificar el codigo
	sprintf(consulta, "DELETE FROM jugador WHERE username = '%s' ", nombre_Usuario);;
	int err;
	err = mysql_query(conn, consulta);
	if (err!=0){
		printf ("Error al consultar datos de la base %u %s\n", mysql_errno(conn),mysql_error(conn));
		exit(1);
	}
}
int UsuarioRegistrado(MYSQL *conn, char nombre_Usuario[20]){
	//Retorna 1 si el usuario ya esta en la base de datos
	//Retorna 0 si no esta registrado
	int registrado;
	MYSQL_RES *resultado;
	MYSQL_ROW fila;
	char consulta[500];
	sprintf (consulta, "SELECT jugador.username from (jugador) where jugador.username ='%s'", nombre_Usuario);
	
	int err;
	err = mysql_query(conn, consulta);
	
	resultado = mysql_store_result(conn);
	fila = mysql_fetch_row(resultado);
	
	if (fila == NULL) //no se ha encontrado al usuario
		registrado = 0;
	else{
		registrado = 1;	//Usuario ya registrado;
	}
	return registrado;
}


int ComprobarContrasenaFn(MYSQL *conn, char password[40], char nombre_Usuario[40]){
	//Retorna 1 si la contraseña y el usuario pasados por parametro coinciden	
	int checked;
	MYSQL_RES *resultado;
	MYSQL_ROW fila;
	char consulta[500];
	sprintf(consulta,"SELECT jugador.username from (jugador) where jugador.username ='%s' AND jugador.password ='%s'", nombre_Usuario, password);
	
	int err;
	err = mysql_query(conn, consulta);
	if (err!=0){
		printf ("Error al consultar datos de la base %u %s\n", mysql_errno(conn),mysql_error(conn));
		exit(1);
	}
	
	resultado = mysql_store_result(conn);
	fila = mysql_fetch_row(resultado);
	
	if(fila == NULL)
		checked = 0;	//No se ha pasado ningun dato, esta vacio
	
	else{
		checked = 1;	//Si la contraseña y el usuario SÍ coinciden
	}
	return checked;
}



void EnviarATodos(ListaJugadores *lista, char RespuestaGeneral2[8092], char UsuarioLocal[128], char respuesta[8092], int Override){
	if(Override!=1){
		for(int i= 0; i < lista->num; i++){
			if(strcmp(UsuarioLocal,lista->jugadores[i].Usuario)!=0){
				write (lista->jugadores[i].Socket,RespuestaGeneral2, strlen(RespuestaGeneral2));
				printf("Respuesta a todos los usuarios : %s | Usuario: %s\n", RespuestaGeneral2,lista->jugadores[i].Usuario);
			}
			else{
				sprintf(respuesta,"%s*%s", respuesta, RespuestaGeneral2);
				write (lista->jugadores[i].Socket,respuesta, strlen(respuesta));
				printf ("Respuesta: %s\n\n", respuesta);
			}
		}
	}
	
}
void EnviarInvitacion(ListaLobbies *ListaLobbies,ListaJugadores *lista, char UsuarioInvitado[128], int IdSala){
	char Envio[1024];
	int Socket;
	char DescripcionSala[256];
	for(int i =0; i < ListaLobbies->num; i++){
		if(ListaLobbies->Lobby[i].ID_Lobby == IdSala)
			strcpy(DescripcionSala,ListaLobbies->Lobby[i].NombreLobby);
	}
	for(int i= 0; i < lista->num; i++){
		if(strcmp(lista->jugadores[i].Usuario,UsuarioInvitado) == 0){
			sprintf(Envio, "34/%d|%s", IdSala,DescripcionSala);
			printf("Flag : Envio de invitación a: %s",UsuarioInvitado);
			write (lista->jugadores[i].Socket,Envio, strlen(Envio));
		}
	}
}
void EmpezarPartida( ListaLobbies *ListaLobbies, int ID_Sala, char *RespuestaGeneral){
	int PosicionMultiX = 0;
	int PosicionMultiY = 0;
	int AsesinoRandom;
	sprintf(RespuestaGeneral,"%s*50/", RespuestaGeneral);
	for(int i = 0 ; i < ListaLobbies->num ; i ++ ){
		if(ListaLobbies->Lobby[i].ID_Lobby== ID_Sala){
			AsesinoRandom = rand() % (ListaLobbies->Lobby[i].numJugadores);
			printf("Asesino: Numero de jugadores: %d  %d",AsesinoRandom, ListaLobbies->Lobby[i].numJugadores);
			for(int j = 0; j < ListaLobbies->Lobby[i].numJugadores; j++){
				
				ListaLobbies->Lobby[i].Jugador[j].Personaje.PosicionX = 1350 + PosicionMultiX * 100;
				ListaLobbies->Lobby[i].Jugador[j].Personaje.PosicionY = 2000 + PosicionMultiY *  300;
				ListaLobbies->Lobby[i].Jugador[j].Personaje.Vivo = 1;
				if(j == AsesinoRandom){
					ListaLobbies->Lobby[i].Jugador[j].Personaje.Asesino = 1;
				}
				else{
					ListaLobbies->Lobby[i].Jugador[j].Personaje.Asesino = 0;
				}
				ListaLobbies->Lobby[i].Jugador[j].Personaje.Tareas = 4;
				PosicionMultiX++;
				if(PosicionMultiX = 5){
					PosicionMultiY++;
				}
				sprintf(RespuestaGeneral, "%s|^|%d||%s||%d|%d|%d|%d|%d", RespuestaGeneral  ,ID_Sala, ListaLobbies->Lobby[i].Jugador[j].Usuario  ,ListaLobbies->Lobby[i].Jugador[j].Personaje.PosicionX  ,ListaLobbies->Lobby[i].Jugador[j].Personaje.PosicionY  ,ListaLobbies->Lobby[i].Jugador[j].Personaje.Vivo  ,ListaLobbies->Lobby[i].Jugador[j].Personaje.Asesino  ,ListaLobbies->Lobby[i].Jugador[j].Personaje.Tareas);
			}
		}
	}
}
void ActualizarPosicion(ListaLobbies *ListaLobbies, int ID_Sala, char *RespuestaGeneral, char UsuarioLocal[128], int PosX , int PosY , int Vivo , int Tareas){
	sprintf(RespuestaGeneral,"%s*52/", RespuestaGeneral);
	for(int i = 0 ; i < ListaLobbies->num ; i ++ ){
		if(ListaLobbies->Lobby[i].ID_Lobby== ID_Sala){
			for(int j = 0; j < ListaLobbies->Lobby[i].numJugadores; j++){
				if(strcmp(ListaLobbies->Lobby[i].Jugador[j].Usuario, UsuarioLocal)== 0){
					ListaLobbies->Lobby[i].Jugador[j].Personaje.PosicionX =  PosX;
					ListaLobbies->Lobby[i].Jugador[j].Personaje.PosicionY = PosY;
					ListaLobbies->Lobby[i].Jugador[j].Personaje.Vivo = Vivo;
					ListaLobbies->Lobby[i].Jugador[j].Personaje.Tareas = Tareas;
					sprintf(RespuestaGeneral, "%s|^|%d||%s||%d|%d|%d|%d", RespuestaGeneral  ,ID_Sala, ListaLobbies->Lobby[i].Jugador[j].Usuario  ,ListaLobbies->Lobby[i].Jugador[j].Personaje.PosicionX  ,ListaLobbies->Lobby[i].Jugador[j].Personaje.PosicionY  ,ListaLobbies->Lobby[i].Jugador[j].Personaje.Vivo  ,ListaLobbies->Lobby[i].Jugador[j].Personaje.Tareas);
					//if(Vivo== 0){
					//	EliminarJugadorSala(ListaLobbies,UsuarioLocal,ID_Sala);
					//}
				}
			}
		}
	}
}
void HaMuerto(ListaLobbies *ListaLobbies, int ID_Sala, char *RespuestaGeneral, char UsuarioLocal[128], char Muerto [128]){
	sprintf(RespuestaGeneral,"%s*53/", RespuestaGeneral);
	for(int i = 0 ; i < ListaLobbies->num ; i ++ ){
		if(ListaLobbies->Lobby[i].ID_Lobby== ID_Sala){
			for(int j = 0; j < ListaLobbies->Lobby[i].numJugadores; j++){
				if(strcmp(ListaLobbies->Lobby[i].Jugador[j].Usuario, Muerto)== 0){
					ListaLobbies->Lobby[i].Jugador[j].Personaje.Vivo = 0;
					ListaLobbies->Lobby[i].Jugador[j].Personaje.Tareas = 0;
					sprintf(RespuestaGeneral, "%s|^|%d||%s||", RespuestaGeneral ,ID_Sala, Muerto);
					EliminarJugadorSala(ListaLobbies,ListaLobbies->Lobby[i].Jugador[j].Usuario,ID_Sala);
					
				}
				
			}
		}
	}
	strcat(RespuestaGeneral,"*");
}

//#######################
//#== ATENDER CLIENTE ==#
//#######################

void *AtenderCliente(void *socket){
	MYSQL *conn;
	conn = mysql_init(NULL);
	
	if (conn==NULL) {
		printf ("Error al crear la conexion: %u %s\n", 
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	
	conn =mysql_real_connect(conn, "localhost", "root","mysql", "juego",0, NULL, 0);
	if (conn == NULL){
		printf("Error al crear la conexion");
		exit(1);
	}
	if (conn == NULL){
		printf ("Error al inicializar");
		exit(1);
	}
	
	//Bucle infinito para recibir peticiones
	int end = 0;
	char respuesta[8092];
	char peticion[500];
	int ret;
	char RespuestaGeneral[16000];
	char UsuarioLocal[128];
	char nombre_Generico[512];
	char nombre_Generico2[512];
	char nombre_Generico3[512];
	int numero_Generico;
	int numero_Generico2;
	int numero_Generico3;
	char MensajeChat[1024];
	int codigo;
	int Codigo1;
	int Codigo2;
	int Codigo3;
	int Codigo4;
	int Codigo5;
	char *p;
	int *s;
	s = (int *) socket;
	int sock_conn = *s;
	int Override = 0;
	
	while (end == 0){
		
		strcpy(RespuestaGeneral,"");
		strcpy(respuesta,"");
		char desconex[5];
		ret = read(sock_conn,peticion, sizeof(peticion));
		printf ("Recibido\n");
		// Tenemos que añadirle la marca de fin de string 
		// para que no escriba lo que hay despues en el buffer
		if(ret == 0){
			pthread_mutex_lock( &mutex );
			end = 1;	//Fin del bucle
			EliminaConectado(&ListaJugador, UsuarioLocal);
			pthread_mutex_unlock( &mutex );
		}
		else{
			peticion[ret]='\0';
			respuesta[0] = '\0';
			printf ("Peticion: %s\n",peticion);
			// vamos a ver que quieren
			p = strtok( peticion, "/");
			int codigo =  atoi (p);
			//Tenemos el codigo de la peticion
			// 1/Juan/contraseña
			if (codigo == 0 || ret == 0 ){	//Peticion de desconexion
				pthread_mutex_lock( &mutex );
				end = 1;	//Fin del bucle
				EliminaConectado(&ListaJugador, UsuarioLocal);
				EliminarJugadorSalaTodo(&ListaLobby, UsuarioLocal);
				pthread_mutex_unlock( &mutex );
				
			}
			
			else if (codigo == 1){ 		//Peticion de login
				pthread_mutex_lock( &mutex );
				
				p = strtok(NULL, "/");
				strcpy(UsuarioLocal, p);
				p = strtok(NULL, "/");
				strcpy(nombre_Generico, p);
				Codigo2 = UsuarioRegistrado(conn,UsuarioLocal);
				if (Codigo2 == 0)
					strcpy(respuesta, "1");	//User not registered
				else{//Usuario registrado
					int checked = ComprobarContrasenaFn(conn, nombre_Generico, UsuarioLocal);
					if (checked == 1){
						strcpy(respuesta, "2");	//Login succesful
						AddConectado(&ListaJugador, UsuarioLocal, sock_conn);
					}
				}
				pthread_mutex_unlock( &mutex );
			}
			else if (codigo == 2){	//Peticion de registro
				pthread_mutex_lock( &mutex );
				p = strtok(NULL, "/");
				strcpy(nombre_Generico, p);
				p = strtok(NULL, "/");
				strcpy(nombre_Generico2, p);
				int num_jugadores;
				Codigo1 = UsuarioRegistrado(conn, nombre_Generico);
				if (Codigo1 == 1){	//Usuario YA registrado
					strcpy(respuesta, "4/");
				}
				else {			//USUARIO NO REGISTRADO, le registramos
					RegistrarUsuario(conn, nombre_Generico, nombre_Generico2, num_jugadores);
					strcpy(respuesta, "5/");	//Registrado
				}
				Override = 1;
				pthread_mutex_unlock( &mutex );
			}
			
			else if (codigo == 4){	//Mensaje de un jugador por el chat
				pthread_mutex_lock( &mutex );
				p = strtok(NULL, "/");
				strcpy(MensajeChat, p);
				p = strtok(NULL, "/");
				Codigo1 = atoi(p);
				p = strtok(NULL, "/");
				Codigo2 = atoi(p);
				EnviarMensaje(&ListaLobby, &ListaJugador, Codigo1 , Codigo2 , MensajeChat, RespuestaGeneral);
				pthread_mutex_unlock( &mutex );
			}
			else if	(codigo == 5){	// Darse de baja
				DarDeBaja(conn, UsuarioLocal);
				EliminaConectado(&ListaJugador, UsuarioLocal);
			}
			else if (codigo == 6){	//Consulta los jugadores que jugaron el dia 29/05/2021
				
			}
			else if (codigo == 30){	//Solicitud crear una sala de lobby
				pthread_mutex_lock( &mutex );
				printf ("Creando Sala...\n");
				p = strtok(NULL, "/");
				strcpy(nombre_Generico, p); //En este caso es el nombre de la sala
				
				Codigo1 = CrearSala(&ListaLobby,&ListaJugador,  nombre_Generico, UsuarioLocal, respuesta);
				AnadirJugadorASala(&ListaJugador,&ListaLobby,UsuarioLocal,Codigo1);
				printf ("Sala creada...\n");
				pthread_mutex_unlock( &mutex );
				
			}
			else if (codigo == 31){	//Usuario se ha unido y necesitamos actualizar la información del lobby
				pthread_mutex_lock( &mutex );
				printf("\nFLAG : RespuestaGen0 : %s\n",RespuestaGeneral);
				EnviarInformacionLobby(&ListaLobby, RespuestaGeneral);
				printf("FLAG : RespuestaGen1 : %s\n",RespuestaGeneral);
				EnviarListaConectados(&ListaJugador, RespuestaGeneral);
				printf("FLAG : RespuestaGen2 : %s\n\n",RespuestaGeneral);
				pthread_mutex_unlock( &mutex );
			}
			else if (codigo == 32){	// Añadir Jugador a sala 
				pthread_mutex_lock( &mutex );
				p = strtok(NULL, "/");
				Codigo1 = atoi(p);
				AnadirJugadorASala(&ListaJugador,&ListaLobby, UsuarioLocal, Codigo1);
				pthread_mutex_unlock( &mutex );
			}
			else if (codigo == 33){	// Eliminando Jugador de la sala 
				pthread_mutex_lock( &mutex );
				p = strtok(NULL, "/");
				Codigo1 = atoi(p);
				EliminarJugadorSala(&ListaLobby, UsuarioLocal , Codigo1);
				EnviarInformacionLobby(&ListaLobby, RespuestaGeneral);
				pthread_mutex_unlock( &mutex );
			}
			else if (codigo == 34){	// Invitar jugador a sala
				pthread_mutex_lock( &mutex );
				p = strtok(NULL, "/");
				strcpy(nombre_Generico, p);
				p = strtok(NULL, "/");
				Codigo1 = atoi(p);
				EnviarInvitacion(&ListaLobby,&ListaJugador,nombre_Generico,Codigo1);
				pthread_mutex_unlock( &mutex );
			}
			else if (codigo == 40){	// Iniciar partida
				pthread_mutex_lock( &mutex );				
				p = strtok(NULL, "/");
				Codigo1 = atoi(p);
				sprintf(RespuestaGeneral,"%s*51/%d", RespuestaGeneral,Codigo1);
				pthread_mutex_unlock( &mutex );
			}
			else if (codigo == 41){	// Iniciar partida; Enviar informacion inicial
				printf("RET %d \n",ret);
				if(ret > 2){
					pthread_mutex_lock( &mutex );				
					p = strtok(NULL, "/");
					Codigo1 = atoi(p);
					EmpezarPartida(&ListaLobby, Codigo1, RespuestaGeneral);
					pthread_mutex_unlock( &mutex );
				}
			}
			else if (codigo == 42){	//Cambio de color de jugador
				printf("RET %d \n",ret);
				if(ret > 2){
					pthread_mutex_lock( &mutex );
					p = strtok(NULL, "/");
					Codigo1 = atoi(p);
					p = strtok(NULL, "/");
					Codigo2 = atoi(p);
					p = strtok(NULL, "/");
					Codigo3 = atoi(p);
					p = strtok(NULL, "/");
					Codigo4 = atoi(p);
					p = strtok(NULL, "/");
					Codigo5 = atoi(p);
					ActualizarPosicion(&ListaLobby,Codigo1,RespuestaGeneral,UsuarioLocal,Codigo2, Codigo3,Codigo4,Codigo5);
					pthread_mutex_unlock( &mutex );
				}
				
			}
			else if (codigo == 43){	//     ASESINATO!!
				pthread_mutex_lock( &mutex );
				p = strtok(NULL, "/");
				Codigo1 = atoi(p);
				p = strtok(NULL, "/");
				strcpy(nombre_Generico, p);
				HaMuerto(&ListaLobby, Codigo1, RespuestaGeneral,  UsuarioLocal, nombre_Generico);
				
				
				pthread_mutex_unlock( &mutex );
			}
			else if (codigo == 50){	//Cambio de color de jugador
				pthread_mutex_lock( &mutex );
				
				pthread_mutex_unlock( &mutex );
			}
			else if (codigo == 70){	
				pthread_mutex_lock( &mutex );
				EnviarListaConectados(&ListaJugador, RespuestaGeneral);
				pthread_mutex_unlock( &mutex );
			}
			if (codigo != 0 && strcmp(respuesta,"0")!= 0)
			{
				pthread_mutex_lock( &mutex );
				// Enviamos respuesta
				printf ("Respuesta general: %s\n", RespuestaGeneral);
				EnviarATodos(&ListaJugador, RespuestaGeneral, UsuarioLocal , respuesta, Override);
				if(Override == 1){
					write (sock_conn,respuesta, strlen(respuesta));
					printf ("Respuesta: %s\n", respuesta);
				}
				//sprintf(respuesta,"%s*%s", respuesta, RespuestaGeneral);
				//printf ("Respuesta: %s\n\n", respuesta);
				//printf("\n");
				//write (sock_conn,respuesta, strlen(respuesta));
				pthread_mutex_unlock( &mutex );
				
			}
			// Se acabo el servicio para este cliente
		}
		
	}
	close(sock_conn);
}
	
int main(int argc, char *argv[]){
	int n;
	int sock_conn, sock_listen, ret;
	int PORT = 9040;
	struct sockaddr_in serv_adr;
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	
	//Fem bind al port
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	
	// asocia el socket a cualquiera de las IP de la m?quina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// establecemos el puerto de escucha
	serv_adr.sin_port = htons(PORT); //cambiamos numero de puerto donde escuchamos
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	if (listen(sock_listen, 5) < 0)
		printf("Error en el Listen"); //hasta 3 peticiones en cola
	
	int end = 0;
	pthread_t thread[100];	//Aqui guardamos los ID de thread de cada usuario
	
	while (end ==0){
		printf("\n\nEscuchando\n");
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexion\n");
		
		sockets[num_sockets] = sock_conn;
		pthread_create(&thread[num_sockets],NULL,AtenderCliente,&sockets[num_sockets]);
		num_sockets= num_sockets +1;
	}	
	close(sock_conn);
}
