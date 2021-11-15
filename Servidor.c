#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <mysql.h>
#include <pthread.h>
#include <my_global.h>

typedef struct{
	char Usuario[20];
	int Socket;
}Jugador;

typedef struct{
	Jugador jugadores[100];
	int num;
}ListaJugadores;

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

//Declaramos parametros
char peticion[512];
char respuesta[512];

ListaPartidas lista_P;
int num_sockets;
int sockets[100];
ListaJugadores lista;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	//inicializar la exclusion

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
	
	conn =mysql_real_connect(conn, "shiva2.upc.es", "root","mysql", "TG9_juego",0, NULL, 0);
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
	int PORT = 50077 ;
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
	
