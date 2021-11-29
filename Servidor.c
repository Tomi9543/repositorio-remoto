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
	char Usuario[20];
	int Socket;
}Jugador;

typedef struct{
	Jugador jugadores[100];
	int num;
}ListaJugadores;

typedef struct{
	Jugador JugadoresPartida[10];
	char fecha[10];
	int num_Jugadores;	//Hasta 10 jugadores por partida
	int duracion_MINUTOS;
	char ganador[20];
}TPartida;

typedef TPartida TablaPartidas [100];


//Declaramos parametros
char peticion[512];
char respuesta[512];

TablaPartidas Tabla_P;
int numFilas;
int num_sockets;
int sockets[100];
ListaJugadores lista;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	//inicializar la exclusion

int CrearPartida(TablaPartidas tabla, char organizador[20], int socket){
	tabla[numFilas].JugadoresPartida[0].Socket = socket;	//Primer añadido a la partida es el que invita
	strcpy(tabla[numFilas].JugadoresPartida[0].Usuario, organizador);
	tabla[numFilas].num_Jugadores = 1;
	tabla[numFilas].duracion_MINUTOS=1;
	numFilas ++;
	if (numFilas ==0)
		return -1;
	else
		return 0;
}
int DameJugadoresPartida(TablaPartidas tabla, int posicion , char j1[20], char j2[20], char j3[20], char j4[20], char j5[20], char j6[20], char j7[20], char j8[20], char j9[20], char j10[20]){
	if ((posicion <numFilas)&&(tabla[posicion].duracion_MINUTOS>0)){
		strcpy(j1, tabla[posicion].JugadoresPartida[0].Usuario);
		strcpy(j2, tabla[posicion].JugadoresPartida[1].Usuario);
		strcpy(j3, tabla[posicion].JugadoresPartida[2].Usuario);
		strcpy(j4, tabla[posicion].JugadoresPartida[3].Usuario);
		strcpy(j5, tabla[posicion].JugadoresPartida[4].Usuario);
		strcpy(j6, tabla[posicion].JugadoresPartida[5].Usuario);
		strcpy(j7, tabla[posicion].JugadoresPartida[6].Usuario);
		strcpy(j8, tabla[posicion].JugadoresPartida[7].Usuario);
		strcpy(j9, tabla[posicion].JugadoresPartida[8].Usuario);
		strcpy(j10, tabla[posicion].JugadoresPartida[9].Usuario);
		return 0;
		
	}
	else {
		printf("FLAG FUNCION 2: Error al buscar JUGADORES\n");
		return -1;
	}
	//Si hubiera 5 jugadores, del los ultimos 5 jugadores serian NULL!
}

int DamePartidaAPartirDeOrganizador(TablaPartidas tabla, char org[20]){
	int i = 0;
	int encontrado = 0;
	while ((i<numFilas) && (encontrado == 0)){
		if (strcmp(tabla[i].JugadoresPartida[0].Usuario, org) == 0)
		{
			encontrado = 1;
		}
		else 
			i++;
	
	if (encontrado == 1){
		return i;
	}
	else 
		return -1;
	}
}

int DameFilas(TablaPartidas tabla){
	int i;
	int filas = 0;
	for (i=0; i<100; i++){
		if (tabla[i].duracion_MINUTOS>0){
			filas++;
		}
	}
	return filas;
}

int AddJugadorPartida(TablaPartidas tabla, char j[20], int socket, int partida){
	int found=0;
	int m=0;
	while((m<tabla[partida].num_Jugadores) &&(found ==0)){
		if (strcmp(tabla[partida].JugadoresPartida[m].Usuario,j) == 0)
			found =1;
		else
			j++;
	}
	if (found == 1){
		tabla[partida].JugadoresPartida[tabla[partida].num_Jugadores].Socket = socket;
		strcpy(tabla[partida].JugadoresPartida[tabla[partida].num_Jugadores].Usuario, j);
		tabla[partida].num_Jugadores++;
		return 1;
	}
	else 
		return -1;
}

int DameOrganizadorPartida(TablaPartidas tabla, int partida, char organizador[20]){
	strcpy(organizador, tabla[partida].JugadoresPartida[0].Usuario);
	if (organizador == NULL){
		printf("No hay organizador\n");
	}
	else
		printf("Organizador: %s\n", organizador);
}
int DameUsuarioAPartirDeSocket(ListaJugadores *lista, int socket, char jugador[20]){
	int encontrado = 0;
	int i = 0;
	while ((i<lista->num) && (encontrado == 0)){
		if (lista->jugadores[i].Socket == socket){
			encontrado =1;
		}
		else
			i++;
	}
	if (encontrado == 0){
		printf("FLAG FUNCION 6:Error al Retornar Usuario desde SOcket\n");
		return 1;
	}
	else {
		strcpy(jugador, lista->jugadores[i].Usuario);
		return 0;
	}
}


int DameSocketAPartirDeUsuario(ListaJugadores *lista, char jugador[20]){
	int encontrado = 0;
	int i=0;
	int socket;
	while ((i<lista->num) && (encontrado == 0)){
		if (strcmp(lista->jugadores[i].Usuario, jugador)==0){
			encontrado =1;
			socket = lista->jugadores[i].Socket;
		}
		else 
			i++;
	}
	if (encontrado == 0)
		return -1;
	else 
		return socket;
}

void AddConectado(ListaJugadores *lista, char usuario[20], int socket){
	int i= 0;
	int encontrado = 0;
	printf("Flag:Lista num = %d\n",lista->num);
	printf("Flag:Socket = %d\n",socket);
	printf("Flag:UsuarioAAñadir = %s\n",usuario);
	while(i< lista->num&& encontrado == 0){
		if(strcmp(lista->jugadores[i].Usuario,usuario)==0){
			
			encontrado=1;
			printf("Flag:Encontrado\n");
		}
		i = i+1;
	}
	if(encontrado==0 ){
		strcpy(lista->jugadores[lista->num].Usuario,usuario);
		lista->jugadores[lista->num].Socket = socket;
		lista->num = lista->num +1 ;
		printf("Flag:AñadidoConectado\n");
	}
}

int EliminaConectado(ListaJugadores *lista, char usuario[20]){
	int i = 0;
	while(i< lista->num){
		if(strcmp(lista->jugadores[i].Usuario,usuario) == 0){
			strcpy(lista->jugadores[i].Usuario,lista->jugadores[lista->num].Usuario);
			lista->jugadores[i].Socket = lista->jugadores[lista->num].Socket;
			strcpy(lista->jugadores[lista->num].Usuario,"");
			lista->jugadores[lista->num].Socket =0 ;
			lista->num = lista->num - 1;
			printf("Flag:Eliminar Conectado\n");
		}
		i = i+1;
	}
	
}

void EnviarListaConectados(int numSockets, int sockets[100], ListaJugadores *lista){
	int i =0;	
	int j = 0 ;
	char envio[1024];
	char conectados[1024];
	conectados[0] = '\0';
	while(j<lista->num){
		strcat(conectados,lista->jugadores[j].Usuario);
		if(j!= lista->num-1){
			strcat(conectados,"-");
		}
		j++;
	}
	sprintf(envio, "99/%s", conectados);	//Asignamos el codigo 99 a enviar la lista de conectados
	printf("Envio:");
	printf(envio);
	printf("\n");
	while (i<numSockets){		//Escribimos en consola los sockets activos (los conectados)
		write(sockets[i],envio,strlen(envio));
		i++;
	}
}
int Usuario_Registrado(MYSQL *conn, char nombre_Usuario[20]){
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

int Password_Check(MYSQL *conn, char password[40], char nombre_Usuario[40]){
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

void Registrar_Usuario(MYSQL *conn, char nombre_Usuario[20], char password[20], int id){
	
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

int JugadoresQueJugaronTalDIA (MYSQL *conn, char day[20] ){
	
	MYSQL_RES *resultado;
	MYSQL_ROW fila;
	char request [500];
	int num_Jugadores = 0;
	
	sprintf(request, "Select distinct jugador.username from (Jugador, Participacion, Partida) where partida.fecha ='%s' and participacion.id_jugador = jugador.id and participacion.id_partida = partida.id ", day);
		
	int err;
	err = mysql_query(conn, request);
	if (err!=0){
		printf ("Error al consultar datos de la base %u %s\n", mysql_errno(conn),mysql_error(conn));
		exit(1);
	}
	
	resultado = mysql_store_result(conn);
	fila = mysql_fetch_row(resultado);
	
	while (fila != NULL){
		if (fila != NULL){
			num_Jugadores++;
		}
		else
			printf("Ha habido un error");
		fila = mysql_fetch_row(resultado);
	return num_Jugadores;
	}
	mysql_close(conn);
	exit(0);	
}

int JugadoresQueJugaronJuntos (MYSQL *conn, char nombre[20], char respuesta){
	MYSQL_RES *resultado;
	MYSQL_ROW fila;
	char request [200];
	//No se hacer la consulta!

}

int DamePerdedores (MYSQL *conn, char nombre[20]){
	MYSQL_RES *resultado;
	MYSQL_ROW fila;
	char request [200];
}


void AtenderCliente(void *socket){
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
	char peticion[500];
	int ret;
	char nombre_Usuario[20];
	char password[20];
	char day[20];
	int *s;
	s = (int *) socket;
	int sock_conn = *s;
	char respuesta[300];
	char respuesta_SI_NO[10];
	char organizador[20];
	int partida;
	char invitada[20];
	
	while (end == 0){
		char conectados[512];
		int added;
		int registrado;
		char desconex[5];
		ret = read(sock_conn,peticion, sizeof(peticion));
		printf ("Recibido\n");
		
		// Tenemos que añadirle la marca de fin de string 
		// para que no escriba lo que hay despues en el buffer
		peticion[ret]='\0';
		respuesta[0] = '\0';
		conectados[0]='\0';	//marcas de final de linea
		
		printf ("Peticion: %s\n",peticion);
		// vamos a ver que quieren
		char *p = strtok( peticion, "/");
		int codigo =  atoi (p);
		//Tenemos el codigo de la peticion
		// 1/Juan/contraseña
		if (codigo == 0){	//Peticion de desconexion
			end = 1;	//Fin del bucle
			EliminaConectado(&lista, nombre_Usuario);
			EnviarListaConectados(num_sockets,sockets,&lista);
		}
		else if (codigo == 1){ 		//Peticion de login
			p = strtok(NULL, "/");
			printf("Flag: p Value = %s\n",p);
			if(p == '\0'){
				printf("Flag:ClienteCrash\n");
				end=1;
				codigo=0;
				EliminaConectado(&lista, nombre_Usuario);
				EnviarListaConectados(num_sockets,sockets,&lista);
			}
			else{
				strcpy(nombre_Usuario, p);
				p = strtok(NULL,"/");
				strcpy(password, p);
				printf("Flag:1\n");
				registrado = Usuario_Registrado(conn,nombre_Usuario);
				printf("Flag:2\n");
				if (registrado == 0)
					strcpy(respuesta, "1");	//User not registered
				else{//Usuario registrado
					printf("Flag:3: %s, %s\n", password, nombre_Usuario);
					int checked = Password_Check(conn, password, nombre_Usuario);
					printf("Flag:4%d\n",checked);
					if (checked == 1){
						printf("Flag:5\n");
						strcpy(respuesta, "2");	//Login succesful
						AddConectado(&lista, nombre_Usuario, sock_conn);
						EnviarListaConectados(num_sockets,sockets,&lista);
						printf("Flag Login2\n");
					}
					else{
						strcpy(respuesta, "3");	//Password wrong
					}
				}
			}
			
		}
		else if (codigo == 2){	//Peticion de registro
			p = strtok(NULL, "/");
			strcpy(nombre_Usuario, p);
			p = strtok(NULL, "/");
			strcpy(password, p);
			int num_jugadores;
			registrado = Usuario_Registrado(conn, nombre_Usuario);
			if (registrado == 1){	//Usuario YA registrado
				strcpy(respuesta, "4");
			}
			else {			//USUARIO NO REGISTRADO, le registramos
				Registrar_Usuario(conn, nombre_Usuario, password, num_jugadores);
				strcpy(respuesta, "5");	//Registrado
			}
		}
		else if (codigo == 4){	//Consulta los jugadores que han jugado con IronMan
			
		}
		else if	(codigo == 5){	//Consulta los jugadores que han ganado a IronMan
			
		}
		else if (codigo == 6){	//Consulta los jugadores que jugaron el dia 29/05/2021
			strcpy(day, "29/05/2021");
			int numero = JugadoresQueJugaronTalDIA (conn, day);
			if (numero != 0){
				strcpy(respuesta, numero);	//Numero de jugadores
				printf("Numero Jugadores: %d \n", numero);
			}
			else{
				strcpy(respuesta, "-1");	//Error
				printf("Error al contar los jugadores \n");
			}
		}
		else if (codigo == 7){				//Añadir conectado a la partida
			int SocketDeLaPersonaInvitada;
			int added;
			int correcto = DameUsuarioAPartirDeSocket(&lista, sock_conn, organizador);
			if (correcto ==0){
				printf("FLAG_Partida1\n");
			}
			else {		//Mensaje tipo: 7/2/Maria
				sprintf(respuesta,"7/%s", organizador);
				printf("Respuesta: %s \n", respuesta);
				p = strtok(NULL,"/");
				partida = atoi(p);	//Posicion en la tabla
				int creado = CrearPartida(Tabla_P,organizador,sock_conn);
				if (creado == -1){
					printf("Partida no creada\n");
				}
				else{
					printf("Partida creada\n");
					p = strtok(NULL, "/");
					while (p!=NULL){
						strcpy(nombre_Usuario, p);	//Persona a la que queremos invitar a partida
						int SocketDeLaPersonaInvitada = DameSocketAPartirDeUsuario(&lista,nombre_Usuario);
						if (SocketDeLaPersonaInvitada==-1)
							printf("Socket no obtenido\n");
						else{
							printf("Socket: %d \n", SocketDeLaPersonaInvitada);
							added = AddJugadorPartida(Tabla_P, nombre_Usuario, SocketDeLaPersonaInvitada, partida);
							if (added == -1){
								printf("Error al añadir a la partida");
							}
							else{
								printf("Jugador/a añadido/a a partida: Nombre: %s\n", nombre_Usuario);
							}
						}
						p = strtok(NULL,"/");
					}
				}
			}
		}
		else if (codigo==8){		//Gestionar la invitacion a partida
			p = strtok(NULL,"/");
			partida = atoi(p);
			printf("ID de Partida: %d\n", partida);
			int creador = DameOrganizadorPartida(Tabla_P, partida, organizador);
			int socketOrganizador;
			if (creador ==-1)
				printf("Nadie organiza la partida\n");
			else{
				printf("Organizador: %s \n", organizador);
				socketOrganizador = DameSocketAPartirDeUsuario(&lista, organizador);
				if (socketOrganizador ==-1)
					printf("Flag2 nadie organiza\n");
				else{
					printf("Socket Organizador: %d\n", socketOrganizador);
					int usuario=DameUsuarioAPartirDeSocket(&lista,sock_conn,invitada);
					if (usuario == -1)
						printf("no hay socket de invitado\n");
					else{
						printf("Persona invitada: %s\n", invitada);
						p = strtok(NULL,"/");
						strcpy(respuesta_SI_NO,p);
						printf("Respuesta del invitado: %s\n",respuesta_SI_NO);
						sprintf(respuesta, "8/%d/%s/%s/",partida, invitada, respuesta_SI_NO);
					}
				}
			}
		}
		if (codigo != 0)
		{
			printf ("Respuesta: %s\n", respuesta);
			// Enviamos respuesta
			write (sock_conn,respuesta, strlen(respuesta));
		}
		// Se acabo el servicio para este cliente
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
	if (listen(sock_listen, 3) < 0)
		printf("Error en el Listen"); //hasta 3 peticiones en cola
	
	int end = 0;
	pthread_t thread[100];	//Aqui guardamos los ID de thread de cada usuario
	
	int creada = CrearPartida(Tabla_P, "Manuel", 15);
	if (creada < 0)
		printf("Error al crear la tabla \n");
	else{
		while (end ==0){
			printf("Escuchando\n");
			sock_conn = accept(sock_listen, NULL, NULL);
			printf ("He recibido conexion\n");
			
			sockets[num_sockets] = sock_conn;
			pthread_create(&thread[num_sockets],NULL,AtenderCliente,&sockets[num_sockets]);
			num_sockets= num_sockets +1;
		}	
		close(sock_conn);
	} 
} 

	
